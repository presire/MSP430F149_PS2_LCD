#include <msp430.h>
#include "Keyboard.h"
#include "code.h"
#include "gdata.h"


void Init_KB(void)
{
    P1DIR &=~ BIT7;     // Connect Clock to P1.7, set as input
    P5DIR &=~ BIT6;     // Connect SID to P5.6, set as input
    P1IES |= BIT7;      // Falling edge interrupt
    P1IFG = 0x00;       // Clear interrupt flag
    P1IE  |= BIT7;      // Enable clock port interrupt
    P1SEL = 0x00;       // Use P1 port as I/O
}

void PutChar(unsigned char c)
{
    kb_buffer[input] = c;

    if (input < (BufferSize - 1)) input++;
    else                          input = 0;
}

unsigned char GetChar(void)
{
    unsigned char temp;

    if (output == input) {
        return 0xff;
    }
    else
    {
        temp = kb_buffer[output];
        if(output < (BufferSize - 1)) output++;
        else                          output = 0;
    }

    return temp;
}

unsigned char Decode(unsigned char sc)
{
    static unsigned char shift = 0;
    static unsigned char up    = 0;
    unsigned char i,
                  flag = 0;

    if (sc == 0xf0) {
        up = 1;
        return 0;
    }
    else if (up == 1) {
        up = 0;
        if((sc == 0x12) || (sc==0x59)) shift = 0;

        return 0;
    }

    if ((sc == 0x12) || (sc == 0x59)) {
        shift = 1;
        flag = 0;
    }
    else {
        if (shift) {
            for(i = 0; (shifted[i][0] != sc) && shifted[i][0]; i++);

            if (shifted[i][0] == sc) {
                PutChar(shifted[i][1]);
                flag = 1;
            }
        }
        else {
            for(i = 0; (unshifted[i][0] != sc) && unshifted[i][0]; i++);

            if (unshifted[i][0] == sc) {
                PutChar(unshifted[i][1]);
                flag = 1;
            }
        }
    }

    if(flag)  return 1;
    else      return 0;
}
