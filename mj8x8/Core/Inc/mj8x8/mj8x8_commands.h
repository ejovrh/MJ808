#ifndef CORE_INC_MJ8X8_MJ8X8_COMMANDS_H_
#define CORE_INC_MJ8X8_MJ8X8_COMMANDS_H_

#pragma once

/* TODO - CAN bootloader
 * http://www.kreatives-chaos.com/artikel/can-bootloader
 */

/* MJ818 - rear & brake light
 current sense resistor is: 10R/0.5W
 decimal val. - duty cycle - current rear light
 00 - 0% (off)		-	4 mA
 10 - 10%					-	4 mA
 20 - 20%					-	7 mA
 30 - 30%					-	9 mA
 40 - 40%					-	11 mA
 50 - 50%					-	13 mA
 60 - 60%					-	15 mA
 70 - 70%					-	17 mA
 80 - 80%					-	20 mA
 90 - 90%					-	23 mA
 100 - 100% (max)	-	25 mA

 current sense resistor is: WSLT2010R2200FEB18 / 0.22Ohm 1W
 decimal val. - duty cycle - current brake light
 00 - 0% (off)		-	4 mA
 10 - 10%					-	16 mA
 20 - 20%					-	60 mA
 30 - 30%					-	115 mA
 40 - 40%					-	176 mA
 50 - 50%					-	178 mA
 60 - 60%					-	235 mA
 70 - 70%					-	278 mA
 80 - 80%					-	296 mA
 90 - 90%					-	330 mA
 100 - 100% (max)	-	335 mA

 75% is MAX. SAFE COUNT
 */

/* MJ808 - front light
 current sense resistor is: WSLT2010R2200FEB18 / 0.22Ohm 1W
 decimal val. - duty cycle - current front light
 00 - 0% (off)		-	4 mA
 10 - 10%					-	20 mA
 20 - 20%					-	83 mA
 30 - 30%					-	120 mA
 40 - 40%					-	169 mA
 50 - 50%					-	197 mA
 60 - 60%					-	246 mA
 70 - 70%					-	295 mA
 80 - 80%					-	327 mA
 90 - 90%					-	396 mA
 100 - 100% (max)	-	417 mA

 75% is MAX. SAFE COUNT
 */

// maximum safe values for light OCRs
#define OCR_MAX_FRONT_LIGHT 75	// decimal - max. count limit for front light PWM
#define OCR_MAX_REAR_LIGHT 100	// decimal - max. count limit for rear light PWM - the sky is the limit
#define OCR_MAX_BRAKE_LIGHT 100	// decimal - max. count limit for brake light PWM

#define OFF 0x00
#define ON 0x01
#define	BLINK	0x02

#define LED_OFF 0	// decimal - off value for any OCR
#define BLINK_DELAY 125	// delay used in the util_led() function
#define REDISCOVER_ITERATION 2	// every (value * 1s) the CAN device will do a re-broadcast

#define MASK_COMMAND 0xF0
#define MASK_ARGUMENT 0x0F

// command byte structure
#define CMND_ANNOUNCE 0x00	// command to register self on other devices (announce-like broadcast)

typedef union  // union of mj8x8 devices: 16 bits or one word
{
	struct  // bit-wise struct for mj8x8 devices
	{  // bit-wise view of devices on the bus,  max. 16 in total
	   // format: n (0 to 15) - device indicator - hex id - name
		uint16_t mj_0 :1;  	// 0 // 0 Alpha - logic unit
		uint16_t mj_1 :1;  	// 1 // 0 Bravo - ?
		uint16_t mj_2 :1;  	// 2 // 0 Charlie - ?
		uint16_t mj828 :1;  // 3 // 0 Delta - dashboard
		uint16_t mj838 :1;  // 4 // 1 Alpha - Čos dynamo/generator
		uint16_t mj_5 :1;  	// 5 // 1 Bravo - ?
		uint16_t mj_6 :1;  	// 6 // 1 Charlie - ?
		uint16_t mj_7 :1;  	// 7 // 1 Delta - ?
		uint16_t mj808 :1;  // 8 // 2 Alpha - front light
		uint16_t mj818 :1;  // 9 // 2 Bravo - rear light
		uint16_t mj_10 :1;	// a // 2 Charlie - ?
		uint16_t mj_11 :1;	// b // 2 Delta - ?
		uint16_t mj_12 :1;	// c // 3 Alpha - ?
		uint16_t mj_13 :1;	// d // 3 Bravo - ?
		uint16_t mj_14 :1;	// e // 3 Charlie - ?
		uint16_t mj_15 :1;	// f // 3 Delta - ?
	};
	uint16_t byte;	// word representation of above bitfield
} device_t;

// mj808 front light/high beam
//#define FRONT_LIGHT 0x00	// with fade - front positional light (mj808) - low beam
//#define FRONT_LIGHT_HIGH 0x01	// WITHOUT fade - front positional light (mj808) - high beam
#define FRONT_HIGHBEAM 0xC8	// 200

// mj818 rear light/brake light
//#define REAR_LIGHT 0x02	// rear positional light (mj818)
//#define REAR_LIGHT_BRAKE 0x03	// brake light (mj818)

#define REAR_BRAKELIGHT 0xC8	// 200

// mj828 LED definitions
#define DEV_DB_LED 0x00
#define RED 0x04
#define GREEN 0x08
#define YELLOW 0x0C
#define BLUE 0x10
#define BATT1 0x14
#define BATT2 0x18
#define BATT3 0x1C
#define BATT4 0x20

//#define CMND_UTIL_LED 0x10	// command for utility LED operation (colour, on, off, blink)

// command classes
//#define CLASS_ANNOUNCE 0x00	//
//#define CLASS_UTIL_LED 0x10	//
//#define CLASS_DASHBOARD	0x20	//
//#define CLASS_DEVICE 0x40	//
//#define CLASS_FW_FLASH 0x70
//#define CLASS_MSG_TIME_SYNC 0x80	//
#define CLASS_MSG_BUTTON_EVENT 0x90	//
#define CLASS_MSG_MEASURE_DATA 0xD0	//
//#define CLASS_MSG_BUS 0xF0	//

#define EVENT00 0
#define EVENT01 1
#define EVENT02 2
#define EVENT03 3
#define EVENT04 4
#define EVENT05 5
#define EVENT06 6
#define EVENT07 7
#define EVENT08 8
#define EVENT09 9
#define EVENT10 10
#define EVENT11 11
#define EVENT12 12
#define EVENT13 13
#define EVENT14 14
#define EVENT15 15

#define SPEED 0	// speed in m/s as derived from zerocross signal
#define	ACCELERATION	1	// acceleration as derived from speed change

#define MSG_MEASUREMENT_SPEED							(CLASS_MSG_MEASURE_DATA | SPEED)
#define MSG_MEASUREMENT_ACCEL							(CLASS_MSG_MEASURE_DATA | ACCELERATION)

//#define MSG_BUS 0xF0	// CAN bus related control messages

//bit fields for command byte
//	B7:B5 are command nibbles and not broken down further
//#define B3 3
//#define B2 2
//#define B1 1
//#define B0 0

// CAN ID defines - see xls; must be manually left shifted by 5 to match a 13-byte CAN ID
// upper byte - b7
#define PRIORITY_LOW 0x0200	// default 0, used by the sender, leave zero on self, except with logic unit 0A (defaults to 1)
#define PRIORITY_HIGH 0x0000

// upper byte - b6
#define BROADCAST 0x0100	// default 0, used by the sender
#define UNICAST 0x0000

typedef enum  // enum describing devices on a mj8x8 bus
{
	  ALL = 0,
	  mj_00 = 0x0,  // sender: 0x00
	  mj_01 = 0x1,  // sender: 0x10
	  mj_02 = 0x2,  // sender: 0x20
	  mj828 = 0x3,	// sender: 0x30
	  mj838 = 0x4,	// sender: 0x40
	  mj_05 = 0x5,  // sender: 0x50
	  mj_06 = 0x6,	// sender: 0x60
	  mj_07 = 0x7,	// sender: 0x70
	  mj808 = 0x8,	// sender: 0x80
	  mj818 = 0x9,	// sender: 0x90
	  mj_10 = 0xA,	// sender: 0xA0
	  mj_11 = 0xB,	// sender: 0xB0
	  mj_12 = 0xC,	// sender: 0xC0
	  mj_13 = 0xD,	// sender: 0xD0
	  mj_14 = 0xE,	// sender: 0xE0
	  mj_15 = 0xF,	// sender: 0xF0
} mj8x8_Devices_t;

#define MSG_BUTTON_EVENT_00							(CLASS_MSG_BUTTON_EVENT | EVENT00)
#define MSG_BUTTON_EVENT_01							(CLASS_MSG_BUTTON_EVENT | EVENT01)
#define MSG_BUTTON_EVENT_02							(CLASS_MSG_BUTTON_EVENT | EVENT02)
#define MSG_BUTTON_EVENT_03							(CLASS_MSG_BUTTON_EVENT | EVENT03)
#define MSG_BUTTON_EVENT_04							(CLASS_MSG_BUTTON_EVENT | EVENT04)
#define MSG_BUTTON_EVENT_05							(CLASS_MSG_BUTTON_EVENT | EVENT05)
#define MSG_BUTTON_EVENT_06							(CLASS_MSG_BUTTON_EVENT | EVENT06)
#define MSG_BUTTON_EVENT_07							(CLASS_MSG_BUTTON_EVENT | EVENT07)
#define MSG_BUTTON_EVENT_08							(CLASS_MSG_BUTTON_EVENT | EVENT08)
#define MSG_BUTTON_EVENT_09							(CLASS_MSG_BUTTON_EVENT | EVENT09)
#define MSG_BUTTON_EVENT_10							(CLASS_MSG_BUTTON_EVENT | EVENT10)
#define MSG_BUTTON_EVENT_11							(CLASS_MSG_BUTTON_EVENT | EVENT11)
#define MSG_BUTTON_EVENT_12 						(CLASS_MSG_BUTTON_EVENT | EVENT12)
#define MSG_BUTTON_EVENT_13 						(CLASS_MSG_BUTTON_EVENT | EVENT13)
#define MSG_BUTTON_EVENT_14 						(CLASS_MSG_BUTTON_EVENT | EVENT14)
#define MSG_BUTTON_EVENT_15 						(CLASS_MSG_BUTTON_EVENT | EVENT15)

#define ARG_BRAKELIGHT_OFF							(FRONT_HIGHBEAM | OFF)
#define ARG_BRAKELIGHT_ON								(FRONT_HIGHBEAM | ON)
#define ARG_HIGHBEAM_OFF								(REAR_BRAKELIGHT | OFF)
#define ARG_HIGHBEAM_ON									(REAR_BRAKELIGHT | ON)

#define ARG_UTIL_LED	0x30
#define	UTIL_LED_RED	0x00
#define	UTIL_LED_GREEN	0x04

#define ARG_UTIL_LED_RED								(ARG_UTIL_LED | UTIL_LED_RED)
#define ARG_UTIL_LED_RED_OFF						(ARG_UTIL_LED | UTIL_LED_RED | OFF)
#define ARG_UTIL_LED_RED_ON							(ARG_UTIL_LED | UTIL_LED_RED | ON)
#define ARG_UTIL_LED_RED_BLINK_FAST			(ARG_UTIL_LED | UTIL_LED_RED | BLINK)

#define ARG_UTIL_LED_GREEN							(ARG_UTIL_LED | UTIL_LED_GREEN)
#define ARG_UTIL_LED_GREEN_OFF					(ARG_UTIL_LED | UTIL_LED_GREEN | OFF)
#define ARG_UTIL_LED_GREEN_ON						(ARG_UTIL_LED | UTIL_LED_GREEN | ON)
#define ARG_UTIL_LED_GREEN_BLINK_FAST		(ARG_UTIL_LED | UTIL_LED_GREEN | BLINK)

#endif /* CORE_INC_MJ8X8_MJ8X8_COMMANDS_H_ */
