#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define threshold 50
#define min_length_dot 4
#define max_length_dot 21
#define min_length_line 30
#define ae_on 1

void process_image_start(void);
int morse_logic_distance(int morse_msg[]);
int morse_logic_speed(int morse_msg[]);
int morse_logic_direction(int morse_msg[]);


#endif /* PROCESS_IMAGE_H */
