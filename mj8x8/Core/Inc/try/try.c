#include "main.h"
#include "try/try.h"

// TODO - lift up button handling (the event handler) into the same translation unit,
// TODO - objectify into try_t
// TODO - rename adequately
// TODO - write comments
// TODO - see that broadcast messages dont end up in branchtable_event()

#ifdef MJ808_
mj808_t *mj808;
#endif
#ifdef MJ818_
mj818_t *mj818;
#endif
#ifdef MJ828_
mj828_t *mj828;
#endif

static inline void _DoNothing(void *foo)  // a function that does nothing
{
	return;
}

// center button hold on mj808 or mj828 - front & rear light on, intensity set by arg
uint16_t _Event00(can_msg_t *msg)
{
#ifdef MJ808_
	Device->led->Shine(msg->ARGUMENT);
#endif
#ifdef MJ818_

#endif
#ifdef MJ828_
	Device->led->Shine(Green);
#endif

	return 0;
}

// center button toggle on mj808 or mj828 - light up red util. led
void _Event01(can_msg_t *msg)
{
#ifdef MJ808_
	if(msg->ARGUMENT)  // on or off
	Device->led->led[Utility].Shine(CMND_UTIL_RED_LED_ON);
	else
	Device->led->led[Utility].Shine(CMND_UTIL_RED_LED_OFF);
#endif
#ifdef MJ828_
	Device->led->Shine(Yellow);
#endif

	return;
}

// mj828 high beam button momentary
void _Event02(can_msg_t *msg)
{
#ifdef MJ808_
	Device->led->led[Front].Shine(msg->ARGUMENT);
#endif

	return;
}

// mj828 brake light button momentary
void _Event03(can_msg_t *msg)
{
#ifdef MJ818_
	Device->led->led[Brake].Shine(msg->ARGUMENT);
#endif

	return;
}

//
void _Event04(can_msg_t *msg)
{
	return;
}

//void _Event05(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event06(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event07(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event08(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event09(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event10(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event11(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event12(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event13(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event14(can_msg_t *msg)
//{
//		return;
//}
//
//void _Event15(can_msg_t *msg)
//	{
//		return;
//	}

static uint32_t (*_BranchtableMessage[])(can_msg_t *msg) =  // branchtable
	{  // MSG_BUTTON_EVENT_00 to MSG_BUTTON_EVENT_15
		(void *)(can_msg_t *)&_Event00,// center button hold on mj808 or mj828 - front & rear light on, intensity set by arg
		(void *)(can_msg_t *)&_Event01,// center button toggle on mj808 or mj828 - light up red util. led
		(void *)(can_msg_t *)&_Event02,// mj828 high beam button momentary
		(void *)(can_msg_t *)&_Event03,// mj828 brake light button momentary
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
		(void *)(can_msg_t *)&_DoNothing,//
	};

static uint32_t (*fptr)(can_msg_t *msg);  // dynamically generated function pointer

// executes function pointer identified by message command
void branchtable_event(can_msg_t *msg)
{
	if((msg->COMMAND& CLASS_MSG_BUTTON_EVENT) != CLASS_MSG_BUTTON_EVENT)
	return;

	uint16_t n = (msg->COMMAND & 0x0F);  // get lower byte use it as a decimal index - 0 to 15

	fptr = _BranchtableMessage[n];// get appropriate function pointer from branchtable
	(fptr)(msg);// execute
}