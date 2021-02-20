#include	"console.h"
#include	"ascii.h"
#include	"io.h"
#include	<stdlib.h>
_io				**_DBG;
uint32_t	debug;
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int			Escape(void) {
int		i=fgetc(stdin);
			if((*stdin->io)->esc == NULL)
				(*stdin->io)->esc=calloc(1,sizeof(esc));
			if(i==__Esc) {
				(*stdin->io)->esc->seq=i;
				(*stdin->io)->esc->timeout=HAL_GetTick()+10;
			} else if(i==EOF) {
				if((*stdin->io)->esc->timeout && (HAL_GetTick() > (*stdin->io)->esc->timeout)) {
					(*stdin->io)->esc->timeout=0;
					return (*stdin->io)->esc->seq;
				}
			} else if((*stdin->io)->esc->timeout) {
				(*stdin->io)->esc->seq=(((*stdin->io)->esc->seq) << 8) | i;
			} else {
				return i;
			}
			return EOF;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char		*cgets(int c, int mode) {
_buffer		*p=(*stdin->io)->gets;
			
			if(!p)
				p=(*stdin->io)->gets=_buffer_init((*stdin->io)->rx->size);
			switch(c) {
				case EOF:		
				case '\n':
					break;
				case '\r':
					*p->_push = '\0';
					p->_push=p->_pull=p->_buf;
					return(p->_buf);
				case 0x08:
				case 0x7F:
					if(p->_push != p->_pull) {
						--p->_push;
						if(mode)
							_print("\b \b");
					}
					break;
				default:
					if(p->_push != &p->_buf[p->size-1])
						*p->_push++ = c;
					else  {
						*p->_push=c;
						if(mode)
							_print("%c",'\b');
					}
					if(mode) {
						if(c < ' ' || c > 127)
							_print("%c%02X%c",'<',c,'>');
						else
							_print("%c",c);
					}
					break;
			}
			return(NULL);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char		*trim(char **c) {
			if(!c)
				return NULL;
			if(*c) {
				while(**c==' ') ++*c;
				for(char *cc=strchr(*c,0); *c != cc && *--cc==' '; *cc=0);
			}
			return *c;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int32_t DecodePlus(char *c) {
			switch(*trim(&c)) {
				case 'd':
				case 'D':
				for(c=strchr(c,' '); c && *c;)
					debug |= (1<<strtoul(++c,&c,10));
				_DBG=stdout->io;
				break;
				default:
					return EOF;
			}
			return NULL;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int32_t DecodeMinus(char *c) {
			switch(*trim(&c)) {
				case 'd':
				case 'D':
				c=strchr(c,' ');
				if(!c)
					debug=0;
				while(c && *c)
					debug &= ~(1<<strtoul(++c,&c,10));
				if(!debug)
					_DBG=NULL;
				break;

				default:
				return EOF;
	}
	return NULL;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int32_t DecodeInq(char *c) {
				switch(*trim(&c)) {
				case 'v':
					printVersion();
				break;

				default:
					return EOF;
	}
	return NULL;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
uint32_t DecodeEq(char *c) {
	return NULL;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
struct cmd {
	char *str;
	uint32_t (*f)(int, char *[]);
} cmds[]=
{
	{"copy",			NULL}
};
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
int32_t		DecodeCom(char *c) {
int32_t		ret=NULL;
//__________________________________________________Prompt only response ____
			if(!c) {
				_print("\r\n>");
			}
			else
//___________________________________________________________________________
				switch(*trim(&c)) {
//__________________________________________________
					case 'v':
					case 'V':
						printVersion();
					break;
//__________________________________________________
				case '?':
					return DecodeInq(++c);
				case '+':
					return DecodePlus(++c);
				case '-':
					return DecodeMinus(++c);
				case '=':
					return DecodeEq(++c);
//_______________________________________________________________________________________
				default: {
					char *q[8];
					int n=0;
					q[n]=strtok(c," ,");
					while(q[n]) {
						q[++n]=strtok(NULL," ,");
					}
					for(int i=0; i<sizeof(cmds)/sizeof(struct cmd); ++i)
						if(q[0] && !strncmp(q[0],cmds[i].str,strlen(q[0])))
							return cmds[i].f(n,q);
					ret = EOF;
					}
				}				
			return ret;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	Parse(int i) {
char	*c;
			switch(i) {
				case EOF:																				// empty usart
					break;				
				
				case __CtrlZ:																		// call watchdog reset
					while(1);				
				
				case __CtrlY:																		// call system reset
					NVIC_SystemReset();	
				
				case __Esc:
					__HAL_TIM_ENABLE(&htim1);
				break;
				
				default:
					c=cgets(i,EOF);				
					if(c) {		
						while(*c==' ') ++c;	
						i=DecodeCom(c);
						if(*c && i)				
							_print("... WTF(%d)",i);
						i=DecodeCom(NULL);
					}
			}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	*console(void *v) {
_io		**in=stdin->io;
_io 	**out=stdout->io;
			_stdio((_io **)v);
			Parse(Escape());
			stdin->io=in;
			stdout->io=out;
			return v;
}	
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
****************************f***************************************************/
void	printVersion() {
			DecodeCom(0);
			_print("  V%d.%02d %s",
				SW_version/100,SW_version%100,__DATE__);	
}
