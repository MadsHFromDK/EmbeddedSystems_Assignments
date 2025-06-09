/***************************** Include files *******************************/
#include <stdint.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "lcd.h"
#include "glob_def.h"
#include "tmodel.h"
#include "FreeRTOS.h"
#include "Task.h"
#include "queue.h"
#include "semphr.h"
#include "glob_def.h"

#include "rotary_encoder.h"
#include "elevator.h"

/*****************************   Variables   *******************************/
extern QueueHandle_t xQueue_elevator, xQueue_encoder;
extern SemaphoreHandle_t xSemaphore_lcd, xSemaphore_enc;
extern elevator_states elevator_state;
extern LED_states led_state;

encoder_state state;
INT8S encoder_cnt = 0;

struct elevator_data data;


char change_int_to_char(INT8U number){
/*****************************************************************************
*   Input    :  integer
*   Output   :  char
*   Function :  Changes integer to char, of numbers 0 to 9
*****************************************************************************/
    switch(number){
        case 0:
            return '0';
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
        case 4:
            return '4';
        case 5:
            return '5';
        case 6:
            return '6';
        case 7:
            return '7';
        case 8:
            return '8';
        case 9:
            return '9';
    }
}

void init_strct(void){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Initializes the values in the elevator data struct
*****************************************************************************/

    data.NUM_TRIPS = 0;
    data.CUR_FLOOR = 0;
    data.ELE_FLOOR = 2;
    data.SELECTED = FALSE;
    data.FLOOR_SELECTION = 0;
    data.REP_DIR = RIGHT;
}


void handle_encoder(void){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Handles the value of encoder
*****************************************************************************/

    INT8U pressed = !(GPIO_PORTA_DATA_R & 0x80);
    INT8U TF;
    static INT8U btn_pressed = FALSE;

    switch(elevator_state){
    case AVAILABLE:
        if(pressed && !btn_pressed && encoder_cnt != 13){   // If button is pressed, and is allowed
            data.FLOOR_SELECTION = encoder_cnt;                     // Update floor selection for elevator
            data.SELECTED = TRUE;                                   // Allow the elevator to start working

            btn_pressed = TRUE;                                     // Set button debounce value
        }
        else if(!pressed && btn_pressed){
            btn_pressed = FALSE;
        }
    break;

    case REPAIR:
        if(pressed && !btn_pressed){
            if(data.REP_DIR == RIGHT){                          // If the repair direction is RIGHT
                if(encoder_cnt >= REPAIR_VAL){                  // Check if the value corresponds to fixed
                    led_state = GREEN;                          // Stop flashing lights

                    data.NUM_TRIPS = 0;                         // Reset the amount of trips until elevator breaks
                    data.REP_DIR = LEFT;                        // Set repair direction to LEFT instead of RIGHT, for next repair

                    if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                        TF = FALSE;
                        xQueueSend( xQueue_elevator, &TF, portMAX_DELAY); // Switch elevator state to LOCKED
                        xSemaphoreGive( xSemaphore_ele_state );
                    }

                    if(xSemaphoreTake(xSemaphore_enc, portMAX_DELAY)){
                        encoder_cnt = data.CUR_FLOOR;           // Set the encoder count to reflect the current floor
                        xSemaphoreGive( xSemaphore_enc );
                    }
                }
            }
            else{
                if(encoder_cnt <= -(INT8S)REPAIR_VAL){          //  Negative values, so should make sure the value is correct format
                    led_state = GREEN;

                    data.NUM_TRIPS = 0;
                    data.REP_DIR = RIGHT;

                    if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                        TF = FALSE;
                        xQueueSend( xQueue_elevator, &TF, portMAX_DELAY); // Switch elevator state to LOCKED
                        xSemaphoreGive( xSemaphore_ele_state );
                    }

                    if(xSemaphoreTake(xSemaphore_enc, portMAX_DELAY)){
                        encoder_cnt = data.CUR_FLOOR;
                        xSemaphoreGive( xSemaphore_enc );
                    }
                }
            }

            btn_pressed = TRUE;                     // Button has been pressed
        }
        else if(!pressed && btn_pressed){           // If button is not pressed, but has been pressed, allow it to register a new press
            btn_pressed = FALSE;
        }
    break;
    }
}

void rotary_encoder_task(void *pvParameters){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Task to handle encoder
*****************************************************************************/
    INT8U TF;
    static INT8U prev_AB = 0;                                   // Previous state of Phase A and Phase B

    init_strct();                                               // Initialize the struct, for data storage

    while(1){
        // Algorithm for encoder can be found in encoder data sheet
        INT8U A_in = (GPIO_PORTA_DATA_R & 0x20) ? 1 : 0;        // Read PA5 - as a value of '1' or '0'
        INT8U B_in = (GPIO_PORTA_DATA_R & 0x40) ? 1 : 0;        // Read PA6

        INT8U current_AB = (A_in << 1) | B_in;

        if(uxQueueMessagesWaiting(xQueue_encoder)){
            if(xSemaphoreTake(xSemaphore_enc, portMAX_DELAY)){
                if(xQueueReceive( xQueue_encoder, &TF, portMAX_DELAY)){

                    if(TF == TRUE){
                        encoder_cnt = 0;
                        data.dir = RIGHT;
                    }
                    else{
                        encoder_cnt = 0;
                        data.dir = LEFT;
                    }
                }

                xSemaphoreGive( xSemaphore_enc );
            }
        }
        // If current AB and prev AB is not identical, then a change has happened - Check also if the button is pressed
        else if (current_AB != prev_AB || !(GPIO_PORTA_DATA_R & 0x80)) {
            INT8U transition = current_AB ^ prev_AB;            // Log the change in encoder position

            if (A_in == B_in){
                if (transition == 0b10){                        // Counter clock wise rotation of encoder
                    if(xSemaphoreTake(xSemaphore_enc, portMAX_DELAY)){
                        data.dir = LEFT;                            // Set the rotation direction - shared
                        encoder_cnt--;                              // Decrement encoder position
                        xSemaphoreGive( xSemaphore_enc );
                    }
                } else if (transition == 0b01){                 // Clock wise rotation of encoder
                    if(xSemaphoreTake(xSemaphore_enc, portMAX_DELAY)){
                        data.dir = RIGHT;
                        encoder_cnt++;                              // Increment encoder position
                        xSemaphoreGive( xSemaphore_enc );
                    }
                }
            }

            if(elevator_state != REPAIR){                       // Set limit for min and max value for encoder, if not in repair state
                if(encoder_cnt > ENCODER_MAX_VAL){
                    if(xSemaphoreTake(xSemaphore_enc, portMAX_DELAY)){
                        encoder_cnt = ENCODER_MAX_VAL;
                        xSemaphoreGive( xSemaphore_enc );
                    }
                }
                else if(encoder_cnt < 0){
                    if(xSemaphoreTake(xSemaphore_enc, portMAX_DELAY)){
                        encoder_cnt = 0;
                        xSemaphoreGive( xSemaphore_enc );
                    }
                }
            }

            prev_AB = current_AB;                               // Update the previous AB value

            handle_encoder();                                   // Call handle encoder function
        }

        vTaskDelay(1 / portTICK_RATE_MS);                       // 1 ms delay
    }
}
