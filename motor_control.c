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



//static int16_t position_to_reach_right = 0; // in [step]
//static int16_t position_to_reach_left = 0; // in [step]
//static uint8_t position_right_reached = 0; // in [step]
//static uint8_t position_left_reached = 0; // in [step]


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
	 	 	 right_motor_set_speed(-200);
	 	 	 left_motor_set_speed(200);
	         break;
	 //turn right(rotation de 90°)
	 case 2: chprintf((BaseSequentialStream *)&SD3, "go right %c ", 0);
		 	 right_motor_set_speed(200);
	 	 	 left_motor_set_speed(-200);
	         break;
	 //go backward (rotation de 180°) comme right or left mais + long
	 case 3: chprintf((BaseSequentialStream *)&SD3, "go backwards %c ", 0);
		 	 right_motor_set_speed(200);
	 	 	 left_motor_set_speed(-200);
	         break;
	}
}


static THD_WORKING_AREA(wamotor_control, 256);
static THD_FUNCTION(motor_control, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    messagebus_topic_t *morse_topic = messagebus_find_topic_blocking(&bus, "/morse");
    systime_t time;
    int morse[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   // rotate(morse_logic_direction());

   // int speed = 0;
  //  speed = def_speed(morse_logic_speed());

    while(1){
        time = chVTGetSystemTime();
        
        // Wait for publish
        messagebus_topic_wait(morse_topic, &morse, sizeof(morse));
        // pourquoi vide ?? psk memset dans process image
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[0]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[1]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[2]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[3]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[4]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[5]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[6]);
		chprintf((BaseSequentialStream *)&SD3, "%d", morse[7]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[8]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[9]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[10]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[11]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[12]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[13]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[14]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[15]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[16]);
        chprintf((BaseSequentialStream *)&SD3, "%d", morse[17]);

        rotate(morse_logic_direction(morse));
        // Reset morse instruction after use
       // memset(morse,0,sizeof morse);

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
