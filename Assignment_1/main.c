

/**
 * main.c
 */

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "systick.h"

#define TIM_1_SEC     200
#define TIM_200_MS    40
#define COUNTER_MAX   0x7

extern int ticks;

int main(void)
{

    int double_timer = 0;
    int auto_timer = 0;

    init_systick();

    int dummy;
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;        // enable the GPIO port that is used for the on-board LEDs and switches
    dummy = SYSCTL_RCGC2_R;                     // dummy read to insert a few cycles after enabling the peripheral
    GPIO_PORTF_DIR_R = 0x0E;                    // set the direction as output for LED pins on PortF (PF1 - PF3)
    GPIO_PORTF_DEN_R = 0x1E;                    // enable the GPIO pins for digital function (PF1 - PF4)
    GPIO_PORTF_PUR_R = 0x10;                    // enable internal pull-up resistor for switch (PF4)

    int counter = COUNTER_MAX;
    int color = 0;                              // Integer for color management.
    int up_down = 1;                            // Boolean indicating counting direction, assumed true, meaning counting down.
    int button_pressed = 0;                     // Boolean for button bouncing fixing.
    int double_click = 0;                       // Boolean used to register double clicks.
    int auto_bool = 0;                          // Boolean used to configure to auto mode

    while(1)                                    // loop forever
    {
        while( !ticks );                        // Delay should always be 5ms; While ticks is not larger than 0, wait

        //Decrement ticks
        ticks--;

        //Reset the color variable
        color = 0x0;

        //If button pressed run this section
        if(!(GPIO_PORTF_DATA_R & 0x10)){

            //Set double timer, 200 ms
            double_timer = TIM_200_MS;

            //If button have been held for 2 seconds, then set auto true
            if(auto_timer == 1){
                auto_bool = 1;
            }

            //Button bouncing fixing, only always this part to run once per button press.
            if(!button_pressed){

                //Set button pressed true
                button_pressed = 1;

                //Set the auto timer, for press and hold (2 seconds)
                auto_timer = 2 * TIM_1_SEC;

                //If button is pressed and auto mode is on, then it should be disabled
                if(auto_bool){
                    auto_bool = 0;
                }

                //If the double timer is alive, and double click is set, then run this part
                if(--double_timer && double_click){
                    //Toggle up_down bool, deciding the counting direction
                    up_down ^= 0x1;

                    //Reset double click bool
                    double_click = 0;

                    //Reset double timer
                    double_timer = 0;
                }

                //Counter handling - Count in the direction decided by up_down
                if(up_down){
                    //If counting down, and counter is larger than 0, then count down can be performed
                    if(counter > 0x0){
                        counter--;
                    }
                    //If counter is == 0x0 while counting down, counter should be set to top of interval again
                    else{
                        counter = 0x7;
                    }
                }
                //Same as for counting down, just opposite direction
                else{
                    if(counter < 0x7){
                        counter++;
                    }
                    else{
                        counter = 0x0;
                    }
                }
            }

            //Should wait one cycle, to correctly read double timer, which can be edited right before.
            dummy = 0x1;

            //If double timer is alive, then double click should be set, meaning a double click has been initiated
            //but not performed.
            if(--double_timer){
                double_click = 1;
            }
        }
        else{
            //Button bouncing fixing, sets button pressed to false, when not pressed
            button_pressed = 0;

            //If the double timer is not alive, then double click should be disabled
            if(! --double_timer){
                double_click = 0;
            }
        }

        //If auto timer is not alive and auto bool is true, then run this part:
        if( ! --auto_timer && auto_bool)
        {
            //Set/revive auto timer
            auto_timer = TIM_1_SEC;

            //Counter handling - Should be replaced with function, to not have code repetition from previous section.
            if(up_down){
                if(counter > 0x0){
                    counter--;
                }
                else{
                    counter = 0x7;
                }
            }
            else{
                if(counter < 0x7){
                    counter++;
                }
                else{
                    counter = 0x0;
                }
            }
        }

          //If first bit is set, then add red to color:
          if(counter & ~(0x3)){
              color |= 0x2;
          }

          //If second bit is set, then add blue to color:
          if(counter & ~(0x5)){
              color |= 0x4;
          }

          //if third bit is set, then add green to color:
          if(counter & ~(0x6)){
              color |= 0x8;
          }

          //Display the color:
          GPIO_PORTF_DATA_R = color; //counter; //Toggles, with ^=; 0x08: Green, 0x04: Blue, 0x02: Red
    }
}
