



// Ports
#define RMCP_UDP_PORT					623


// Header Offsets and Length
#define RMCP_HEADER_OFFSET_VERSION 		0x00
#define RMCP_HEADER_OFFSET_RESERVED 	0x01	 // not used
#define RMCP_HEADER_OFFSET_SEQ_NR 		0x02
#define RMCP_HEADER_OFFSET_CLASS 		0x03
#define RMCP_HEADER_LENGTH 				0x04

// Header Masks
#define RCMP_HEADER_CLASS_MASK			0b00001111	// 6: ASF, 7: IPMI, etc

// Header Values
#define RMCP_HEADER_VERSION				0x06	//
#define RMCP_HEADER_RESERVED			0x00

#define RMCP_HEADER_CLASS_ASF			0x06
#define RMCP_HEADER_CLASS_IPMI			0x07
#define RMCP_HEADER_SEQ_NOACK			0xff



protocol_data* rmcp_process_packet(protocol_data* );
