#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "udp-server.h"
#include "rmcp.h"
#include "asf.h"
#include "ipmi-session.h"



protocol_data* rmcp_process_packet(protocol_data* packet_in) {
	protocol_data* packet_out = malloc(sizeof(protocol_data));

	if (packet_in->data[RMCP_HEADER_OFFSET_VERSION] == RMCP_HEADER_VERSION) {
		if (packet_in->data[RMCP_HEADER_OFFSET_RESERVED] == RMCP_HEADER_RESERVED)  {
			if (packet_in->data[RMCP_HEADER_OFFSET_SEQ_NR] == RMCP_HEADER_SEQ_NOACK) {
				// no RMCP-ACK needed
				if ((packet_in->data[RMCP_HEADER_OFFSET_CLASS] & RCMP_HEADER_CLASS_MASK) == RMCP_HEADER_CLASS_ASF) {
#ifdef DEBUG
					printf("Got valid RMCP Header with ASF in it\n");
#endif
					// craft new packet with pointer to right position
					protocol_data* asf_packet_in = malloc(sizeof(protocol_data));
					asf_packet_in->length = packet_in->length-RMCP_HEADER_LENGTH;
					asf_packet_in->data = packet_in->data + (RMCP_HEADER_LENGTH*sizeof(unsigned char));
					// hand over packet for processing
					protocol_data* asf_packet_out = asf_process_packet(asf_packet_in);
					free(asf_packet_in);
					// check if processing worked
					if (asf_packet_out->length > 0) {

						// craft final packet
						packet_out->length = asf_packet_out->length+RMCP_HEADER_LENGTH;
						packet_out->data = malloc(packet_out->length*sizeof(unsigned char));
						// copy first 4 bytes of incoming RMCP packet
						memcpy(packet_out->data,packet_in->data, 4*sizeof(unsigned char));
						// copy ASF packet
						memcpy(packet_out->data + (RMCP_HEADER_LENGTH * sizeof(unsigned char)), asf_packet_out->data, asf_packet_out->length);
					} else {
						printf("Some Error happened down the Road.\n");
					}
					// cleanup
					free(asf_packet_out->data);
					free(asf_packet_out);
					return packet_out;
				}

				// IPMI
				if ((packet_in->data[RMCP_HEADER_OFFSET_CLASS] & RCMP_HEADER_CLASS_MASK) == RMCP_HEADER_CLASS_IPMI) {
#ifdef DEBUG
					printf("Got valid RMCP Header with IPMI in it\n");
#endif
					// craft new packet with pointer to right position
					protocol_data* ipmi_session_packet_in = malloc(sizeof(protocol_data));
					ipmi_session_packet_in->length = packet_in->length-RMCP_HEADER_LENGTH;
					ipmi_session_packet_in->data = packet_in->data + (RMCP_HEADER_LENGTH*sizeof(unsigned char));
					// hand over packet for processing
					protocol_data* ipmi_session_packet_out = ipmi_session_process_packet(ipmi_session_packet_in);
					free(ipmi_session_packet_in);
					// check if processing worked
					if (ipmi_session_packet_out->length > 0) {
						// craft final packet
						packet_out->length = ipmi_session_packet_out->length+RMCP_HEADER_LENGTH;
						packet_out->data = malloc(packet_out->length*sizeof(unsigned char));
						// copy first 4 bytes of incoming RMCP packet
						memcpy(packet_out->data,packet_in->data, RMCP_HEADER_LENGTH*sizeof(unsigned char));
						// copy ASF packet
						memcpy(packet_out->data + (RMCP_HEADER_LENGTH * sizeof(unsigned char)), ipmi_session_packet_out->data, ipmi_session_packet_out->length);

					} else {
						printf("Some Error happened down the Road.\n");
						packet_out->length = -1;
					}
					// cleanup
					if (ipmi_session_packet_out->length > 0 ) {
						free(ipmi_session_packet_out->data);
					}
					free(ipmi_session_packet_out);
					return packet_out;


				}
			} else {
				// reply with RMCP-ACK
				packet_out->data = malloc(RMCP_HEADER_LENGTH);
				packet_out->length = RMCP_HEADER_LENGTH;
				memcpy(packet_out->data, packet_in->data, RMCP_HEADER_LENGTH);
				// set ACK bit (copy class of message field with bit 7 set to 1)
				packet_out->data[RMCP_HEADER_OFFSET_CLASS] = packet_out->data[RMCP_HEADER_OFFSET_CLASS] | (1<<7);
				return packet_out;
			}
		}
	}
	printf("Some Error in RMCP happend.\n");
	packet_out->length = -1;
	return packet_out;
}
