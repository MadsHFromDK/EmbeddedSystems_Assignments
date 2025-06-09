/*
 * btn.c
 *
 *  Created on: 13 May 2025
 *      Author: madsa
 */

#ifndef BTN_C_
#define BTN_C_

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "glob_def.h"
#include "systick_frt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tmodel.h"
#include "semphr.h"

#include "btn.h"
#include "rotary_encoder.h"
#include "elevator.h"

#define BUTTON_CHECK_INTERVAL_MS 50
#define REQUIRED_PRESS_TIME_MS 600

extern QueueHandle_t xQueue_lcd, xQueue_button;
extern elevator_states elevator_state;
extern struct elevator_data data;

INT8U button_pushed()
{
  return( !(GPIO_PORTF_DATA_R & 0x10) );                        // Return button pressed bit, not'ed as it is active low.
}

void switch_task(void *pvParameters){

INT8U TF;
INT16U long_press_check = 0;

    while(1){
        if(button_pushed()){
            long_press_check += BUTTON_CHECK_INTERVAL_MS;       // Increment time button has been pressed
            if(long_press_check >= REQUIRED_PRESS_TIME_MS){     // Check if button has been pressed longer than threshold
                if(data.CUR_FLOOR == data.ELE_FLOOR){           // If the elevator is on the same floor as the person, then the elevator should go to code input - Locked state
                    TF = FALSE;
                    xQueueSend( xQueue_button, &TF, portMAX_DELAY);
                }
                else{
                    data.FLOOR_SELECTION = data.CUR_FLOOR;      // If elevator is not on the current floor, the elevator should be called to the current floor
                    data.SELECTED = TRUE;
                    xQueueReset(xQueue_lcd);
                }
           }
        }
        else{
            long_press_check = 0;                               // Reset long press counter if not pressed
        }

       vTaskDelay(pdMS_TO_TICKS(BUTTON_CHECK_INTERVAL_MS)); // Delay before checking again

    }
}



#endif /* BTN_C_ */
