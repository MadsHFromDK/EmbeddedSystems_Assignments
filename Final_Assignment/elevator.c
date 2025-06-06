/*
 * elevator.c
 *
 *  Created on: 9 May 2025
 *      Author: madsa
 */

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "Task.h"
#include "queue.h"
#include "semphr.h"
#include "emp_type.h"
#include "glob_def.h"

#include "elevator.h"
#include "lcd.h"
#include "rotary_encoder.h"
#include "adc.h"

/*****************************   Variables   *******************************/
extern struct elevator_data data;
extern struct adc_data data_adc;
extern QueueHandle_t xQueue_lcd, xQueue_elevator;
extern INT8S encoder_cnt;

LED_states led_state;
elevator_states elevator_state;

void init_elevator_state( void ){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Initialize the elevator state enum
*****************************************************************************/

    elevator_state = AWAY;
}

void elevator_task(void *pvParamters){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Elevator task
*****************************************************************************/

    init_elevator_state();

    FP32 vel = 0.0, acc = 0.5, acc_flr;
    static INT8U ele_flr, trgt_flr;
    static BOOLEAN start = FALSE, dir_up;
    INT16U val;
    INT8U digit1;
    INT8U digit2;
    char char_digit1;
    char char_digit2;

    INT8U TF;

    while(1){

        if(uxQueueMessagesWaiting(xQueue_elevator)){
            if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                if(xQueueReceive( xQueue_elevator, &TF, portMAX_DELAY)){
                    if(TF == TRUE){
                        elevator_state = AVAILABLE;
                    }
                    else{
                        elevator_state = LOCKED;
                    }
                    xSemaphoreGive(xSemaphore_ele_state);
                }
            }
        }
        else if(elevator_state == AVAILABLE || elevator_state == AWAY || elevator_state == MOVING){
            if(data.SELECTED && !start){                                    // If data has been selected and the elevator has not ran the start sequence
                start = TRUE;

                if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                    ele_flr = data.ELE_FLOOR;
                    xSemaphoreGive( xSemaphore_ele_flr );
                }

                trgt_flr = data.FLOOR_SELECTION;

                if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                    elevator_state = MOVING;
                    xSemaphoreGive( xSemaphore_ele_state );
                }

                if(trgt_flr > ele_flr){                                     // Calculate the floor at which the elevator will be accelerating until, based on the target floor and current floor
                    dir_up = TRUE;                                          // Set travel direction, used for incrementing or decrementing elevator floor position
                    if((trgt_flr + 1) != ele_flr){                          // If the difference between target and current floor is larger than one
                        acc_flr = ele_flr + (trgt_flr - ele_flr)/2;
                    }
                    else{
                        acc_flr = ele_flr;
                    }
                }
                else{
                    dir_up = FALSE;
                    if((trgt_flr -1) != ele_flr){
                        acc_flr = trgt_flr + (ele_flr - trgt_flr)/2;
                    }
                    else{
                        acc_flr = ele_flr;
                    }
                }
            }
            else if(elevator_state == MOVING){      // If elevator is in MOVING state

                if(dir_up == TRUE){                 // Moving upwards
                    if(ele_flr < acc_flr){          // If accelerating
                        if(vel < 3){                // Max velocity is 3
                            vel += acc;
                        }

                        if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                            ele_flr += vel;
                            xSemaphoreGive( xSemaphore_ele_flr );
                        }

                        led_state = YELLOW;         // Blink yellow when accelerating
                    }
                    else{                           // If decelerating
                        if(vel > 1){
                            vel -= acc;
                        }

                        if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                            ele_flr += vel;
                            xSemaphoreGive( xSemaphore_ele_flr );
                        }

                        led_state = RED;            // Blink red
                    }
                }
                else{                               // Moving downwards
                    if(ele_flr > acc_flr){
                        if(vel < 3){
                            vel += acc;
                        }

                        if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                            ele_flr -= vel;
                            xSemaphoreGive( xSemaphore_ele_flr );
                        }

                        led_state = YELLOW;
                    }
                    else{

                        if(vel > 1){
                            vel -= acc;
                        }

                        if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                            ele_flr -= vel;
                            xSemaphoreGive( xSemaphore_ele_flr );
                        }

                        led_state = RED;
                    }
                }

            move_LCD(0,1);                                              // Position LCD cursor

                digit1 = ele_flr/10;                                    // Show the traveling information
                digit2 = ele_flr%10;                                    // Calculate each digit

                char_digit1 = change_int_to_char(digit1);               // Convert to char
                char_digit2 = change_int_to_char(digit2);

                xQueueSend( xQueue_lcd, &char_digit1, portMAX_DELAY );
                xQueueSend( xQueue_lcd, &char_digit2, portMAX_DELAY );

                move_LCD(8,1);

                xQueueSend( xQueue_lcd, "v", portMAX_DELAY );
                xQueueSend( xQueue_lcd, "e", portMAX_DELAY );
                xQueueSend( xQueue_lcd, "l", portMAX_DELAY );
                xQueueSend( xQueue_lcd, ":", portMAX_DELAY );
                xQueueSend( xQueue_lcd, " ", portMAX_DELAY );

                digit1 = vel;
                digit2 = (INT8U)(vel*10)%10;

                char_digit1 = change_int_to_char(digit1);
                char_digit2 = change_int_to_char(digit2);

                xQueueSend( xQueue_lcd, &char_digit1, portMAX_DELAY );
                xQueueSend( xQueue_lcd, &char_digit2, portMAX_DELAY );

                if(dir_up == TRUE){
                    if(ele_flr >= trgt_flr){                                    // Reset information for next trip
                        vel = 0;
                        start = FALSE;

                        data.NUM_TRIPS++;
                        data.SELECTED = FALSE;

                        if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                            data.ELE_FLOOR = ele_flr;
                            xSemaphoreGive( xSemaphore_ele_flr );
                        }

                        data.CUR_FLOOR = data.FLOOR_SELECTION;
                        led_state = GREEN;

                        if(data.CUR_FLOOR == 20){                               // Handle state switching
                            if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                                elevator_state = HOME;
                                xSemaphoreGive( xSemaphore_ele_state );
                            }
                        }
                        else if(data.NUM_TRIPS == 4){
                            encoder_cnt = 0;
                            led_state = FLASHING;
                            if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                                elevator_state = REPAIR;
                                xSemaphoreGive( xSemaphore_ele_state );
                            }

                            val = data_adc.val;                          // Use current potentiometer position to make "truly" random value for repair

                            if(val < 200){
                                data_adc.trgt_val = rand()%(val*2);
                            }
                            else if (val >200 && val < 400){
                                data_adc.trgt_val = rand()%(val/2);
                            }
                        }
                        else if(data.CUR_FLOOR == data.ELE_FLOOR){
                            if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                                elevator_state = LOCKED;
                                xSemaphoreGive( xSemaphore_ele_state );
                            }
                        }
                    }
                }
                else if(ele_flr <= trgt_flr){
                    vel = 0;

                    data.NUM_TRIPS++;
                    data.SELECTED = FALSE;

                    if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                        data.ELE_FLOOR = ele_flr;
                        xSemaphoreGive( xSemaphore_ele_flr );
                    }

                    data.CUR_FLOOR = data.FLOOR_SELECTION;
                    start = FALSE;
                    led_state = GREEN;

                    if(data.CUR_FLOOR == 20){
                        if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                            elevator_state = HOME;
                            xSemaphoreGive( xSemaphore_ele_state );
                        }
                    }
                    else if(data.NUM_TRIPS == 4){
                        encoder_cnt = 0;
                        led_state = FLASHING;
                        if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                            elevator_state = REPAIR;
                            xSemaphoreGive( xSemaphore_ele_state );
                        }

                        val = data_adc.val;

                        if(val < 200){
                            data_adc.trgt_val = rand()%(val*2);
                        }
                        else if (val >200 && val < 400){
                            data_adc.trgt_val = rand()%(val/2);
                        }
                    }
                    else if(data.CUR_FLOOR == data.ELE_FLOOR){
                        if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                            elevator_state = LOCKED;
                            xSemaphoreGive( xSemaphore_ele_state );
                        }
                    }
                }

                vTaskDelay( 1000 / portTICK_RATE_MS);
            }
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

