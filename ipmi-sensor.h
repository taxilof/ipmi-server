#include "ipmi-sdr.h"

#ifndef IPMI_SENSOR_H_
#define IPMI_SENSOR_H_

#ifdef TARGET_RPI
	#define  SENSOR_COUNT 	4
#elif TARGET_ODROID
	#define  SENSOR_COUNT 	14
#else
	#define  SENSOR_COUNT 	3
#endif

struct sensor {
	union {
		uint16_t record_id;
		struct {
			uint8_t record_id_h;
			uint8_t record_id_l;
		};
	};

	uint8_t sdr_version;
	uint8_t record_type;
	uint8_t record_length;
	struct sdr_record_full_sensor sdr;		/* Full SDR Record */
	int (*functionPtr)(char*);				/* Function for reading out the physical sensor */
	char* path;								/* Path to sensor file (onl odroid) */
};


void sensors_init() ;
void sensors_cleanup();
struct sensor* get_sensor(uint8_t sensor_number);
void sensor_update(struct sensor *sensor);
int get_next_sensor(int sensor_number);

/* debug functions */
int read_current();
int read_voltage();
/* generic functions */
int generic_read_loadavg();
int generic_check_for_running_stress();

/* Raspberry pi functions */
int rpi_i2c_init();
int rpi_i2c_read_current(char*);
int rpi_i2c_read_bus_voltage(char*);
int rpi_i2c_read_power(char* );
/* Odroid functions */
int odroid_sensor_read(char*);
int odroid_fan_speed(char*);

#endif /* IPMI_SENSOR_H_ */
