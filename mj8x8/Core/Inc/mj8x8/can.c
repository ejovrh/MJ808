#include "main.h"
#include "can.h"

#define TCAN334_STANDBY	GPIO_PIN_SET	// TCAN ds. p. 23
#define TCAN334_WAKE GPIO_PIN_RESET	// TCAN ds. p. 23

static GPIO_InitTypeDef GPIO_InitStruct =
	{0};
static CAN_HandleTypeDef _hcan =  // CAN object
	{0};
static CAN_TxHeaderTypeDef _TXHeader =  // CAN header object
	{0};
static CAN_RxHeaderTypeDef _RXHeader =  // CAN header object
	{0};
static CAN_FilterTypeDef _FilterCondfig =  // CAN filter configuration object
	{0};
static uint32_t _TXMailbox = 0;  // TX mailbox identifier
uint8_t TxData[8];

typedef struct	// can_t actual
{
	can_t public;  // public struct
	void (*init)(void);  // private - CAN init

	uint8_t __tec;	// Transmit Error Counter
	uint8_t __rec;	// Receive Error Counter
	uint8_t __in_sleep :1;	// is MCP2561 CAN transceiver in sleep or not
} __can_t;

extern __can_t __CAN;  // declare can_t actual

// fetches a CAN frame from the RX FIFO and loads it into a can_msg_t object
static void _tcan334_can_msg_receive(can_msg_t *const msg)
{
	// FIXME - implement via FIFO interrupts
	HAL_CAN_GetRxMessage(&_hcan, CAN_RX_FIFO0, &_RXHeader, msg->data);	// fetch msg from fifo0

	msg->sidh = _RXHeader.StdId;

	msg->dlc = _RXHeader.DLC;

	return;
}

// Add a message to the first free Tx mailbox and activate the corresponding transmission request
static void _tcan334_can_msg_send(can_msg_t *const msg)
{
	if(__CAN.__in_sleep)  // if sleeping...
		__CAN.public.BusActive(1);  // wake up

	_TXHeader.IDE = CAN_ID_STD;  // set the ID to standard
	_TXHeader.RTR = CAN_RTR_DATA;  //	set to DATA
	_TXHeader.StdId = msg->sidh;  // populate the standard Id
	_TXHeader.DLC = msg->dlc;  // set the length

	uint8_t mboxFreeCount = 0;  // start with the assumption that there are no free mailboxes

	do  // do at least once:
		{
			mboxFreeCount = HAL_CAN_GetTxMailboxesFreeLevel(&_hcan);	// get count of free mailboxes

			if(HAL_CAN_AddTxMessage(&_hcan, &_TXHeader, msg->data, &_TXMailbox) == HAL_OK)  // add the message to the mailbox and if all is ok...
				return;  // get out
		}
	while(mboxFreeCount == 0);  // loop until the free mailbox level is non-zero
}

// configure GPIO from CAN RX to EXTI
static inline void _RXtoEXTI(void)
{
	GPIO_InitStruct.Pin = CAN_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(CAN_RX_GPIO_Port, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);  // EXTI11 - CAN RX scenario when uC is in stop mode and needs to be woken up
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

// configure GPIO from EXTI to CAN RX
static inline void _EXTItoRX(void)
{
	GPIO_InitStruct.Pin = CAN_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
}

//	sets CAN infrastructure into standby mode
static inline void __can_go_into_standby_mode(void)
{
	if(__CAN.__in_sleep)	// if we already are asleep ...
		return;  // ... do nothing

	__CAN.__in_sleep = 1;  // mark as sleeping

//	HAL_CAN_AbortTxRequest(&_hcan, CAN_TX_MAILBOX0);
//	HAL_CAN_AbortTxRequest(&_hcan, CAN_TX_MAILBOX1);
//	HAL_CAN_AbortTxRequest(&_hcan, CAN_TX_MAILBOX2);
	HAL_CAN_RequestSleep(&_hcan);  // put internal CAN peripheral to sleep
	HAL_GPIO_WritePin(TCAN334_Standby_GPIO_Port, TCAN334_Standby_Pin, TCAN334_STANDBY);  // put transceiver to sleep

	_RXtoEXTI();	// configure GPIO from CAN RX pin to EXTI (so that CAN frame reception can wake up the uC from stop mode)
}

//	sets CAN infrastructure into normal operating mode
static inline void __can_go_into_active_mode(void)
{
	if(!__CAN.__in_sleep)  // if we already are awake ...
		return;  // ... do nothing

	__CAN.__in_sleep = 0;  // mark as awake

	_EXTItoRX();	// configure GPIO from EXTI to CAN RX

	HAL_CAN_WakeUp(&_hcan);  // wake up internal CAN peripheral
//	HAL_CAN_ResetError(&_hcan);
	HAL_GPIO_WritePin(TCAN334_Standby_GPIO_Port, TCAN334_Standby_Pin, TCAN334_WAKE);	// wake up CAN transceiver
}

// puts the whole CAN infrastructure to sleep; 1 - sleep, 0 - awake
static void _busactive(const uint8_t awake)
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
	.public.BusActive = &_busactive,  // set up function pointer for public methods
	.public.RequestToSend = &_tcan334_can_msg_send,  // ditto
	.public.FetchMessage = &_tcan334_can_msg_receive,  // ditto
	};

static inline void _ConfigFilters(void)
{
	_FilterCondfig.FilterBank = 0;	// block all on fifo1
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

	_FilterCondfig.FilterBank = 1;	// block all on fifo1
	_FilterCondfig.FilterMode = CAN_FILTERMODE_IDMASK;
	_FilterCondfig.FilterScale = CAN_FILTERSCALE_32BIT;
	_FilterCondfig.FilterIdHigh = 0x0000;
	_FilterCondfig.FilterIdLow = 0x0000;

	_FilterCondfig.FilterMaskIdHigh = 0xFFFF;
	_FilterCondfig.FilterMaskIdLow = 0xFFFF;
	_FilterCondfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	_FilterCondfig.FilterActivation = ENABLE;
	HAL_CAN_ConfigFilter(&_hcan, &_FilterCondfig);
}

// CAN init
inline static void _CANInit(void)
{
	HAL_GPIO_WritePin(TCAN334_Shutdown_GPIO_Port, TCAN334_Shutdown_Pin, GPIO_PIN_RESET);	// bring device out of shutdown
	HAL_GPIO_WritePin(TCAN334_Standby_GPIO_Port, TCAN334_Standby_Pin, GPIO_PIN_RESET);	// bring device out of sleep

	_hcan.Instance = CAN;  // see RM0091, 29.7.7 - pp. 840
	_hcan.Init.Prescaler = 10;  // TODO - elaborate
	_hcan.Init.Mode = CAN_MODE_NORMAL;  //
	_hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;  //
	_hcan.Init.TimeSeg1 = CAN_BS1_13TQ;  //
	_hcan.Init.TimeSeg2 = CAN_BS2_2TQ;  //
	_hcan.Init.TimeTriggeredMode = DISABLE;  //
	_hcan.Init.AutoBusOff = DISABLE;  //
	_hcan.Init.AutoWakeUp = ENABLE;  //
	_hcan.Init.AutoRetransmission = ENABLE;  //
	_hcan.Init.ReceiveFifoLocked = ENABLE;  //
	_hcan.Init.TransmitFifoPriority = ENABLE;  //
	__HAL_RCC_CAN1_CLK_ENABLE();
	HAL_CAN_Init(&_hcan);  // Initialize CAN Bus

	_ConfigFilters();  // set up filters

	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);

	HAL_CAN_ActivateNotification(&_hcan, (CAN_IT_ERROR | CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_WAKEUP));  // enable interrupts
	HAL_CAN_Start(&_hcan);	// start CAN
}

// object constructor
can_t* can_ctor(void)
{
	// at this point STM32's TCAN334 Stby and Shdn pins are initialised and pulled up
	// TCAN334 is in shutdown/standby mode
	_CANInit();  // initialize & configure STM32's CAN peripheral

	return &__CAN.public;  // return address of public part; calling code accesses it via pointer
}

//	CAN global interrupts
void CEC_CAN_IRQHandler(void)
{
	uint32_t interrupts = READ_REG(_hcan.Instance->IER);
	uint32_t rf0rflags = READ_REG(_hcan.Instance->RF0R);
	uint32_t msrflags = READ_REG(_hcan.Instance->MSR);

	if((interrupts & CAN_IT_RX_FIFO0_MSG_PENDING) != 0U)  // Receive FIFO 0 message pending interrupt management
		{
			if((rf0rflags & CAN_RF0R_FMP0) != 0U)  // Check if message is still pending */
				Device->mj8x8->PopulatedBusOperation(MsgHandler);  // let the particular device deal with the message
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

// CAN RX GPIO EXIT ISR
void EXTI4_15_IRQHandler(void)
{  // called on activity on CAN RX GPIO
	__CAN.public.BusActive(1);  // wake up CAN

	HAL_GPIO_EXTI_IRQHandler(CAN_RX_Pin);  // service the interrupt
}
