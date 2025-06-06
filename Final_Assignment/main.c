/**
 * main.c
 */

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

#include "UI_task.h"
#include "key.h"
#include "leds.h"
#include "elevator.h"
#include "gpio.h"
#include "rotary_encoder.h"
#include "lcd.h"
#include "adc.h"
#include "btn.h"

#define USERTASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define IDLE_PRIO 0
#define LOW_PRIO  1
#define MED_PRIO  2
#define HIGH_PRIO 3

#define QUEUE_LEN   128

QueueHandle_t xQueue_keypad, xQueue_lcd, xQueue_elevator, xQueue_encoder;
SemaphoreHandle_t xSemaphore_keypad, xSemaphore_lcd, xSemaphore_ele_state, xSemaphore_ele_flr, xSemaphore_enc;

static void setupHardware(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{

  // Warning: If you do not initialize the hardware clock, the timings will be inaccurate
  init_systick();
  init_gpio();
  init_adc();

  xQueue_lcd = xQueueCreate( QUEUE_LEN , sizeof( INT8U ));
  xQueue_keypad = xQueueCreate( QUEUE_LEN , sizeof( INT8U ));
  xQueue_elevator = xQueueCreate( 1 , sizeof( INT8U ));
  xQueue_encoder = xQueueCreate( 1 , sizeof( INT8U ));
  xSemaphore_lcd = xSemaphoreCreateMutex();
  xSemaphore_keypad = xSemaphoreCreateMutex();
  xSemaphore_ele_state = xSemaphoreCreateMutex();
  xSemaphore_ele_flr = xSemaphoreCreateMutex();
  xSemaphore_enc = xSemaphoreCreateMutex();
}

int main(void)
{
    setupHardware();

    xTaskCreate(elevator_task, "ELEVATOR", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    xTaskCreate(rotary_encoder_task, "ENCODER", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    xTaskCreate(adc_task, "ADC", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    xTaskCreate(lcd_task, "LCD", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    xTaskCreate(key_task, "KEYPAD",USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    xTaskCreate(led_task, "LED", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    xTaskCreate(switch_task, "LONGPRESS", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
    xTaskCreate(UI_task, "UI",USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);

    vTaskStartScheduler();
    return 0;
}
