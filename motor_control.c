#include "motor_control.h"
#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <main.h>
#include <motors.h>
#include <process_image.h>



// Def speed
int def_speed (int speed){
	if (speed){
		return HIGH_SPEED;
	}
	return LOW_SPEED;
}


// Rotate function
void rotate (uint8_t direction){

	switch (direction) {
	//go forward
	 case 0: motor_set_position(0, 0, 0, 0);
	         break;
	  //turn left (rotation de 90°)
	 case 1: motor_set_position(PERIMETER_EPUCK/4, PERIMETER_EPUCK/4, SPEED_ROTATION, -SPEED_ROTATION);
	         break;
	 //turn right(rotation de 90°)
	 case 2: motor_set_position(PERIMETER_EPUCK/4, PERIMETER_EPUCK/4, -SPEED_ROTATION, SPEED_ROTATION);
	         break;
	 //go backward (rotation de 180°)
	 case 3:motor_set_position(PERIMETER_EPUCK/2, PERIMETER_EPUCK/2, SPEED_ROTATION, -SPEED_ROTATION);
	         break;
	}
}


static THD_WORKING_AREA(wamotor_control, 256);
static THD_FUNCTION(motor_control, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;
    rotate(morse_logic_direction(char morse_msg[]));

    int speed = def_speed(morse_logic_speed(char morse_msg[]));

    while(1){
        time = chVTGetSystemTime();
        


		right_motor_set_speed(speed);
		left_motor_set_speed(speed);

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void motor_control_start(void){
	chThdCreateStatic(wamotor_control, sizeof(wamotor_control), NORMALPRIO, motor_control, NULL);
}
