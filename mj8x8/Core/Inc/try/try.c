#include "main.h"
#include "try/try.h"

static activity_t _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15;  // device activity containers for all 16 devices

activity_t *_BusActivityArray[16] =  // array for addresses of activity containers
	{&_0, &_1, &_2, &_3, &_4, &_5, &_6, &_7, &_8, &_9, &_10, &_11, &_12, &_13, &_14, &_15};

typedef struct	// try_t actual
{
	try_t public;  // public struct

	uint32_t (*_MsgBtnEventfptr)(can_msg_t *msg);  // dynamically generated function pointer
	uint32_t (*_Eventfptr)(void);  // dynamically generated function pointer
} __try_t;

static __try_t __Try __attribute__ ((section (".data")));  // preallocate __Try object in .data

// a function that does nothing
static inline void _DoNothing(void *foo)  // a function that does nothing
{
	return;
}

// error event
static inline void _EventHandlerEventError(void)
{  // TODO - implement _EventHandlerEventError()
#ifdef MJ808_
	;
//	Device->led->Reset();
#endif
#ifdef MJ818_
	;
//	Device->led->Reset();
#endif
#ifdef MJ828_
	;
//	Device->led->Reset();
#endif
#ifdef MJ838_
	;
#endif
	;
}

// mj808 center button hold
// mj828 lever back - braking action
static inline void _EventHandlerEvent02(void)
{
#ifdef MJ808_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Try->BusActivity->mj828->AutoLight)
		return;

	if(Device->button->button[PushButton]->Hold)
		{
			_payload = 50;  // argument is intensity in percent
			Device->led->Shine(50);  // turn the device on/off
		}
	else
		{
			_payload = 0;  // argument is intensity in percent
			Device->led->Shine(0);  // turn the device on/off
		}

	MsgHandler->SendMessage(ALL, MSG_BUTTON_EVENT_00, &_payload, 2);  // send it
#endif
#ifdef MJ828_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->button->button[LeverBrake]->Momentary)
		{
			_payload = (REAR_BRAKELIGHT | ON);  // turn on (0xC8 is a special value)
			Device->led->led[Red].Shine(ON);
		}
	else
		{
			_payload = (REAR_BRAKELIGHT | OFF);  // turn off (0xc9 is a special value)
		}

	MsgHandler->SendMessage(mj818, MSG_BUTTON_EVENT_03, &_payload, 2);  // send it
#endif
#ifdef MJ838_
	;
#endif
	;
}

// mj808 button toggle
// mj828 lever front - high beam
static inline void _EventHandlerEvent03(void)
{
#ifdef MJ808_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->button->button[PushButton]->Toggle)
		{
			_payload = (RED | BLINK);
			Device->led->led[Red].Shine(BLINK);
		}
	else
		{
			_payload = (RED | OFF);
			Device->led->led[Red].Shine(OFF);
		}
	MsgHandler->SendMessage(mj828, MSG_BUTTON_EVENT_01, &_payload, 2);// send it
#endif
#ifdef MJ828_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->button->button[LeverFront]->Momentary)
		{
			_payload = (FRONT_HIGHBEAM | ON);  // turn on (0xC8 is a special value)
			Device->led->led[Blue].Shine(ON);
		}
	else
		{
			_payload = (FRONT_HIGHBEAM | OFF);  // turn off (0xc9 is a special value)
			Device->led->led[Blue].Shine(OFF);
		}

	MsgHandler->SendMessage(mj818, MSG_BUTTON_EVENT_02, &_payload, 2);  // send it
#endif
#ifdef MJ838_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->AutoCharge->FlagLoadisConnected)
		_payload = OFF;  // argument is remaining charge in percent
	else
		_payload = ON;  // argument is remaining charge in percent

	MsgHandler->SendMessage(mj828, MSG_BUTTON_EVENT_05, &_payload, 2);  // send it
#endif
	;
}

// mj828 center pushbutton momentary
static inline void _EventHandlerEvent04(void)
{
#ifdef MJ828_
	Device->autobatt->DisplayBatteryVoltage();  // light up BatteryX LEDs according to voltage read at Vbat
#endif
	;
}

// mj828 pushbutton hold
static inline void _EventHandlerEvent05(void)
{
#ifdef MJ828_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->button->button[PushButton]->Hold)	//
		{
			_payload = 10;	// argument is intensity in percent
			Device->led->led[Green].Shine(ON);
			Device->adc->Start();
		}
	else
		{
			_payload = OFF;  // argument is intensity in percent
			Device->led->led[Green].Shine(OFF);
		}

	MsgHandler->SendMessage(mj818, MSG_BUTTON_EVENT_00, &_payload, 2);  // send it
#endif
	;
}

// mj828 pushbutton toggle
static inline void _EventHandlerEvent06(void)
{
#ifdef MJ828_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->button->button[PushButton]->Toggle)
		{
			_payload = ON;  // turn on
			Device->mj8x8->UpdateActivity(AUTOLIGHT, ON);  // mark activity
			Device->led->led[Yellow].Shine(ON);
			Device->adc->Start();
		}
	else
		{
			_payload = OFF;  // turn off
			Device->mj8x8->UpdateActivity(AUTOLIGHT, OFF);	// mark inactivity
			Device->led->led[Yellow].Shine(OFF);
		}
	MsgHandler->SendMessage(mj818, MSG_BUTTON_EVENT_01, &_payload, 2);  // send it
#endif
	;
}

// AutoLight detects darkness/light
static inline void _EventHandlerEvent07(void)
{
#ifdef MJ828_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->autolight->FlagLightisOn)  //	AutoLight feature is on
		{
			_payload = 30;  // argument is intensity in percent
			Device->led->led[Green].Shine(ON);	// turn green indicator on
			Device->adc->Start();
		}
	else
		{
			_payload = OFF;  // turn off
			Device->led->led[Green].Shine(OFF);  //	turn green indicator off
		}

	MsgHandler->SendMessage(mj818, MSG_BUTTON_EVENT_00, &_payload, 2);  // send it
#endif
	;
}

//	AutoBattLight detects battery status
static inline void _EventHandlerEvent08(void)
{
#ifdef MJ808_
	;
#endif
#ifdef MJ828_
	uint8_t _payload;  // payload for a single byte message, in addition to the command byte

	if(Device->autobatt->FlagBatteryisCritical)
		_payload = 10;  // argument is remaining charge in percent
	else
		_payload = 100;  // argument is remaining charge in percent

	MsgHandler->SendMessage(mj818, MSG_BUTTON_EVENT_04, &_payload, 2);  // send it
#endif
	;
}

////
//static inline void _EventHandlerEvent09(void)
//{
//	;
//}
//
//
//static inline void _EventHandlerEvent10(void)
//{
//	;
//}
//
////
//static inline void _EventHandlerEvent11(void)
//{
//	;
//}
//
////
//static inline void _EventHandlerEvent12(void)
//{
//	;
//}
//
////
//static inline void _EventHandlerEvent13(void)
//{
//	;
//}
//
////
//static inline void _EventHandlerEvent14(void)
//{
//	;
//}
//
////
//static inline void _EventHandlerEvent15(void)
//{
//	;
//}
//
////
//static inline void _EventHandlerEvent16(void)
//{
//	;
//}

//
static uint32_t (*_BranchtableEventHandler[])(void) =  // branch table
	{  // MSG_BUTTON_EVENT_00 to MSG_BUTTON_EVENT_15
		(void *)&_DoNothing,//
		(void *)&_EventHandlerEventError,// error event
		(void *)&_EventHandlerEvent02,//
		(void *)&_EventHandlerEvent03,//
		(void *)&_EventHandlerEvent04,//
		(void *)&_EventHandlerEvent05,//
		(void *)&_EventHandlerEvent06,//
		(void *)&_EventHandlerEvent07,//
		(void *)&_EventHandlerEvent08,//
		(void *)&_DoNothing,//
		(void *)&_DoNothing,//
		(void *)&_DoNothing,//
		(void *)&_DoNothing,//
		(void *)&_DoNothing,//
		(void *)&_DoNothing,//
		(void *)&_DoNothing,//
	};

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
void _EventHandler(const uint8_t val)
{
#ifdef MJ808_
	Device->mj8x8->UpdateActivity(BUTTONPRESSED, (Device->button->button[PushButton]->Momentary) > 0);  // translate button press into true or false
#endif
#ifdef MJ828_
	Device->mj8x8->UpdateActivity(BUTTONPRESSED,  //	set device to state according to button press
	(  //
	Device->button->button[PushButton]->Momentary ||  // ORed byte values indicate _some_ button press is active
	Device->button->button[LeverFront]->Momentary ||  //
	Device->button->button[LeverBrake]->Momentary) > 0  // translate the above fact into true or false
	);
#endif

	__Try._Eventfptr = _BranchtableEventHandler[val - 1];  // get appropriate function pointer from branch table
	(__Try._Eventfptr)();  // execute
}

// mj808 center button hold
// mj828 center button hold
uint16_t _MsgBtnEvent00(can_msg_t *msg)
{
#ifdef MJ808_
	Device->led->led[Front].Shine(msg->ARGUMENT);
#endif
#ifdef MJ818_
	Device->led->Shine(msg->ARGUMENT);
#endif
#ifdef MJ828_
	Device->led->led[Green].Shine((msg->ARGUMENT>0) );
	Device->adc->Start();
#endif
#ifdef MJ838_
	;
#endif

	return 0;
}

// mj808 center button toggle
// mj828 center button toggle
static inline void _MsgBtnEvent01(can_msg_t *msg)
{
#ifdef MJ828_
	Device->led->led[Blue].Shine(msg->ARGUMENT);	// argument is OFF, ON, BLINK
	Device->led->Shine(msg->ARGUMENT);// argument is (LED | (OFF, ON, BLINK)) - e.g. (YELLOW | BLINK)
	Device->adc->Start();
#endif
#ifdef MJ838_
	;
#endif

	return;
}

// mj828 high beam button momentary
static inline void _MsgBtnEvent02(can_msg_t *msg)
{
#ifdef MJ808_
	Device->led->led[Front].Shine(msg->ARGUMENT);
#endif

	return;
}

// mj828 brake light button momentary
static inline void _MsgBtnEvent03(can_msg_t *msg)
{
#ifdef MJ818_
	Device->led->led[Brake].Shine(msg->ARGUMENT);
#endif

	return;
}

// AutoBat status
static inline void _MsgBtnEvent04(can_msg_t *msg)
{
#ifdef MJ808_
	uint8_t arg = msg->ARGUMENT;

	if (arg <= 10)	// 10% warning
		Device->led->led[Front].Shine(msg->ARGUMENT);

	if (arg > 10)// 10% warning
		Device->led->led[Front].Shine(75);
#endif
#ifdef MJ818_
	uint8_t arg = msg->ARGUMENT;

	if (arg <= 10)	// 10% warning
		Device->led->led[Rear].Shine(msg->ARGUMENT);

	if (arg > 10)// 10% warning
		Device->led->led[Rear].Shine(100);

#endif

	return;
}

// mj838 autocharger on/off
static inline void _MsgBtnEvent05(can_msg_t *msg)
{
#ifdef MJ828_
	Device->led->led[Red].Shine(msg->ARGUMENT);	// argument is OFF, ON, BLINK
	Device->adc->Start();
#endif

	return;
}
//
//static inline void _MsgBtnEvent06(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent07(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent08(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent09(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent10(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent11(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent12(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent13(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent14(can_msg_t *msg)
//{
//		return;
//}
//
//static inline void _MsgBtnEvent15(can_msg_t *msg)
//	{
//		return;
//	}

// branch table for CLASS_MSG_BUTTON_EVENT message digest
static uint32_t (*_BranchtableMsgBtnEvent[])(can_msg_t *msg) =  // branch table
	{  // MSG_BUTTON_EVENT_00 to MSG_BUTTON_EVENT_15
		(void *)(can_msg_t *)&_MsgBtnEvent00,// center button hold on mj808 or mj828 - front & rear light on, intensity set by arg
		(void *)(can_msg_t *)&_MsgBtnEvent01,// center button toggle on mj808 or mj828 - light up red utility led
		(void *)(can_msg_t *)&_MsgBtnEvent02,// mj828 high beam button momentary
		(void *)(can_msg_t *)&_MsgBtnEvent03,// mj828 brake light button momentary
		(void *)(can_msg_t *)&_MsgBtnEvent04,// mj838 AutoBat status
		(void *)(can_msg_t *)&_MsgBtnEvent05,// mj838 AutoCharge on/off
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

// executes function pointer identified by message command
void _PopulatedBusOperation(message_handler_t *const in_handler)
{
	can_msg_t *msg = in_handler->GetMessage();

//	if((msg->COMMAND& GET_DEVICE_STATUS) == GET_DEVICE_STATUS)	// if asked for device status of self
//		{
//			MsgHandler->SendMessage(SET_DEVICE_STATUS, Device->activity->byte, 2);	// reply broadcast with own device status
//			return;
//		}
//
//	if((msg->COMMAND& SET_DEVICE_STATUS) == SET_DEVICE_STATUS)	// set device status identified by SID
//		{
//			uint8_t *statusptr = (uint8_t *) __Try._status;
//			*(statusptr + ( (msg->sidh >> 2) & 0x0F) * sizeof(uint8_t) ) = msg->ARGUMENT;  // set the status of the particular device identified by its SID
//			return;
//		}

//	if((msg->COMMAND& CLASS_MSG_BUTTON_EVENT) != CLASS_MSG_BUTTON_EVENT)	// get out if it isn't the proper message
//	return;

	if((msg->COMMAND& CLASS_MSG_MEASURE_DATA) == CLASS_MSG_MEASURE_DATA)	// get out if it isn't the proper message
	return;

	if((msg->COMMAND& CLASS_MSG_BUTTON_EVENT) == CLASS_MSG_BUTTON_EVENT)	// get out if it isn't the proper message
		{
			uint16_t n = (msg->COMMAND& 0x0F);  // get lower byte use it as a decimal index - 0 to 15

			__Try._MsgBtnEventfptr = _BranchtableMsgBtnEvent[n];// get appropriate function pointer from branch table
			(__Try._MsgBtnEventfptr)(msg);// execute
		}
}

// defines device operation on empty bus
void _EmptyBusOperation(void)
{
#ifdef MJ808_
	;
#endif
#ifdef MJ818_
	if(Device->mj8x8->GetActivity(REARLIGHT) == 0)  // run once: check that e.g. rear light is off (which it is on a lonely power on)
		Device->led->Shine(10);  // operate on component part
#endif
#ifdef MJ828_
	;
#endif
#ifdef MJ838_
	;
#endif
}

static __try_t __Try =  // instantiate can_t actual and set function pointers
	{  //
	.public.BusActivity = (status_t*) &_BusActivityArray,  // bus-wide device status of all devices
	.public.PopulatedBusOperation = &_PopulatedBusOperation,  // tie in function pointer
	.public.EmptyBusOperation = &_EmptyBusOperation,  // ditto
	.public.EventHandler = &_EventHandler  // ditto
	};

void try_ctor(void)
{
#if defined futureMJ_0
	_BusActivityArray[0] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_1
	_BusActivityArray[1] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_2
	_BusActivityArray[2] = (activity_t*) Device->activity;
#endif

#if defined MJ828_
	_BusActivityArray[3] = (activity_t*) Device->activity;
#endif

#if defined MJ838_
	_BusActivityArray[4] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_5
	_BusActivityArray[5] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_6
	_BusActivityArray[6] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_7
	_BusActivityArray[8] = (activity_t*) Device->activity;
#endif

#if defined MJ808_
	_BusActivityArray[8] = (activity_t*) Device->activity;
#endif

#if defined MJ818_
	_BusActivityArray[9] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_10
	_BusActivityArray[10] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_11
	_BusActivityArray[11] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_12
	_BusActivityArray[12] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_13
	_BusActivityArray[13] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_14
	_BusActivityArray[14] = (activity_t*) Device->activity;
#endif

#if defined futureMJ_15
	_BusActivityArray[15] = (activity_t*) Device->activity;
#endif

	Try->BusActivity->_0 = _BusActivityArray[0];
	Try->BusActivity->_1 = _BusActivityArray[1];
	Try->BusActivity->_2 = _BusActivityArray[2];
	Try->BusActivity->mj828 = (mj828_activity_t*) _BusActivityArray[3];
	Try->BusActivity->mj838 = (mj838_activity_t*) _BusActivityArray[4];
	Try->BusActivity->_5 = _BusActivityArray[5];
	Try->BusActivity->_6 = _BusActivityArray[6];
	Try->BusActivity->_7 = _BusActivityArray[7];
	Try->BusActivity->mj808 = (mj808_activity_t*) _BusActivityArray[8];
	Try->BusActivity->mj818 = (mj818_activity_t*) _BusActivityArray[9];
	Try->BusActivity->_10 = _BusActivityArray[10];
	Try->BusActivity->_11 = _BusActivityArray[11];
	Try->BusActivity->_12 = _BusActivityArray[12];
	Try->BusActivity->_13 = _BusActivityArray[13];
	Try->BusActivity->_14 = _BusActivityArray[14];
	Try->BusActivity->_15 = _BusActivityArray[15];
}

try_t *const Try = &__Try.public;  // set pointer to Try public part
