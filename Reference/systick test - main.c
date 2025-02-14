#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "systick.h"

#define TIM_1_SEC   200

extern int ticks;

int main(void)
{
    int alive_timer = TIM_1_SEC;

    init_systick();

    int dummy;

        //Enable the GPI0 port that is used for the on-board LEDs and switches.
        SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

        //Do a dummy read to insert a few cycles after enabling the peripheral.
        dummy = SYSCTL_RCGC2_R;

        //Set the direction as output (PF1-PF3)
        GPIO_PORTF_DIR_R = 0x0E; //(Direction Register - set Switches as input and LEDs as output)

        //Enable the GPI0 pins for digital function (PF1-PF4)
        GPIO_PORTF_DEN_R = 0x1E; //(Digital Enable Register)

        GPIO_PORTF_PUR_R = 0x10; //(Enable Pull-Up Resistor Register)

    //One button:
    while(1){

        while( !ticks );

        if( ! --alive_timer )
                {
                  alive_timer        = TIM_1_SEC;
                  GPIO_PORTF_DATA_R ^= 0x04;
                }

            /*if(GPIO_PORTF_DATA_R & 0x10){
                GPIO_PORTF_DATA_R = 0x02;
            }
            else{
                GPIO_PORTF_DATA_R = 0x04;
            }*/
    };

    return 0;
}

