#ifndef PROCESS_FLASH_H
#define PROCESS_FLASH_H

#define THRESHOLD 50
#define MIN_LENGTH_DOT 4
#define MAX_LENGTH_DOT 21
#define MIN_LENGTH_LINE 30

#define MSG_LOADED 18

// A dot is represented as a 0
#define DOT 0
// A lime is represented as a 1
#define LINE 1

void process_flash_start(void);
int morse_logic_distance(int morse_msg[]);
int morse_logic_speed(int morse_msg[]);
int morse_logic_direction(int morse_msg[]);


#endif /* PROCESS_FLASH_H */
