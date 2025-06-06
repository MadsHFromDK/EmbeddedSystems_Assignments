/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: glob_def.h
*
* PROJECT....: ECP
*
* DESCRIPTION: Definements of variable types.
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 050128  KA    Module created.
*
*****************************************************************************/

#ifndef _GLOB_DEF_H
  #define _GLOB_DEF_H

/***************************** Include files *******************************/

/*****************************    Defines    *******************************/

#include "FreeRTOS.h"
#include "tmodel.h"
#include "queue.h"

#define FALSE       0
#define TRUE        !FALSE
#define NULL        ((void *)0)
#define NEGATIVE    0
#define POSITIVE    1
//#define LEFT        0
//#define RIGHT       1
#define RESET       0

#define ENCODER_MAX_VAL 20
#define REPAIR_VAL      30


/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/

/****************************** End Of Module *******************************/
#endif
