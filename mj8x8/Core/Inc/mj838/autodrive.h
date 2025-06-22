#ifndef CORE_INC_MJ838_AUTODRIVE_H_
#define CORE_INC_MJ838_AUTODRIVE_H_

#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#define USE_DYNAMIC_LIGHT 1	// use dynamic light adjustment

#define ODOMETER_REFRESH_PERIOD 60 // odometer refresh period in seconds

#define FIXED_POINT_SCALE 1000 // Scaling factor for fixed-point math

typedef struct	// struct describing the AutoDrive functionality
{
	union mps  // meters per second
	{
		uint32_t uint32;  // e.g. 1.234 m/s is 1234
		uint8_t Bytes[sizeof(uint32_t)];
	} mps;
	union kph  // kilometres per hour
	{
		uint32_t uint32;  // e.g. 1.234 km/s is 1234
		uint8_t Bytes[sizeof(uint32_t)];
	} kph;
	union m  // distance in meters
	{
		uint32_t uint32;  // e.g. 54120 is 54.120 m
		uint8_t Bytes[sizeof(uint32_t)];
	} m;
	union Odometer  // distance in meters
	{
		uint32_t uint32;  // Fixed-point representation (scaled by FIXED_POINT_SCALE)
		uint8_t Bytes[sizeof(uint32_t)];
	} Odometer;

	void (*Do)(void);  // AutoDrive functionality
	void (*AutoDriveOff)(void);  // turns AutoDrive off
	void (*AutoDriveOn)(void);  // turns AutoDrive on
	void (*UpdateOdometer)(void);  // updates odometer value in FeRAM
} autodrive_t;

autodrive_t* autodrive_ctor(void);	// the AutoDrive constructor

#endif

#endif /* CORE_INC_MJ838_AUTODRIVE_H_ */
