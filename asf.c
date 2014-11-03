#include <stdio.h>
#include <stdlib.h>
#include "udp-server.h"
#include "rmcp.h"
#include "asf.h"



protocol_data* asf_process_packet(protocol_data* packet_inc) {
#ifdef DEBUG
	unsigned int iana_num = (packet_inc->data[ASF_HEADER_OFFSET_IANA_NUM] << 24 ) +
			(packet_inc->data[ASF_HEADER_OFFSET_IANA_NUM + 1] << 16 ) +
			(packet_inc->data[ASF_HEADER_OFFSET_IANA_NUM + 2] << 8 ) +
			(packet_inc->data[ASF_HEADER_OFFSET_IANA_NUM + 3] << 0 );

	printf("ASF IANA NUM: 0x%08x", iana_num);
	if (iana_num ==  ASF_IANA_NUM) {
		printf(" -> valid\n");
	} else {
		printf(" -> invalid\n");
	}

	printf("ASF Type: ");
#endif
	switch (packet_inc->data[ASF_HEADER_OFFSET_MSG_TYPE]) {
		case ASF_MSG_TYPE_PONG:
#ifdef DEBUG
			printf(" Pong\n");
#endif
			break;

		case ASF_MSG_TYPE_PING: {
	#ifdef DEBUG
				printf(" Ping\n");
	#endif

				protocol_data*  ret =  (protocol_data *) malloc( sizeof(protocol_data) );
				ret->length = 8+16;
				ret->data = malloc(ret->length * sizeof(unsigned char));


				// Fill in Header
				ret->data[ASF_HEADER_OFFSET_IANA_NUM + 0] = 0;
				ret->data[ASF_HEADER_OFFSET_IANA_NUM + 1] = 0;
				ret->data[ASF_HEADER_OFFSET_IANA_NUM + 2] = 0x11;		// iana
				ret->data[ASF_HEADER_OFFSET_IANA_NUM + 3] = 0xbe;		// iana
				ret->data[ASF_HEADER_OFFSET_MSG_TYPE] = ASF_MSG_TYPE_PONG;
				ret->data[ASF_HEADER_OFFSET_MSG_TAG] = 0;
				ret->data[ASF_HEADER_OFFSET_RESERVED] = 0;
				ret->data[ASF_HEADER_OFFSET_DATA_LEN] = 16;				// Pong specific

				// Fill in Data
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_IANA_NUM + 0] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_IANA_NUM + 1] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_IANA_NUM + 2] = 0x11;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_IANA_NUM + 3] = 0xbe;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_OEM + 0 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_OEM + 1 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_OEM + 2 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_OEM + 3 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_SUPPORTED_ENT ] = 0x81;	// IPMI with ASF 1.0
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_SUPPORTED_INTER] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_RESERVED + 0 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_RESERVED + 1 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_RESERVED + 2 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_RESERVED + 3 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_RESERVED + 4 ] = 0;
				ret->data[ASF_HEADER_LENGTH+ASF_DATA_OFFSET_RESERVED + 5 ] = 0;
	#ifdef DEBUG
				printf("Sent ASF Pong\n");
	#endif
				return ret;
			}
			break;

		default:
#ifdef DEBUG
			printf(" unknown\n");
#endif
			break;
	}


	// Error:
	protocol_data* error;
	error =  (protocol_data *) malloc( sizeof(protocol_data) );
	error->length = -1;
	return error;
}



