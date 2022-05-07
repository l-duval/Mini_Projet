#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H


//define  motors' speeds
#define SPEED_ROTATION	5
#define LOW_SPEED 		100 // step/s
#define HIGH_SPEED  	300 // step/s

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)


//start the motor_control thread
void motor_control_start(void);
void rotate (int direction);
int def_speed (int speed);


#endif /* MOTOR_CONTROL_H */
