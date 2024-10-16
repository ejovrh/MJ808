#include "main.h"

#if defined(USE_AUTOMOTION)	// if this particular device is active

#include "automotion.h"
#include "adxl367/adxl367.h"	// accelerometer

typedef struct	// automotion_t actual
{
	automotion_t public;  // public struct

	adxl367_t *adxl345;  //
} __automotion_t;

static __automotion_t              __AutoMotion              __attribute__ ((section (".data")));  // preallocate __AutoMotion object in .data
uint8_t foo;
// AutoMotion functionality based on accelerometer input
static void _Do(void)
{
	;
}

static __automotion_t              __AutoMotion =  // instantiate automotion_t actual and set function pointers
	{  //
	.public.Do = &_Do,  // set function pointer
	};

automotion_t* automotion_ctor(void)  //
{
	__AutoMotion.adxl345 = adxl367_ctor(ADXL367_CS_GPIO_Port, ADXL367_CS_Pin);	//

	while(1)
		__AutoMotion.adxl345->Read(STATUS_2, &foo, 1);

	return &__AutoMotion.public;  // set pointer to AutoMotion public part
}

#endif
