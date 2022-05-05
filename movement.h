//define  motors' speeds

#ifndef SPEED_ROTATION	5
#ifndef LOW_SPEED 		5
#ifndef HIGH_SPEED  	10

#define SPEED_ROTATION	5
#define LOW_SPEED 		5 // cm/sec
#define HIGH_SPEED  	10 // cm/sec

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)
#endif

/** Define motor speed movement in function of the results of the camera (morse code)
 *
 * @parameter [in] desired speed
 *
 * @return Low or High speed
 */
int def_speed (int  speed);


/** The robot turns in the desired direction (code morse)
 *
 * @parameter [in] desired direction
 *
 */
void rotate (uint8_t direction);


/** The robot moves in the right direction at the right distance, at the desired speed if no obstacles
 * 1. move in the right direction
 * 2. check if obstacle detected
 * 3. if there is, LED
 * 4. if not, move forward to the desired distance at the right speed
 *
 * @parameter [in] direction Desired direction
 * @parameter [in] goal Desired distance to travel
 * @parameter [in] speed desired speed
 *
 */
void movement(uint8_t direction, uint8_t goal, int speed);


/** Thanks to the TOF, the robot detects if there are obstacles before the goal distance
 *
 * @parameter [in] goal Desired distance to travel
 *
 * @return true if there is an obstacle before the goal distance to travel else false
 */
bool obstacle_detection(uint8_t goal);
