#define HAVE_PRAGMA_PACK 1
#define ATTRIBUTE_PACKING
#include <stdint.h>

#ifndef IPMI_SDR_H
#define IPMI_SDR_H

#ifdef HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct entity_id {
	uint8_t	id;			/* physical entity id */
#if WORDS_BIGENDIAN
	uint8_t	logical     : 1;	/* physical/logical */
	uint8_t	instance    : 7;	/* instance number */
#else
	uint8_t	instance    : 7;	/* instance number */
	uint8_t	logical     : 1;	/* physical/logical */
#endif
} ATTRIBUTE_PACKING;
#ifdef HAVE_PRAGMA_PACK
#pragma pack(0)
#endif



// defines for converting to m, b, acc, r, tol, etc (p. 552 in IPMI spec v2 rev 1.1
#define __M_TOL_TO_MTOL(m, tol)							(uint16_t)((m & 0xff) | ((m & 0b1100000000) << 6) | ((tol & 0b00111111) << 8) )
#define __B_TOL_TO_BTOL(b, tol) 						__M_TOL_TO_MTOL(b, tol)
#define __M_TO_M_LS(m)  		(m & 0xff)						// lower 8 bits
#define __M_TO_M_MS(m)  		((m & 0b1100000000) >> 8)		// upper 2 bits

#define __R_EXP_B_EXP_MERGE(r_exp, b_exp)
#if WORDS_BIGENDIAN
# define __TO_TOL(mtol)     (uint16_t)(mtol & 0x3f)
# define __TO_M(mtol)       (int16_t)(tos32((((mtol & 0xff00) >> 8) | ((mtol & 0xc0) << 2)), 10))
# define __TO_B(bacc)       (int32_t)(tos32((((bacc & 0xff000000) >> 24) | ((bacc & 0xc00000) >> 14)), 10))
# define __TO_ACC(bacc)     (uint32_t)(((bacc & 0x3f0000) >> 16) | ((bacc & 0xf000) >> 6))
# define __TO_ACC_EXP(bacc) (uint32_t)((bacc & 0xc00) >> 10)
# define __TO_R_EXP(bacc)   (int32_t)(tos32(((bacc & 0xf0) >> 4), 4))
# define __TO_B_EXP(bacc)   (int32_t)(tos32((bacc & 0xf), 4))
#else
# define __TO_TOL(mtol)     (uint16_t)(BSWAP_16(mtol) & 0x3f)
# define __TO_M(mtol)       (int16_t)(tos32((((BSWAP_16(mtol) & 0xff00) >> 8) | ((BSWAP_16(mtol) & 0xc0) << 2)), 10))
# define __TO_B(bacc)       (int32_t)(tos32((((BSWAP_32(bacc) & 0xff000000) >> 24) | \
                            ((BSWAP_32(bacc) & 0xc00000) >> 14)), 10))
# define __TO_ACC(bacc)     (uint32_t)(((BSWAP_32(bacc) & 0x3f0000) >> 16) | ((BSWAP_32(bacc) & 0xf000) >> 6))
# define __TO_ACC_EXP(bacc) (uint32_t)((BSWAP_32(bacc) & 0xc00) >> 10)
# define __TO_R_EXP(bacc)   (int32_t)(tos32(((BSWAP_32(bacc) & 0xf0) >> 4), 4))
# define __TO_B_EXP(bacc)   (int32_t)(tos32((BSWAP_32(bacc) & 0xf), 4))
#endif


#ifdef HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct sdr_record_mask {
	union {
		struct {
			uint16_t assert_event;	/* assertion event mask */
			uint16_t deassert_event;	/* de-assertion event mask */
			uint16_t read;	/* discrete reading mask */
		} ATTRIBUTE_PACKING discrete;
		struct {
#if WORDS_BIGENDIAN
			uint16_t reserved:1;
			uint16_t status_lnr:1;
			uint16_t status_lcr:1;
			uint16_t status_lnc:1;
			uint16_t assert_unr_high:1;
			uint16_t assert_unr_low:1;
			uint16_t assert_ucr_high:1;
			uint16_t assert_ucr_low:1;
			uint16_t assert_unc_high:1;
			uint16_t assert_unc_low:1;
			uint16_t assert_lnr_high:1;
			uint16_t assert_lnr_low:1;
			uint16_t assert_lcr_high:1;
			uint16_t assert_lcr_low:1;
			uint16_t assert_lnc_high:1;
			uint16_t assert_lnc_low:1;
#else
			uint16_t assert_lnc_low:1;
			uint16_t assert_lnc_high:1;
			uint16_t assert_lcr_low:1;
			uint16_t assert_lcr_high:1;
			uint16_t assert_lnr_low:1;
			uint16_t assert_lnr_high:1;
			uint16_t assert_unc_low:1;
			uint16_t assert_unc_high:1;
			uint16_t assert_ucr_low:1;
			uint16_t assert_ucr_high:1;
			uint16_t assert_unr_low:1;
			uint16_t assert_unr_high:1;
			uint16_t status_lnc:1;
			uint16_t status_lcr:1;
			uint16_t status_lnr:1;
			uint16_t reserved:1;
#endif
#if WORDS_BIGENDIAN
			uint16_t reserved_2:1;
			uint16_t status_unr:1;
			uint16_t status_ucr:1;
			uint16_t status_unc:1;
			uint16_t deassert_unr_high:1;
			uint16_t deassert_unr_low:1;
			uint16_t deassert_ucr_high:1;
			uint16_t deassert_ucr_low:1;
			uint16_t deassert_unc_high:1;
			uint16_t deassert_unc_low:1;
			uint16_t deassert_lnr_high:1;
			uint16_t deassert_lnr_low:1;
			uint16_t deassert_lcr_high:1;
			uint16_t deassert_lcr_low:1;
			uint16_t deassert_lnc_high:1;
			uint16_t deassert_lnc_low:1;
#else
			uint16_t deassert_lnc_low:1;
			uint16_t deassert_lnc_high:1;
			uint16_t deassert_lcr_low:1;
			uint16_t deassert_lcr_high:1;
			uint16_t deassert_lnr_low:1;
			uint16_t deassert_lnr_high:1;
			uint16_t deassert_unc_low:1;
			uint16_t deassert_unc_high:1;
			uint16_t deassert_ucr_low:1;
			uint16_t deassert_ucr_high:1;
			uint16_t deassert_unr_low:1;
			uint16_t deassert_unr_high:1;
			uint16_t status_unc:1;
			uint16_t status_ucr:1;
			uint16_t status_unr:1;
			uint16_t reserved_2:1;
#endif
			union {
				struct {
#if WORDS_BIGENDIAN			/* settable threshold mask */
					uint16_t reserved:2;
					uint16_t unr:1;
					uint16_t ucr:1;
					uint16_t unc:1;
					uint16_t lnr:1;
					uint16_t lcr:1;
					uint16_t lnc:1;
					/* padding lower 8 bits */
					uint16_t readable:8;
#else
					uint16_t readable:8;
					uint16_t lnc:1;
					uint16_t lcr:1;
					uint16_t lnr:1;
					uint16_t unc:1;
					uint16_t ucr:1;
					uint16_t unr:1;
					uint16_t reserved:2;
#endif
				} ATTRIBUTE_PACKING set;
				struct {
#if WORDS_BIGENDIAN			/* readable threshold mask */
					/* padding upper 8 bits */
					uint16_t settable:8;
					uint16_t reserved:2;
					uint16_t unr:1;
					uint16_t ucr:1;
					uint16_t unc:1;
					uint16_t lnr:1;
					uint16_t lcr:1;
					uint16_t lnc:1;
#else
					uint16_t lnc:1;
					uint16_t lcr:1;
					uint16_t lnr:1;
					uint16_t unc:1;
					uint16_t ucr:1;
					uint16_t unr:1;
					uint16_t reserved:2;
					uint16_t settable:8;
#endif
				} ATTRIBUTE_PACKING read;
			} ATTRIBUTE_PACKING;
		} ATTRIBUTE_PACKING threshold;
	} ATTRIBUTE_PACKING type;
} ATTRIBUTE_PACKING;
#ifdef HAVE_PRAGMA_PACK
#pragma pack(0)
#endif


// SDR stuff

#define SDR_RECORD_TYPE_FULL_SENSOR				0x01
#define SDR_RECORD_TYPE_COMPACT_SENSOR			0x02
#define SDR_RECORD_TYPE_EVENTONLY_SENSOR		0x03
#define SDR_RECORD_TYPE_ENTITY_ASSOC			0x08
#define SDR_RECORD_TYPE_DEVICE_ENTITY_ASSOC		0x09
#define SDR_RECORD_TYPE_GENERIC_DEVICE_LOCATOR	0x10
#define SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR		0x11
#define SDR_RECORD_TYPE_MC_DEVICE_LOCATOR		0x12
#define SDR_RECORD_TYPE_MC_CONFIRMATION			0x13
#define SDR_RECORD_TYPE_BMC_MSG_CHANNEL_INFO	0x14
#define SDR_RECORD_TYPE_OEM						0xc0


// common sensor stuff
#ifdef HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct sdr_record_common_sensor {
	struct {
		uint8_t owner_id;
#if WORDS_BIGENDIAN
		uint8_t channel:4;	/* channel number */
		uint8_t __reserved:2;
		uint8_t lun:2;	/* sensor owner lun */
#else
		uint8_t lun:2;	/* sensor owner lun */
		uint8_t __reserved:2;
		uint8_t channel:4;	/* channel number */
#endif
		uint8_t sensor_num;	/* unique sensor number */
	} ATTRIBUTE_PACKING keys;

	struct entity_id entity;

	struct {
		struct {
#if WORDS_BIGENDIAN
			uint8_t __reserved:1;
			uint8_t scanning:1;
			uint8_t events:1;
			uint8_t thresholds:1;
			uint8_t hysteresis:1;
			uint8_t type:1;
			uint8_t event_gen:1;
			uint8_t sensor_scan:1;
#else
			uint8_t sensor_scan:1;
			uint8_t event_gen:1;
			uint8_t type:1;
			uint8_t hysteresis:1;
			uint8_t thresholds:1;
			uint8_t events:1;
			uint8_t scanning:1;
			uint8_t __reserved:1;
#endif
		} ATTRIBUTE_PACKING init;
		struct {
#if WORDS_BIGENDIAN
			uint8_t ignore:1;
			uint8_t rearm:1;
			uint8_t hysteresis:2;
			uint8_t threshold:2;
			uint8_t event_msg:2;
#else
			uint8_t event_msg:2;
			uint8_t threshold:2;
			uint8_t hysteresis:2;
			uint8_t rearm:1;
			uint8_t ignore:1;
#endif
		} ATTRIBUTE_PACKING capabilities;
		uint8_t type;
	} ATTRIBUTE_PACKING sensor;

	uint8_t event_type;	/* event/reading type code */

	struct sdr_record_mask mask;

	struct {
#if WORDS_BIGENDIAN
		uint8_t analog:2;
		uint8_t rate:3;
		uint8_t modifier:2;
		uint8_t pct:1;
#else
		uint8_t pct:1;
		uint8_t modifier:2;
		uint8_t rate:3;
		uint8_t analog:2;
#endif
		struct {
			uint8_t base;
			uint8_t modifier;
		} ATTRIBUTE_PACKING type;
	} ATTRIBUTE_PACKING unit;
} ATTRIBUTE_PACKING;


#ifdef HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct sdr_record_full_sensor {
	struct sdr_record_common_sensor cmn;

#define SDR_SENSOR_L_LINEAR     0x00
#define SDR_SENSOR_L_LN         0x01
#define SDR_SENSOR_L_LOG10      0x02
#define SDR_SENSOR_L_LOG2       0x03
#define SDR_SENSOR_L_E          0x04
#define SDR_SENSOR_L_EXP10      0x05
#define SDR_SENSOR_L_EXP2       0x06
#define SDR_SENSOR_L_1_X        0x07
#define SDR_SENSOR_L_SQR        0x08
#define SDR_SENSOR_L_CUBE       0x09
#define SDR_SENSOR_L_SQRT       0x0a
#define SDR_SENSOR_L_CUBERT     0x0b
#define SDR_SENSOR_L_NONLINEAR  0x70

	uint8_t linearization;	/* 70h=non linear, 71h-7Fh=non linear, OEM */
//	uint16_t mtol;		/* M, tolerance */
	struct {
		uint8_t m_ls:8;		/* m: 8 LS bits */
		uint8_t m_tol:6;	/* m tolerance */
		uint8_t m_ms:2;		/* m: 2 MS bits */
	} ATTRIBUTE_PACKING mtol;

	struct {
		uint8_t b_ls;			/* b: 8 LS bits */

		uint8_t acc_ls:6;		/* accuracy: 6 LS bits */
		uint8_t b_ms:2;			/* b: 2 MS bits */

		uint8_t sensor_dir:2;	/* sensor direction: 2  bits*/
		uint8_t acc_exp:2;		/* accuracy exponent: 2 bits*/
		uint8_t acc_ms:4;		/* accuracy: 4 MS bits*/






	} ATTRIBUTE_PACKING b_acc;


	struct {
		uint8_t b_exp:4;		/* b exponent: 4 bit signed */
		uint8_t r_exp:4;		/* r exponent: 4 bit signed */

	} ATTRIBUTE_PACKING r_b_exp;


	struct {
#if WORDS_BIGENDIAN
		uint8_t __reserved:5;
		uint8_t normal_min:1;	/* normal min field specified */
		uint8_t normal_max:1;	/* normal max field specified */
		uint8_t nominal_read:1;	/* nominal reading field specified */
#else
		uint8_t nominal_read:1;	/* nominal reading field specified */
		uint8_t normal_max:1;	/* normal max field specified */
		uint8_t normal_min:1;	/* normal min field specified */
		uint8_t __reserved:5;
#endif
	} ATTRIBUTE_PACKING analog_flag;

	uint8_t nominal_read;	/* nominal reading, raw value */
	uint8_t normal_max;	/* normal maximum, raw value */
	uint8_t normal_min;	/* normal minimum, raw value */
	uint8_t sensor_max;	/* sensor maximum, raw value */
	uint8_t sensor_min;	/* sensor minimum, raw value */

	struct {
		struct {
			uint8_t non_recover;
			uint8_t critical;
			uint8_t non_critical;
		} ATTRIBUTE_PACKING upper;
		struct {
			uint8_t non_recover;
			uint8_t critical;
			uint8_t non_critical;
		} ATTRIBUTE_PACKING lower;
		struct {
			uint8_t positive;
			uint8_t negative;
		} ATTRIBUTE_PACKING hysteresis;
	} ATTRIBUTE_PACKING threshold;
	uint8_t __reserved[2];
	uint8_t oem;		/* reserved for OEM use */
	uint8_t id_code;	/* sensor ID string type/length code */
	uint8_t id_string[16];	/* sensor ID string bytes, only if id_code != 0 */
} ATTRIBUTE_PACKING;
#ifdef HAVE_PRAGMA_PACK
#pragma pack(0)
#endif


#define	SDR_HEADER_LENGTH				0x05
// Sensor Type Codes
#define SDR_SENSOR_TYPE_RESERVED	 	0x00
#define SDR_SENSOR_TYPE_TEMPERATURE 	0x01
#define SDR_SENSOR_TYPE_VOLTAGE 		0x02
#define SDR_SENSOR_TYPE_CURRENT			0x03
#define SDR_SENSOR_TYPE_FAN 			0x04
#define SDR_SENSOR_TYPE_OTHER 			0x0B

// Event/Reading Type Codes
#define SDR_EVENT_TYPE_UNSPEC			0x00
#define SDR_EVENT_TYPE_THRESHOLD		0x01
#define SDR_EVENT_TYPE_GENERIC			0x02

// Base Units (s. p. 554 IPMI spec v2 rev 1.1)
#define SDR_SENSOR_UNIT_UNSPEC			0x00
#define SDR_SENSOR_UNIT_DEGREE_C		0x01
#define SDR_SENSOR_UNIT_VOLTS			0x04
#define SDR_SENSOR_UNIT_AMPS			0x05
#define SDR_SENSOR_UNIT_WATTS			0x06
#define SDR_SENSOR_UNIT_RPM				0x12
#define SDR_SENSOR_UNIT_HZ				0x13
#define SDR_SENSOR_UNIT_USECONDS		0x14
#define SDR_SENSOR_UNIT_MSECONDS		0x15
#define SDR_SENSOR_UNIT_SECONDS			0x16
#define SDR_SENSOR_UNIT_MINUTE			0x17
#define SDR_SENSOR_UNIT_HOUR			0x18
#define SDR_SENSOR_UNIT_DAY				0x19
#define SDR_SENSOR_UNIT_WEEK			0x1A
#define SDR_SENSOR_UNIT_			0x0



#endif /* IPMI_SDR_H */

