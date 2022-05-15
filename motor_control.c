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
#include "process_flash.h"
#include <leds.h>



// Def speed : low or high
int def_speed (int speed){
	if (speed){
		return HIGH_SPEED;
	}
	return LOW_SPEED;
}

// Rotation of the robot in the wished direction
void rotate (int direction){
	systime_t time_to_rotate = 0;
	switch (direction) {
	//go forward
	case 0:
			right_motor_set_speed(0);
			left_motor_set_speed(0);
			break;
	//turn left (rotation of 90°)
	case 1:
			right_motor_set_speed(ROTATION_SPEED);
			left_motor_set_speed(-ROTATION_SPEED);
			time_to_rotate = (SEC_TO_MS*NSTEP_ONE_TURN/WHEEL_PERIMETER*PERIMETER_EPUCK)/(4*ROTATION_SPEED);
			chThdSleepMilliseconds(time_to_rotate);
			right_motor_set_speed(0);
			left_motor_set_speed(0);
			break;
	 //turn right(rotation of 90°)
	case 2:
			right_motor_set_speed(-ROTATION_SPEED);
			left_motor_set_speed(ROTATION_SPEED);
			time_to_rotate = (SEC_TO_MS*NSTEP_ONE_TURN/WHEEL_PERIMETER*PERIMETER_EPUCK)/(4*ROTATION_SPEED);
			chThdSleepMilliseconds(time_to_rotate);
			right_motor_set_speed(0);
			left_motor_set_speed(0);
			break;
	 //go backward (rotation of 180°)
	case 3:
			right_motor_set_speed(-ROTATION_SPEED);
			left_motor_set_speed(ROTATION_SPEED);
			time_to_rotate = (SEC_TO_MS*NSTEP_ONE_TURN/WHEEL_PERIMETER*PERIMETER_EPUCK)/(2*ROTATION_SPEED);
			chThdSleepMilliseconds(time_to_rotate);
			right_motor_set_speed(0);
			left_motor_set_speed(0);
			break;
	}
}

//thread motor_control
static THD_WORKING_AREA(wamotor_control, 512);
static THD_FUNCTION(motor_control, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    //reception ofthe table "morse" from the tread process_flash
    messagebus_topic_t *morse_topic = messagebus_find_topic_blocking(&bus, "/morse");
    uint32_t distance = 0;
    uint32_t speed = 0;
    int morse[MSG_LOADED] = {0};

    while(1){
        // Wait for publish of the table "morse"
        messagebus_topic_wait(morse_topic, &morse, sizeof(morse));
        // Rotation of e puck in the good direction
        rotate(morse_logic_direction(morse));
        //init the distance and the speed thanks to the table "morse"
        distance = (uint32_t)morse_logic_distance(morse);
        speed = (uint32_t)def_speed(morse_logic_speed(morse));
        // 3 seconds to get flash device out of the way if going forward, else it will detects a false obstacle
        chThdSleepMilliseconds(3000);
        //search for obstacles before moving forward with the thread TOF
        if(VL53L0X_get_dist_mm() > (CM_TO_MM*distance)){ // if no obstacle ==> go straight at the right speed
        	right_motor_set_speed(speed);
        	left_motor_set_speed(speed);
       		systime_t time_to_dist = (NSTEP_ONE_TURN/WHEEL_PERIMETER*SEC_TO_MS*distance)/speed;
       	// when he has reached the end of his race ==> stop
       		if(time_to_dist != 0){
       			chThdSleepMilliseconds(time_to_dist);
       		}
        }
        //inits body led
        set_body_led(LED_ON);
        // Wait 1 sec with body led turned on to indicate end of movement or obstacle = not enough distance to advance
        chThdSleepMilliseconds(1000);
        set_body_led(LED_OFF); // turns off bode leds
        // stop the motors
        right_motor_set_speed(0);
        left_motor_set_speed(0);
    }
}

void motor_control_start(void){
	chThdCreateStatic(wamotor_control, sizeof(wamotor_control), NORMALPRIO, motor_control, NULL);
}
