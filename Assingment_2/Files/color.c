/*
 * color.c
 *
 *  Created on: 22 Feb 2025
 *      Author: madsa
 */

#include "color.h"

void color_handler(int counter, Event event){

    static int dir = 0;
    static int prev_counter = 0;

    if(event == LONG_PRESS){

        if(counter < prev_counter){
            dir = 1;
        }
        else{
            dir = 0;
        }

        if((counter & 0x1) && !(counter & 0x2)){
                GPIO_PORTF_DATA_R = 0x2;
            }
            else if(!(counter & 0x1) && (counter & 0x2)){
                if(dir){
                    GPIO_PORTF_DATA_R = (0x8 | 0x2);
                }
                else{
                    GPIO_PORTF_DATA_R = (0x4 | 0x2);
                }
            }
            else{
                GPIO_PORTF_DATA_R = 0x8;
            }

        prev_counter = counter;
    }
    else if(event == SINGLE_PRESS){
        GPIO_PORTF_DATA_R ^= (0x8 | 0x2);
    }
    else{
        GPIO_PORTF_DATA_R = 0x2;
    }
}


