#ifndef GPIO_MODES_H_
#define GPIO_MODES_H_

gpio_conf(MCP2515_INT_pin, INPUT, HIGH);				// INT1, active low
gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);		// low (on), high (off)

gpio_conf(ICSP_DO_MOSI, OUTPUT, LOW);						// data out - output pin
gpio_conf(ICSP_DI_MISO, INPUT, LOW);						// data in - input pin
gpio_conf(SPI_SCK_pin, OUTPUT, LOW);						// low for proper CPOL = 0 waveform
gpio_conf(SPI_SS_MCP2515_pin, OUTPUT, HIGH);		// //high (device inert), low (device selected)

gpio_conf(PWM_front_light_pin, OUTPUT, LOW);		// low (off), high (on)
gpio_conf(RED_LED_pin, OUTPUT, HIGH);						// low (on), high (off)
gpio_conf(GREEN_LED_pin, OUTPUT, HIGH);					// low (on), high (off)
gpio_conf(PUSHBUTTON_pin, INPUT, LOW);					// SPST-NO - high on press, low on release


#endif /* GPIO_MODES_H_ */