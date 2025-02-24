

/**
 * main.c
 */

#include "systick.h"
#include "counter.h"
#include "button.h"
#include "color.h"

#define TIM_1_SEC   200

extern int ticks;

int main(void)
{

    int alive_timer = TIM_1_SEC;

    init_systick();

    int dummy;
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;        // enable the GPIO port that is used for the on-board LEDs and switches
    dummy = SYSCTL_RCGC2_R;                     // dummy read to insert a few cycles after enabling the peripheral
    GPIO_PORTF_DIR_R = 0x0E;                    // set the direction as output for LED pins on PortF (PF1 - PF3)
    GPIO_PORTF_DEN_R = 0x1E;                    // enable the GPIO pins for digital function (PF1 - PF4)
    GPIO_PORTF_PUR_R = 0x10;                    // enable internal pull-up resistor for switch (PF4)

    Event event = SINGLE_PRESS;
    Event temp = NONE;

    while(1)                                    // loop forever
    {
        while( !ticks );                        // Delay should always be 5ms; While ticks is not larger than 0, wait

        //Decrement ticks
        ticks--;

        if( ! --alive_timer ){

          temp = select_button();

          alive_timer = TIM_1_SEC;

          if(!(temp == NONE)){
              event = temp;
          }

          color_handler(counter(event), event);
        }
    }
}
