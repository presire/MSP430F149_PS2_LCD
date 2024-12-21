#include  <msp430.h>
#include "BoardConfig.h"
#include "Keyboard.h"
#include "cry1602.h"
#include "gdata.h"


// LCD control
volatile unsigned char wait_count   = 12;  // LCD shift count counter
volatile unsigned char repeat_count = 2;   // Number of timer standby iterations (for 500ms standby)
uchar *s1 = "welcome!";                    // String to display


// PS/2 control
/// Key code
volatile unsigned char disptmp = 0x00;

/// Read serial data input (data bits) on PS/2 keyboard (used in PORT1_ISR interrupt handler)
/// Check the status of P5.6
#define SIDval  P5IN & BIT6

/// Buzzer control
volatile unsigned int  duration_counter = 0;
volatile unsigned int  target_duration  = 0;
volatile unsigned char buzzer_active    = 0;

/// Buzzer sound length definition (x 10ms)
#define DURATION_1  100    // 1sec
#define DURATION_2  200    // 2sec
#define DURATION_3  300    // 3sec
#define DURATION_4  400    // 4sec
#define DURATION_5  500    // 5sec
#define DURATION_6  600    // 6sec
#define DURATION_7  700    // 7sec
#define DURATION_8  800    // 8sec


// Interrupts wait processing for a specified number of times
void wait_with_interrupt(unsigned char count, unsigned char repeats)
{
    wait_count = count;                  // Set the number of shifts
    repeat_count = repeats;              // Set standby time
    IFG1 &= ~WDTIFG;                     // Clear WDT flag
    IE1 |= WDTIE;                        // Enable WDT interrupt
    LcdWriteData(0x20);                  // Writing space characters
    __bis_SR_register(LPM1_bits + GIE);  // Set to LPM1 and enable interrupts
}


void lcdWriteData(unsigned char repeats)
{
    LcdWriteData(0x20);

    // Wait for 250ms x repeats
    uchar j;
    for (j = repeats; j--;) {
        IFG1 &= ~WDTIFG;
        while(!(IFG1 & WDTIFG));
        IFG1 &= ~WDTIFG;
    }
}


// Buzzer control
void Init_Buzzer(void)
{
    P6DIR |= BIT7;  // Set P6.7 is set to output
    P6OUT |= BIT7;  // Initial state is HIGH (Buzzer stops)
}


// Set the ringing time according to key input
void Set_Buzzer_Duration(unsigned char key)
{
    switch(key) {
        case 'a':  // 1sec
        case 'A':
            target_duration = DURATION_1;
            buzzer_active = 1;
            P6OUT &= ~BIT7;    // Buzzer on
            break;
        case 's':  // 2sec
        case 'S':
            target_duration = DURATION_2;
            buzzer_active = 1;
            P6OUT &= ~BIT7;    // Buzzer on
            break;
        case 'd':  // 3sec
        case 'D':
            target_duration = DURATION_3;
            buzzer_active = 1;
            P6OUT &= ~BIT7;    // Buzzer on
            break;
        case 'f':  // 4sec
        case 'F':
            target_duration = DURATION_4;
            buzzer_active = 1;
            P6OUT &= ~BIT7;    // Buzzer on
            break;
        case 'g':  // 5sec
        case 'G':
            target_duration = DURATION_5;
            buzzer_active = 1;
            P6OUT &= ~BIT7;    // Buzzer on
            break;
        case 'h':  // 6sec
        case 'H':
            target_duration = DURATION_6;
            buzzer_active = 1;
            P6OUT &= ~BIT7;    // Buzzer on
            break;
        case 'j':  // 7sec
        case 'J':
            target_duration = DURATION_7;
            buzzer_active = 1;
            P6OUT &= ~BIT7;    // Buzzer on
            break;
//        case 'k':
//        case 'K':
//            if (!playing) {
//                // Start only if not playing
//                playing = 1;
//                curr_addr = 0;   // Play from the beginning of the song
//                counter = 0;     // Reset counter
//                TBCTL |= MC_1;   // Timer_B0 start
//                P6OUT &= ~BIT7;  // Buzzer on
//            }
//            break;
//        case 'l':
//        case 'L':
//            if (playing) {
//                // Stop if playing
//                playing = 0;
//                TBCTL &= ~MC_1;  // Timer_B0 stop
//                P6OUT |= BIT7;   // Buzzer off
//                curr_addr = 0;   // Reset address
//            }
//            break;
        default:
            buzzer_active = 0;  // Stop buzzer for undefined key
            P6OUT |= BIT7;      // Buzzer off
            break;
    }

    duration_counter = 0;  // Reset counter
}


// Initialization Microcontroller and Peripheral
void Init_System()
{
    uchar i;

    // Power off unneeded devices (use 0xf8 for PS/2 and LCD requirements)
    BoardConfig(0x78);

    // Set system clock to 8[MHz]
    BCSCTL1 &= ~XT2OFF;              // XT2 oscillator is turned on
    do {
        IFG1 &= ~OFIFG;              // Clear oscillator fault flag
        for (i = 0xFF; i > 0; i--);  // Wait for XT2 oscillator to stabilize
    } while (IFG1 & OFIFG);          // Wait for XT2 oscillator to stabilize

    BCSCTL2 |= SELM_2 + SELS;        // MCLK and SMCLK sources to XT2
                                     // SELM_2 : MCLK source to XT2
                                     // SELS :   SMCLK source to XT2

    // WDT set as 250ms interval timer
    WDTCTL = WDT_ADLY_250;

    // Set Timer_A as 10ms interrupt timer (when using SMCLK)
    // TACTL   = TASSEL_2 + ID_3 + MC_1;  // SMCLK selected, frequency divider ratio 8, up mode, initial state is start mode
    // TACCR0  = 10000;                   // Interrupt every 10ms (16[MHz] / 8 = 2[MHz])
    // TACCTL0 = CCIE;                    // Enable Timer_A interrupt

    // Set Timer_A as 10ms interrupt timer (when using ACLK)
    TACTL   = TASSEL_1 + MC_1;    // ACLK selected, up mode
                                  // To make 10ms at 32.768[kHz] : 32.768[kHz] x 0.01s = 328 counts
    TACCR0  = 328;                // Interrupt every 10ms (at 32.768[kHz])
    TACCTL0 = CCIE;               // Enable Timer_A interrupt
    TACCTL0 &= ~CCIFG;            // Clear TimerA interrupt flag

    // Set Timer_B0 as 10ms interrupt timer (when using SMCLK)
    // TBCTL   = TBSSEL_2 + ID_3 + MC_1;  // SMCLKを選択, 分周比8, 初期状態は停止モード
    // TBCCR0  = 10000;                   // Interrupt every 10ms (at 1[MHz] x 10ms)
    // TBCCTL0 = CCIE;                    // CCR0 interrupt allowed
    // TBCCTL0 &= ~CCIFG;                 // Clear TimerB interrupt flag

    Init_KB();                    // Initialization keyboard
    Init_Buzzer();                // Initialization buzzer
    LcdReset();                   // Initialization LCD
}


void main(void)
{
    unsigned char i;

    Init_System();

    // LCD initial display setting
    DispStr(4, 0, s1);
    LocateXY(0, 9);             // Set cursor position to column 9
    LcdWriteCommand(0x07, 1);   // Set entire display to left shift mode

    // initial scroll
    for (i = 12; i > 0; i--) {
        wait_with_interrupt(1, 1);
    }

    // Global interrupt allowed
    _EINT();

    while (1) {
        // Set LCD to right shift mode
        LcdWriteCommand(0x05, 1);
        for (i = 24; i > 0; i--) {
            wait_with_interrupt(1, 2);
        }

        //  LCD set to left shift mode
        LcdWriteCommand(0x07, 1);
        for (i = 24; i > 0; i--) {
            wait_with_interrupt(1, 2);
        }
    }
}


// WDT interrupt handler - called every 250ms
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
    // Decrements the number of timer iterations
    if(--repeat_count == 0) {
        repeat_count = 2;                          // Reset repeat counter

        // Decrements the number of shifts
        if(--wait_count == 0) {
            IE1 &= ~WDTIE;                         // Disable WDT interrupts
            __bic_SR_register_on_exit(LPM1_bits);  // Return from LPM1
        }
        else {
            LcdWriteData(0x20);                    // Write the next space character
        }
    }

    IFG1 &= ~WDTIFG;                               // Clear WDT flag
}


// Timer_A interrupt handler
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A(void)
{
    if (buzzer_active) {
        duration_counter++;

        if (duration_counter >= target_duration) {
            P6OUT |= BIT7;        // Buzzer off
            buzzer_active = 0;
            duration_counter = 0;
        }
    }
}


// Timer_B0 interrupt handler
// #pragma vector = TIMERB0_VECTOR
// __interrupt void TimerB0_ISR(void)
// {
//    counter++;
// }


// PORT1 interrupt handler
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    if (P1IFG & BIT7) {
        P1IFG &=~ BIT7;

        if (bitcount == 11) {
            if(SIDval) return;
            else       bitcount--;
        }
        else if (bitcount == 2) {
            if (SIDval) pebit = 1;
            else        pebit = 0;
            bitcount--;
        }
        else if (bitcount == 1) {
            if (SIDval) {
                bitcount = 11;
                if (Decode(recdata)) {
                    // If the key is decoded correctly
                    __bic_SR_register_on_exit(LPM1_bits);  // Return from LPM1

                    // Get keystrokes
                    uchar disptmp = GetChar();

                    // Set the beeper to ring for a set amount of time
                    Set_Buzzer_Duration(disptmp);
                }
                recdata = 0;
            }
            else {
                bitcount = 11;
                recdata = 0;
            }
        }
        else {
            recdata >>= 1;
            if (SIDval) recdata |= 0x80;
            bitcount--;
        }
   }
}
