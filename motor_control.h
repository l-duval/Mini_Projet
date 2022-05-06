#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

//start the motor_control thread
void motor_control_start(void);
void rotate (uint8_t direction);
int def_speed (int speed);


#endif /* MOTOR_CONTROL_H */
