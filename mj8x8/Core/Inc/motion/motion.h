#ifndef CORE_INC_MOTION_MOTION_H_
#define CORE_INC_MOTION_MOTION_H_

typedef struct motion_t  // struct describing the motion detector functionality
{
	void (*Start)(void);	// starts the motion detection peripheral
	void (*Stop)(void);  // stops the motion detection peripheral

} motion_t;

#endif /* CORE_INC_MOTION_MOTION_H_ */
