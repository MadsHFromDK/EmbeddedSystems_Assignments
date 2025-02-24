/*
 * button.c
 *
 *  Created on: 22 Feb 2025
 *      Author: madsa
 */

#include "button.h"

#define TIM_200_MS = 40
#define TIM_1_SEC = 200

Event select_button(){

    static int button_pressed = 0;
    static int double_click = 0;
    static int double_timer = 0;
    static int hold_timer = 0;

    if(!(GPIO_PORTF_DATA_R & 0x10)){
        if(!double_click){
            button_pressed = 1;
            hold_timer = 3; //400
            double_click = 1;
            double_timer = 50;
            GPIO_PORTF_DATA_R = 0x0;
            return SINGLE_PRESS;
        }
        else if(/*double_click &&*/ double_timer && !(button_pressed)){
            double_click = 0;
            double_timer = 0;
            return DOUBLE_PRESS;
        }
        else if(button_pressed && (hold_timer == 1)){
            return LONG_PRESS;
        }
    }
    else{
        button_pressed = 0;
        hold_timer = 0;

        if(!double_timer){
            double_click = 0;
        }
    }

    hold_timer--;
    double_timer--;

    return NONE;
}


