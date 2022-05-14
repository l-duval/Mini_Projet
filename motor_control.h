#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H


//define  motors' speeds
#define SPEED_ROTATION	5
#define LOW_SPEED 		300 // step/s
#define HIGH_SPEED  	500 // step/s
#define ROTATION_SPEED 200 // step/s
#define NSTEP_ONE_TURN      1000 // number of step for 1 turn of the motor
#define POSITION_NOT_REACHED	0
#define POSITION_REACHED       	1
#define WHEEL_PERIMETER     13 // [cm]
#define PI                  3.1415926536f
#define WHEEL_DISTANCE      5.20f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)


//start the motor_control thread
void motor_control_start(void);
// Rotate function
void rotate (int direction);
// High or Low Speed selection
int def_speed (int speed);


#endif /* MOTOR_CONTROL_H */
