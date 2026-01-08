#ifndef CORE_INC_MJ838_MJ838_H_
#define CORE_INC_MJ838_MJ838_H_

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		/*  0x3F - if any of bits 0 though 5 are set - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus on
		 * additionally: if CANBUS_ACTIVE_MASK has bits not set, CANbus will be off
		 */
		// bit 0
		uint8_t DoHeartbeat :1;  // DoHB // HeartBeat is running
		uint8_t CANActive :1;  // CAN // CAN is actively being used *is used as a flag to avoid re-entering e.g. __can_go_into_active_mode()

		// 0x3C - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus off
		uint8_t ZeroCross :1;  // ZC // zero-cross signal present or not
		uint8_t Motion :1;  // M // motion detected by accelerometer
		uint8_t AutoCharge :1;	// AC // AutoCharge is operating
		uint8_t AutoDrive :1;  // AD // AutoDrive is active

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj838_activity_t;

// CSV header definition
typedef enum log_field_index_e
{
	  LOG_TIME = 0,  // descr=n,div=1,dec=0,sign=false;
	  LOG_ZC_FREQ = 1,	// descr=fZC [Hz],div=1000,dec=3,sign=false;
	  LOG_KPH = 2,	// descr=v [km/h],div=1000,dec=3,sign=false;
	  LOG_MPS = 3,	// descr=v [m/s],div=1000,dec=3,sign=false;
	  LOG_VOLTAGE_AL = 4,  // descr=U adj. load [mV],div=1,dec=0,sign=false;
	  LOG_CURRENT_AL = 5,  // descr=I adj. load [mA],div=1,dec=0,sign=true;
	  LOG_POWER_AL = 6,  // descr=P adj. load [mW],div=1,dec=0,sign=false;
	  LOG_TEMPERATURE = 7,	// descr=t [deg C],div=1,dec=0,sign=true;
	  LOG_HUMIDITY = 8,  // descr=rel. humidity [%],div=1,dec=0,sign=false;
	  LOG_ADJUSTABLE_LOAD_STATE = 9,	// descr=adj. load set [mV],div=1,dec=0,sign=false;
	  LOG_FIELD_COUNT
} log_field_index_t;
// CSV header definition

#include "main.h"
#if defined(MJ838_)	// if this particular device is active
#define WIPE_FRAM 0	// if activated, statistics will be zeroed out in EXTI0 ISR

#define USE_I2C 1	// use I2C
#define USE_SPI 1	// use SPI
#define USE_FERAM 1 // use FERAM
#define USE_SHT40 1 // use humidity sensor
#define USE_INA219 1 // use Power Monitor
#define USE_TLC59208 1 // use LED driver for SSRs
#define USE_ADXL367 0 // use ADXL367 accelerometer
#define USE_DAC121C081 1 // use USE_DAC121C081 ADC for adjustable 12R load
#define USE_LOGGER 1 // use logging functionality
#define USE_SDCARD 1 // use SD card

#define USE_EVENTHANDLER 1	// shall EventHandler code be included

#define USE_RATE_CALC 0	// if zero cross rate of change calculation is to be included

#define USE_APPLICATION_LOAD 0	// use application load switch (not the 12R adjustable load)
#define USE_ADJUSTABLE_LOAD 1	// use 12R adjustable load switch (not the application load switch)

#if USE_SPI && USE_SDCARD
#include "sdcard/sdcard.h"
#define LOG_FILE_NAME "log01.raw"  // log file name for logger
#endif

#define ZEROCROSS 2
#define	MOTION 3	// TODO - ADXL345 via I2C
#define AUTOCHARGE 4
#define AUTODRIVE 5

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER2_PERIOD 2499 // periodic frequency measurement of timer3 data - default 250ms
#define TIMER3_PERIOD	0xFFFFFFFF // input capture of zero-cross signal on rising edge
#define TIMER14_PERIOD 9999	// TODO - determine power measurement time base - 1s
#define TIMER16_PERIOD 9999	// odometer & co. - 1s
#define TIMER17_PERIOD 24	// event handling - 2.5ms
#define TIMER3_IC_FILTER 0xF	// with TIM_ICPSC_DIV8 and 0xF the pulse needs to be at least 35us wide

#define F_CPU 8000000	// 8MHz
#define SCALED_CPU_TICK (uint16_t)(F_CPU / (TIMER_PRESCALER + 1)) // re-compute scaled down CPU freq. due to prescaler

//#define WHEEL_CIRCUMFERENCE 1945	// red 26" training wheel circumference in millimeters
#define WHEEL_CIRCUMFERENCE 2350 // Schwalbe Marathon Mondial 29x2.25" in millimeters
//#define WHEEL_CIRCUMFERENCE 2342	// Schwalbe Jumbo Jim 26x4", in millimeters
//#define WHEEL_CIRCUMFERENCE 2095	// Marathon Mondial 26x2", in millimeters

// FeRAM memory addresses
#define ODOMETER_ADDR	0x0000, 4	// 4 bytes for odometer float
#define NEXT_FREE_ADDR 0x0004, 2	//

#define POLE_COUNT 13	// number of dynamo pole pairs
// 5 minutes #define SLEEPTIMEOUT_COUNTER 1200 // N * 0.25s = foo seconds - time to stay idle, then stop zero-cross
#define SLEEPTIMEOUT_COUNTER 20 // N * 0.25s = foo seconds - time to stay idle, then stop zero-cross

#include "mj8x8\mj8x8.h"
#include "button\button.h"
#include "fm24cl\fm24cl.h"	// FeRAM
#include "sht40\sht40.h" // SHT40 humidity sensor

#include "zerocross\zerocross.h"
#include "mj838\autodrive.h"
#include "mj838\autocharge.h"

// TODO - mj838 debug GPIO - remove once debugging is complete
#define SIGNAL_GENERATOR_INPUT 1	// ZeroCross signal input is signal generator output
#define GPIO_DEBUG_OUT 1

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_15	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c

// FIXME - ZC circuit on rev.1c has worse slow speed (low voltage?) response than rev.1a
// FIXME - for some reason, PB8 has to be tied to GND for the board to work

#define LED1_Pin GPIO_PIN_2	// dual colour LED pin 1
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_3	// dual colour LED pin 2
#define LED2_GPIO_Port GPIOA

#define LED_Reset_Pin GPIO_PIN_0	// SSR LED Driver reset: low - in reset/standby; high - active
#define LED_Reset_GPIO_Port GPIOA

#define ZeroCross_Pin GPIO_PIN_0	// ZeroCross signal in
#define ZeroCross_GPIO_Port GPIOB

// TODO - remove since load is controlled differently
#define AppLoadFet_Pin GPIO_PIN_1	// Application Load Switch
#define AppLoadFet_GPIO_Port GPIOA

#define I2C_SDA_Pin GPIO_PIN_0 // see i2c_ctor()
#define I2C_SCL_Pin GPIO_PIN_1// see i2c_ctor()
#define I2C_GPIO_Port GPIOF

#define SPI_MOSI_Pin GPIO_PIN_7	// SPI1 MOSI
#define SPI_MOSI_GPIO_Port GPIOA
#define SPI_MISO_Pin GPIO_PIN_6	// SPI1 MISO
#define SPI_MISO_GPIO_Port GPIOA
#define SPI_SCK_Pin GPIO_PIN_5	// SPI1 SCK
#define SPI_SCK_GPIO_Port GPIOA
#define SD_Card_CS_Pin GPIO_PIN_4	// SPI1 CS
#define SD_Card_Detect_Pin GPIO_PIN_1	// SD card detect - pulled to GND when card is present
#define SD_Card_GPIO_Port GPIOA
#define SPI_GPIO_Port GPIOA

#if GPIO_DEBUG_OUT
#define YellowTestPad_Pin GPIO_PIN_3// debug pin 0
#define YellowTestPad_GPIO_Port GPIOB
#define BlueTestPad_Pin GPIO_PIN_4 // debug pin 1
#define BlueTestPad_GPIO_Port GPIOB
#endif

#define PB3_Pin GPIO_PIN_3	// general GPIO
#define PB3_GPIO_Port GPIOB
#define PB4_Pin GPIO_PIN_4	// general GPIO
#define PB4_GPIO_Port GPIOB
#define PB5_Pin GPIO_PIN_5	// general GPIO
#define PB5_GPIO_Port GPIOB
#define PB6_Pin GPIO_PIN_6	// general GPIO
#define PB6_GPIO_Port GPIOB
#define PB7_Pin GPIO_PIN_7	// general GPIO
#define PB7_GPIO_Port GPIOB
#define PB8_Pin GPIO_PIN_8	// general GPIO
#define PB8_GPIO_Port GPIOB

// definitions of device/PCB layout-dependent hardware pins

typedef struct	// struct describing devices on MJ838
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj838_activity_t *activity;  // pointer to struct(union) indicating device activity status
	zerocross_t *ZeroCross;  // zero-cross object
	autodrive_t *AutoDrive;  // automatic drive handling feature
	autocharge_t *AutoCharge;  // automatic charger
	fm24cl_t *FeRAM;  // pointer to FeRAM object
	sht40_t *Humidity;  // pointer to humidity sensor object
#if USE_SPI && USE_SDCARD
	sdcard_t *SDCard;  // pointer to SD card object
#endif
	void (*StopTimer)(TIM_HandleTypeDef *timer);  // stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument

	uint32_t Time;  // global time counter, increments every TIM16 interrupt
} mj838_t;

void mj838_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj838_t *const Device;  // declare pointer to public struct part

extern void *log_description[LOG_FIELD_COUNT];

#endif // MJ838_

#endif /* CORE_INC_MJ838_MJ838_H_ */
