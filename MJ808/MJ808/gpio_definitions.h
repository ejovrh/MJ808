#ifndef GPIO_DEFINITIONS_H_
#define GPIO_DEFINITIONS_H_

#define    GREEN_LED_pin						D,   0,   0	// green LED
#define    RED_LED_pin							D,   1,   1	// red LED
#define    MCP2515_INT_pin					D,   3,   3	// INT1
#define		 PUSHBUTTON_pin						D,	 4,		4 // tactile pushbutton

#define    MCP2561_standby_pin			B,   1,   1	// MCP2561 standby
#define    PWM_front_light_pin   		B,   3,   3	// PWM - front light

#define    SPI_SS_MCP2515_pin   		B,   4,   4	// SPI - SS
#define    SPI_MOSI_pin   					B,   5,   5	// SPI - MOSI
#define    SPI_MISO_pin   					B,   6,   6	// SPI - MISO
#define    SPI_SCK_pin   						B,   7,   7	// SPI - SCK

#endif /* GPIO_DEFINITIONS_H_ */