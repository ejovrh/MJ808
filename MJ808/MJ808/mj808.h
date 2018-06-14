#ifndef MJ808_H_
#define MJ808_H_

#define OCR_FRONT_LIGHT OCR1A // Output Compare Register for PWM of front light
#define GREEN_LED 0 // green led
#define RED_LED 1 // red led
#define FADE_DOWN 0 // fade direction down
#define FADE_UP 1 // fade direction up
#define BLINK_DELAY 125

#define CAN_IN can_msg_incoming
#define CAN_OUT can_msg_outgoing
#define COMMAND data[0]
#define ARGUMENT data[1]

// command byte structure
#define CMND_UTIL_LED 0x10 // command for utility LED operation (color, on, off, blink)
#define CMND_POS_LED 0x40 // command for positional LED operation (front, rear, brake light on/off, fade)
#define CMND_FW_FLASH 0x70 // command for flashing firmware
#define MSG_TIME_SYNC 0x80 // time synchronization message
#define MSG_BUTTON_EVENT 0x90 // message for button events
#define MSG_MEASUREMENT_DATA 0xD0 // message containing various measurements
#define MSG_BUS 0xF0 // CAN bus related control messages

#define LED_FRONT 0x00 // ID for front light
#define LED_REAR 0x02 // ID for rear light
#define LED_BREAK 0x03 // ID for brake light

//bit fields for command byte
//	B7:B5 are command nibbles and not broken down further
#define B3 3
#define B2 2
#define B1 1
#define B0 0


/*
		timer/counter 0 and timer/counter 1 both operate in 8bit mode
		hex val. - duty cycle - current front light
		0x00 - 0% (off)				-	20 mA
		0x10 - 6.26%					-	20 mA
		0x20 - 12.5%					-	30 mA
		0x40 - 25.05%					-	60 mA
		0x80 - 50.1%					-	100 mA
		0xA0 - 62.6%					-	120 mA
		0xC0 - 75.15%					-	150 mA
		0xE0 - 87.84%					-	170 mA
		0xF0 - 94.12%					-	180 mA
		0xFF - 100% (max)			-	200 mA

		0x80 MAX. SAFE COUNT WITH REV2 BOARD -- with this the FET is dangerously hot
		0x40 is less stressing
*/
#define CONTER_FRONT_LIGHT_MAX 0x80 // max. OCR1A count limit for front light PWM

void util_led(uint8_t in_val); // controls utility LED (red, green, on, off, blink)

#endif /* MJ808_H_ */