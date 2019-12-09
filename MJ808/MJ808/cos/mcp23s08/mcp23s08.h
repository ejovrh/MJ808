#ifndef MCP23S08_H_
#define MCP23S08_H_

typedef struct mcp23s08_t												// mcp23s08_t struct describing the port expander as a whole
{

} mcp23s08_t __attribute__((aligned(8)));

mcp23s08_t *mcp23s08_ctor();											// mcp23s08_t object constructor - does function pointer & hardware initialization

#endif /* MCP23S08_H_ */