#include <stdint.h>

#define IPMI_SES_HEADER_OFFSET_AUTH_TYPE		0x00	// 1 byte  0 (see below)
#define IPMI_SES_HEADER_OFFSET_SEQ_NUM			0x01	// 4 bytes 1 2 3 4
#define IPMI_SES_HEADER_OFFSET_SES_ID			0x05	// 4 bytes 5 6 7 8
#define IPMI_SES_HEADER_OFFSET_MSG_AUTH_CODE	0x09	// 16 bytes, field only present when auth type != none
#define IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_NONE 0x09	// 1 byte 9
#define IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_TRUE 0x19	// 1 byte 25

#define IPMI_SES_HEADER_LENGTH_NO_AUTH			10
#define IPMI_SES_HEADER_LENGTH_AUTH				26

#define IPMI_SES_HEADER_AUTH_TYPE_NONE			0x00
#define IPMI_SES_HEADER_AUTH_TYPE_MD2			0x01
#define IPMI_SES_HEADER_AUTH_TYPE_MD5			0x02
#define IPMI_SES_HEADER_AUTH_TYPE_PW			0x04




#pragma pack(1)
typedef struct  {
	uint8_t auth_type;
	uint32_t seq_num;
	uint32_t ses_id;
	uint8_t auth_code[16];
	uint8_t ipmi_msg_len;
	uint8_t auth_valid;
} ipmi_session_auth;

protocol_data* ipmi_session_process_packet(protocol_data* );
uint8_t * ipmi_auth_md5(ipmi_session_auth* , uint8_t * , int,  char * );
const char * buf2str(uint8_t * , int );


