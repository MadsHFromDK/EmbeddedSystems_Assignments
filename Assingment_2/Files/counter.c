/*
 * counter.c
 *
 *  Created on: 22 Feb 2025
 *      Author: madsa
 */

#include "counter.h"

#define COUNTER_MAX   0x3
#define COUNTER_MIN   0x1

int counter(Event event){
    static int counter = 0;
    static int dir = 0;

    if(dir){
        if(counter > COUNTER_MIN){
                counter--;
            }
            //If counter is == 0x0 while counting down, counter should be set to top of interval again
            else{
                //counter = COUNTER_MAX;
                dir ^= 0x1;
            }
    }
    else{
        if(counter < COUNTER_MAX){
            counter++;
        }
        else{
            dir ^= 0x1;
        }
    }

    return counter;
}

