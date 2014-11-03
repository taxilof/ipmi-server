

int handle_asf(unsigned char*, int, unsigned char*);
protocol_data* asf_process_packet(protocol_data* );
#define ASF_IANA_NUM		0x11be

#define ASF_MSG_TYPE_RESET	0x10
#define ASF_MSG_TYPE_PONG	0x40
#define ASF_MSG_TYPE_PING	0x80

// Header Offsets
#define ASF_HEADER_OFFSET_IANA_NUM		0x00	// 4 bytes
#define ASF_HEADER_OFFSET_MSG_TYPE		0x04
#define ASF_HEADER_OFFSET_MSG_TAG		0x05
#define ASF_HEADER_OFFSET_RESERVED		0x06	// for future use
#define ASF_HEADER_OFFSET_DATA_LEN		0x07
#define ASF_HEADER_LENGTH				0x08


// Data Offsets
#define ASF_DATA_OFFSET_IANA_NUM		0x00	// Length 4 byte 0 1 2 3
#define ASF_DATA_OFFSET_OEM				0x04	// OEM defined values, Length 4 byte 4 5 6 7
#define ASF_DATA_OFFSET_SUPPORTED_ENT	0x08	// Supported Entities, 0x81 for IPMI, Length 1 byte 8
#define ASF_DATA_OFFSET_SUPPORTED_INTER	0x09	// Supported Interactions, Length 1 byte 9
#define ASF_DATA_OFFSET_RESERVED		0x0A	// for future use, Length 6 byte 10 11 12 13 14 15, set all to 0

