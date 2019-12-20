#ifndef TPS630701_H_
#define TPS630701_H_

#include <inttypes.h>

/* operation:
 *	TPS630701 is enabled by default, accepts PWM
 *	PWM directly accesses OCR SFR and sets the duty cycle, feeds PS/SYNC pin on Buck-Boost controller
 *		hardware pin: TPS630702_PWM_pin, pin B2, floating on power on
 *
 *	PWM states:
 *		0x00 - (LOW) forced PWM mode,
 *		0xFF - (HIGH) PWM/PFM (power save) mode
 *		PWM signal - determines Buck-Boost controller switching frequency
 *
 *	TPS630701's 5V0 output voltage and current can be read out via GetValues()
 *		and are obtained via SPI from lmp92064sd_t
 */

typedef struct															// struct describing the Buck-Boost controller object
{
	volatile uint8_t *PWM;												// direct access to OCR SFR
	void (* GetValues)(volatile uint8_t *data_array);					// download current and voltage measurement into external container
} tps630701_t __attribute__((aligned(8)));

tps630701_t *tps630701_ctor(void);										// initializes tps630701_t object

#endif /* TPS630701_H_ */