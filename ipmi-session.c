#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#include "udp-server.h"
#include "ipmi-session.h"
#include "ipmi-msg.h"



protocol_data* ipmi_session_process_packet(protocol_data* packet_inc) {
	// copy over packet data into nice struct
	ipmi_session_auth* ipmi_session_in = malloc(sizeof(ipmi_session_auth));
	ipmi_session_in->auth_valid = 0;
	unsigned char ipmi_session_length;
	ipmi_session_in->auth_type = packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE];

	// copy over sequence num and session id
	memcpy(&ipmi_session_in->seq_num, packet_inc->data+IPMI_SES_HEADER_OFFSET_SEQ_NUM, 8);

	// check if there is a auth code present (it is if auth type != none)
	if (ipmi_session_in->auth_type == IPMI_SES_HEADER_AUTH_TYPE_NONE) {
		ipmi_session_length = IPMI_SES_HEADER_LENGTH_NO_AUTH;
		ipmi_session_in->ipmi_msg_len = packet_inc->data[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_NONE];
	} else {
		ipmi_session_length = IPMI_SES_HEADER_LENGTH_AUTH;
		memcpy(&ipmi_session_in->auth_code, packet_inc->data+IPMI_SES_HEADER_OFFSET_MSG_AUTH_CODE, 16); // 16 bytes of auth code
		ipmi_session_in->ipmi_msg_len = packet_inc->data[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_TRUE];
	}


	protocol_data*  packet_out =  (protocol_data *) malloc(sizeof(protocol_data));
#ifdef DEBUG
	printf("(ipmi-session) Session ID: %d\n", ipmi_session_in->ses_id);
	printf("(ipmi-session) Sequence Number: %d \n", ipmi_session_in->seq_num);
#endif


	switch (packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE]) {
		case IPMI_SES_HEADER_AUTH_TYPE_NONE:
			// auth none
			#ifdef DEBUG
				printf("(ipmi-session) Auth Type: 0x%02x (None)\n", packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE]);
			#endif
			break;
		case IPMI_SES_HEADER_AUTH_TYPE_MD2:
			// auth MD2
			#ifdef DEBUG
				printf("(ipmi-session) Auth Type: 0x%02x (MD2)\n", packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE]);
			#endif
			break;

		case IPMI_SES_HEADER_AUTH_TYPE_MD5:
			// auth MD5

			if (memcmp(ipmi_auth_md5(ipmi_session_in, packet_inc->data + (ipmi_session_length*sizeof(unsigned char)), ipmi_session_in->ipmi_msg_len, password_padded) ,ipmi_session_in->auth_code, 16) == 0) {
				ipmi_session_in->auth_valid = 1;
			}


			#ifdef DEBUG
				if (ipmi_session_in->auth_valid) {
					printf("(ipmi-session) Auth Type: 0x%02x (MD5, valid)\n", packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE]);
				} else {
					printf("(ipmi-session) Auth Type: 0x%02x (MD5, invalid)\n", packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE]);
				}
			#endif
			break;

		case IPMI_SES_HEADER_AUTH_TYPE_PW:
			// auth straight password
			// check pw

			#ifdef DEBUG
				printf("(ipmi-session) Auth Type: 0x%02x (Password)\n", packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE]);
			#endif
			break;
		default:
			// error
			packet_out->data = malloc(sizeof(unsigned char)); // so we don't free unallocated space later
			packet_out->length = -1;
			return packet_out;
			break;
	}
	#ifdef DEBUG
		printf("(ipmi-session) IPMI Message Length: %d \n", ipmi_session_in->ipmi_msg_len);
	#endif
	// hand over to ipmi handler
	protocol_data* ipmi_msg_packet_in = malloc(sizeof(protocol_data));
	ipmi_msg_packet_in->length = ipmi_session_in->ipmi_msg_len;
	ipmi_msg_packet_in->data = packet_inc->data + (ipmi_session_length*sizeof(unsigned char));
	protocol_data* ipmi_msg_packet_out = ipmi_msg_process_packet(ipmi_msg_packet_in, ipmi_session_in);

	// generate auth code
	switch (packet_inc->data[IPMI_SES_HEADER_OFFSET_AUTH_TYPE]) {
		case IPMI_SES_HEADER_AUTH_TYPE_PW:
			memcpy(ipmi_session_in->auth_code, password_padded, 16);
		break;
		case IPMI_SES_HEADER_AUTH_TYPE_MD5:
			memcpy(ipmi_session_in->auth_code,ipmi_auth_md5(ipmi_session_in, ipmi_msg_packet_out->data, ipmi_msg_packet_out->length, password_padded), 16);
		break;
	}

	// craft packet for ipmi session (include crafted ipmi message from ipmi_msg_packet_out)

	packet_out->length = ipmi_msg_packet_out->length + ipmi_session_length;
	packet_out->data = malloc(packet_out->length * sizeof(unsigned char));
	// copy over data bytes from ipmi message
	memcpy(packet_out->data + (ipmi_session_length * sizeof(unsigned char)), ipmi_msg_packet_out->data, ipmi_msg_packet_out->length);

	// copy over session stuff (this is crappy!)
	memcpy(packet_out->data, ipmi_session_in, ipmi_session_length);
	if (ipmi_session_in->auth_type == IPMI_SES_HEADER_AUTH_TYPE_NONE) {
		packet_out->data[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_NONE] = ipmi_msg_packet_out->length;
	} else {
		packet_out->data[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_TRUE] = ipmi_msg_packet_out->length;
	}
	// clean up
	if (ipmi_msg_packet_out->length != 0) {
		free(ipmi_msg_packet_out->data);
	}
	free(ipmi_msg_packet_out);
	free(ipmi_msg_packet_in);
	free(ipmi_session_in);
	return packet_out;
}


const char * buf2str(uint8_t * buf, int len)
{
	static char str[2049];
	int i;

	if (len <= 0 || len > 1024)
		return NULL;

	memset(str, 0, 2049);

	for (i=0; i<len; i++)
		sprintf(str+i+i, "%2.2x", buf[i]);

	str[len*2] = '\0';

	return (const char *)str;
}

/*
 * multi-session authcode generation for MD5
 * H(password + session_id + msg + session_seq + password)
 *
 * Use OpenSSL implementation of MD5 algorithm
 */
uint8_t * ipmi_auth_md5(ipmi_session_auth* s, uint8_t * data, int data_len, char * pass_zeroed)
{
	MD5_CTX ctx;
	static uint8_t md[16];
	uint32_t temp;
	temp = (s->seq_num);
	//printf("seqnu: 0x%x\n", temp);




	// padd password with zeros
//	uint8_t pass_zeroed[16];
//	memset(pass_zeroed, 0, 16);
//	memcpy(pass_zeroed, password,	MIN(strlen(password), 16));

	memset(md, 0, 16);
	memset(&ctx, 0, sizeof(MD5_CTX));

	MD5_Init(&ctx);
	MD5_Update(&ctx, (const uint8_t *)pass_zeroed, 16);
	MD5_Update(&ctx, (const uint8_t *)&s->ses_id, 4);
	MD5_Update(&ctx, (const uint8_t *)data, data_len);
	MD5_Update(&ctx, (const uint8_t *)&temp, sizeof(uint32_t));
	MD5_Update(&ctx, (const uint8_t *)pass_zeroed, 16);
	MD5_Final(md, &ctx);

	//printf("  MD5 AuthCode incoming calculated  : %s\n", buf2str(md, 16));

	return md;
}
