#include "main.h"
#include "can.h"

static CAN_HandleTypeDef hcan;	// CAN object

typedef struct	// can_t actual
{
	can_t public;  // public struct
	void (*init)(void);  // private - CAN init

	// preserve byte order for sequential reads/writes
	uint8_t __canintf;	// contents of CANINTF register, datasheet p. 53
	uint8_t __eflg;  // contents of EFLG register, datasheet p. 49
	uint8_t __canstat;	// contents of the CANSTAT register, datasheet p. 61
	uint8_t __canctrl;	// contents of the CANCTRL register, datasheet p. 60
	uint8_t __tec;	// Transmit Error Counter - TEC, datasheet p. 48
	uint8_t __rec;	// Receive Error Counter - REC, datasheet p. 48
	// preserve byte order for sequential reads/writes

	uint8_t __in_sleep :1;	// is MCP2561 CAN transceiver in sleep or not
	uint8_t __icod :3;	// Interrupt Codes

} __can_t;

extern __can_t __CAN;  // declare can_t actual
static
void _mcp2515_opcode_bit_modify(const uint8_t addr, const uint8_t mask, const uint8_t byte)
{
	return;
}

static void _mcp2515_opcode_read_bytes(const uint8_t addr, volatile uint8_t *data, const uint8_t len)
{
	return;
}

static void _mcp2515_change_opmode(const uint8_t mode)
{
	return;
}

static void _mcp2515_can_msg_receive(volatile can_msg_t *const msg)
{
	return;
}

static void _mcp2515_can_msg_send(volatile can_msg_t *const msg)
{
	if(__CAN.__in_sleep)  // if sleeping...
		__CAN.public.Sleep(0);	// wake up

	// start to send message
	return;
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
	{.public.Sleep = &_can_sleep,  // set up function pointer for public methods
	.public.RequestToSend = &_mcp2515_can_msg_send,  // ditto
	.public.FetchMessage = &_mcp2515_can_msg_receive,  // ditto
	.public.ChangeOpMode = &_mcp2515_change_opmode,  // ditto
	.public.ReadBytes = &_mcp2515_opcode_read_bytes,  // ditto
	.public.BitModify = &_mcp2515_opcode_bit_modify,  // ditto
	};

// CAN init, as generated via ioc file
inline static void _CANInit(void)
{
	hcan.Instance = CAN;	//
	hcan.Init.Prescaler = 8;	//
	hcan.Init.Mode = CAN_MODE_NORMAL;  //
	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;	//
	hcan.Init.TimeSeg1 = CAN_BS1_16TQ;	//
	hcan.Init.TimeSeg2 = CAN_BS2_3TQ;  //
	hcan.Init.TimeTriggeredMode = DISABLE;	//
	hcan.Init.AutoBusOff = DISABLE;  //
	hcan.Init.AutoWakeUp = DISABLE;  //
	hcan.Init.AutoRetransmission = DISABLE;  //
	hcan.Init.ReceiveFifoLocked = DISABLE;	//
	hcan.Init.TransmitFifoPriority = DISABLE;  //
	HAL_CAN_Init(&hcan);
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
	HAL_CAN_IRQHandler(&hcan);	// TODO - implement & verify CAN message interrupts/reception
}
