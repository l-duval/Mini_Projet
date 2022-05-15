#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H


//define  motors' speeds
#define LOW_SPEED 		300 // step/s
#define HIGH_SPEED  	500 // step/s
#define ROTATION_SPEED 200 // step/s
#define NSTEP_ONE_TURN      1000 // number of step for 1 turn of the motor
#define WHEEL_PERIMETER     13 // [cm]
#define PI                  3.1415926536f
#define WHEEL_DISTANCE      5.20f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)
#define SEC_TO_MS 1000
#define CM_TO_MM 10


//start the motor_control thread
void motor_control_start(void);
// Rotate function
void rotate (int direction);
// High or Low Speed selection
int def_speed (int speed);


#endif /* MOTOR_CONTROL_H */
