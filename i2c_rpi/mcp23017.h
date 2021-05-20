
// I2C adresses

// mcp23017: 0100 A2 A1 A0 r/-w
#define mcp23017_U1 (0x40 / 2)
#define mcp23017_U2 (0x42 / 2)

int init_mcp23017();

/*
Bank mode = 0:
Address   specifies register for port A,
Address+1 specifies register for port B
*/

#define IODIR   0
#define IPOL    2
#define GPINTEN 4
#define DEFVAL  6
#define INTCON  8
#define IOCON   0x0a
#define GPPU    0x0c
#define INTF    0x0e
#define INTCAP  0x10
#define GPIO    0x12      // used to read port
#define OLAT    0x14      // used to write port A

// init values
// Port Expander U1 (all outputs)
#define IOCON_IVAL_U1   0b00000000  // Bank mode 0, sequential operation
#define IODIRA_IVAL_U1  0           // all output
#define IODIRB_IVAL_U1  0 //0x28        // all outputs, but 3+5 not used, layout error !
#define IPOL_IVAL_U1    0xff        // invert outputs (are also inverted by mosfet drivers)

// Port Expander U2 (all inputs)
#define IOCON_IVAL_U2   0b00000000  // Bank mode 0, sequential operation
#define IODIR_IVAL_U2   0xff        // all input
#define GPPU_IVAL_U2    0xff        // pull up on input
