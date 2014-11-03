#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "udp-server.h"
#include "ipmi-session.h"
#include "ipmi-msg.h"
#include "ipmi-sdr.h"
#include "ipmi-sensor.h"

struct sensor* sensor_tmp;
protocol_data* ipmi_msg_process_packet(protocol_data* packet_inc, ipmi_session_auth* ipmi_session_in) {



	// craft incoming message from packet
	ipmi_msg_in* msg_in = malloc(sizeof(ipmi_msg_in));
	// if the incoming packet is just 7 byte, the structure is like our struct, so just memcopy
	if (packet_inc->length == 7) {
		memcpy(msg_in, packet_inc->data, 7);
		msg_in->data = malloc(sizeof(unsigned char));
	} else {
		memcpy(msg_in, packet_inc->data, 6); 	// just first 6 bytes match to eur struct
		msg_in->data = malloc((packet_inc->length-6-1)*sizeof(unsigned char)); // malloc for data (-6 see above, -1 for checksum byte at end)
		memcpy(msg_in->data, packet_inc->data+6*sizeof(unsigned char), packet_inc->length-6-1);
		msg_in->checksum_data = packet_inc->data[packet_inc->length-1];	// last byte is checksum
	}
#ifdef DEBUG
	printf("(ipmi-msg) rs_sa: 0x%02x rs_lun: 0x%02x net_fn: 0x%02x\n", msg_in->rs_sa, msg_in->rs_lun, msg_in->net_fn);
	printf("(ipmi-msg) rq_sa: 0x%02x rq_seq: 0x%02x rq_lun: 0x%02x\n", msg_in->rq_sa, msg_in->rq_seq, msg_in->rq_lun);
	printf("(ipmi-msg) header checksum: 0x%02x (calculated: 0x%02x)\n", msg_in->checksum_header, ipmi_checksum(packet_inc->data, 2));
	printf("(ipmi-msg) data checksum: 0x%02x (calculated: 0x%02x)\n", msg_in->checksum_data, ipmi_checksum(packet_inc->data, packet_inc->length-1));
	printf("(ipmi-msg) cmd: 0x%02x data len: %d\n", msg_in->cmd, packet_inc->length-6-1);
#endif
	// check incoming checksum
	if (msg_in->checksum_header != ipmi_checksum(msg_in, 2)) {
		printf("ipmi-msg) IPMI header checksum failed");
	}
	if (msg_in->checksum_data != ipmi_checksum(packet_inc->data, packet_inc->length-1) ) {
		printf("ipmi-msg) IPMI data checksum failed");
	}

	// create outgoing msg
	ipmi_msg_out* msg_out = malloc(sizeof(ipmi_msg_out));
	msg_out->rq_sa = msg_in->rq_sa;


	msg_out->net_fn = msg_in->net_fn+1;	// answer net_fn is usually one higher, see p 41 in IPMI v2.0 spec
	msg_out->rq_lun = msg_in->rq_lun;
	msg_out->checksum_header = ipmi_checksum(msg_out, 2);
	msg_out->rs_sa = msg_in->rs_sa;
	msg_out->rq_seq = msg_in->rq_seq;
	msg_out->rs_lun = msg_in->rs_lun;
	msg_out->cmd = msg_in->cmd;




	switch(msg_in->net_fn) {
	case IPMI_NETFN_APP:
		switch(msg_in->cmd) {
		case IPMI_CMD_GET_CHAN_AUTH_CAPA: // Get Channel Authentication Capabilities
			msg_out->data_len = 9;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[CMD_OFFSET_COMPL_CODE] = 0;	// completion code: Command Completed Normally
			msg_out->data[GCAC_CHANNEL_NUM] = 0;	// channel number
			msg_out->data[GCAC_AUTH_TYPE] = GCAC_AUTO_TYPE_BIT_MD5 | GCAC_AUTO_TYPE_BIT_PWKEY | GCAC_AUTO_TYPE_BIT_NONE;
			msg_out->data[GCAC_AUTH_STATUS] = 0b00000111;	// login stuff: msg auth, user level auth, anon login
			msg_out->data[4] = 1;	// ipmi v1.5: reserved => 0
			msg_out->data[5] = 0;	// oem
			msg_out->data[6] = 0;	// oem
			msg_out->data[7] = 0;	// oem
			msg_out->data[8] = 0;	// oem
			break;
		case IPMI_CMD_GET_SESS_CHALLENGE: // Get Session Challenge
			msg_out->data_len = 21;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = ((ipmi_session_in->ses_id >> 0) & 0xff);			// byte 2 tmp session 0
			msg_out->data[2] = ((ipmi_session_in->ses_id >> 8) & 0xff);			// byte 3 tmp session 1
			msg_out->data[3] = ((ipmi_session_in->ses_id >> 16) & 0xff);		// byte 4 tmp session 2
			msg_out->data[4] = ((ipmi_session_in->ses_id >> 24) & 0xff);		// byte 5 tmp session 3
			char* test = "chall_str"; // nice to have: use a random challenge string
			memcpy(msg_out->data+5*sizeof(unsigned char), test, 10 );

			break;
		case IPMI_CMD_ACTIVE_SESS: // Activate Session
			// check for valid auth here?
			msg_out->data_len = 11;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = msg_in->data[0]; 	// Authentication Type for remainder of session: same as requested
			msg_out->data[2] = (ipmi_session_in->ses_id & 0xff); 			// byte 3  Session ID byte 1
			msg_out->data[3] = ((ipmi_session_in->ses_id >> 8) & 0xff); 	// byte 4  Session ID byte 2
			msg_out->data[4] = ((ipmi_session_in->ses_id >> 16) & 0xff); 	// byte 5  Session ID byte 3
			msg_out->data[5] = ((ipmi_session_in->ses_id >> 24) & 0xff); 	// byte 6  Session ID byte 4
			msg_out->data[6] = 0;//(int)(rand() % 128); 	// byte 7  Session seq # byte 1
			msg_out->data[7] = 0;//(int)(rand() % 128); 	// byte 8  Session seq # byte 2
			msg_out->data[8] = 0;//(int)(rand() % 128); 	// byte 9  Session seq # byte 3
			msg_out->data[9] = 0;//(int)(rand() % 128);	// byte 10 Session seq # byte 4
			msg_out->data[10] = 0x04; 	// byte 11 Maximum privilege level allowed for this session: Admin

			break;
		case IPMI_CMD_GET_DEVICE_ID: // Get Device ID
			msg_out->data_len = 12;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = 0x11; 	// byte 2 Device ID
			msg_out->data[2] = 0x1; 	// byte 3  Device Revision
			msg_out->data[3] = 0x1; 	// byte 4  Firmware Revision
			msg_out->data[4] = 0x1; 	// byte 5  Firmware Minor Revision
			msg_out->data[5] = 0x51; 	// byte 6  IPMI Version: 1.5
			msg_out->data[6] = 0x1+0x2;	// byte 7  Additional Device Support: Sensor Device and SDR
			msg_out->data[7] = 0x0; 	// byte 8  Manufactorer ID byte 1
			msg_out->data[8] = 0x0; 	// byte 9  Manufactorer ID byte 2
			msg_out->data[9] = 0x0; 	// byte 10 Manufactorer ID byte 3
			msg_out->data[10] = 0x0; 	// byte 11 Product ID byte 1
			msg_out->data[11] = 0x0; 	// byte 12 Product ID byte 2f

			break;
		case IPMI_CMD_SET_SES_PRIV_LEVEL:
			msg_out->data_len = 2;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = msg_in->data[0]; 	// privilege level: as requested
			break;

		case IPMI_CMD_CLOSE_SESSION:
			msg_out->data_len = 1;
			msg_out->data= calloc(msg_out->data_len,  sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			break;

		default:
#ifdef DEBUG
			printf("(ipmi-msg) Unknown cmd: 0x%02x\n", msg_in->cmd);
#endif
			msg_out->data_len = 1;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0xd5;	// completion code: Cannot execute command. Command, or request parameter(s), not supported in present state.
			break;
		}

		break;	// switch close: IPMI_NETFN_APP

	case IPMI_NETFN_STORAGE:
		switch(msg_in->cmd) {
		case IPMI_CMD_GET_SDR_REPO_INFO:
			msg_out->data_len = 15;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = 0x51;	// SDR Version
			msg_out->data[2] = SENSOR_COUNT & 0xff;// Number of Records, LSB
			msg_out->data[3] = (SENSOR_COUNT & 0xff00)>>8;		// Number of Records, MSB
			msg_out->data[4] = 0;		// Free Space in Bytes, LSB
			msg_out->data[5] = 0;		// Free Space in Bytes, MSB
			msg_out->data[6] = 0;		// Most recent addition timestamp, LSB
			msg_out->data[7] = 0;		// Most recent addition timestamp,
			msg_out->data[8] = 0;		// Most recent addition timestamp,
			msg_out->data[9] = 0;		// Most recent addition timestamp, MSB
			msg_out->data[10] = 0;		// Most recent erase timestamp, LSB
			msg_out->data[11] = 0;		// Most recent erase timestamp,
			msg_out->data[12] = 0;		// Most recent erase timestamp,
			msg_out->data[13] = 0;		// Most recent erase timestamp, MSB
			msg_out->data[14] = 0b00000000;		// Operation Support: none at all
			break;

		case IPMI_CMD_RESERVE_SDR_REPO:
			msg_out->data_len = 3;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = 0x34;	// Reservation ID, LSB
			msg_out->data[2] = 0x12;	// Reservation ID, MSB


			break;
		case IPMI_CMD_GET_SDR:
			// get the sensor data:
			sensor_tmp = get_sensor(msg_in->data[2]);

			int offset = msg_in->data[4];
			int bytes_to_read = msg_in->data[5];

			// update the sensor reading and sensor reading factors
			// offset == 0: header is read (or whole sensor at once)
			// offset == 5: only sdr without header is read
			if (offset == 0 || offset == 5) {
				sensor_update(sensor_tmp);
			}
			if (offset == 0) {
				if (bytes_to_read > (SDR_HEADER_LENGTH + sensor_tmp->record_length))
					bytes_to_read = SDR_HEADER_LENGTH + sensor_tmp->record_length;
			} else {
				if (bytes_to_read > (SDR_HEADER_LENGTH + sensor_tmp->record_length - offset))
					bytes_to_read = SDR_HEADER_LENGTH + sensor_tmp->record_length - offset;
			}
//			printf("     record length: %d offset: %d bytes to read: %d\n", sensor_tmp->record_length, offset, bytes_to_read);
			msg_out->data_len = 3+bytes_to_read;
			msg_out->data= calloc(msg_out->data_len, sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			if (get_next_sensor(msg_in->data[2]) == -1) { // check if this is the last record/sensor
				msg_out->data[1] = 0xff;	// Record ID for next record LSB: no more records
				msg_out->data[2] = 0xff;	// Record ID for next record MSB: no more records
			} else {
				msg_out->data[1] = get_next_sensor(msg_in->data[2]);	// Record ID for next record LSB
				msg_out->data[2] = 0;									// Record ID for next record MSB
			}








			//memcpy(msg_out->data+3 , (&sensor_tmp->record_id_h)+offset, bytes_to_read);

			memcpy(msg_out->data+3 , ((char *) sensor_tmp) + offset, bytes_to_read);

			/*
			if (offset == 0) {	// first 4 bytes
				msg_out->data[3] = sensor_tmp->record_id_l;	// Record ID LSB
				msg_out->data[4] = sensor_tmp->record_id_h;	// Record ID MSB
				msg_out->data[5] = sensor_tmp->sdr_version;	// SDR Version
				msg_out->data[6] = sensor_tmp->record_type;	// Record Type
				if (bytes_to_read>5) {
					msg_out->data[7] = sizeof(sensor_tmp->sdr);	//
				} else {
					msg_out->data[7]
				}
			}

			if (msg_in->data[5] == 5 && msg_in->data[4] == 0) { // send header only if offset zero
				msg_out->data[3] = 0;	// Record ID LSB
				msg_out->data[4] = 0;	// Record ID MSB
				msg_out->data[5] = 0x51;	// SDR Version
				msg_out->data[6] = SDR_RECORD_TYPE_FULL_SENSOR;	// Record Type
				msg_out->data[7] = sizeof(sensor_tmp->sdr);		// remaining record bytes
			} else if (  msg_in->data[4] == 5 && msg_in->data[5] == sizeof(sensor_tmp->sdr) ) {	// offset 5 and 48 data bytes requested



				memcpy(msg_out->data+3 , &sensor_tmp->sdr, msg_in->data[5]);
			}
			*/
			break;

		default:
#ifdef DEBUG
			printf("(ipmi-msg) Unknown cmd: 0x%02x\n", msg_in->cmd);
#endif
			msg_out->data_len = 1;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0xd5;	// completion code: Cannot execute command. Command, or request parameter(s), not supported in present state.
			break;
		}

		break; // switch close: IPMI_NETFN_STORAGE
	case IPMI_NETFN_SENSOR:
		switch(msg_in->cmd) {
		case IPMI_CMD_GET_SENSOR_READING:
			msg_out->data_len = 3;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = (int)(get_sensor(msg_in->data[0])->sdr.nominal_read);			// Sensor Reading for given sensor number msg_in->data[0]
			msg_out->data[2] = 0b01000000;		// Sensor Scanning disabled
			break;
		case IPMI_CMD_GET_SENSOR_FACTORS:
			msg_out->data_len = 8;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			msg_out->data[1] = msg_in->data[0];			// Next Reading: use the one supplied by request
			// copy over m, m_tol, b, accuracy, r_exp and b_exp from the sdr
			memcpy(msg_out->data +2* sizeof(unsigned char), &(get_sensor(msg_in->data[0])->sdr.mtol) , 6);
			break;
		case IPMI_CMD_GET_SENSOR_THRESHOLDS:
			msg_out->data_len = 9;
			msg_out->data= calloc(msg_out->data_len,  sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			break;
		case IPMI_CMD_GET_SENSOR_EVENT_STATUS:
			msg_out->data_len = 3;
			msg_out->data= calloc(msg_out->data_len,  sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			break;
		case IPMI_CMD_GET_SENSOR_EVENT_ENABLE:
			msg_out->data_len = 2;
			msg_out->data= calloc(msg_out->data_len,  sizeof(unsigned char));
			msg_out->data[0] = 0;		// completion code: Command Completed Normally
			break;

		default:
#ifdef DEBUG
			printf("(ipmi-msg) Unknown cmd: 0x%02x\n", msg_in->cmd);
#endif
			msg_out->data_len = 1;
			msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
			msg_out->data[0] = 0xd5;	// completion code: Cannot execute command. Command, or request parameter(s), not supported in present state.
			break;
		}

		break; // switch close: IPMI_NETFN_SENSOR


	default:
#ifdef DEBUG
		printf("(ipmi-msg): netfn unknown: %d", msg_in->net_fn);
#endif
		msg_out->data_len = 1;
		msg_out->data= malloc(msg_out->data_len* sizeof(unsigned char));
		msg_out->data[0] = 0xd5;	// completion code: Cannot execute command. Command, or request parameter(s), not supported in present state.
		break;
	}


	// copy crafted outgoing msg
	protocol_data* packet_out = malloc(sizeof(protocol_data));
	packet_out->length = IPMI_MSG_HEADER_LENGTH + msg_out->data_len + 1; // header + len(data) + 1 data crc
	packet_out->data = malloc(packet_out->length*sizeof(unsigned char));
	memcpy(packet_out->data, msg_out, IPMI_MSG_HEADER_LENGTH);			// first IPMI_MSG_HEADER_LENGTH bytes are same structure, so just copy them direct

	// copy data if there is any
	if (msg_out->data_len != 0) {
		memcpy(packet_out->data+IPMI_MSG_HEADER_LENGTH*sizeof(unsigned char), msg_out->data, msg_out->data_len);
		free(msg_out->data);
	}

	// generate crc for msg (last byte in packet_out->data):
	packet_out->data[packet_out->length-1] = ipmi_checksum(packet_out->data, packet_out->length-1);

	// cleanup
	free(msg_out);
	free(msg_in->data);
	free(msg_in);

	return packet_out;
}

// source http://svn.savannah.gnu.org/viewvc/tags/Release-1_4_1/libfreeipmi/util/ipmi-util.c?root=freeipmi&view=markup
static uint8_t _checksum (const void *buf, unsigned int buflen, uint8_t checksum_initial)
{
  register unsigned int i = 0;
  register int8_t checksum = checksum_initial;

  if (buf == NULL || buflen == 0)
    return (checksum);

  for (; i < buflen; i++)
    checksum = (checksum + ((uint8_t *)buf)[i]) % 256;

  return (checksum);
}

uint8_t ipmi_checksum (const void *buf, unsigned int buflen)
{
  uint8_t checksum;
  checksum = _checksum (buf, buflen, 0);
  return (-checksum);
}
