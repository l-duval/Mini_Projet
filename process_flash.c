#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include <stdio.h>
#include <stdbool.h>
#include <main.h>
#include <string.h>
#include <msgbus/messagebus.h>
#include <camera/po8030.h>
#include <process_flash.h>
#include <selector.h>

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static int morse[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Morse logic avec direction / distance / speed ??? ou les 3 d'un coup ?
int morse_logic_direction(int morse_msg[]){

	// B for Backwards "1000"
	if((morse_msg[0]==line)&&(morse_msg[1]==dot)&&(morse_msg[2]==dot)&&(morse_msg[3]==dot)){
		return 3;
	}
	// R for Right "0001" Replaced by V in morse
	if((morse_msg[0]==dot)&&(morse_msg[1]==dot)&&(morse_msg[2]==dot)&&(morse_msg[3]==line)){
		return 2;
	}
	// F for forward "0010"
	if((morse_msg[0]==dot)&&(morse_msg[1]==dot)&&(morse_msg[2]==line)&&(morse_msg[3]==dot)){
		chprintf((BaseSequentialStream *)&SD3, "go forward process image %c ", 0);
		return 0;
	}
	// L for Left "0100"
	if((morse_msg[0]==dot)&&(morse_msg[1]==line)&&(morse_msg[2]==dot)&&(morse_msg[3]==dot)){
		return 1;
	}
	// If error stays forward
	chprintf((BaseSequentialStream *)&SD3, "return de base %c ", 0);
	return 0;
}

// en cm max 50 cm ?
// Distance is stored from 5th to 14th flash instruction
int morse_logic_distance(int morse_msg[]){

	int dizaine = 0;
	int unite = 0;

	// Dizaine
	// Zero "11111"
	if((morse_msg[4]==line)&&(morse_msg[5]==line)&&(morse_msg[6]==line)&&(morse_msg[7]==line)&&(morse_msg[8]==line)){
		dizaine = 0;
	}
	// Un "01111"
	if((morse_msg[4]==dot)&&(morse_msg[5]==line)&&(morse_msg[6]==line)&&(morse_msg[7]==line)&&(morse_msg[8]==line)){
		dizaine = 1;
	}
	// Deux "00111"
	if((morse_msg[4]==dot)&&(morse_msg[5]==dot)&&(morse_msg[6]==line)&&(morse_msg[7]==line)&&(morse_msg[8]==line)){
		dizaine = 2;
	}
	// Trois "00011"
	if((morse_msg[4]==dot)&&(morse_msg[5]==dot)&&(morse_msg[6]==dot)&&(morse_msg[7]==line)&&(morse_msg[8]==line)){
		dizaine = 3;
	}
	// Quatre "00001"
	if((morse_msg[4]==dot)&&(morse_msg[5]==dot)&&(morse_msg[6]==dot)&&(morse_msg[7]==dot)&&(morse_msg[8]==line)){
		dizaine = 4;
	}
	// Cinq "00000"
	if((morse_msg[4]==dot)&&(morse_msg[5]==dot)&&(morse_msg[6]==dot)&&(morse_msg[7]==dot)&&(morse_msg[8]==dot)){
		dizaine = 5;
	}

	// Unite
	// Zero "11111"
	if((morse_msg[9]==line)&&(morse_msg[10]==line)&&(morse_msg[11]==line)&&(morse_msg[12]==line)&&(morse_msg[13]==line)){
		unite = 0;
	}
	// Un "01111"
	if((morse_msg[9]==dot)&&(morse_msg[10]==line)&&(morse_msg[11]==line)&&(morse_msg[12]==line)&&(morse_msg[13]==line)){
		unite = 1;
	}
	// Deux "00111"
	if((morse_msg[9]==dot)&&(morse_msg[10]==dot)&&(morse_msg[11]==line)&&(morse_msg[12]==line)&&(morse_msg[13]==line)){
		unite = 2;
	}
	// Trois "00011"
	if((morse_msg[9]==dot)&&(morse_msg[10]==dot)&&(morse_msg[11]==dot)&&(morse_msg[12]==line)&&(morse_msg[13]==line)){
		unite = 3;
	}
	// Quatre "00001"
	if((morse_msg[9]==dot)&&(morse_msg[10]==dot)&&(morse_msg[11]==dot)&&(morse_msg[12]==dot)&&(morse_msg[13]==line)){
		unite = 4;
	}
	// Cinq "00000"
	if((morse_msg[9]==dot)&&(morse_msg[10]==dot)&&(morse_msg[11]==dot)&&(morse_msg[12]==dot)&&(morse_msg[13]==dot)){
		unite = 5;
	}
	// Six "10000"
	if((morse_msg[9]==line)&&(morse_msg[10]==dot)&&(morse_msg[11]==dot)&&(morse_msg[12]==dot)&&(morse_msg[13]==dot)){
		unite = 6;
	}
	// Sept "11000"
	if((morse_msg[9]==line)&&(morse_msg[10]==line)&&(morse_msg[11]==dot)&&(morse_msg[12]==dot)&&(morse_msg[13]==dot)){
		unite = 7;
	}
	// Huit "11100"
	if((morse_msg[9]==line)&&(morse_msg[10]==line)&&(morse_msg[11]==line)&&(morse_msg[12]==dot)&&(morse_msg[13]==dot)){
		unite = 8;
	}
	// Neuf "11110"
	if((morse_msg[9]==line)&&(morse_msg[10]==line)&&(morse_msg[11]==line)&&(morse_msg[12]==line)&&(morse_msg[13]==dot)){
		unite = 9;
	}

	return (dizaine*10 + unite);
}

// Morse Logic Speed
// Speed is stored at the last 4 flash instructions
int morse_logic_speed(int morse_msg[]){
	 // H for High speed
	 if((morse_msg[14]==dot)&&(morse_msg[15]==dot)&&(morse_msg[16]==dot)&&(morse_msg[17]==dot)){
		return 1;
	}
	// L for Low Speed "0100"
	if((morse_msg[14]==dot)&&(morse_msg[15]==line)&&(morse_msg[16]==dot)&&(morse_msg[17]==dot)){
		return 0;
	}
	//If error low speed
	return 0;
}

// Thread qui traite les datas des flashs
static THD_WORKING_AREA(waProcessFlash, 1024);
static THD_FUNCTION(ProcessFlash, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	po8030_advanced_config(FORMAT_RGB565, 315, 235,10,10, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

	 // Declares the topic on the bus.
	messagebus_topic_t morse_topic;
	MUTEX_DECL(morse_topic_lock);
	CONDVAR_DECL(morse_topic_condvar);
	messagebus_topic_init(&morse_topic, &morse_topic_lock, &morse_topic_condvar, &morse, sizeof(morse));
	messagebus_advertise_topic(&bus, &morse_topic, "/morse");

	int counter = 0;
	int counter_delayed = 0;
	int index = 0;
	int selector_actual = 0;
	int selector_previous = 0;
	selector_previous = get_selector();
	bool ready_to_send = false;

	chprintf((BaseSequentialStream *)&SD3, "selector %d", selector_actual);

	while(1){
		selector_actual = get_selector();
		if((selector_actual != selector_previous)&&(ready_to_send)){
			po8030_set_ae(ae_on);
			systime_t time;
			time = chVTGetSystemTime();
			 //starts a capture
			dcmi_capture_start();
			//waits for the capture to be done
			wait_image_ready();
			//signals an image has been captured
			chBSemSignal(&image_ready_sem);
			//chprintf((BaseSequentialStream *)&SD3, "ct %d", chVTGetSystemTime()-time);
			counter_delayed = counter;
			// Justifier Threshold
			if (chVTGetSystemTime()-time <= threshold){
				++counter;
			}
			else{
				counter = 0;
			}
			//chprintf((BaseSequentialStream *)&SD3, "capture time 2 = %d\n", chVTGetSystemTime()-time);
			//chprintf((BaseSequentialStream *)&SD3, "cnt= %d", counter);
			// && ou &
			if((counter_delayed > min_length_dot)&&(counter_delayed <= max_length_dot)){
				if(counter == 0){
					chprintf((BaseSequentialStream *)&SD3, " dot %c  ", 0);
					morse[index] = dot;
					++index;
				}
			}
			// mettre un bool pr desactiver une fois fait one time puis reactiver apres ?
			if(counter_delayed >= min_length_line){
				if(counter == 0){
				chprintf((BaseSequentialStream *)&SD3, " line %c  ", 0);
				morse[index] = line;
				++index;
				}
			}
			if(index == MSG_LOADED){
				selector_previous = selector_actual;
				// pas nécessaire je crois puisque le selector bloque tout
				po8030_set_exposure(2048,0);
				// Reset index
				index = 0;
				messagebus_topic_publish(&morse_topic, &morse, sizeof(morse));
			}
		}
		ready_to_send = true;
	}
}

void process_flash_start(void){
	chThdCreateStatic(waProcessFlash, sizeof(waProcessFlash), NORMALPRIO, ProcessFlash, NULL);
}
