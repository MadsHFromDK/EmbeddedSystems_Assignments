/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: leds.h
*
* PROJECT....: ECP
*
* DESCRIPTION: Test.
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 050128  KA    Module created.
*
*****************************************************************************/

#ifndef _LCD_H
  #define _LCD_H

/***************************** Include files *******************************/

/*****************************    Defines    *******************************/
// Special ASCII characters
// ------------------------

#define LF      0x0A
#define FF      0x0C
#define CR      0x0D

#define ESC     0x1B


/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/
void wr_str_LCD( INT8U* );
void move_LCD( INT8U, INT8U );
INT8U wr_ch_LCD( INT8U Ch );
void clr_LCD();

void lcd_task( void *pvParameters );
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Test function
******************************************************************************/


/****************************** End Of Module *******************************/
#endif

