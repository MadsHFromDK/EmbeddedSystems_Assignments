/*
 * button.h
 *
 *  Created on: 22 Feb 2025
 *      Author: madsa
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "tm4c123gh6pm.h"
#include <stdint.h>

typedef enum{
        SINGLE_PRESS,
        DOUBLE_PRESS,
        LONG_PRESS,
        NONE
    } Event;

Event select_button();

#endif /* BUTTON_H_ */
