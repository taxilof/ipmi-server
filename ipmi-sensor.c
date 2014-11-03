#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
	#include <linux/i2c-dev.h>
	#include <sys/ioctl.h>


#include "ipmi-sdr.h"
#include "ipmi-sensor.h"



struct sensor* sensors[SENSOR_COUNT];
void sensors_init() {
	// generate sensor with default values
	struct sensor* sensor_default = calloc(1, sizeof(struct sensor));
	sensor_default->record_type = SDR_RECORD_TYPE_FULL_SENSOR;
	sensor_default->record_length = sizeof(sensor_default->sdr);

	sensor_default->sdr_version = 0x51;
	sensor_default->sdr.cmn.keys.owner_id = 0x20;	// local
	sensor_default->sdr.cmn.entity =  (struct entity_id){.id=0x13, .logical=0, .instance=0};	// Entity ID: Power Module
	sensor_default->sdr.cmn.event_type = SDR_EVENT_TYPE_THRESHOLD;	// Event/Reading Type: Threshold
	sensor_default->sdr.linearization = SDR_SENSOR_L_LINEAR;
	sensor_default->sdr.mtol.m_ls = __M_TO_M_LS(511);
	sensor_default->sdr.mtol.m_ms = __M_TO_M_MS(511);
	sensor_default->sdr.mtol.m_tol = 0;
	sensor_default->sdr.b_acc.b_ls = __M_TO_M_LS(0);
	sensor_default->sdr.b_acc.b_ms = __M_TO_M_MS(0);
	sensor_default->sdr.r_b_exp.b_exp = 0;
	sensor_default->sdr.r_b_exp.r_exp = -3;
	sensor_default->sdr.id_code = 16 ;//+ (1<<7) + (1<<6);	// unicode


#ifdef TARGET_RPI
	// init device specific sensors
	rpi_i2c_init();
	// generate current sensor:
	sensors[0] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[0], sensor_default, sizeof(struct sensor));
	sensors[0]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_CURRENT;
	sensors[0]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_AMPS;
	sensors[0]->functionPtr = &rpi_i2c_read_current;
	sensors[0]->sdr.cmn.keys.sensor_num = 0;	// must be the same as array-position!
	sensors[0]->record_id = sensors[0]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[0]->sdr.id_string, "RPi current", 11);


	// generate voltage sensor:
	sensors[1] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[1], sensor_default, sizeof(struct sensor));
	sensors[1]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_VOLTAGE;
	sensors[1]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_VOLTS;
	sensors[1]->functionPtr = &rpi_i2c_read_bus_voltage;
	sensors[1]->sdr.cmn.keys.sensor_num = 1;	// must be the same as array-position!
	sensors[1]->record_id = sensors[1]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[1]->sdr.id_string, "RPi bus voltage", 15);

	// generate power sensor:
	sensors[2] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[2], sensor_default, sizeof(struct sensor));
	sensors[2]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER;
	sensors[2]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_WATTS;
	sensors[2]->functionPtr = &rpi_i2c_read_power;
	sensors[2]->sdr.cmn.keys.sensor_num = 2;	// must be the same as array-position!
	sensors[2]->record_id = sensors[2]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[2]->sdr.id_string, "RPi power", 9);

	// generate cpu load sensor:
	sensors[3] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[3], sensor_default, sizeof(struct sensor));
	sensors[3]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER;
	sensors[3]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_UNSPEC;
	sensors[3]->functionPtr = &generic_read_loadavg;
	sensors[3]->sdr.cmn.keys.sensor_num = 3;	// must be the same as array-position!
	sensors[3]->record_id = sensors[3]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[3]->sdr.id_string, "RPi loadavg", 11);

	// generate stress-running sensor:
	sensors[3] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[3], sensor_default, sizeof(struct sensor));
	sensors[3]->record_id = 3;
	sensors[3]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER;
	sensors[3]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_UNSPEC;	// Sensor Base Unit: Voltage
	sensors[3]->functionPtr = &generic_check_for_running_stress;
	sensors[3]->sdr.cmn.keys.sensor_num = 3;	// must be the same as array-position!
	memcpy(sensors[3]->sdr.id_string, "debug stress", 12);

#elif TARGET_ODROID
	// generate A7 current sensor:
	sensors[0] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[0], sensor_default, sizeof(struct sensor));
	sensors[0]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_CURRENT;
	sensors[0]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_AMPS;
	sensors[0]->functionPtr = &odroid_sensor_read;
	sensors[0]->path = "/sys/bus/i2c/drivers/INA231/4-0045/sensor_A";
	sensors[0]->sdr.cmn.keys.sensor_num = 0;	// must be the same as array-position!
	sensors[0]->record_id = sensors[0]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[0]->sdr.id_string, "A7 current", 10);
	// generate A7 voltage sensor:
	sensors[1] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[1], sensor_default, sizeof(struct sensor));
	sensors[1]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_VOLTAGE;
	sensors[1]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_VOLTS;
	sensors[1]->functionPtr = &odroid_sensor_read;
	sensors[1]->path = "/sys/bus/i2c/drivers/INA231/4-0045/sensor_V";
	sensors[1]->sdr.cmn.keys.sensor_num = 1;	// must be the same as array-position!
	sensors[1]->record_id = sensors[1]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[1]->sdr.id_string, "A7 voltage", 10);
	// generate A7 watts sensor:
	sensors[2] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[2], sensor_default, sizeof(struct sensor));
	sensors[2]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER ;
	sensors[2]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_WATTS;
	sensors[2]->functionPtr = &odroid_sensor_read;
	sensors[2]->path = "/sys/bus/i2c/drivers/INA231/4-0045/sensor_W";
	sensors[2]->sdr.cmn.keys.sensor_num = 2;	// must be the same as array-position!
	sensors[2]->record_id = sensors[2]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[2]->sdr.id_string, "A7 power", 8);

	// generate A15 current sensor:
	sensors[3] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[3], sensor_default, sizeof(struct sensor));
	sensors[3]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_CURRENT;
	sensors[3]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_AMPS;
	sensors[3]->functionPtr = &odroid_sensor_read;
	sensors[3]->path = "/sys/bus/i2c/drivers/INA231/4-0040/sensor_A";
	sensors[3]->sdr.cmn.keys.sensor_num = 3;	// must be the same as array-position!
	sensors[3]->record_id = sensors[3]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[3]->sdr.id_string, "A15 current", 11);
	// generate A15 voltage sensor:
	sensors[4] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[4], sensor_default, sizeof(struct sensor));
	sensors[4]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_VOLTAGE;
	sensors[4]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_VOLTS;
	sensors[4]->functionPtr = &odroid_sensor_read;
	sensors[4]->path = "/sys/bus/i2c/drivers/INA231/4-0040/sensor_V";
	sensors[4]->sdr.cmn.keys.sensor_num = 4;	// must be the same as array-position!
	sensors[4]->record_id = sensors[4]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[4]->sdr.id_string, "A15 voltage", 11);
	// generate A15 watts sensor:
	sensors[5] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[5], sensor_default, sizeof(struct sensor));
	sensors[5]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER ;
	sensors[5]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_WATTS;
	sensors[5]->functionPtr = &odroid_sensor_read;
	sensors[5]->path = "/sys/bus/i2c/drivers/INA231/4-0040/sensor_W";
	sensors[5]->sdr.cmn.keys.sensor_num = 5;	// must be the same as array-position!
	sensors[5]->record_id = sensors[5]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[5]->sdr.id_string, "A15 power", 9);

	// generate GPU current sensor:
	sensors[6] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[6], sensor_default, sizeof(struct sensor));
	sensors[6]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_CURRENT;
	sensors[6]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_AMPS;
	sensors[6]->functionPtr = &odroid_sensor_read;
	sensors[6]->path = "/sys/bus/i2c/drivers/INA231/4-0041/sensor_A";
	sensors[6]->sdr.cmn.keys.sensor_num = 6;	// must be the same as array-position!
	sensors[6]->record_id = sensors[6]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[6]->sdr.id_string, "GPU current", 11);
	// generate GPU voltage sensor:
	sensors[7] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[7], sensor_default, sizeof(struct sensor));
	sensors[7]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_VOLTAGE;
	sensors[7]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_VOLTS;
	sensors[7]->functionPtr = &odroid_sensor_read;
	sensors[7]->path = "/sys/bus/i2c/drivers/INA231/4-0041/sensor_V";
	sensors[7]->sdr.cmn.keys.sensor_num = 7;	// must be the same as array-position!
	sensors[7]->record_id = sensors[7]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[7]->sdr.id_string, "GPU voltage", 11);
	// generate GPU watts sensor:
	sensors[8] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[8], sensor_default, sizeof(struct sensor));
	sensors[8]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER ;
	sensors[8]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_WATTS;
	sensors[8]->functionPtr = &odroid_sensor_read;
	sensors[8]->path = "/sys/bus/i2c/drivers/INA231/4-0041/sensor_W";
	sensors[8]->sdr.cmn.keys.sensor_num = 8;	// must be the same as array-position!
	sensors[8]->record_id = sensors[8]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[8]->sdr.id_string, "GPU power", 9);

	// generate memory current sensor:
	sensors[9] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[9], sensor_default, sizeof(struct sensor));
	sensors[9]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_CURRENT;
	sensors[9]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_AMPS;
	sensors[9]->functionPtr = &odroid_sensor_read;
	sensors[9]->path = "/sys/bus/i2c/drivers/INA231/4-0041/sensor_A";
	sensors[9]->sdr.cmn.keys.sensor_num = 9;	// must be the same as array-position!
	sensors[9]->record_id = sensors[9]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[9]->sdr.id_string, "memory current", 14);
	// generate memory voltage sensor:
	sensors[10] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[10], sensor_default, sizeof(struct sensor));
	sensors[10]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_VOLTAGE;
	sensors[10]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_VOLTS;
	sensors[10]->functionPtr = &odroid_sensor_read;
	sensors[10]->path = "/sys/bus/i2c/drivers/INA231/4-0041/sensor_V";
	sensors[10]->sdr.cmn.keys.sensor_num = 10;	// must be the same as array-position!
	sensors[10]->record_id = sensors[10]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[10]->sdr.id_string, "memory voltage", 14);
	// generate memory watts sensor:
	sensors[11] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[11], sensor_default, sizeof(struct sensor));
	sensors[11]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER ;
	sensors[11]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_WATTS;
	sensors[11]->functionPtr = &odroid_sensor_read;
	sensors[11]->path = "/sys/bus/i2c/drivers/INA231/4-0041/sensor_W";
	sensors[11]->sdr.cmn.keys.sensor_num = 11;	// must be the same as array-position!
	sensors[11]->record_id = sensors[11]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[11]->sdr.id_string, "memory power", 12);

	// generate fan speed sensor:
	sensors[12] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[12], sensor_default, sizeof(struct sensor));
	sensors[12]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER ;
	sensors[12]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_UNSPEC;
	sensors[12]->functionPtr = &odroid_fan_speed;
	sensors[12]->path = "/sys/bus/platform/devices/odroidxu-fan/pwm_duty";
	sensors[12]->sdr.cmn.keys.sensor_num = 12;	// must be the same as array-position!
	sensors[12]->record_id = sensors[12]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[12]->sdr.id_string, "fan speed in %", 14);

	// generate cpu loadavg:
	sensors[13] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[13], sensor_default, sizeof(struct sensor));
	sensors[13]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER;
	sensors[13]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_UNSPEC;
	sensors[13]->functionPtr = &generic_read_loadavg;
	sensors[13]->sdr.cmn.keys.sensor_num = 13;	// must be the same as array-position!
	sensors[13]->record_id = sensors[13]->sdr.cmn.keys.sensor_num;
	memcpy(sensors[13]->sdr.id_string, "cpu loadavg", 11);
#else
	// generate current sensor:
	sensors[0] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[0], sensor_default, sizeof(struct sensor));
	sensors[0]->record_id = 0;
	sensors[0]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_CURRENT;
	sensors[0]->sdr.cmn.unit.type.base = 0x05;	// Sensor Base Unit: Current
	sensors[0]->functionPtr = &read_current;
	sensors[0]->sdr.cmn.keys.sensor_num = 0;	// must be the same as array-position!
	memcpy(sensors[0]->sdr.id_string, "debug current", 13);


	// generate voltage sensor:
	sensors[1] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[1], sensor_default, sizeof(struct sensor));
	sensors[1]->record_id = 1;
	sensors[1]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_VOLTAGE;
	sensors[1]->sdr.cmn.unit.type.base = 0x04;	// Sensor Base Unit: Voltage
	sensors[1]->functionPtr = &read_voltage;
	sensors[1]->sdr.cmn.keys.sensor_num = 1;	// must be the same as array-position!
	memcpy(sensors[1]->sdr.id_string, "debug voltage", 13);

	// generate stress-running sensor:
	sensors[2] = calloc(1, sizeof(struct sensor));
	memcpy(sensors[2], sensor_default, sizeof(struct sensor));
	sensors[2]->record_id = 2;
	sensors[2]->sdr.cmn.sensor.type = SDR_SENSOR_TYPE_OTHER;
	sensors[2]->sdr.cmn.unit.type.base = SDR_SENSOR_UNIT_UNSPEC;	// Sensor Base Unit: Voltage
	sensors[2]->functionPtr = &generic_check_for_running_stress;
	sensors[2]->sdr.cmn.keys.sensor_num = 2;	// must be the same as array-position!
	memcpy(sensors[2]->sdr.id_string, "debug stress", 12);


#endif



	// Cleanup
	free(sensor_default);

}

/* Clean up atexit() */
void sensors_cleanup() {
	int i;
	for (i=0; i<SENSOR_COUNT; i++) {
		free(sensors[i]);
	}
}

#ifdef TARGET_ODROID
int odroid_fan_speed(char* path) {
	FILE * fp;
	int  value = 0;

	fp = fopen(path, "r");
	if (fp == NULL) {

		printf("(ipmi-sensor) Could not open %s\n", path);
		return -1;
	}

	fscanf (fp, "%d", &value);
//	printf("(ipmi-sensor) odroid sensor read: %d\n", value);
	fclose (fp);

	return value*100/255*1000;

}
int odroid_sensor_read(char* path) {
	FILE * fp;
	double  value = 0;

	fp = fopen(path, "r");
	if (fp == NULL) {
		printf("(ipmi-sensor) Could not open %s\n", path);
		return -1;
	}

	fscanf (fp, "%lf", &value);
//	printf("(ipmi-sensor) odroid sensor read: %lf\n", value);
	fclose (fp);

	return value*1000;
}



#endif

void sensor_update(struct sensor *sensor) {
	// get new reading
	int reading = sensor->functionPtr(sensor->path);
	// save it
	sensor->sdr.nominal_read = reading / 511;
	// Update b value
	sensor->sdr.b_acc.b_ls = __M_TO_M_LS(reading % 511);
	sensor->sdr.b_acc.b_ms = __M_TO_M_MS(reading % 511);
}

struct sensor* get_sensor(uint8_t sensor_number) {
	return sensors[sensor_number];
}

int get_next_sensor(int sensor_number) {
	if (sensor_number < SENSOR_COUNT -1) {
		return sensor_number + 1;
	} else {
		return -1;
	}
}
int read_current() {
	return 1234;
}
int read_voltage() {
	return 4321;
}

// check if 'stress' is running
int generic_check_for_running_stress() {
	FILE * fp;
	char pid[10];
	fp = popen("pgrep -x stress", "r");

	if (fgets(pid, 10, fp) == NULL) {
		fclose(fp);
		return 0; 	// not running
	} else {
		fclose(fp);
		return 1000; 	// stress is running
	}
}


// read avg load, return in percent
int generic_read_loadavg () {
        double load[1];
        getloadavg(load,1);
        return (int)(load[0]*1000.0);
}

#ifdef TARGET_RPI
int i2c_file = 0;

int rpi_i2c_init() {
	char *filename =  "/dev/i2c-1";		/* the i2c dev file */
	int addr = 0x40; 					/* The i2c address */

	// Open i2c file
	i2c_file = open(filename, O_RDWR);
	if (i2c_file < 0) {
		printf("(ipmi-sensor): Could not open i2c-dev-file, Error nr %i\n",errno);
		return -1;
	}

	// Set i2c Device Address
	if (ioctl(i2c_file, I2C_SLAVE, addr) < 0) {
		printf("(ipmi-sensor): Could set i2c device address, Error nr %i\n",errno);
		exit(1);
	}
	return 1;

}

int rpi_i2c_read_current(char* path) {
	__u8 reg = 0x01; /* Device register to access */
	__s32 res;
	float resres;
	/* Using SMBus commands */
	res = i2c_smbus_read_word_data(i2c_file, reg);
	if (res < 0) {
		/* ERROR HANDLING: i2c transaction failed */
		printf("(ipmi-sensor): Error at reading i2c register, errno %i\n",errno);
		return 0;
	} else {
		/* res contains the read word */
		res = ((res & 0x00FF)<<8) | (res>>8); // switch bytes
		/* calculate shunt current:
		 *  - shunt voltage LSB: 10uV == 0.01mV  	=> /100
		 *  - shunt resistor:	50mOhm == 0.05Ohm	=> /0.05 or * 20
		 *  current = raw / 100 * 20
		 */
		res = res * 20;
		resres = res / 100.0;
	//	printf("(ipmi-sensor) Read Done, res is: %3.1fmA\n", resres);
	}
	return resres;
}

int rpi_i2c_read_bus_voltage(char* path) {
	__u8 reg = 0x02; /* Device register to access */
	__u32 res;
	float resres;
	/* Using SMBus commands */
	res = i2c_smbus_read_word_data(i2c_file, reg);
	if (res < 0) {
		/* ERROR HANDLING: i2c transaction failed */
		printf("(ipmi-sensor): Error at reading i2c register, errno %i\n",errno);
	} else {
		/* res contains the read word */
		res = ((res & 0x00FF)<<8) | (res>>8); // switch bytes
		/* calculate bus voltage:
		 *  - register it not right aligned => shift 3 right
		 *  - LSB is 4mV 					=> shift 2 left
		 * 					final result: 	=> shift 1 right
		 */
		resres = res >> 1;
	//	printf("(ipmi-sensor) Read Done, res is: %3.1fmV\n", resres);
	}
	return resres;
}
int rpi_i2c_read_power(char* path) {
	int voltage = rpi_i2c_read_bus_voltage(0);	// mV
	int current = rpi_i2c_read_current(0);		// mA
	float power = voltage*current/1000.0;
	//printf("(ipmi-sensor): voltage: %d current: %d power: %f\n",voltage, current, power);
	return (int)power;
}
#endif
