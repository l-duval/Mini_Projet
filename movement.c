#include "ch.h"
#include "hal.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stm32f4xx.h>
#include <gpio.h>

#include <main.h>
#include <motors.h>

motor_init();


//define speed motor
#define SPEED_ROTATION	5
#define LOW_SPEED 		500
#define HIGH_SPEED  	700

//define motor speed movement in function of the results of the camera
int def_speed (int  speed){

	if (speed){
		return HIGH_SPEED ;
	return LOW_SPEED;
}

//define the direction in function of the results of the camera
void def_direction (uint8_t direction, int speed){

	int MOTOR_SPEED = def_speed(speed);
	switch (direction) {
	//go forward
	 case 0: motor_set_position(0, 0, 0, 0);
	         break;
	  /turn left (rotation de 90°)
	 case 1: motor_set_position(PERIMETER_EPUCK/4, PERIMETER_EPUCK/4, SPEED_ROTATION, -SPEED_ROTATION);
	         break;
	 //turn right(rotation de 90°)
	 case 2: motor_set_position(PERIMETER_EPUCK/4, PERIMETER_EPUCK/4, -SPEED_ROTATION, SPEED_ROTATION);
	         break;
	 //go backward (rotation de 180°)
	 case 3:motor_set_position(PERIMETER_EPUCK/2, PERIMETER_EPUCK/2, SPEED_ROTATION, -SPEED_ROTATION);
	         break;
	 default: printf("I am default\n");
	}
}


void movement(uint8_t direction, uint8_t goal, int speed){

	def_direction(direction, speed);

	if (obstacle_detection(goal)){

	}
	else {
		int MOTOR_SPEED = def_speed(speed);
		left_motor_set_speed(MOTOR_SPEED);
		right_motor_set_speed(MOTOR_SPEED);
	}
	return ;
}

//Le robot detecte les obstables avant la distance goal a atteindre
bool obstacle_detection(uint8_t goal){

	 bool  obstacle = false;
	 uint8_t i = 0;
	 while(obstacle == false && i <= goal){
		i=+1;
		 obstacle = true;
	 }

	 return obstacle;
}
