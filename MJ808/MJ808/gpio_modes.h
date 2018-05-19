#ifndef GPIO_MODES_H_
#define GPIO_MODES_H_

gpio_conf(MCP2515_INT_pin, INPUT, LOW);
gpio_conf(MCP2561_standby_pin, OUTPUT, HIGH);						// low (on), high (off)

gpio_conf(SPI_MISO_pin, INPUT, LOW);
gpio_conf(SPI_MOSI_pin, OUTPUT, LOW);
gpio_conf(SPI_SCK_pin, OUTPUT, HIGH);
gpio_conf(SPI_SS_MCP2515_pin, OUTPUT, HIGH);					// //high (device inert), low (device selected)

gpio_conf(PWM_front_light_pin, OUTPUT, LOW);						// low (off), high (on)
gpio_conf(RED_LED_pin, OUTPUT, LOW);										// low (on), high (off)
gpio_conf(GREEN_LED_pin, OUTPUT, LOW);									// low (on), high (off)
gpio_conf(PUSHBUTTON_pin, INPUT, HIGH);								//


#endif /* GPIO_MODES_H_ */