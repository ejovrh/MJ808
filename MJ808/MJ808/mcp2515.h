#ifndef MCP2515_H_
#define MCP2515_H_

#define CAN_MAX_MSG_LEN		8 // 8 bytes are the max. length of a CAN message

// low-level device instructions, see datasheet p. 66, table 12.1
#define MCP2515_OPCODE_RESET						0xC0	// software reset instruction
#define MCP2515_OPCODE_READ							0x03	// read instruction
#define MCP2515_OPCODE_READ_RX_BUFFER		0x90	//TODO - add bit fields
#define MCP2515_OPCODE_WRITE						0x02	// write instruction
#define MCP2515_OPCODE_LOAD_TX_BUFFER		0x40	// load TX buffer + bit fields
#define MCP2515_OPCODE_RTS							0x80	// RTS command + bit fields; p. 65 chap 12.7 & figure 12.6 on p. 68
#define MCP2515_OPCODE_READ_STATUS			0xA0	// single instruction read of status bits
#define MCP2515_OPCODE_RX_STATUS				0xB0	// determines received message filter and type
#define MCP2515_OPCODE_BIT_MODIFY				0x05	// instruction for modification of single bits in selected registers: shaded fields in table 11.1

// read status result byte
#define STAT_CANINTF_TX2IF		7
#define STAT_TXB2CNTRL_TXREQ	6
#define STAT_CANINTF_TX1IF		5
#define STAT_TXB1CNTRL_TXREQ	4
#define STAT_CANINTF_TX0IF		3
#define STAT_TXB0CNTRL_TXREQ	2
#define STAT_RX1IF						1
#define STAT_RX0IF						0



// register address definitions
#define RXF0SIDH 0x00 // RXF0SIDH – FILTER 0 STANDARD IDENTIFIER HIGH	datasheet p.34
#define RXF0SIDL 0x01 // RXF0SIDL – FILTER 0 STANDARD IDENTIFIER LOW	datasheet p.34
#define RXF0EID8 0x02 // RXF0EID8 – FILTER 0 EXTENDED IDENTIFIER HIGH	datasheet p.35
#define RXF0EID0 0x03 // RXF0EID0 – FILTER 0 EXTENDED IDENTIFIER LOW	datasheet p.35
#define RXF1SIDH 0x04 // RXF1SIDH – FILTER 1 STANDARD IDENTIFIER HIGH	datasheet p.34
#define RXF1SIDL 0x05 // RXF1SIDL – FILTER 1 STANDARD IDENTIFIER LOW	datasheet p.34
#define RXF1EID8 0x06 // RXF1EID8 – FILTER 1 EXTENDED IDENTIFIER HIGH	datasheet p.35
#define RXF1EID0 0x07 // RXF1EID0 – FILTER 1 EXTENDED IDENTIFIER LOW	datasheet p.35
#define RXF2SIDH 0x08 // RXF2SIDH – FILTER 2 STANDARD IDENTIFIER HIGH	datasheet p.34
#define RXF2SIDL 0x09 // RXF2SIDL – FILTER 2 STANDARD IDENTIFIER LOW	datasheet p.34
#define RXF2EID8 0x0A // RXF2EID8 – FILTER 2 EXTENDED IDENTIFIER HIGH datasheet p.35
#define RXF2EID0 0x0B // RXF2EID0 – FILTER 2 EXTENDED IDENTIFIER LOW	datasheet p.35
#define RXF3SIDH 0x10 // RXF3SIDH – FILTER 3 STANDARD IDENTIFIER HIGH	datasheet p.34
#define RXF3SIDL 0x11 // RXF3SIDL – FILTER 3 STANDARD IDENTIFIER LOW	datasheet p.34
#define RXF3EID8 0x12 // RXF3EID8 – FILTER 3 EXTENDED IDENTIFIER HIGH	datasheet p.35
#define RXF3EID0 0x13 // RXF3EID0 – FILTER 3 EXTENDED IDENTIFIER LOW	datasheet p.35
#define RXF4SIDH 0x14 // RXF4SIDH – FILTER 4 STANDARD IDENTIFIER HIGH	datasheet p.34
#define RXF4SIDL 0x15 // RXF4SIDL – FILTER 4 STANDARD IDENTIFIER LOW	datasheet p.34
#define RXF4EID8 0x16 // RXF4EID8 – FILTER 4 EXTENDED IDENTIFIER HIGH	datasheet p.35
#define RXF4EID0 0x17 // RXF4EID0 – FILTER 4 EXTENDED IDENTIFIER LOW	datasheet p.35
#define RXF5SIDH 0x18 // RXF5SIDH – FILTER 5 STANDARD IDENTIFIER HIGH	datasheet p.34
#define RXF5SIDL 0x19 // RXF5SIDL – FILTER 5 STANDARD IDENTIFIER LOW	datasheet p.34
#define RXF5EID8 0x1A // RXF5EID8 – FILTER 5 EXTENDED IDENTIFIER HIGH	datasheet p.35
#define RXF5EID0 0x1B // RXF5EID0 – FILTER 5 EXTENDED IDENTIFIER LOW	datasheet p.35
#define RXM0SIDH 0x20 // RXM0SIDH – MASK 0 STANDARD IDENTIFIER HIGH	datasheet p.35
#define RXM0SIDL 0x21	// RXM0SIDL – MASK 0 STANDARD IDENTIFIER LOW datasheet p.36
#define RXM0EID8 0x22 // RXM0EID8 – MASK 0 EXTENDED IDENTIFIER HIGH datasheet p.36
#define RXM0EID0 0x23 // RXM0EID0 – MASK 0 EXTENDED IDENTIFIER LOW datasheet p.37
#define RXM1SIDH 0x24 // RXM1SIDH – MASK 1 STANDARD IDENTIFIER HIGH	datasheet p.35
#define RXM1SIDL 0x25	// RXM1SIDL – MASK 1 STANDARD IDENTIFIER LOW datasheet p.36
#define RXM1EID8 0x26 // RXM1EID8 – MASK 1 EXTENDED IDENTIFIER HIGH datasheet p.36
#define RXM1EID0 0x27 // RXM1EID0 – MASK 1 EXTENDED IDENTIFIER LOW datasheet p.37

// RXnBF PIN CONTROL AND STATUS
#define BFPCTRL	0x0C // datasheet p.29
#define B1BFS	5			// RX1BF Pin State bit
#define B0BFS	4			// RX0BF Pin State bit
#define B1BFE	3			// RX1BF Pin Function Enable bit
#define B0BFE	2			// RX0BF Pin Function Enable bit
#define B1BFM 1			// RX1BF Pin Operation mode bit
#define B0BFM 0			// RX0BF Pin Operation mode bit

// TXRTSCTRL – TXnRTS PIN CONTROL AND STATUS REGISTER
#define TXRTSCTRL 0x0D // datasheet p.19
#define B2RTS 5		// B2RTS: TX2RTS Pin State bit
#define	B1RTS 4		// B1RTS: TX1RTX Pin State bit
#define B0RTS 3		// B0RTS: TX0RTS Pin State bit
#define	B2RTSM 2	// B2RTSM: TX2RTS Pin mode bit
#define	B1RTSM 1	// B1RTSM: TX1RTS Pin mode bit
#define	B0RTSM 0	// B0RTSM: TX0RTS Pin mode bit

// CANSTAT register, datasheet p. 61
#define CANSTAT	0x0E
#define OPMOD2		7 // OPMOD2:0 - Operation mode
#define	OPMOD1		6 //
#define	OPMOD0		5 //
#define	ICOD2			3 // ICOD2:0 - Interrupt Flag Code
#define ICOD1			2	//
#define ICOD0			1 //

// CANCTRL register, datasheet p. 60
#define CANCTRL	0x0F
#define REQOP2		7	// REQOPQ2:0 - operational mode
#define REQOP1		6	//
#define REQOP0		5	//
#define ABAT			4	// Abort All Pending Transmissions
#define OSM				3	// One-Shot mode
#define CLKEN			2	// CLKOUT Pin Enable
#define CLKPRE1		1	// CLKPRE1:0 - CLKOUT Pin Prescaler
#define CLKPRE0		0	//

// TEC – TRANSMIT ERROR COUNTER
#define TEC 0x1C // datasheet p.48

// REC – RECEIVER ERROR COUNTER
#define REC 0x1D // datasheet p.48

// CNF3 - CONFIGURATION 1
#define CNF3 0x28 // datasheet p.45
#define SOF 7	// SOF: Start-of-Frame signal bit
#define WAKFIL 6	// WAKFIL: Wake-up Filter bit
#define PHSEG22 2	// PHSEG2<2:0>: PS2 Length bits
#define PHSEG21 1
#define PHSEG20 0

// CNF2 – CONFIGURATION 1
#define CNF2 0x29 // datasheet p.44
#define BTLMODE 7	// BTLMODE: PS2 Bit Time Length bit
#define SAM 6	// SAM: Sample Point Configuration bit
#define PHSEG12 5 // PHSEG1<2:0>: PS1 Length bits
#define PHSEG11 4
#define PHSEG10 3
#define PRSEG2 2	// PRSEG<2:0>: Propagation Segment Length bits
#define PRSEG1 1
#define PRSEG0 0

// CNF1 – CONFIGURATION 1
#define CNF1 0x2A // datasheet p.44
#define SJW1 7 // SJW<1:0>: Synchronization Jump Width Length bits
#define SJW0 6
#define BRP5 5 // BRP<5:0>: Baud Rate Prescaler bits
#define BRP4 4
#define BRP3 3
#define BRP2 2
#define BRP1 1
#define BRP0 0

// CANINTE – INTERRUPT ENABLE
#define CANINTE 0x2B // datasheet p.52
#define MERRE 7 // MERRE: Message Error Interrupt Enable bit
#define WAKIE 6 // WAKIE: Wake-up Interrupt Enable bit
#define ERRIE 5 // ERRIE: Error Interrupt Enable bit (multiple sources in EFLG register)
#define TX2IE 4 // TX2IE: Transmit Buffer 2 Empty Interrupt Enable bit
#define TX1IE 3 // TX1IE: Transmit Buffer 1 Empty Interrupt Enable bit
#define TX0IE 2 // TX0IE: Transmit Buffer 0 Empty Interrupt Enable bit
#define RX1IE 1 // RX1IE: Receive Buffer 1 Full Interrupt Enable bit
#define RX0IE 0 // RX0IE: Receive Buffer 0 Full Interrupt Enable bit

// CANINTF – INTERRUPT FLAG
#define CANINTF 0x2C // datasheet p.53
#define MERRF 7 // MERRF: Message Error Interrupt Flag bit
#define WAKIF 6 // WAKIF: Wake-up Interrupt Flag bit
#define ERRIF 5 // ERRIF: Error Interrupt Flag bit (multiple sources in EFLG register)
#define TX2IF 4 // TX2IF: Transmit Buffer 2 Empty Interrupt Flag bit
#define TX1IF 3 // TX1IF: Transmit Buffer 1 Empty Interrupt Flag bit
#define TX0IF 2 // TX0IF: Transmit Buffer 0 Empty Interrupt Flag bit
#define RX1IF 1 // RX1IF: Receive Buffer 1 Full Interrupt Flag bit
#define RX0IF 0 // RX0IF: Receive Buffer 0 Full Interrupt Flag bit

// EFLG – ERROR FLAG
#define EFLG 0x2D // datasheet p.49
#define RX1OVR 7 // RX1OVR: Receive Buffer 1 Overflow Flag bit
#define RX0OVR 6 // RX0OVR: Receive Buffer 0 Overflow Flag bit
#define TXBO 5	// TXBO: Bus-Off Error Flag bit
#define TXEP 4	// TXEP: Transmit Error-Passive Flag bit
#define RXEP 3	// RXEP: Receive Error-Passive Flag bit
#define TXWAR 2	// TXWAR: Transmit Error Warning Flag bit
#define RXWAR 1 // RXWAR: Receive Error Warning Flag bit
#define EWARN 0 // EWARN: Error Warning Flag bit

// TXBnCTRL – TRANSMIT BUFFER 0 CONTROL REGISTER
#define TXB0CTRL 0x30 // datasheet p.18
#define ABTF 6	// ABTF: Message Aborted Flag bit
#define MLOA 5	// MLOA: Message Lost Arbitration bit
#define TXERR 4	// TXERR: Transmission Error Detected bit
#define TXREQ 3	// TXREQ: Message Transmit Request bit
#define TXP1 1	// TXP<1:0>: Transmit Buffer Priority bits
#define TXP0 0

// TXB0SIDH – TRANSMIT BUFFER 0 STANDARD IDENTIFIER HIGH
#define TXB0SIDH 0x31 // datasheet p.19

// TXB0SIDL – TRANSMIT BUFFER 0 STANDARD IDENTIFIER LOW
#define TXB0SIDL 0x32 // datasheet p.19
#define EXIDE 3 // EXIDE: Extended Identifier Enable bit; applies for all TX buffers

// TXB0EID8 – TRANSMIT BUFFER 0 EXTENDED IDENTIFIER HIGH
#define TXB0EID8 0x33 // datasheet p.20

// TXB0EID0 – TRANSMIT BUFFER 0 EXTENDED IDENTIFIER LOW
#define TXB0EID0 0x34 // datasheet p.20

// TXB0DLC - TRANSMIT BUFFER 0 DATA LENGTH CODE
#define TXB0DLC 0x35 // datasheet p.21
#define RTR	6 // RTR: Remote Transmission Request bit
#define DLC3 3 // DLC<3:0>: Data Length Code bits, Sets the number of data bytes to be transmitted (0 to 8 bytes)
#define DLC2 2
#define DLC1 1
#define DLC0 0

// TXB0Dm – TRANSMIT BUFFER 0 DATA BYTE m
#define TXB0Dm 0x36 // datasheet p.21

// TXB1CTRL – TRANSMIT BUFFER 1 CONTROL REGISTER
#define TXB1CTRL 0x40	// datasheet p.18

// TXB1SIDH – TRANSMIT BUFFER 1 STANDARD IDENTIFIER HIGH
#define TXB1SIDH 0x41 // datasheet p.19

// TXB1SIDL – TRANSMIT BUFFER 1 STANDARD IDENTIFIER LOW
#define TXB1SIDL 0x42 // datasheet p.19

// TXB1EID8 – TRANSMIT BUFFER 1 EXTENDED IDENTIFIER HIGH
#define TXB1EID8 0x43 // datasheet p.20

// TXB1EID0 – TRANSMIT BUFFER 1 EXTENDED IDENTIFIER LOW
#define TXB1EID0 0x44 // datasheet p.20

// TXB1DLC - TRANSMIT BUFFER 1 DATA LENGTH CODE
#define TXB1DLC 0x45 // datasheet p.21

// TXB1Dm – TRANSMIT BUFFER 1 DATA BYTE m
#define TXB1Dm 0x46 // datasheet p.21

// TXB2CTRL – TRANSMIT BUFFER 2 CONTROL REGISTER
#define TXB2CTRL 0x50	// datasheet p.18

// TXB2SIDH – TRANSMIT BUFFER 1 STANDARD IDENTIFIER HIGH
#define TXB2SIDH 0x51 // datasheet p.19

// TXB2SIDL – TRANSMIT BUFFER 2 STANDARD IDENTIFIER LOW
#define TXB2SIDL 0x52 // datasheet p.20

// TXB2EID8 – TRANSMIT BUFFER 2 EXTENDED IDENTIFIER HIGH
#define TXB2EID8 0x53 // datasheet p.20

// TXB2EID0 – TRANSMIT BUFFER 2 EXTENDED IDENTIFIER LOW
#define TXB2EID0 0x54 // datasheet p.20

// TXB2DLC - TRANSMIT BUFFER 2 DATA LENGTH CODE
#define TXB2DLC 0x55 // datasheet p.21

// TXB2Dm – TRANSMIT BUFFER 2 DATA BYTE m
#define TXB2Dm 0x56 // datasheet p.21

// RXB0CTRL – RECEIVE BUFFER 0 CONTROL
#define RXB0CTRL 0x60	// datasheet p.18
#define RXM1 6	// RXM<1:0>: Receive Buffer Operating mode bits
#define RXM0 5
#define RXRTR 3	// RXRTR: Received Remote Transfer Request bit
#define BUKT 2	// BUKT: Rollover Enable bit
#define BUKT1 1	// BUKT1: Read-only Copy of BUKT bit (used internally by the MCP2515)
#define FILHIT0 0	// FILHIT0: Filter Hit bit – indicates which acceptance filter enabled reception of message

// RXB0SIDH – RECEIVE BUFFER 0 STANDARD IDENTIFIER HIGH
#define RXB0SIDH 0x61 // datasheet p.29

// RXB0SIDH – RECEIVE BUFFER 0 STANDARD IDENTIFIER LOW
#define RXB0SIDL 0x62 // datasheet p.30

// RXB0EID8 – RECEIVE BUFFER 0 EXTENDED IDENTIFIER HIGH
#define RXB0EID8 0x63	// datasheet p.30

// RXB0EID0 – RECEIVE BUFFER 0 EXTENDED IDENTIFIER LOW
#define RXB0EID0 0x64 // datasheet p.31

//RXB0DLC – RECEIVE BUFFER 0 DATA LENGTH CODE
#define RXB0DLC 0x65 // datasheet p.31

// RXB0DM – RECEIVE BUFFER 0 DATA BYTE M
#define RXB0DM 0x66 // datasheet p.31

// RXB1CTRL – RECEIVE BUFFER 1 CONTROL
#define RXB1CTRL 0x70 // datasheet p.28
#define RSM1 6 // RXM<1:0>: Receive Buffer Operating mode bits
#define RXM0 5
#define RXRTR 3	// RXRTR: Received Remote Transfer Request bit
#define FILHIT2 2	// FILHIT<2:0>: Filter Hit bits - indicates which acceptance filter enabled reception of message
#define FILHIT1 1
#define FILHIT0 0

// RXB1SIDH – RECEIVE BUFFER 1 STANDARD IDENTIFIER HIGH
#define RXB1SIDH 0x71 // datasheet p.29

// RXB1SIDH – RECEIVE BUFFER 1 STANDARD IDENTIFIER LOW
#define RXB1SIDL 0x72 // datasheet p.30

// RXB1EID8 – RECEIVE BUFFER 1 EXTENDED IDENTIFIER HIGH
#define RXB1EID8 0x73	// datasheet p.30

// RXB1EID0 – RECEIVE BUFFER 1 EXTENDED IDENTIFIER LOW
#define RXB1EID0 0x74 // datasheet p.31

//RXB1DLC – RECEIVE BUFFER 1 DATA LENGTH CODE
#define RXB1DLC 0x75 // datasheet p.31

// RXB1DM – RECEIVE BUFFER 1 DATA BYTE M
#define RXB1DM 0x76 // datasheet p.31 - 0x76 up to 0x6C / 6 bytes


typedef struct can_message
{
	uint8_t		sidh;			// Standard Identifier High Byte
	uint8_t		sidl;			// Standard Identifier Low Byte
	uint8_t   dlc;			// Data Length Code and others
	uint8_t   data[8];	// Data, length identified by DLC
} can_message;


// initialization & configuration after power on
void mcp2515_init(void);

// fetches a received CAN message from the MCP2515, triggered by RX interrupt
void mcp2515_can_msg_receive(can_message *msg);

// sends a CAN message onto the bus
void mcp2515_can_msg_send(can_message *msg);

#endif /* MCP2515_H_ */