#ifndef __mw_H
#define __mw_H

extern uint32_t ADValue[6];
uint8_t Read_BrailleKeys(void);
char Braille_To_Char(uint8_t);
void Flag(void);
void clear(void);
void sennum(void);
extern uint8_t f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,num;
#endif
