#ifndef MJ8x8_H_
#define MJ8x8_H_

/* TODO - CAN bootloader
 * http://www.kreatives-chaos.com/artikel/can-bootloader
 *
 *
 *
 *
 *
 */


// OCR defines
#define OCR_FRONT_LIGHT OCR0A											// Output Compare Register for PWM of front light
#define OCR_REAR_LIGHT OCR0A											// Output Compare Register for PWM of rear light
#define OCR_BRAKE_LIGHT OCR1A											// Output Compare Register for PWM of brake light

	/* MJ818 - rear & brake light
		timer/counter 0 and timer/counter 1 both operate in 8bit mode
		hex val. - duty cycle - current backlight - current brake light
		0x00 - 0% (off)				-	20 mA -	20 mA
		0x10 - 6.26%					-	20 mA - 20 mA
		0x20 - 12.5%					-	20 mA -	30
		0x40 - 25.05%					-	30 mA -	50
		0x80 - 50.1%					-	50 mA -	90
		0xA0 - 62.6%					-	50 mA -	110
		0xC0 - 75.15%					-	60 mA -	130
		0xE0 - 87.84%					-	70 mA -	150
		0xF0 - 94.12%					- 70 mA -	160
		0xFF - 100% (max)			-	80 mA -	170

		0xFF OCR0A MAX. SAFE COUNT WITH REV2 BOARD
		0x80 OCR1A MAX. SAFE COUNT WITH REV2 BOARD

*/
	/* MJ808 - front light
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

// maximum safe values for light OCRs
#define OCR_MAX_FRONT_LIGHT 0xE0										// max. OCR1A count limit for front light PWM
#define OCR_MAX_REAR_LIGHT 0xFF											// max. OCR0A count limit for rear light PWM - the sky is the limit
#define OCR_MAX_BRAKE_LIGHT 0xF8										// max. OCR1A count limit for brake light PWM

#define LED_OFF 0x00													// off value for any OCR
#define BLINK_DELAY 125													// delay used in the util_led() function
#define REDISCOVER_ITERATION 2											// every (value * 1s) the CAN device will do a re-broadcast
#define BUTTON_MIN_PRESS_TIME 20										// number times 25ms duration: 500ms
#define BUTTON_MAX_PRESS_TIME 120										// number times 25ms duration: 3s

#define CAN_IN can_msg_incoming
#define CAN_OUT can_msg_outgoing
#define COMMAND data[0]
#define ARGUMENT data[1]

// command byte structure
#define CMND_ANNOUNCE 0x00												// command to register self on other devices (announce-like broadcast)
	#define DEV_0A 0 													// main (1st) logic unit
		#define LU 0
	#define DEV_0B 1 													//	2nd logic unit
	#define DEV_0C 2 													//	3rd logic unit
	#define DEV_0D 3													//	4th logic unit
		#define MJ828 3													// dashboard
	#define DEV_1A 4													//	dynamo1
		#define DYN1 4
	#define DEV_1B 5 													//	dynamo2
		#define DYN2 5
	#define DEV_1C 6 													//	battery
		#define BATT 6
	#define DEV_1D 7 													//	solar cell
	#define DEV_2A 8 													//	mj808
		#define MJ808 8
	#define DEV_2B 9 													//	mj818
		#define MJ818 9
	#define DEV_2C 10 													//	??
	#define DEV_2D 11 													//	??
	#define DEV_3A 12													//	cadence
	#define DEV_3B 13													//	radar
	#define DEV_3C 14													//	??
	#define DEV_3D 15 													//	??
#define CMND_UTIL_LED 0x10 												// command for utility LED operation (color, on, off, blink)
	#define LEDS 0x07
//	#define GREEN 0x00
	// TODO - change LED command layout - RED becomes 0x01
	#define RED 0x00
	#define GREEN 0x01
	#define YELLOW 0x02
	#define BLUE 0x03
	#define BLUE_BATT_INDICATOR1 0x04
	#define BLUE_BATT_INDICATOR2 0x05
	#define BLUE_BATT_INDICATOR3 0x06
	#define BLUE_BATT_INDICATOR4 0x07
	#define UTIL_LED_GREEN_OFF 0x10 									// utility LED - off
	#define UTIL_LED_GREEN_ON 0x17 										// utility LED - on
	#define UTIL_LED_GREEN_BLINK_1X 0x11 								// utility LED - blink
	#define UTIL_LED_GREEN_BLINK_2X 0x12 								// utility LED - blink
	#define UTIL_LED_GREEN_BLINK_3X 0x13 								// utility LED - blink
	#define UTIL_LED_GREEN_BLINK_4X 0x14 								// utility LED - blink
	#define UTIL_LED_GREEN_BLINK_5X 0x15 								// utility LED - blink
	#define UTIL_LED_GREEN_BLINK_6X 0x16								// utility LED - blink
	#define UTIL_LED_RED_OFF 0x18										// utility LED - off
	#define UTIL_LED_RED_ON 0x1F										// utility LED - on
	#define UTIL_LED_RED_BLINK_1X 0x19									// utility LED - blink
	#define UTIL_LED_RED_BLINK_2X 0x1A									// utility LED - blink
	#define UTIL_LED_RED_BLINK_3X 0x1B									// utility LED - blink
	#define UTIL_LED_RED_BLINK_4X 0x1C									// utility LED - blink
	#define UTIL_LED_RED_BLINK_5X 0x1D										// utility LED - blink
	#define UTIL_LED_RED_BLINK_6X 0x1E									// utility LED - blink
	#define LED_STATE_ON 0x01
	#define LED_STATE_OFF 0x00
	#define LED_STATE_MASK 0x01
	#define LED_BLINK_MASK 0x06

// TODO - define MJ828 LED & switch commands

#define CMND_DB 0x20

#define CMND_DEVICE 0x40												// command for device (00 - logic unit, 01 - power sources, 02 - lights, 03 sensors)
	#define DEV_LU 0x00													// logic unit device
	#define DEV_PWR_SRC 0x04											// power source device
	#define DEV_LIGHT 0x08												// positional light device
			#define FRONT_LIGHT 0x00									// front positional light (mj808) - low beam
			#define FRONT_LIGHT_HIGH 0x01								// front positional light (mj808) - high beam
			#define REAR_LIGHT 0x02										// rear positional light (mj818)
			#define BRAKE_LIGHT 0x03									// brake light (mj818)
	#define DEV_SENSOR 0x0C												// sensor device
#define CMND_FW_FLASH 0x70												// command for flashing firmware
#define MSG_TIME_SYNC 0x80												// time synchronization message
#define MSG_BUTTON_EVENT 0x90											// message for button events
	#define BUTTON0_OFF 0x00											// button n off
	#define BUTTON0_ON 0x01												// button n on
	#define BUTTON1_OFF 0x02
	#define BUTTON1_ON 0x03
	#define BUTTON2_OFF 0x04
	#define BUTTON2_ON 0x05
	#define BUTTON3_OFF 0x06
	#define BUTTON3_ON 0x07
	#define BUTTON4_OFF 0x08
	#define BUTTON4_ON 0x09
	#define BUTTON5_OFF 0x0A
	#define BUTTON5_ON 0x0B
	#define BUTTON6_OFF 0x0C
	#define BUTTON6_ON 0x0D
	#define BUTTON7_OFF 0x0E
	#define BUTTON7_ON 0x0F
#define MSG_MEASUREMENT_DATA 0xD0										// message containing various measurements
#define MSG_BUS 0xF0													// CAN bus related control messages

#define LED_FRONT 0x00													// ID for front light
#define LED_REAR 0x02													// ID for rear light
#define LED_BREAK 0x03													// ID for brake light

//bit fields for command byte
//	B7:B5 are command nibbles and not broken down further
#define B3 3
#define B2 2
#define B1 1
#define B0 0

// SID defines, must be manually left shifted by 5 (MCP2515 datasheet, p.29 & 30)
// b7
#define PRIORITY_LOW 0x80												// default 0, used by the sender, leave zero on self, except with logic unit 0A (defaults to 1)
#define PRIORITY_HIGH 0x00
// b6
#define BROADCAST 0x40													// default 0, used by the sender
#define UNICAST 0x00
// b5:4
#define SENDER_DEV_CLASS_SENSOR 0x30									// sender device class, always populate on self
#define SENDER_DEV_CLASS_LIGHT 0x20
#define SENDER_DEV_CLASS_PWR_SRC 0x10
#define SENDER_DEV_CLASS_LU 0x00
#define SENDER_DEV_CLASS_BLANK 0x00
// b3:2
#define SENDER_DEV_D 0x0C												// sender device ID, always populate on self
#define SENDER_DEV_C 0x08
#define SENDER_DEV_B 0x04
#define SENDER_DEV_A 0x00
#define SENDER_DEV_BLANK 0x00
// b1:0
#define RCPT_DEV_CLASS_SENSOR 0x03										// recipient device class, populate when sending, on self use RCPT_DEV_BLANK
#define RCPT_DEV_CLASS_LIGHT 0x02
#define RCPT_DEV_CLASS_PWR_SRC 0x01
#define RCPT_DEV_CLASS_LU 0x00
#define RCPT_DEV_CLASS_BLANK 0x00

// b7:6
#define RCPT_DEV_D 0xC0													// recipient device ID, populate when sending, on self use RCPT_DEV_BLANK
#define RCPT_DEV_C 0x80
#define RCPT_DEV_B 0x40
#define RCPT_DEV_A 0x00
#define RCPT_DEV_BLANK 0x00
// b5
#define BLANK 0x00



// TODO - get rid of unions
typedef union															// union of bit fields and uint16_t - representation discovered devices on bus
{
	struct																// bit fields - one bit for each device on the bus
	{
		uint8_t _LU :1;													// 1 indicates device present, 0 otherwise
		uint8_t _DEV_0B :1;												//	ditto
		uint8_t _DEV_0C :1;												//	ditto
		uint8_t _MJ828 :1;												//	ditto
		uint8_t _DEV_1A :1;												//	ditto
		uint8_t _DEV_1B :1;												//	ditto
		uint8_t _DEV_1C :1;												//	ditto
		uint8_t _DEV_1D :1;												//	ditto
		uint8_t _MJ808 :1;												//	ditto
		uint8_t _MJ818 :1;												//	ditto
		uint8_t _DEV_2C :1;												//	ditto
		uint8_t _DEV_2D :1;												//	ditto
		uint8_t _DEV_3A :1;												//	ditto
		uint8_t _DEV_3B :1;												//	ditto
		uint8_t _DEV_3C :1;												//	ditto
		uint8_t _DEV_3D :1;												//	ditto
	};
	uint16_t uint16_val;												// the bit field as one uint16_t
} u_devices;

typedef struct															// struct describing the CAN bus state
{
	uint8_t status;														// status info
	uint8_t broadcast_iteration_count : 4;								// device counter for discovery
	u_devices devices;													// indicator of devices discovered, 16 in total; B0 - 1st device (0A), B1 - 2nd device (0B), ..., B15 - 16th device (3D)
	uint8_t numerical_self_id ;											// ordered device number - A0 (0th device) until 3C (15th device)
	uint8_t sleep_iteration : 3;										// how many times did we wakeup, sleep and wakeup again
} canbus_t;

typedef struct															// struct describing a single generic LED
{
	uint8_t on : 1;														// 0 - off, 1 - on
	uint8_t blink_count : 3;											// 000 - no blink, 001 - blink 1x, 010 - blink 2x, 011 - blink 3x, 100 - blink forever
} led_t;

#if defined(MJ808_)	|| defined(MJ818_)									// leds_t struct for mj808/mj818
typedef struct															// struct describing LEDs on device MJ828
{
	led_t leds[2];														// array of led_t - one for each LED
	uint8_t led_count : 3;												// number of LEDs on device, max 8

	uint8_t flag_any_glow : 1;											// flag indicating if anything at all shall glow
} leds_t;
#endif

#if defined(MJ828_)														// leds_t struct for mj828
typedef struct															// struct describing LEDs on device MJ828
{
	led_t leds[8];														// array of led_t - one for each LED
	uint8_t led_count : 3;												// number of LEDs on device, max 8

	uint8_t flag_any_glow : 1;											// flag indicating if anything at all shall glow
} leds_t;
#endif


enum led_enum															// TODO - investigate if enum actually has move value over defines
{
	red,
	green,
	blue,
	yellow,
	battery_led1,
	battery_led2,
	battery_led3,
	battery_led4
} ;

volatile leds_t LED;													// forward declaration of LED instance

typedef struct															// struct describing a generic pushbutton
{
	uint8_t state : 2;													// something akin to a "counter" used for debouncing
	uint8_t *PIN;														// PIN register address of button pin
	uint8_t pin_number;													// pin number (0, 1...6) to which the button is connected
	uint8_t is_pressed :1;												// flag indicating if button is pressed right now
	uint8_t was_pressed :1;												// flag indicating if button was released after a stable state (used to remember previous state)
	uint8_t toggle :1;													// flag indicating the toggle state
	uint8_t hold_temp :1;												// flag indicating a button press for a duration of BUTTON_MIN_PRESS_TIME (up to BUTTON_MAX_PRESS_TIME) seconds, followed by button release
	uint8_t hold_error :1;												// flag indicating constant button press (by error, object leaning on pushbutton, etc.)
	uint8_t hold_counter;												// counter to count button press duration for hold_X states
	uint8_t is_at_default :1;											// 1 - default values, 0 otherwise
} button_t;

#if defined (MJ808_)													// mj808_t struct for mj808
typedef  struct															// struct describing devices on MJ808
{
	//volatile uint8_t timer_counter[2];								// timer counter array
	volatile button_t button[1];										// array of button_t - one buttons
	volatile uint8_t button_count : 2;									// max. 4 buttons
	volatile leds_t	*led;												// pointer to LED structure
	volatile can_t *can;												// pointer to the CAN structure
} mj808_t;

volatile mj808_t mj808;													// forward declaration of mj828_t struct for mj828
#endif

#if defined (MJ818_)													// mj808_t struct for mj818
typedef struct															// struct describing devices on MJ818
{
	//volatile uint8_t timer_counter[2];								// timer counter array
	volatile leds_t	*led;												// pointer to LED structure
	volatile can_t *can;												// pointer to the CAN structure
} mj818_t;

volatile mj818_t mj818;															// forward declaration of mj828_t struct for mj828
#endif

#if defined (MJ828_)													// mj828_t struct for mj828
typedef struct															// struct describing devices on MJ828
{
	//volatile uint8_t timer_counter[2];								// timer counter array
	volatile button_t button[2];										// array of button_t - two buttons
	volatile leds_t	*led;												// pointer to LED structure
	volatile can_t *can;												// pointer to the CAN structure
} mj828_t;

volatile mj828_t mj828;															// forward declaration of mj828_t struct for mj828
#endif

// command handling functions
void util_led(uint8_t in_val);											// interprets CMND_UTIL_LED command - utility LED (red, green, on, off, blink)
void dev_sensor(can_message_t *msg);									// interprets CMND_DEVICE-DEV_SENSOR command - TODO - sensor related stuff
void dev_pwr_src(can_message_t *msg);									// interprets CMND_DEVICE-DEV_PWR_SRC command - TODO - power source related stuff
void dev_logic_unit(can_message_t *msg);								// interprets CMND_DEVICE-DEV_LU command - TODO - logic unit related stuff
void dev_light(can_message_t *msg);										// interprets CMND_DEVICE-DEV_LIGHT command - positional light control
void msg_button(can_message_t *msg, uint8_t button);					// conveys button press event to the CAN bus
void button_debounce(volatile button_t *in_button);						// marks a button as pressed if it was pressed for the duration of 2X ISR iterations

#if defined(MJ828_)
void charlieplexing_handler(volatile leds_t *in_led);					// handles LEDs in charlieplexed configuration
#endif

// bus handling functions
void discovery_announce(volatile canbus_t *canbus_status, can_message_t *msg); //
void discovery_behave(volatile canbus_t *canbus_status); //

#endif /* MJ8x8_H_ */