#include "motor_control.h"
#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <main.h>
#include <motors.h>
#include <msgbus/messagebus.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <process_image.h>

// Def speed
int def_speed (int speed){
	if (speed){
		return HIGH_SPEED;
	}
	return LOW_SPEED;
}

// Rotate function
void rotate (int direction){

	switch (direction) {
	//go forward
	 case 0: left_motor_set_pos(-500);
	 	 	 right_motor_set_pos(500);
	         break;
	  //turn left (rotation de 90°)
	 case 1:left_motor_set_pos(-500);
	 	 	right_motor_set_pos(500);
	         break;
	 //turn right(rotation de 90°)
	 case 2: left_motor_set_pos(-500);
	 	 	 right_motor_set_pos(500);
	         break;
	 //go backward (rotation de 180°)
	 case 3:left_motor_set_pos(-500);
	 	 	right_motor_set_pos(500);
	         break;
	}
}

void obstacle(int distance){
}


static THD_WORKING_AREA(wamotor_control, 256);
static THD_FUNCTION(motor_control, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    messagebus_topic_t *morse_topic = messagebus_find_topic_blocking(&bus, "/morse");
    systime_t time;
    char morse[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   // rotate(morse_logic_direction());

    int speed = 0;
  //  speed = def_speed(morse_logic_speed());

    while(1){
        time = chVTGetSystemTime();
        
        // Wait for publish
        //wait for new measures to be published
        messagebus_topic_wait(morse_topic, &morse, sizeof(morse));
        chprintf((BaseSequentialStream *)&SD3, " motor gooo %c  ", morse[1]);
        // Reset morse instruction after use
        memset(morse,0,sizeof morse);

        // Wait if not complete
        //	ChVTPrintf ...

        //	right_motor_set_speed(speed);
        //	left_motor_set_speed(speed);


        // reset message ??


        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void motor_control_start(void){
	chThdCreateStatic(wamotor_control, sizeof(wamotor_control), NORMALPRIO, motor_control, NULL);
}
