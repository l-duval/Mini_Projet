#include "ch.h"
#include "hal.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stm32f4xx.h>

#include <gpio.h>
#include <main.h>
#include <motors.h>
#include <movement.h>

int def_speed (int  speed){
	if (speed){
		return HIGH_SPEED ;
	return LOW_SPEED;
	}
}

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
	 default: printf("I am default\n");
	}
}

void movement(uint8_t direction, uint8_t goal, int speed){

	def_direction(direction);

	if (obstacle_detection(goal)){
		//LED clignotent et le robot n'avance pas
	}
	else {
		//le robot avance
		int MOTOR_SPEED = def_speed(speed);
		left_motor_set_speed(MOTOR_SPEED);
		right_motor_set_speed(MOTOR_SPEED);
		//pas sure du tout car goal uint8_t et non uint32_t
		left_motor_set_position(goal);
		right_motor_set_position(goal);

	}
	return ;
}


bool obstacle_detection(uint8_t goal){

	 bool  obstacle = false;
	 VL53L0X_start();
	 uint16_t obstacle_dist = VL53L0X_get_dist_mm();
	 VL53L0X_stop();
	 //attention mettre soit goal en mm soit obstacle_dist en cm
	 //est ce que l'on peut comparer uint16_t et uint8_t ?
	 if (obstacle_dist < goal) {
		 obstacle = true;
	 }
	 return obstacle;
}
