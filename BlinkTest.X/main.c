#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = ON // allow multiple reconfigurations
#pragma config IOL1WAY = ON // allow multiple reconfigurations
#pragma config FUSBIDIO = OFF // USB pins controlled by USB module
#pragma config FVBUSONIO = OFF // USB BUSON controlled by USB module


void setup() {
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;
    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;
    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    TRISCbits.TRISC6 = 1; // C6 (switch 1) is an input pin
    //TRISCbits.TRISC7 = 1; // C7 (switch 2) is an input pin
    //TRISBbits.TRISB8 = 1; // B8 (switch 3) is an input pin
    //TRISBbits.TRISB9 = 1; // B9 (switch 4) is an input pin
    
    // initialize output pins for LED
    TRISAbits.TRISA4 = 0; // A4 is an output pin (LED power)
    LATAbits.LATA4 = 1;   // A4 is HIGH (LED on)
    
    // Turn off charger
    //TRISCbits.TRISC4 = 0; // C4 is an output pin (enable input)
    //LATCbits.LATC4 = 1;   // C4 is HIGH (Disable charger)  

}

int main() {
    // initialization
    __builtin_disable_interrupts(); // disable interrupts while initializing things
    setup();
    __builtin_enable_interrupts();
    
    while (1) {
        // if all switches are low, set A0 to high and set OC2 duty cycle to 12.5%. Else, set OC2 duty cycle to 2.5% 
        // 24MHz core timer
        _CP0_SET_COUNT(0);
        if(PORTCbits.RC6 == 0){ // pull-up resistor, to GND when button pushed
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < ((48000000/2)/10000)) {   // wait .1 ms to remove switch bounce
            }
            if(PORTCbits.RC6 == 0){
                LATAbits.LATA4 = 1;   // Turn on LED
            }
        }
        else {  // if either switch is not pushed
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < ((48000000/2)/10000)) {   // wait .1 ms to remove switch bounce
            }
            if(PORTCbits.RC6 == 1){   // if switch is indeed high, turn off battery charger
                LATAbits.LATA4 = 1;   // Turn on LED
            }
        }
    }
}