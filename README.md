# Preface

This circuit uses the MSP430F149 microcontroller to control LCD (1602) and PS/2 devices (keyboard, etc.).  
Circuit and firmware are included.  
<br>

In this circuit, pressing a key on a particular PS/2 keyboard causes a buzzer to sound for a specified time.  
<br>

The PS/2 keyboard uses the PORT1 interrupt handler to decode and read the key.  
<br>

This firmware can also be customized to play a melody from the buzzer.  
For example, using Timer_A, etc.  
<br>

The MSP430F149 microcontroller, LCD (1602), and PS/2 connector are connected as shown below.  
<br>

|<center>MSP430F149 Schematic</center>|<center>MSP430F149 Schematic 1602</center>|
|---|---|
|![](Schematic/MSP430F149_PS2_LCD_Root.png)|![](Schematic/MSP430F149_PS2_LCD_1602.png)|
<br>

