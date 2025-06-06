// -----------------
// adc.c
// -----------------
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "glob_def.h"
#include "tmodel.h"
#include "FreeRTOS.h"
#include "Task.h"
#include "queue.h"
#include "semphr.h"
#include "glob_def.h"

#include "adc.h"

struct adc_data data_adc;               // Struct for sharing adc value

INT16U get_adc()
{
  return( ADC0_SSFIFO3_R );             // Return adc value register
}

void adc_task(void *pvParameters){

    while(1){
        data_adc.val = get_adc()/10;         // Divide adc value by 10, to limit measurement uncertainty

        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void init_adc(){
  SYSCTL_RCGC0_R |= SYSCTL_RCGC0_ADC0;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;

  // Set ADC0 Sequencer priorities.
  // SS3(bit12-13): Priority = 0 ; Highest
  // SS2(bit8-9):   Priority = 1
  // SS1(bit4-5):   Priority = 2
  // SS0(bit0-1):   Priority = 3 ; Lowest
  ADC0_SSPRI_R = 0x00000123;

  //Disable all sequencers
  ADC0_ACTSS_R = 0;

  // Trigger for Sequencer 3 (bit 12-15) = 0xF = Always.
  ADC0_EMUX_R = 0x0000F000;

  //sample multiplexer input, sequencer 3 select, ADC 11 (0x0B) enable.
  ADC0_SSMUX3_R = 0x0B;

  //ADC sample sequence control 3 = END0
  ADC0_SSCTL3_R =  0x00000002;

  //enable sequencer 3
  ADC0_ACTSS_R = 0x00000008;

  // Start conversion at sequencer 3
  ADC0_PSSI_R = 0x08;
}
