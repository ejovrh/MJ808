#ifndef CORE_INC_MJ838_AUTODRIVE_H_
#define CORE_INC_MJ838_AUTODRIVE_H_

#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#define USE_DYNAMIC_LIGHT 1	// use dynamic light adjustment

#define ODOMETER_REFRESH_PERIOD 60 // odometer refresh period in seconds

typedef struct	// struct describing the AutoDrive functionality
{
	union mps  // meters per second
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} mps;
	union kph  // kilometres per hour
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} kph;
	union m  // distance in meters
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} m;
	union Odometer  // distance in meters
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} Odometer;

	void (*Do)(void);  // AutoDrive functionality
	void (*AutoDriveOff)(void);  // turns AutoDrive off
	void (*AutoDriveOn)(void);  // turns AutoDrive on
	void (*UpdateOdometer)(void);  // updates odometer value in FeRAM
} autodrive_t;

autodrive_t* autodrive_ctor(void);	// the AutoDrive constructor

#endif

#endif /* CORE_INC_MJ838_AUTODRIVE_H_ */
