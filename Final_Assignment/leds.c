/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: leds.c
*
* PROJECT....: ECP
*
* DESCRIPTION: See module specification file (.h-file).
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 050128  KA    Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "Task.h"
#include "queue.h"
#include "semphr.h"
#include "emp_type.h"

#include "elevator.h"

/*****************************    Defines    *******************************/
extern LED_states led_state;
/*****************************   Constants   *******************************/

/*****************************   Variables   *******************************/

/*****************************   Functions   *******************************/

void red_led_init(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{
  INT8S dummy;
  // Enable the GPIO port that is used for the on-board LED.
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

  // Do a dummy read to insert a few cycles after enabling the peripheral.
  dummy = SYSCTL_RCGC2_R;

  GPIO_PORTF_DIR_R |= 0x02;
  GPIO_PORTF_DEN_R |= 0x02;
}

void yellow_led_init(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{
  INT8S dummy;
  // Enable the GPIO port that is used for the on-board LED.
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

  // Do a dummy read to insert a few cycles after enabling the peripheral.
  dummy = SYSCTL_RCGC2_R;

  GPIO_PORTF_DIR_R |= 0x04;
  GPIO_PORTF_DEN_R |= 0x04;
}

void green_led_init(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{
  INT8S dummy;
  // Enable the GPIO port that is used for the on-board LED.
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

  // Do a dummy read to insert a few cycles after enabling the peripheral.
  dummy = SYSCTL_RCGC2_R;

  GPIO_PORTF_DIR_R |= 0x08;
  GPIO_PORTF_DEN_R |= 0x08;
}

void init_leds(void){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Run the initializing functions
*****************************************************************************/

    red_led_init();
    yellow_led_init();
    green_led_init();
}

void led_task(void *pvParameters){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Task for handling LEDs
*****************************************************************************/

   init_leds();                                     // Initialize the LEDs

   static LED_states prev_state = RED;

   while(1){

       if(prev_state != led_state){                 // Reset the LEDs when state changes
           GPIO_PORTF_DATA_R = 0xFF;
       }

       switch(led_state){
            case GREEN:
                prev_state = led_state;
                GPIO_PORTF_DATA_R &= ~0x08;         // Make sure the green led bit is low - Active low
            break;

            case YELLOW:
                prev_state = led_state;
                GPIO_PORTF_DATA_R ^= 0x04;          // Toggle yellow LED to blink
            break;

            case RED:
                prev_state = led_state;
                GPIO_PORTF_DATA_R ^= 0x02;          // Toggle red LED to blink
            break;

            case FLASHING:
                prev_state = led_state;
                GPIO_PORTF_DATA_R ^= 0x02 | 0x04 | 0x08;        // Toggle all LEDs to blink
            break;
       }

       vTaskDelay( 200 / portTICK_RATE_MS);         // 200 ms delay
   }
}

/****************************** End Of Module *******************************/




