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


// Enlever Motor.h ??


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
	//go forward valeur test ca devrait etre (0)
	 case 0:
		 	 chprintf((BaseSequentialStream *)&SD3, "go forward %c ", 0);
		 	 right_motor_set_speed(0);
	 	 	 left_motor_set_speed(0);
	         break;
	  //turn left (rotation de 90°)
	 case 1: chprintf((BaseSequentialStream *)&SD3, "go left %c ", 0);
	 	 	 right_motor_set_speed(200);
	 	 	 left_motor_set_speed(-200);
	 	 	 chThdSleepMilliseconds(1700);
	 	 	 right_motor_set_speed(0);
	 	     left_motor_set_speed(0);
	         break;
	 //turn right(rotation de 90°)
	 case 2: chprintf((BaseSequentialStream *)&SD3, "go right %c ", 0);
	 	 	 right_motor_set_speed(-200);
	 	 	 left_motor_set_speed(200);
	 	 	 chThdSleepMilliseconds(1700);
	 	 	 right_motor_set_speed(0);
	 	     left_motor_set_speed(0);
	         break;
	 //go backward (rotation de 180°) comme right or left mais + long
	 case 3: chprintf((BaseSequentialStream *)&SD3, "go backwards %c ", 0);
	 	 	 right_motor_set_speed(-200);
	 	 	 left_motor_set_speed(200);
	 	 	 chThdSleepMilliseconds(3400);
	 	 	 right_motor_set_speed(0);
	 	 	 left_motor_set_speed(0);
	         break;
	}
}

void movement(int distance, int speed, int morse_msg[]){
	speed = def_speed(morse_logic_speed(morse_msg));
	int time_to_dist = 0 ;
	int tof_distance = 0;
	tof_distance = (int)VL53L0X_get_dist_mm()*10; // convert to cm
	if(tof_distance > distance){
		right_motor_set_speed(speed);
		left_motor_set_speed(speed);
		//Bon calcul ??
		time_to_dist = distance/speed;
		chThdSleepMilliseconds(time_to_dist);
	}
	right_motor_set_speed(0);
	left_motor_set_speed(0);
// else led ???
}




// Remettre a 256 ?? to 1024 ???
static THD_WORKING_AREA(wamotor_control, 512);
static THD_FUNCTION(motor_control, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    messagebus_topic_t *morse_topic = messagebus_find_topic_blocking(&bus, "/morse");
    systime_t time;
    uint16_t distance = 0;
    uint16_t speed = 0;
    int morse[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    while(1){
        time = chVTGetSystemTime();
        
        // Wait for publish
        messagebus_topic_wait(morse_topic, &morse, sizeof(morse));
        rotate(morse_logic_direction(morse));
        chThdSleepMilliseconds(1000);
        chprintf((BaseSequentialStream *)&SD3, "tof dist %d ", VL53L0X_get_dist_mm());
        distance = (uint32_t)morse_logic_distance(morse);
        chprintf((BaseSequentialStream *)&SD3, "morse dist %d ", distance);
        speed = (uint32_t)def_speed(morse_logic_speed(morse));
        chprintf((BaseSequentialStream *)&SD3, "morse speed %d ", speed);
        chprintf((BaseSequentialStream *)&SD3, "dist*10 %d ", 10*distance);
        if(VL53L0X_get_dist_mm() > (10*distance)){
        	chprintf((BaseSequentialStream *)&SD3, "obstacle ok %d ",0);
        	right_motor_set_speed(speed);
        	left_motor_set_speed(speed);
        	// essayer de justifier ca
       		systime_t time_to_dist = NSTEP_ONE_TURN/WHEEL_PERIMETER*1000*distance/speed;
       		chprintf((BaseSequentialStream *)&SD3, "time to dist %d", time_to_dist);
        	// sinon plutot boucle avec compteur nbr de step pour dist/perimetre roue*1000 / step/s =
        	//systime_t time_to_dist = distance/speed;
       		chThdSleepMilliseconds(time_to_dist);
        }
        right_motor_set_speed(0);
        left_motor_set_speed(0);
       // movement(morse_logic_distance(morse),morse_logic_speed(morse),morse);
        //100Hz

        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void motor_control_start(void){
	chThdCreateStatic(wamotor_control, sizeof(wamotor_control), NORMALPRIO, motor_control, NULL);
}
