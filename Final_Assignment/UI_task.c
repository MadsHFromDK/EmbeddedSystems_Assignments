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
#include "emp_type.h"
#include "FreeRTOS.h"
#include "Task.h"
#include "queue.h"
#include "semphr.h"
#include "glob_def.h"
#include "elevator.h"
#include "UI_task.h"
#include "adc.h"
#include "key.h"
#include "lcd.h"
#include "elevator.h"
#include "rotary_encoder.h"


/*****************************    Defines    *******************************/

/*****************************   Constants   *******************************/

/*****************************   Variables   *******************************/

extern QueueHandle_t xQueue_keypad, xQueue_lcd, xQueue_elevator, xQueue_encoder;
extern SemaphoreHandle_t xSemaphore_keypad, xSemaphore_lcd, xSemaphore_ele_state, xSemaphore_ele_flr;
extern elevator_states elevator_state;
extern INT8S encoder_cnt;
extern struct elevator_data data;
extern struct adc_data data_adc;

typedef enum{
    POT,
    ENC,
    WRONG_DIR
} repair_states;

/*****************************   Functions   *******************************/

char change_int_to_char1(INT8U number){
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
    return '0';
}

void string_to_LCD(INT8U text_length, char text[]) {
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Inputs char array into the xQueue_lcd
*****************************************************************************/

    INT8U text_count = 0;
    char send_lcd;

    if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){

        while (text_count < text_length) {
            send_lcd = text[text_count];

            xQueueSend(xQueue_lcd, &send_lcd, portMAX_DELAY);
            text_count++;
        }

        xSemaphoreGive( xSemaphore_lcd );
    }
}

INT8U convert_string_to_int(char *input, INT8U length){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Converts a string to integers
*****************************************************************************/

    INT8U result = 0;
    INT8U i = 0;

    while (i < length) {
        if (input[i] >= '0' && input[i] <= '9') {  // Ensure it's a valid digit
            result = result * 10 + (input[i] - '0');  // Convert char to integer
        }
        i++;
    }
    return result;
}

void UI_task(void *pvParameters){
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  UI Task, responsible for outputting correct information to the LCD queue
*****************************************************************************/

char idle_text[] = "Call elevator";
INT8U idle_text_length = 13;
char locked_text[] = "Enter password: ";
INT8U locked_text_length = 17;
char unlocked_text[] = "Choose floor: ";
INT8U unlocked_text_length = 13;
char repair_enc_text[] = "Fix - Encoder:";
INT8U repair_enc_text_length = 14;
char repair_pot_text[] = "Fix - Pot:";
INT8U repair_pot_text_length = 10;
char right_text[] = "Right: ";
INT8U right_text_length = 7;
char left_text[] = "Left: ";
INT8U left_text_length = 6;
char home_text[] = "Welcome Home ";
INT8U home_text_length = 13;
char moving_text[] = "Current Floor:";
INT8U moving_text_length = 14;
char wrong_dir_text[] = "Wrong direction!";
INT8U wrong_dir_text_length = 16;

INT8U key_press;
INT8U key_temp;
INT8U key_count = 0;
INT8U code_length = 4;
char key_code[] = "xxxx";
char key_code_prev[] = "qqqq";
char key_code_entered[] = "xxxx";
INT8U code_entered = FALSE;
INT8U key_code_int;

INT8U digit1;
INT8U digit2;
INT8U digit3;
char char_digit1;
char char_digit2;
char char_digit3;

static elevator_states prev_state = AVAILABLE;
INT8U reset_lcd = 0xFF;
INT8U TF;

INT16U encoder_deg;

static INT16U prev_deg = 1;
static INT8S prev_enc_cnt = !0;
static INT16U prev_adc_val;

static repair_states repair_state = POT;
static repair_states prev_repair;

    while(1){
        switch(elevator_state){                                             // Switch between different states
        case LOCKED:
            if(prev_state != elevator_state){                               // Reset information and output correct string to queue, when switching into the state
                key_count = 0;
                key_code[0] = 'x';
                key_code[1] = 'x';
                key_code[2] = 'x';
                key_code[3] = 'x';

                prev_state = elevator_state;

                if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                    xQueueReset(xQueue_lcd);
                    xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                    xSemaphoreGive( xSemaphore_lcd );
                }

                move_LCD(0,0);
                string_to_LCD(locked_text_length, locked_text);
                move_LCD(0,1);
            }

            if(uxQueueMessagesWaiting(xQueue_keypad)){

                if(xSemaphoreTake( xSemaphore_keypad, portMAX_DELAY)){

                    if( xQueueReceive( xQueue_keypad, &key_press, portMAX_DELAY )){
                        key_temp = key_press;
                        key_code[key_count] = change_int_to_char(key_temp);
                        key_count++;
                    }
                }

                xSemaphoreGive( xSemaphore_keypad );
            }

            // Update input code
            if(key_code[0] != key_code_prev[0] || key_code[1] != key_code_prev[1] || key_code[2] != key_code_prev[2] || key_code[3] != key_code_prev[3]){
                move_LCD(0,1);
                string_to_LCD(code_length, key_code);
            }

            // Check if whole code has been input
            if(key_count == code_length){

                // Convert the code to readable
                key_code_int = convert_string_to_int(&key_code, code_length);
                key_count = 0;

                // Check if code is valid
                if(key_code_int % 8 == 0 & code_entered == FALSE){

                    if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                        TF = TRUE;
                        xQueueSend( xQueue_elevator, &TF, portMAX_DELAY); // Unlock elevator
                        xSemaphoreGive( xSemaphore_ele_state );
                    }

                    code_entered = TRUE;
                    key_code_entered[0] = key_code[0];              // Save code
                    key_code_entered[1] = key_code[1];
                    key_code_entered[2] = key_code[2];
                    key_code_entered[3] = key_code[3];
                }
                else if(code_entered == TRUE & key_code_entered[0] == key_code[0] & key_code_entered[1] == key_code[1] & key_code_entered[2] == key_code[2] & key_code_entered[3] == key_code[3]){

                    if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                        TF = TRUE;
                        xQueueSend( xQueue_elevator, &TF, portMAX_DELAY); // Unlock elevator if code matches previously used code
                        xSemaphoreGive( xSemaphore_ele_state );
                    }

                }
                else{

                    if(xSemaphoreTake(xSemaphore_ele_state, portMAX_DELAY)){
                        TF = FALSE;
                        xQueueSend( xQueue_elevator, &TF, portMAX_DELAY); // Require elevator to be called again, if code is not correct
                        xSemaphoreGive( xSemaphore_ele_state );
                    }
                }
            }

            key_code_prev[0] = key_code[0];                         // Store code
            key_code_prev[1] = key_code[1];
            key_code_prev[2] = key_code[2];
            key_code_prev[3] = key_code[3];

            break;

            case AWAY:
                if(prev_state != elevator_state){
                    key_count = 0;
                    key_code[0] = 'x';
                    key_code[1] = 'x';
                    key_code[2] = 'x';
                    key_code[3] = 'x';

                    if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                        xQueueReset(xQueue_lcd);
                        xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                        xSemaphoreGive( xSemaphore_lcd );
                    }

                    prev_state = elevator_state;

                    string_to_LCD(idle_text_length, idle_text);
                    move_LCD(0,0);
                }

            break;

            case AVAILABLE:
                if(prev_state != elevator_state){
                    key_count = 0;
                    key_code[0] = 'x';
                    key_code[1] = 'x';
                    key_code[2] = 'x';
                    key_code[3] = 'x';


                    if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                        xQueueReset(xQueue_lcd);
                        xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                        xSemaphoreGive( xSemaphore_lcd );
                    }

                    string_to_LCD(unlocked_text_length, unlocked_text);
                    move_LCD(0,1);
                    prev_state = elevator_state;
                }

                if(encoder_cnt != prev_enc_cnt){                            // Check if display should be updated

                    prev_enc_cnt = encoder_cnt;

                    digit1 = abs(encoder_cnt)/10;
                    digit2 = abs(encoder_cnt)%10;

                    char_digit1 = change_int_to_char(digit1);
                    char_digit2 = change_int_to_char(digit2);

                    if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                        xQueueSend( xQueue_lcd, &char_digit1, portMAX_DELAY );
                        xQueueSend( xQueue_lcd, &char_digit2, portMAX_DELAY );
                        xSemaphoreGive( xSemaphore_lcd );
                    }

                    move_LCD(0,1);
                }

            break;

            case MOVING:
                if(prev_state != elevator_state){
                    prev_state = MOVING;

                    key_count = 0;
                    key_code[0] = 'x';
                    key_code[1] = 'x';
                    key_code[2] = 'x';
                    key_code[3] = 'x';

                    if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                        xQueueReset(xQueue_lcd);
                        xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                        xSemaphoreGive( xSemaphore_lcd );
                    }

                    prev_state = elevator_state;

                    move_LCD(0,0);
                    string_to_LCD(moving_text_length, moving_text);
                    move_LCD(0,1);
                }

            break;

            case REPAIR:
                if(prev_state != elevator_state){
                    repair_state = POT;                                         // Repair should start in potentio meter repair state
                    prev_repair = ENC;                                          // Prev repair state should start as !pot, enc state is used, for consistency

                    key_count = 0;
                    key_code[0] = 'x';
                    key_code[1] = 'x';
                    key_code[2] = 'x';
                    key_code[3] = 'x';

                    if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                        xQueueReset(xQueue_lcd);
                        xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                        xSemaphoreGive( xSemaphore_lcd );
                    }

                    prev_state = elevator_state;
                }

                switch(repair_state){
                case POT:

                    if(repair_state != prev_repair){                                // First time entering the state, write static text
                        prev_repair = repair_state;

                        if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                            xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                            xSemaphoreGive( xSemaphore_lcd );
                        }

                        move_LCD(0,0);
                        string_to_LCD(repair_pot_text_length, repair_pot_text);
                        move_LCD(0,1);
                    }

                    if(prev_adc_val != data_adc.val){                               // If analog-digital converter val is changed, display "dynamic" text

                        digit1 = abs(data_adc.trgt_val)/100;
                        digit2 = abs(data_adc.trgt_val/10)%10;
                        digit3 = abs(data_adc.trgt_val)%10;

                        char_digit1 = change_int_to_char(digit1);
                        char_digit2 = change_int_to_char(digit2);
                        char_digit3 = change_int_to_char(digit3);

                        if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                            xQueueSend( xQueue_lcd, &char_digit1, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit2, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit3, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, " ", portMAX_DELAY );
                            xSemaphoreGive( xSemaphore_lcd );
                        }

                        digit1 = abs(data_adc.val)/100;
                        digit2 = abs(data_adc.val/10)%10;
                        digit3 = abs(data_adc.val)%10;

                        char_digit1 = change_int_to_char(digit1);
                        char_digit2 = change_int_to_char(digit2);
                        char_digit3 = change_int_to_char(digit3);

                        if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                            xQueueSend( xQueue_lcd, &char_digit1, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit2, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit3, portMAX_DELAY );
                            xSemaphoreGive( xSemaphore_lcd );
                        }

                        move_LCD(0,1);

                        if(data_adc.val > (data_adc.trgt_val - 5) && data_adc.val < (data_adc.trgt_val + 5)){   // If the value of the potentiometer is within five units of the random target, switch state to encoder.
                            repair_state = ENC;
                        }
                    }

                    break;

                case ENC:

                    encoder_deg = abs(encoder_cnt*12);                      // Convert encoder position to degrees, 360 deg/30 ticks = 12 deg/tick

                    if(repair_state != prev_repair){
                        prev_repair = repair_state;

                        if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                            xQueueReset(xQueue_lcd);
                            xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                            xSemaphoreGive( xSemaphore_lcd );
                        }

                        move_LCD(0,0);
                        string_to_LCD(repair_enc_text_length, repair_enc_text);
                        move_LCD(0,1);

                        if(data.REP_DIR == RIGHT){
                            string_to_LCD(right_text_length, right_text);
                        }
                        else{
                            string_to_LCD(left_text_length, left_text);
                        }

                        digit1 = abs(encoder_deg)/100;
                        digit2 = abs(encoder_deg/10)%10;
                        digit3 = abs(encoder_deg)%10;

                        char_digit1 = change_int_to_char(digit1);
                        char_digit2 = change_int_to_char(digit2);
                        char_digit3 = change_int_to_char(digit3);

                        if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                            xQueueSend( xQueue_lcd, &char_digit1, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit2, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit3, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, " ", portMAX_DELAY );
                            xQueueSend( xQueue_lcd, "d", portMAX_DELAY );
                            xQueueSend( xQueue_lcd, "e", portMAX_DELAY );
                            xQueueSend( xQueue_lcd, "g", portMAX_DELAY );
                            xSemaphoreGive( xSemaphore_lcd );
                        }

                        move_LCD(0,1);
                    }

                    if(encoder_deg != prev_deg){
                        prev_deg = encoder_deg;

                        if(data.REP_DIR == RIGHT){
                            if(data.dir == LEFT){
                                prev_repair = WRONG_DIR;
                                move_LCD(0,0);
                                string_to_LCD(wrong_dir_text_length, wrong_dir_text);
                                move_LCD(0,1);

                                if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                                    TF = TRUE;
                                    xQueueSend( xQueue_encoder, &TF, portMAX_DELAY);
                                    xSemaphoreGive( xSemaphore_ele_flr );
                                }

                                vTaskDelay( 3000 / portTICK_RATE_MS);
                            }
                            else{
                                string_to_LCD(right_text_length, right_text);
                            }
                        }
                        else{
                            if(data.dir == RIGHT){
                                prev_repair = WRONG_DIR;
                                move_LCD(0,0);
                                string_to_LCD(wrong_dir_text_length, wrong_dir_text);
                                move_LCD(0,1);

                                if(xSemaphoreTake(xSemaphore_ele_flr, portMAX_DELAY)){
                                    TF = FALSE;
                                    xQueueSend( xQueue_encoder, &TF, portMAX_DELAY);
                                    xSemaphoreGive( xSemaphore_ele_flr );
                                }

                                vTaskDelay( 3000 / portTICK_RATE_MS);
                            }
                            else{
                                string_to_LCD(left_text_length, left_text);
                            }
                        }

                        digit1 = abs(encoder_deg)/100;
                        digit2 = abs(encoder_deg/10)%10;
                        digit3 = abs(encoder_deg)%10;

                        char_digit1 = change_int_to_char(digit1);
                        char_digit2 = change_int_to_char(digit2);
                        char_digit3 = change_int_to_char(digit3);

                        if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                            xQueueSend( xQueue_lcd, &char_digit1, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit2, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, &char_digit3, portMAX_DELAY );
                            xQueueSend( xQueue_lcd, " ", portMAX_DELAY );
                            xQueueSend( xQueue_lcd, "d", portMAX_DELAY );
                            xQueueSend( xQueue_lcd, "e", portMAX_DELAY );
                            xQueueSend( xQueue_lcd, "g", portMAX_DELAY );
                            xSemaphoreGive( xSemaphore_lcd );
                        }

                        move_LCD(0,1);
                    }

                    break;
                }

            break;

            case HOME:

                if(prev_state != elevator_state){
                    key_count = 0;
                    key_code[0] = 'x';
                    key_code[1] = 'x';
                    key_code[2] = 'x';
                    key_code[3] = 'x';

                    if(xSemaphoreTake( xSemaphore_lcd, portMAX_DELAY)){
                        xQueueReset(xQueue_lcd);
                        xQueueSend( xQueue_lcd, &reset_lcd, portMAX_DELAY);
                        xSemaphoreGive( xSemaphore_lcd );
                    }

                    prev_state = elevator_state;
                }

                string_to_LCD(home_text_length, home_text);

                break;
        }

        vTaskDelay( 50 / portTICK_RATE_MS);
    }

}

