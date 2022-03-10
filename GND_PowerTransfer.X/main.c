#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

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
//#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
//#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
//#pragma config FUSBIDIO = ON // USB pins controlled by USB module
//#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module


void setup() {
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;
    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;
    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // initialize microswitches
    // switch configuration:
    // 1--------2
    // |        |
    // |  ATAG  |
    // |        |
    // 4--------3
    TRISBbits.TRISB6 = 1; // B6 (switch 1) is an input pin
    TRISBbits.TRISB7 = 1; // B7 (switch 2) is an input pin
    //TRISBbits.TRISB8 = 1; // B8 (switch 3) is an input pin
    //TRISBbits.TRISB9 = 1; // B9 (switch 4) is an input pin
    
    // initialize transistor output 
    TRISBbits.TRISB10 = 0; // B10 is an output pin
    LATBbits.LATB10 = 0;   // B10 is LOW 

    // initialize servo output
    //RPB15Rbits.RPB15R = 0b0101; // A1 pin is set to OC2 
    // use Timer 2 for servo PWM. Initialize output compare
    // system clock runs at 48MHz. PWM Timer runs at 50Hz
    //T2CONbits.TCKPS = 0b111;     // 1:256 timer input clock prescaler
    //PR2 = 3749;              // PR = PBCLK / N / desired F - 1 = 48000000/256/50-1 = 3749
    //TMR2 = 0;                // initial TMR2 count is 0
    //OC2CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC2CON bits are defaults
    //OC2RS = 0;             // duty cycle = OC2RS/(PR2+1)
    //OC2R = 0;              // initialize before turning OC2 on; afterward it is read-only
    //T2CONbits.ON = 1;        // turn on Timer2
    //OC2CONbits.ON = 1;       // turn on OC2
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
        if(PORTBbits.RB6 == 0 && PORTBbits.RB7 == 0){ // pull-up resistor, to GND when button pushed
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < ((48000000/2)/10000)) {   // wait .1 ms to remove switch bounce
            }
            
            if(PORTBbits.RB6 == 0 && PORTBbits.RB7 == 0) {  // check if all switches are low (pushed)
                _CP0_SET_COUNT(0);
                while(_CP0_GET_COUNT() < (48000000)) {    // wait for 2 sec
                    if(PORTBbits.RB6 == 1 || PORTBbits.RB7 == 1) {   // if any switch is high, turn off output and break out from 2 second delay
                        int c = _CP0_GET_COUNT();
                        while(_CP0_GET_COUNT() < (c+((48000000/2)/10000))) {   // wait .1 ms to remove switch bounce
                        }
                        LATBbits.LATB10 = 0;
                        break;
                    }
                }
                if(PORTBbits.RB6 == 0 && PORTBbits.RB7 == 0) {  // if all switches are still low (pushed)
                    LATBbits.LATB10 = 1;
                }
            }
            else {
                LATBbits.LATB10 = 0;
            }
        }
        else {
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < ((48000000/2)/10000)) {   // wait .1 ms to remove switch bounce
            }
            if(PORTBbits.RB6 == 1 || PORTBbits.RB7 == 1){   // if switch is indeed high, turn off output
                LATBbits.LATB10 = 0;
            }
        }
    }
}