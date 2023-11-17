#include "main.h"
#include "can.h"

#define TCAN334_STANDBY	GPIO_PIN_SET	// TCAN ds. p. 23
#define TCAN334_WAKE GPIO_PIN_RESET	// TCAN ds. p. 23
#define CAN_TIMEOUT_VALUE 2000U

static GPIO_InitTypeDef GPIO_InitStruct =
	{0};
static CAN_HandleTypeDef _hcan =  // CAN object
	{0};
static CAN_TxHeaderTypeDef _TXHeader =  // CAN header object
	{0};
static CAN_FilterTypeDef _FilterCondfig =  // CAN filter configuration object
	{0};

static activity_t _activity;	// actual activity object (is used only through references)

typedef struct	// can_t actual
{
	can_t public;  // public struct

	void (*init)(void);  // private - CAN init
} __can_t;

extern __can_t __CAN;  // declare can_t actual

// fetches a CAN frame from a RX FIFO and loads it into the message handler object
static void _tcan334_can_msg_receive(message_handler_t *in_handler, const uint8_t in_fifo)
{
	can_msg_t _msg;  // static or not ?
	CAN_RxHeaderTypeDef _nRXH;  // static or not ?

	HAL_CAN_GetRxMessage(&_hcan, in_fifo, &_nRXH, _msg.data);  // fetch message from FIFO

	_msg.sidh = _nRXH.StdId;
	_msg.dlc = _nRXH.DLC;

	in_handler->SetMessage(&_msg);  // upload into the message handler

#if USE_CAN_BUSOFF
	__CAN.public.GoBusActive(0);
#endif
}

// Add a message to the first free Tx mailbox and activate the corresponding transmission request
static void _tcan334_can_msg_send(can_msg_t *const msg)
{
	volatile uint16_t i = 0;  // safeguard counter
	uint32_t _TXMailbox;  // TX mailbox identifier

#if USE_CAN_BUSOFF
	if((__CAN.public.activity->byte & POWERSAVE_CANBUS_ACTIVE_MASK) == 0)  // if sleeping...
		__CAN.public.GoBusActive(1);  // wake up
#endif

	_TXHeader.IDE = CAN_ID_STD;  // set the ID to standard
	_TXHeader.RTR = CAN_RTR_DATA;  //	set to DATA
	_TXHeader.StdId = msg->sidh;  // populate the standard Id
	_TXHeader.DLC = msg->dlc;  // set the length

	uint8_t mboxFreeCount = 0;  // start with the assumption that there are no free mailboxes

	do  // do at least once:
		{
			mboxFreeCount = HAL_CAN_GetTxMailboxesFreeLevel(&_hcan);	// get count of free mailboxes

			if(i > CAN_TIMEOUT_VALUE)  // safeguard
				{
					asm("NOP");
					// FIXME - CAN msg send - no free mailbox timeout
					// TODO - CAN msg send - no free mailbox - implement error handling
					break;
				}

			++i;
		}
	while(mboxFreeCount == 0);  // loop until the free mailbox level is non-zero

	HAL_CAN_AddTxMessage(&_hcan, &_TXHeader, msg->data, &_TXMailbox);  // add the message to the mailbox

	i = 0;	// reinitialise safeguard counter
	while(HAL_CAN_IsTxMessagePending(&_hcan, _TXMailbox))
		{
			if(i > CAN_TIMEOUT_VALUE)
				{
					// TODO - CAN msg send - msg tx pending - implement error handling
					// FIXME - CAN msg send - msg tx pending timeout
					HAL_CAN_AbortTxRequest(&_hcan, _TXMailbox);
				}

			++i;
		}

#if USE_CAN_BUSOFF
	__CAN.public.GoBusActive(0);
#endif
}

// configure GPIO from CAN RX to EXTI
static inline void _RXtoEXTI(void)
{
	GPIO_InitStruct.Pin = CAN_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(CAN_RX_GPIO_Port, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);  // EXTI11 - CAN RX scenario when uC is in stop mode and needs to be woken up
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

// configure GPIO from EXTI to CAN RX
static inline void _EXTItoRX(void)
{
	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);

	GPIO_InitStruct.Pin = CAN_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

//	sets CAN infrastructure into standby mode
static inline void __can_go_into_standby_mode(void)
{
	uint8_t i = 0;	// safeguard counter

	if(__CAN.public.activity->CANActive == 0)  // if we already are asleep ...
		return;  // ... do nothing

//	__disable_irq();	// uninterrupted...
	HAL_GPIO_WritePin(TCAN334_Standby_GPIO_Port, TCAN334_Standby_Pin, TCAN334_STANDBY);  // put transceiver to sleep

	do
		{
			HAL_CAN_RequestSleep(&_hcan);  // try to put CAN peripheral to sleep

			if(i > CAN_TIMEOUT_VALUE)  // safeguard
				{
					asm("NOP");
					// FIXME - CAN sleep fail due to timeout
					// TODO - CAN sleep fails - implement error handling
					break;
				}

			++i;
		}
	while(HAL_CAN_IsSleepActive(&_hcan) == 0);  // check if CAN peripheral is still active

	_RXtoEXTI();	// configure GPIO from CAN RX pin to EXTI (so that CAN frame reception can wake up the uC from stop mode)

	__CAN.public.activity->CANActive = 0;  // mark as sleeping

//	__enable_irq();
}

//	sets CAN infrastructure into normal operating mode
static inline void __can_go_into_active_mode(void)
{
	volatile uint8_t i = 0;  // safeguard counter

	if(__CAN.public.activity->CANActive == 1)  // if we already are awake ...
		return;  // ... do nothing

//	__disable_irq();	// uninterrupted...

	_EXTItoRX();	// configure GPIO from EXTI to CAN RX

	do
		{
			HAL_CAN_WakeUp(&_hcan);  // try to wake up internal CAN peripheral

			if(i > CAN_TIMEOUT_VALUE)  // safeguard
				{
					asm("NOP");
					// TODO - CAN sleep fails - implement error handling
					break;
				}

			++i;
		}
	while(HAL_CAN_IsSleepActive(&_hcan));  // check if CAN peripheral is still asleep

	HAL_GPIO_WritePin(TCAN334_Standby_GPIO_Port, TCAN334_Standby_Pin, TCAN334_WAKE);	// wake up CAN transceiver

	__CAN.public.activity->CANActive = 1;  // mark as awake

//	__enable_irq();
}

// puts the whole CAN infrastructure to sleep; 1 - sleep, 0 - awake
static void _GoBusActive(const uint8_t awake)
{
	/* TCAN334 operating modes (DS. p.22)
	 * 		1. normal mode
	 * 		2. standby mode with wake
	 * 		(3. shutdown mode) - not used on mj8x8 devices
	 */

	if(awake)
		__can_go_into_active_mode();
	else
		__can_go_into_standby_mode();
}

__can_t __CAN =  // instantiate can_t actual and set function pointers
	{  //
	.public.activity = &_activity,  // tie in private activity union into can_t object (is tied in again one level up)
	.public.GoBusActive = &_GoBusActive,  // set up function pointer for public methods
	.public.RequestToSend = &_tcan334_can_msg_send,  // ditto
	};

static inline void _ConfigFilters(void)
{
	// TODO - CAN filters - configure filters
	_FilterCondfig.FilterBank = 0;	// allow all for FIFOo
	_FilterCondfig.FilterMode = CAN_FILTERMODE_IDMASK;
	_FilterCondfig.FilterScale = CAN_FILTERSCALE_32BIT;
	_FilterCondfig.FilterIdHigh = 0x0000;
	_FilterCondfig.FilterIdLow = 0x0000;
	_FilterCondfig.FilterMaskIdHigh = 0x0000;
	_FilterCondfig.FilterMaskIdLow = 0x0000;
//	_FilterCondfig.FilterMaskIdHigh = 0xFFFF;
//	_FilterCondfig.FilterMaskIdLow = 0xFFFF;
	_FilterCondfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	_FilterCondfig.FilterActivation = ENABLE;
	HAL_CAN_ConfigFilter(&_hcan, &_FilterCondfig);

	_FilterCondfig.FilterBank = 1;	// allow all for FIFO1
	_FilterCondfig.FilterMode = CAN_FILTERMODE_IDMASK;
	_FilterCondfig.FilterScale = CAN_FILTERSCALE_32BIT;
	_FilterCondfig.FilterIdHigh = 0x0000;
	_FilterCondfig.FilterIdLow = 0x0000;

//	_FilterCondfig.FilterMaskIdHigh = 0xFFFF;
//	_FilterCondfig.FilterMaskIdLow = 0xFFFF;
	_FilterCondfig.FilterFIFOAssignment = CAN_RX_FIFO1;
	_FilterCondfig.FilterActivation = ENABLE;
	HAL_CAN_ConfigFilter(&_hcan, &_FilterCondfig);
}

// CAN init
inline static void _CANInit(void)
{
	HAL_GPIO_WritePin(TCAN334_Standby_GPIO_Port, TCAN334_Standby_Pin, TCAN334_WAKE);  // bring device out of standby

	_hcan.Instance = CAN;  // see RM0091, 29.7.7 - pp. 840

	/* bit timing
	 * 	TCAN334 has a wakeup filter time of max. 4us (DS. p. 8).
	 * 	this means a maximum speed of 250kbit/s can be used so that a WUP pattern can be generated safely by any CAN frame.
	 *
	 *	fCPU is 8MHz, with a pre-scaler value of 10 we are getting 800kHz, thus a 0.125 us period.
	 *	nominal bit-time is 20us, thus 50kbit/s is the baud rate.
	 *	nominal bit time is 1us, sample point is at 87.5%
	 */

	_hcan.Init.Prescaler = 10;  //
	_hcan.Init.Mode = CAN_MODE_NORMAL;  //
	_hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;  // @suppress("Field cannot be resolved")
	_hcan.Init.TimeSeg1 = CAN_BS1_13TQ;  // @suppress("Field cannot be resolved")
	_hcan.Init.TimeSeg2 = CAN_BS2_2TQ;  // @suppress("Field cannot be resolved")
	_hcan.Init.TimeTriggeredMode = DISABLE;  // @suppress("Field cannot be resolved")
	_hcan.Init.AutoBusOff = DISABLE;  // @suppress("Field cannot be resolved")
	_hcan.Init.AutoWakeUp = DISABLE;  // @suppress("Field cannot be resolved")
	_hcan.Init.AutoRetransmission = ENABLE;  // @suppress("Field cannot be resolved")
	_hcan.Init.ReceiveFifoLocked = ENABLE;  // @suppress("Field cannot be resolved")
	_hcan.Init.TransmitFifoPriority = ENABLE;  // @suppress("Field cannot be resolved")
	__HAL_RCC_CAN1_CLK_ENABLE();
	HAL_CAN_Init(&_hcan);  // Initialize CAN Bus

	_ConfigFilters();  // set up filters

	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);

	HAL_CAN_ActivateNotification(&_hcan, (CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO0_FULL | CAN_IT_RX_FIFO1_FULL));  // enable interrupts
	HAL_CAN_Start(&_hcan);	// start CAN

}

// object constructor
can_t* can_ctor(void)
{
// at this point STM32's TCAN334 Stby and Shdn pins are initialised and pulled up
// TCAN334 is in shutdown/standby mode
	_CANInit();  // initialize & configure STM32's CAN peripheral

	__CAN.public.GoBusActive(1);	// start with CAN active by default
	return &__CAN.public;  // return address of public part; calling code accesses it via pointer
}

//	CAN global interrupts
void CEC_CAN_IRQHandler(void)
{
	uint32_t interrupts = READ_REG(_hcan.Instance->IER);
	uint32_t msrflags = READ_REG(_hcan.Instance->MSR);

	// CAN RX message pending in FIFO0
	if((interrupts & CAN_IT_RX_FIFO0_MSG_PENDING) != 0U)  // Receive FIFO 0 message pending interrupt
		{
			if((_hcan.Instance->RF0R & CAN_RF0R_FMP0) != 0U)  // Check if message is still pending
				{
					_tcan334_can_msg_receive(MsgHandler, CAN_RX_FIFO0);
					Device->mj8x8->PopulatedBusOperation(MsgHandler);  // let the particular device deal with the message
				}
		}

	// CAN RX message pending in FIFO1
	if((interrupts & CAN_IT_RX_FIFO1_MSG_PENDING) != 0U)  // Receive FIFO 1 message pending interrupt
		{
			if((_hcan.Instance->RF1R & CAN_RF1R_FMP1) != 0U)  // Check if message is still pending
				{
					_tcan334_can_msg_receive(MsgHandler, CAN_RX_FIFO1);  // load CAN message from FIFO into message_t object
					Device->mj8x8->PopulatedBusOperation(MsgHandler);  // let the particular device deal with the message in the message_t object
				}
		}

	// CAN RX FIFO0 full
	if((interrupts & CAN_IT_RX_FIFO0_FULL) != 0U)
		{
			if((_hcan.Instance->RF1R & CAN_RF0R_FULL0) != 0U)
				{
					__HAL_CAN_CLEAR_FLAG(&_hcan, CAN_FLAG_FF0);  // Clear FIFO 0 full Flag
				}
		}

	// CAN RX FIFO1 full
	if((interrupts & CAN_IT_RX_FIFO1_FULL) != 0U)
		{
			if((_hcan.Instance->RF1R & CAN_RF1R_FULL1) != 0U)
				{
					__HAL_CAN_CLEAR_FLAG(&_hcan, CAN_FLAG_FF1);  // Clear FIFO 0 full Flag
				}
		}

	if((interrupts & CAN_IT_SLEEP_ACK) != 0U)  // sleep acknowledge interrupt
		{
			if((msrflags & CAN_MSR_SLAKI) != 0U)
				{
					asm("NOP");
				}
			else
				{
					asm("NOP");
				}
		}

	if((interrupts & CAN_IT_WAKEUP) != 0U)	// wake interrupt
		{
			if((msrflags & CAN_MSR_WKUI) != 0U)
				{
					asm("NOP");
				}
			else
				{
					asm("NOP");
				}
		}

	HAL_CAN_IRQHandler(&_hcan);  // service the interrupt
}

// CAN RX GPIO EXTI ISR
void EXTI4_15_IRQHandler(void)
{
	// interrupt source detection
	if(__HAL_GPIO_EXTI_GET_IT(CAN_RX_Pin))
		{
			// called on falling edge on CAN RX GPIO (when configured as an EXTI GPIO instead of CAN RX)
			__CAN.public.GoBusActive(1);  // ...wake up CAN
			__CAN.public.Timer1Start();  // start timer1
			HAL_GPIO_EXTI_IRQHandler(CAN_RX_Pin);  // service the interrupt
		}
}
