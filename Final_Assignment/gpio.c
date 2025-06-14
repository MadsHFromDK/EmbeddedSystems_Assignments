/*
 * gpio.c
 *
 *  Created on: 5 May 2025
 *      Author: madsa
 */

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "gpio.h"

void init_rot_enc(void){

    GPIO_PORTA_DIR_R &= ~0xE0; // Set PA5 and PA6 as inputs

    GPIO_PORTA_DEN_R |= 0xE0; // Enable digital function for PA5 and PA6

    //GPIO_PORTA_PUR_R |= 0x60; // Enable pull-up resistors for PA5 and PA6

    GPIO_PORTA_PUR_R |= 0xE0;
}

void init_gpio(void)
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : The super loop.
******************************************************************************/
{
  int dummy;

  // Enable the GPIO port that is used for the on-board LED.
  SYSCTL_RCGC2_R  =  SYSCTL_RCGC2_GPIOA |SYSCTL_RCGC2_GPIOC | SYSCTL_RCGC2_GPIOD | SYSCTL_RCGC2_GPIOE |SYSCTL_RCGC2_GPIOF;
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0;

  // Do a dummy read to insert a few cycles after enabling the peripheral.
  dummy = SYSCTL_RCGC2_R;

  // Set the direction as output (PF1, PF2 and PF3).
  GPIO_PORTA_DIR_R = 0x1C;
  GPIO_PORTC_DIR_R = 0xF0;
  GPIO_PORTD_DIR_R = 0x4C;
  GPIO_PORTF_DIR_R = 0x0E;

  // Enable the GPIO pins for digital function (PF0, PF1, PF2, PF3, PF4).
  GPIO_PORTA_DEN_R = 0x1C;
  GPIO_PORTC_DEN_R = 0xF0;
  GPIO_PORTD_DEN_R = 0x4C;
  GPIO_PORTE_DEN_R = 0x0F;
  GPIO_PORTF_DEN_R = 0x1F;

  // Enable internal pull-up (PF0 and PF4).
  GPIO_PORTF_PUR_R = 0x11;

  init_rot_enc();
}
