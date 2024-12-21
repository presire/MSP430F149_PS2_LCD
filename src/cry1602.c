#include <msp430x14x.h>
#include "cry1602.h"

typedef unsigned char uchar;
typedef unsigned int  uint;


// Macro definitions
#define DataDir     P4DIR
#define DataPort    P4OUT
#define Busy        0x80
#define CtrlDir     P3DIR
#define CLR_RS      P3OUT &= ~BIT0;  // RS = P3.0
#define SET_RS      P3OUT |= BIT0;
#define CLR_RW      P3OUT &= ~BIT1;  // RW = P3.1
#define SET_RW      P3OUT |= BIT1;
#define CLR_EN      P3OUT &= ~BIT2;  // EN = P3.2
#define SET_EN      P3OUT |= BIT2;


// Continuously display a string on LCD from a specific position
// x : Column coordinate of the position
// y : Row coordinate of the position
// ptr : Pointer to the location where the string is stored
void DispStr(uchar x, uchar y, uchar *ptr)
{
    uchar *temp;
    uchar i,
          n = 0;

    temp = ptr;
    while(*ptr++ != '\0') n++;    // Calculate the number of valid characters in the string

    for (i = 0; i < n; i++) {
        Disp1Char(x++, y, temp[i]);
        if (x == 0x0f) {
           x  = 0; 
           y ^= 1;
        }
    }
}


// Display N characters continuously on LCD from a specific position
// x : Column coordinate of the position
// y : Row coordinate of the position
// n : Number of characters
// ptr : Pointer to the location where characters are stored
void DispNChar(uchar x, uchar y, uchar n, uchar *ptr)
{
    uchar i;
    for (i = 0; i < n; i++) {
        Disp1Char(x++, y, ptr[i]);
        if (x == 0x0f) {
            x = 0;
            y ^= 1;
        }
    }
}


// Input coordinate information of the displayed character position to LCD
// x : Column coordinate of the position
// y : Row coordinate of the position
void LocateXY(uchar x, uchar y)
{
    uchar temp;

    temp = x & 0x0f;
    y    &= 0x01;
    if(y) temp |= 0x40;  // For second line
    temp |= 0x80;

    LcdWriteCommand(temp, 1);
}


// Display a character at a specific position
// x : Column coordinate of the position
// y : Row coordinate of the position
// data : Character data to be displayed
void Disp1Char(uchar x, uchar y, uchar data)
{
    LocateXY(x, y);
    LcdWriteData(data);
}


// Reset operation for LCD1602 module
void LcdReset(void) 
{
    CtrlDir |= 0x07;                 // Set control line port to output mode
    DataDir  = 0xFF;                 // Set data port to output mode

    LcdWriteCommand(0x38, 0);        // Specified reset operations
    Delay5ms();
    LcdWriteCommand(0x38, 0);
    Delay5ms();
    LcdWriteCommand(0x38, 0);
    Delay5ms();

    LcdWriteCommand(0x38, 1);       // Set display mode
    LcdWriteCommand(0x08, 1);       // Display off
    LcdWriteCommand(0x01, 1);       // Clear display
    LcdWriteCommand(0x06, 1);       // Write characters when entire display is not moving
    LcdWriteCommand(0x0c, 1);       // Display on, cursor on, no blinking
}


// Write command to LCD module
// cmd : Command
// chk : Flag for busy check
//       1 : Check busy status
//       0 : Don't check busy status
void LcdWriteCommand(uchar cmd, uchar chk)
{

    if (chk) WaitForEnable();   // Whether to detect busy signal

    CLR_RS;
    CLR_RW; 
    _NOP();

    DataPort = cmd;             // Write command word to data port
    _NOP();
    
    SET_EN;                     // Generate enable pulse signal
    _NOP();
    _NOP();
    CLR_EN;
}


// Write display data to LCD's current address
// data : Character data to display
void LcdWriteData(uchar data)
{
    WaitForEnable();        // Wait until LCD busy state ends

    SET_RS;
    CLR_RW;
    _NOP();

    DataPort = data;        // Write display data to data port
    _NOP();

    SET_EN;                 // Generate enable pulse signal
    _NOP();
    _NOP();
    CLR_EN;
}


// Wait until LCD1602 completes internal operation
void WaitForEnable(void)
{
    P4DIR &= 0x00;  // Switch P4 port to input mode

    CLR_RS;
    SET_RW;
    _NOP();
    SET_EN;
    _NOP();
    _NOP();

    while((P4IN & Busy) != 0);  // Detect busy flag

    CLR_EN;

    P4DIR |= 0xFF;  // Switch P4 port to output mode
}


// Wait for 5ms
void Delay5ms(void)
{ 
    uint i = 40000;
    while (i != 0) i--;
}
