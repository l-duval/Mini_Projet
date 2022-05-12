#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H


//define  motors' speeds
#define SPEED_ROTATION	5
#define LOW_SPEED 		300 // step/s
#define HIGH_SPEED  	500 // step/s
#define NSTEP_ONE_TURN      1000 // number of step for 1 turn of the motor
#define POSITION_NOT_REACHED	0
#define POSITION_REACHED       	1
#define WHEEL_PERIMETER     13 // [cm]

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)


//start the motor_control thread
void motor_control_start(void);
void rotate (int direction);
int def_speed (int speed);


#endif /* MOTOR_CONTROL_H */
