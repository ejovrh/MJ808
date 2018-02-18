#ifndef GPIO_DEFINITIONS_H_
#define GPIO_DEFINITIONS_H_

#define    MCP2515_INT_pin					D,   3,   3	// INT1
#define		 LDO_EN_pin								D,	 4,		4	// 5V LDO enable pin
#define    MCP2561_standby_pin			D,   5,   5	// MCP2561 standby

#define    RED_LED_pin							B,   0,   0	// red LED
#define		 PUSHBUTTON_pin						B,	 1,		1 // tactile pushbutton
#define    GREEN_LED_pin						B,   2,   2	// green LED
#define    PWM_front_light_pin   		B,   3,   3	// PWM - front light

#define    SPI_SS_MCP2515_pin   		B,   4,   4	// SPI - SS
#define    SPI_MOSI_pin   					B,   5,   5	// SPI - MOSI
#define    SPI_MISO_pin   					B,   6,   6	// SPI - MISO
#define    SPI_SCK_pin   						B,   7,   7	// SPI - SCK

#endif /* GPIO_DEFINITIONS_H_ */