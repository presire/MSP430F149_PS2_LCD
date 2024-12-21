#ifndef KEYBOARD_H
#define KEYBOARD_H

void PutChar(unsigned char c);
unsigned char GetChar(void);
void Init_KB(void);
unsigned char Decode(unsigned char sc);

#endif
