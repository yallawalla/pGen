#ifndef		_CONSOLE_H
#define		_CONSOLE_H
#include	"stm32f4xx_hal.h"
#include	"io.h"

int				Escape(void);
enum			err_parse	{
					_PARSE_OK=0,
					_PARSE_ERR_SYNTAX,
					_PARSE_ERR_ILLEGAL,
					_PARSE_ERR_MISSING,
					_PARSE_ERR_NORESP,
					_PARSE_ERR_OPENFILE,
					_PARSE_ERR_MEM
};

void			Parse(int);
void			*console(void *);

extern uint32_t debug;
extern _io**		_DBG;

enum dbg {    
	DBG_CAN_RX=0,
	DBG_CAN_TX,
	DBG_CRC,
	DBG_TIMING,
	DBG_LED,
	DBG_USEC,
	DBG_CONSOLE,
	DBG_STAT,
	DBG_VOLT
};

#define _DEBUG(n,f, ...) 				\
	do {													\
		if(debug & (1<<(n))) {			\
			_io **io=_stdio(_DBG);		\
			_print(f,__VA_ARGS__);		\
			_stdio(io);								\
		} 													\
	} while(0)	
#endif
#define SW_version 100
void	printVersion(void);
