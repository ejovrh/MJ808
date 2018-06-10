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
#define FOO3 7 // acknowledge bit: 1 ack ?
#define FOO2 6 //
#define FOO1 5 //
#define BUTTON 4 //
#define SIGNAL_LED 3 //
#define REAR_LIGHT 2 // expects 1 byte argument: off (0x00), on (0xff), PWM intensity (0x01 - 0xfe)
#define FRONT_LIGHT 1 // expects 1 byte argument: off (0x00), on (0xff), PWM intensity (0x01 - 0xfe)
#define BRAKE_LIGHT 0 // expects 1 byte argument: off (0x00), on (0xff), PWM intensity (0x01 - 0xfe)



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

void blink_red(uint8_t led, uint8_t count);

#endif /* MJ808_H_ */