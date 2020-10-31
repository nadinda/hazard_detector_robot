/* TI RSLK MAX Hazard Detector
 * Sneha Iyer, Nadinda, YuJu and Juliana Brum
 * 31/10/20
 * Code for TI RSLK for oil/fluid spill detection in factories
 * using IR and Bumper sensors 
 * v1.0 31/10/20 Team Hexenkraft 
 */

#include "msp.h"
#include <stdint.h>
unsigned long i;
unsigned int time=1000;
unsigned long readout=0;
unsigned long readoutleft=0;
unsigned long readoutright=0;
unsigned long cycles;
unsigned long new_cycles;
unsigned long direction;
unsigned long drive;

int main(void) {                            // this code checks the line sensor signal and decides to activate the left motor, the right motor, or both
    volatile uint32_t i;                    // It should move forward turn left, turn right and go back to start position

    WDT_A->CTL = WDT_A_CTL_PW |             // Stop WDT
                 WDT_A_CTL_HOLD;

    P5->DIR = 0x38;                         // P5.4 and P5.5 motor drive as output and set P5.3 IR LED output driver
    P2->DIR = 0xC0;                         // P2.6 and P2.7 as output set to output driving the motor
    P3->DIR = 0xC0;                         // P3.6 and P3.7 as output to control motor deriver sleep mode
    P8->DIR = 0xA0;                         // LED P8.5 and 8.7
    P8->OUT = 0x00;                         //LEDs P8.5 and 8.7 set to low
    P9->DIR = 0x04;                         // P9.2 LED driver control set to output driver
    P9->OUT = 0x00;                         // P9.2 clear LED driver control
    P4->DIR = 0x00;                         // P4.0 to P4.7 as input -> Interrupt setup for bumper switches
    P4->OUT = 0xED;                         // set pull-up function on P4.0 to P4.7 without P4.1 and P4.4
    P4->REN = 0xFF;                         // enable pull-up/pull-down on P4.0 to P4.7
    P4->IES = 0xFF;                         // Input edge select 1->0
    P4->IFG = 0x00;                         // clear pending interrupt flags
    P5->OUT = 0x00;                         // P5.4 and P5.5 set to low -> set direction to forward for left (P5.4) and right (5.5) motor
    P3->OUT = 0xC0;                         // P3.6 and P3.7 set to high -> disable sleep mode for left (P3.7) and right (3.6) motor
    P2->OUT = 0x00;                         // P2.6 and P2.7 set to zero
    NVIC->ISER[1] = 1 << ((PORT4_IRQn) & 31);   // Enable Port 4 interrupt on the NVIC
    P4->IE = 0xED;                              // Set Port 4 interrupt enable bits

    while(1)
    {                                       
        P5->DIR = 0x38;                     // set P5.3 (IR LED) as output
        P5->OUT = 0x08;                     // turn on P5.3 (IR LED)
        P9->OUT = 0x04;                     // turn on P9.2 (IR LED)
        P7->DIR = 0xFF;                     // P7.0 to P7.7 (IR Sensors) as output
        P7->OUT = 0xFF;                     // turn on P7 to charge caps
        __delay_cycles(10);                 // wait 10 x 1us before start
        P7->DIR = 0x00;                     // P7.0 to P7.7 (IR Sensors) as inputs
        __delay_cycles(2000);               // wait 1000 x 1us for caps to discharge
         readout = P7->IN;                  // read IR sensor values
        P5->OUT = 0x00;                     // turn off P5.3 (IR LED)
        readoutleft = readout/16;           // separate left and right side of line sensor
        readoutright = readout%16;
        
        if(readout!= 0)
        {
            i = 1000;                              // set i to 1000 -> number of cycles
            while(i > 0)                           // in this loop the forward moving is realized with a PWM signal
            {
                __delay_cycles(5);                  // wait x cycles
                P2->OUT = 0xC0;                     // P2.6 and P2.7 set to high -> enable signal for left (P2.7) and right (2.6) motor
                P2->OUT = 0;                        // P2.6 and P2.7 set to low -> no signal for left (P2.7) and right (2.6) motor
                i--;                                // decrement cycle counter
            }
        }
        else
        {
            i = 100000;                              // set i to 100000 -> number of cycles
            while(i > 0)                            // in this loop the forward moving is realized with a PWM signal
            {
                P8->OUT = 0x80;                      //turns the led P8.7 on                  
                __delay_cycles(100);                  // wait x cycles
                P2->OUT = 0xC0;                    // P2.6 and P2.7 set to high -> enable signal for left (P2.7) and right (2.6) motor
                P2->OUT = 0;                        // P2.6 and P2.7 set to low -> no signal for left (P2.7) and right (2.6) motor
                P8->OUT = 0;                        //P8.7 set to low
                i--;                                // decrement cycle counter
                
            }
            
        }

    }
}
int drive_back(void){
    while(cycles > 0)                           // cycles defined in ISR
    {
        P8->OUT = 0x20;                         //Turns the led P8.5 on as an indicator to the control room or send a message to the serial monitor
        P2->OUT = 0;                            // P2.6 and P2.7 set to low -> no signal for left (P2.7) and right (2.6) motor
        __delay_cycles(5);                      // wait 5 cycles
        P5->OUT = direction;                    // change direction of left/right motor to backwards
        P2->OUT = drive;                   		// set left/right motor signal (P2.6/7) to high
        P2->OUT = 0;                            // P2.6 and P2.7 set to low -> no signal for left (P2.7) and right (2.6) motor
        P8->OUT = 0;                            //P8.5 set to low
        cycles--;                               // decrement cycle counter
    }
    
    return;
}

void PORT4_IRQHandler(void)                     // Port4 ISR //
{
    if(P4->IFG){                                // check if one bumpers switch high
        
        cycles = 70000;                    	    // Turn 70000 cycles ~ 90 degrees
        direction = 0x20;                       // Used for turn bit of one motor P5 to make a turn
        drive = 0xC0;                           // Motor drive bit P2
        drive_back();                           // call function for driving backward
        P4->IFG =0;                             // clear all interrupt flags
        P8->OUT = 0;                            //set P8.5 to low
    }
}

