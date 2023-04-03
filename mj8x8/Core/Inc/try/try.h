#include "mj8x8/can_msg.h"

void PopulatedBusOperation(message_handler_t *const in_handler);	// executes function pointer identified by message command
void BranchtableEventHandler(const uint8_t val);	// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
void EmptyBusOperation(void);  // defines device operation on empty bus
