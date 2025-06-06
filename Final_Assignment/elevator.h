/*
 * elevator.h
 *
 *  Created on: 9 May 2025
 *      Author: madsa
 */

#ifndef ELEVATOR_H_
#define ELEVATOR_H_

typedef enum{
    AWAY,
    AVAILABLE,
    MOVING,
    REPAIR,
    LOCKED,
    HOME
} elevator_states;

extern elevator_states elevator_state;
extern SemaphoreHandle_t xSemaphore_ele_state, xSemaphore_ele_flr;

typedef enum{
    GREEN,
    RED,
    YELLOW,
    FLASHING
} LED_states;

extern LED_states led_state;

void elevator_task(void *pvParamters);

#endif /* ELEVATOR_H_ */
