#include "main.h"
#include "can.h"

static CAN_HandleTypeDef _hcan =  // CAN object
	{0};
static CAN_TxHeaderTypeDef _TXHeader =  // CAN header object
	{0};
static CAN_RxHeaderTypeDef _RXHeader =  // CAN header object
	{0};
static CAN_FilterTypeDef _FilterCondfig =  // CAN filter configuration object
	{0};
static uint32_t _TXMailbox = 0;  // TX mailbox identifier

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

	msg->sidh = (_RXHeader.StdId >> 8);
	msg->sidl = (uint8_t) _RXHeader.StdId;
	msg->dlc = _RXHeader.DLC;

	return;
}

// Add a message to the first free Tx mailbox and activate the corresponding transmission request
static void _tcan334_can_msg_send(can_msg_t *const msg)
{
	if(__CAN.__in_sleep)  // if sleeping...
		__CAN.public.Sleep(0);  // wake up

	_TXHeader.IDE = CAN_ID_STD;  // set the ID to standard
	_TXHeader.RTR = CAN_RTR_DATA;  //	set to DATA
	_TXHeader.StdId = ((msg->sidh << 8) | msg->sidl);  // populate the standard Id
	_TXHeader.DLC = msg->dlc;  // set the length

	uint8_t mboxFreeCount = 0;  // start with the assumption that there are no free mailboxes

	do  // do at least once:
		{
			mboxFreeCount = HAL_CAN_GetTxMailboxesFreeLevel(&_hcan);	// get count of free mailboxes

			if(mboxFreeCount)  // if there are any free mailboxes
				if(HAL_CAN_AddTxMessage(&_hcan, &_TXHeader, msg->data, &_TXMailbox) == HAL_OK)  // add the message to the mailbox and if all is ok...
					break;  // ...break the loop
		}
	while(mboxFreeCount == 0);  // loop until the free mailbox level is non-zero
}

// puts the whole CAN infrastructure to sleep; 1 - sleep, 0 - awake
static void _can_sleep(const uint8_t in_val)
{
	/* operating modes
	 * DS. p.22
	 * 1. normal mode
	 * 2. standby mode with wake
	 * 3. shutdown mode
	 */

	if(!(__CAN.__in_sleep) && in_val)  // if CAN is awake and is set to sleep
		{
			__CAN.__in_sleep = 1;  // mark as sleeping
		}

	if(__CAN.__in_sleep && !in_val)  // if CAN is sleeping and set to wake up
		{
			__CAN.__in_sleep = 0;  // mark as awake
		}
}

__can_t __CAN =  // instantiate can_t actual and set function pointers
	{  //
	.public.Sleep = &_can_sleep,  // set up function pointer for public methods
	.public.RequestToSend = &_tcan334_can_msg_send,  // ditto
	.public.FetchMessage = &_tcan334_can_msg_receive,  // ditto
	};

// CAN init
inline static void _CANInit(void)
{
	_hcan.Instance = CAN;  // see RM0091, 29.7.7 - pp 840
	_hcan.Init.Prescaler = 8;  // 1TQ is 1us, time for one bit is 20us
	_hcan.Init.Mode = CAN_MODE_NORMAL;  //
	_hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;  //
	_hcan.Init.TimeSeg1 = CAN_BS1_16TQ;  // this gives 50kbit/s
	_hcan.Init.TimeSeg2 = CAN_BS2_3TQ;  //
	_hcan.Init.TimeTriggeredMode = DISABLE;  //
	_hcan.Init.AutoBusOff = DISABLE;  //
	_hcan.Init.AutoWakeUp = ENABLE;  //
	_hcan.Init.AutoRetransmission = DISABLE;  //
	_hcan.Init.ReceiveFifoLocked = DISABLE;  //
	_hcan.Init.TransmitFifoPriority = ENABLE;  //
	HAL_CAN_Init(&_hcan);

	_FilterCondfig.FilterActivation = CAN_FILTER_DISABLE;  // disable filter on fifo0
	_FilterCondfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	HAL_CAN_ConfigFilter(&_hcan, &_FilterCondfig);

	_FilterCondfig.FilterBank = 0;	// block all on fifo1
	_FilterCondfig.FilterMode = CAN_FILTERMODE_IDMASK;
	_FilterCondfig.FilterScale = CAN_FILTERSCALE_32BIT;
	_FilterCondfig.FilterIdHigh = 0x0000;
	_FilterCondfig.FilterIdLow = 0x0000;
	_FilterCondfig.FilterMaskIdHigh = 0xFFFF;
	_FilterCondfig.FilterMaskIdLow = 0xFFFF;
	_FilterCondfig.FilterFIFOAssignment = CAN_RX_FIFO1;
	_FilterCondfig.FilterActivation = ENABLE;
	_FilterCondfig.SlaveStartFilterBank = 14;
	HAL_CAN_ConfigFilter(&_hcan, &_FilterCondfig);

	HAL_CAN_RequestSleep(&_hcan);  //Initialize CAN Bus
	HAL_CAN_ActivateNotification(&_hcan, (CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_ERROR));  // enable interrupts
}

// object constructor
can_t* can_ctor(void)
{
	// at this point STM32's TCAN334 Stby and Shdn pins are in analog mode,
	// TCAN334's own Stby and Shdn have internal pulldown so right now the transceiver is on and bus-passive
	_CANInit();  // initialize & configure STM32's CAN peripheral

	return &__CAN.public;  // return address of public part; calling code accesses it via pointer
}

//	CAN global interrupts
void CEC_CAN_IRQHandler(void)
{
	uint32_t interrupts = READ_REG(_hcan.Instance->IER);

	if((interrupts & CAN_IT_RX_FIFO0_MSG_PENDING) != 0U)  // Receive FIFO 0 message pending interrupt management
		{
			if((_hcan.Instance->RF0R & CAN_RF0R_FMP0) != 0U)  // Check if message is still pending */
				Device->mj8x8->PopulatedBusOperation(MsgHandler);  // let the particular device deal with the message
		}

	HAL_CAN_IRQHandler(&_hcan);  // service the interrupt
}
