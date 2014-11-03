
typedef struct  {
	unsigned char* data;		// array with the data-bytes
	int length;					// length of array
} protocol_data;


#define MIN(a,b) (((a)<(b))?(a):(b))
#define BSWAP_32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) |(((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))
char* password_padded;
