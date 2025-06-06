/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: key.c
*
* PROJECT....: EMP
*
* DESCRIPTION: See module specification file (.h-file).
*
* Change Log:
*****************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 150321  MoH   Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "tmodel.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "glob_def.h"
#include "tmodel.h"
#include "Task.h"
#include "key.h"


extern QueueHandle_t xQueue_keypad;
extern SemaphoreHandle_t xSemaphore_keypad;


INT8U row( INT8U y )
{
  INT8U result = 0;

  switch( y )
  {
    case 0x01: result = 1; break;
    case 0x02: result = 2; break;
    case 0x04: result = 3; break;
    case 0x08: result = 4; break;
  }
  return( result );
}

INT8U key_catch( x, y )
INT8U x, y;
{
  const INT8U matrix[3][4] = {{'*','7','4','1'},
                              {'0','8','5','2'},
                              {'#','9','6','3'}};

  return( matrix[x-1][y-1] );
}


INT8U get_keyboard()
{
    INT8U ch;

    if(uxQueueMessagesWaiting(xQueue_keypad)){
        if( xSemaphoreTake( xSemaphore_keypad, portMAX_DELAY)){ //( TickType_t ) 100 ) == pdTRUE )
            if( xQueueReceive( xQueue_keypad, &ch, portMAX_DELAY )){
                xSemaphoreGive( xSemaphore_keypad );
                return ch;
            }
            xSemaphoreGive( xSemaphore_keypad );
        }
    }
    vTaskDelay(portTICK_RATE_MS); // wait 100-1000 ms. (200-1000)
    return 0;
}

BOOLEAN check_column(INT8U x)
{
    INT8U ch;

    if( xSemaphoreTake( xSemaphore_keypad, portMAX_DELAY)){ //( TickType_t ) 100 ) == pdTRUE )
        INT8U y = GPIO_PORTE_DATA_R & 0x0F;             // Save the values of the 4 bits for the rows
        if( y )                                         // If one of them are set...
        {                                               // ...we first find the row number with the function row()
            INT8U button = key_catch( x, row(y) );          // Now that we have the row and column we look up the corresponding character using the function key_catch
            xSemaphoreGive( xSemaphore_keypad );
            return( xQueueSend( xQueue_keypad, &button, portMAX_DELAY));
        }
        xSemaphoreGive( xSemaphore_keypad );
    }

    return 0;
}

#define DEBOUNCE_DELAY 50  // Delay in milliseconds
#define POLLING_INTERVAL 5 // Short delay to recheck button state

extern void key_task(void *pvParameters)
{
    INT8U my_state = 0;

    while (1)
    {
        switch (my_state)
        {
        case 0:
            GPIO_PORTA_DATA_R &= 0xE3; // Clear column bits
            GPIO_PORTA_DATA_R |= 0x10; // Activate column 1
            if (check_column(1))
            {
                vTaskDelay(DEBOUNCE_DELAY / portTICK_RATE_MS); // Debounce delay
                if (GPIO_PORTE_DATA_R & 0x0F)  // Re-check button state
                {
                    my_state = 1;  // Register button press
                }
                break;
            }

            GPIO_PORTA_DATA_R &= 0xE3;
            GPIO_PORTA_DATA_R |= 0x08; // Activate column 2
            if (check_column(2))
            {
                vTaskDelay(DEBOUNCE_DELAY / portTICK_RATE_MS);
                if (GPIO_PORTE_DATA_R & 0x0F)
                {
                    my_state = 1;
                }
                break;
            }

            GPIO_PORTA_DATA_R &= 0xE3;
            GPIO_PORTA_DATA_R |= 0x04; // Activate column 3
            if (check_column(3))
            {
                vTaskDelay(DEBOUNCE_DELAY / portTICK_RATE_MS);
                if (GPIO_PORTE_DATA_R & 0x0F)
                {
                    my_state = 1;
                }
                break;
            }
            break;

        case 1:
            while (GPIO_PORTE_DATA_R & 0x0F) // Wait until key is released
            {
                vTaskDelay(POLLING_INTERVAL / portTICK_RATE_MS);
            }
            my_state = 0; // Reset state after key release
            break;
        }
    }
}


