#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define threshold 58

void process_image_start(void);
int morse_logic_distance(char morse_msg[]);
int morse_logic_speed(char morse_msg[]);
int morse_logic_direction(char morse_msg[]);


#endif /* PROCESS_IMAGE_H */
