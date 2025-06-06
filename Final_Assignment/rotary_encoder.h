#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

typedef enum{
    RIGHT,
    LEFT,
    NONE
} encoder_dir;

typedef enum{
    DIR_SEL,
    CNT_SEL
} encoder_state;

struct elevator_data{
    FP32 vel;
    encoder_dir dir;
    encoder_dir REP_DIR;
    BOOLEAN SELECTED;
    INT8U CUR_FLOOR;
    INT8U ELE_FLOOR;
    INT8U FLOOR_SELECTION;
    INT8U NUM_TRIPS;
};

extern INT8S encoder_cnt;
extern struct elevator_data data;

//void reset_encoder_data();
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :  Initializing encoder info
*****************************************************************************/

char change_int_to_char(INT8U number);

void handle_encoder(void);

void rotary_encoder_task(void *pvParameters);
/*****************************************************************************
*   Input    :  Void pointer to a parameter
*   Output   :  -
*   Function :  Task for encoder
*****************************************************************************/

#endif /* ROTARY_ENCODER_H_ */
