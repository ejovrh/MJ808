#ifndef GPIO_MODES_H_
#define GPIO_MODES_H_

#include "gpio.h"														// macros for pin definitions

gpio_conf(MCP2515_INT_pin, INPUT, HIGH);								// INT1, active low
gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);							// low (on), high (off)

gpio_conf(ICSP_DO_MOSI, OUTPUT, LOW);									// data out - output pin
gpio_conf(ICSP_DI_MISO, INPUT, LOW);									// data in - input pin
gpio_conf(SPI_SCK_pin, OUTPUT, LOW);									// low for proper CPOL = 0 waveform
gpio_conf(SPI_SS_MCP2515_pin, OUTPUT, HIGH);							// high (device inert), low (device selected)

gpio_conf(PUSHBUTTON1_pin, INPUT, LOW);									// SPST-NO - high on press, low on release
gpio_conf(PUSHBUTTON2_pin, INPUT, LOW);									// SPST-NO - high on press, low on release

gpio_conf(LED_CP1_pin, INPUT, LOW);										// Charlie-plexed pin1
gpio_conf(LED_CP2_pin, INPUT, LOW);										// Charlie-plexed pin2
gpio_conf(LED_CP3_pin, INPUT, LOW);										// Charlie-plexed pin3
gpio_conf(LED_CP4_pin, OUTPUT, LOW);									// Charlie-plexed pin4

#endif /* GPIO_MODES_H_ */