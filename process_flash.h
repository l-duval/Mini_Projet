#ifndef PROCESS_FLASH_H
#define PROCESS_FLASH_H

#define threshold 50
#define min_length_dot 4
#define max_length_dot 21
#define min_length_line 30

#define MSG_LOADED 18

#define ae_on 1
// A dot is represented as a 0
#define dot 0
// A lime is represented as a 1
#define line 1



void process_flash_start(void);
int morse_logic_distance(int morse_msg[]);
int morse_logic_speed(int morse_msg[]);
int morse_logic_direction(int morse_msg[]);


#endif /* PROCESS_FLASH_H */
