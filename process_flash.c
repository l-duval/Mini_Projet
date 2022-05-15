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

static int morse[MSG_LOADED] = {0};

// Morse logic avec direction
int morse_logic_direction(int morse_msg[]){

	// B for Backwards "1000"
	if((morse_msg[0]==LINE)&&(morse_msg[1]==DOT)&&(morse_msg[2]==DOT)&&(morse_msg[3]==DOT)){
		return 3;
	}
	// R for Right "0001" Replaced by V in morse
	if((morse_msg[0]==DOT)&&(morse_msg[1]==DOT)&&(morse_msg[2]==DOT)&&(morse_msg[3]==LINE)){
		return 2;
	}
	// F for forward "0010"
	if((morse_msg[0]==DOT)&&(morse_msg[1]==DOT)&&(morse_msg[2]==LINE)&&(morse_msg[3]==DOT)){
		return 0;
	}
	// L for Left "0100"
	if((morse_msg[0]==DOT)&&(morse_msg[1]==LINE)&&(morse_msg[2]==DOT)&&(morse_msg[3]==DOT)){
		return 1;
	}
	// If error stays forward
	return 0;
}


// Distance is stored from 5th to 14th flash instruction
// From 5 to 50 cm
int morse_logic_distance(int morse_msg[]){

	int dizaine = 0;
	int unite = 0;

	// Dizaine
	// Zero "11111"
	if((morse_msg[4]==LINE)&&(morse_msg[5]==LINE)&&(morse_msg[6]==LINE)&&(morse_msg[7]==LINE)&&(morse_msg[8]==LINE)){
		dizaine = 0;
	}
	// Un "01111"
	if((morse_msg[4]==DOT)&&(morse_msg[5]==LINE)&&(morse_msg[6]==LINE)&&(morse_msg[7]==LINE)&&(morse_msg[8]==LINE)){
		dizaine = 1;
	}
	// Deux "00111"
	if((morse_msg[4]==DOT)&&(morse_msg[5]==DOT)&&(morse_msg[6]==LINE)&&(morse_msg[7]==LINE)&&(morse_msg[8]==LINE)){
		dizaine = 2;
	}
	// Trois "00011"
	if((morse_msg[4]==DOT)&&(morse_msg[5]==DOT)&&(morse_msg[6]==DOT)&&(morse_msg[7]==LINE)&&(morse_msg[8]==LINE)){
		dizaine = 3;
	}
	// Quatre "00001"
	if((morse_msg[4]==DOT)&&(morse_msg[5]==DOT)&&(morse_msg[6]==DOT)&&(morse_msg[7]==DOT)&&(morse_msg[8]==LINE)){
		dizaine = 4;
	}
	// Cinq "00000"
	if((morse_msg[4]==DOT)&&(morse_msg[5]==DOT)&&(morse_msg[6]==DOT)&&(morse_msg[7]==DOT)&&(morse_msg[8]==DOT)){
		dizaine = 5;
	}

	// Unite
	// Zero "11111"
	if((morse_msg[9]==LINE)&&(morse_msg[10]==LINE)&&(morse_msg[11]==LINE)&&(morse_msg[12]==LINE)&&(morse_msg[13]==LINE)){
		unite = 0;
	}
	// Un "01111"
	if((morse_msg[9]==DOT)&&(morse_msg[10]==LINE)&&(morse_msg[11]==LINE)&&(morse_msg[12]==LINE)&&(morse_msg[13]==LINE)){
		unite = 1;
	}
	// Deux "00111"
	if((morse_msg[9]==DOT)&&(morse_msg[10]==DOT)&&(morse_msg[11]==LINE)&&(morse_msg[12]==LINE)&&(morse_msg[13]==LINE)){
		unite = 2;
	}
	// Trois "00011"
	if((morse_msg[9]==DOT)&&(morse_msg[10]==DOT)&&(morse_msg[11]==DOT)&&(morse_msg[12]==LINE)&&(morse_msg[13]==LINE)){
		unite = 3;
	}
	// Quatre "00001"
	if((morse_msg[9]==DOT)&&(morse_msg[10]==DOT)&&(morse_msg[11]==DOT)&&(morse_msg[12]==DOT)&&(morse_msg[13]==LINE)){
		unite = 4;
	}
	// Cinq "00000"
	if((morse_msg[9]==DOT)&&(morse_msg[10]==DOT)&&(morse_msg[11]==DOT)&&(morse_msg[12]==DOT)&&(morse_msg[13]==DOT)){
		unite = 5;
	}
	// Six "10000"
	if((morse_msg[9]==LINE)&&(morse_msg[10]==DOT)&&(morse_msg[11]==DOT)&&(morse_msg[12]==DOT)&&(morse_msg[13]==DOT)){
		unite = 6;
	}
	// Sept "11000"
	if((morse_msg[9]==LINE)&&(morse_msg[10]==LINE)&&(morse_msg[11]==DOT)&&(morse_msg[12]==DOT)&&(morse_msg[13]==DOT)){
		unite = 7;
	}
	// Huit "11100"
	if((morse_msg[9]==LINE)&&(morse_msg[10]==LINE)&&(morse_msg[11]==LINE)&&(morse_msg[12]==DOT)&&(morse_msg[13]==DOT)){
		unite = 8;
	}
	// Neuf "11110"
	if((morse_msg[9]==LINE)&&(morse_msg[10]==LINE)&&(morse_msg[11]==LINE)&&(morse_msg[12]==LINE)&&(morse_msg[13]==DOT)){
		unite = 9;
	}
	return (dizaine*10 + unite);
}

// Morse Logic Speed
// Speed is stored at the last 4 flash instructions
int morse_logic_speed(int morse_msg[]){
	 // H for High speed "0000"
	 if((morse_msg[14]==DOT)&&(morse_msg[15]==DOT)&&(morse_msg[16]==DOT)&&(morse_msg[17]==DOT)){
		return 1;
	}
	// L for Low Speed "0100"
	if((morse_msg[14]==DOT)&&(morse_msg[15]==LINE)&&(morse_msg[16]==DOT)&&(morse_msg[17]==DOT)){
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
	int selector_previous = get_selector();

	while(1){
		selector_actual = get_selector();
		if((selector_actual != selector_previous)){
			systime_t time;
			time = chVTGetSystemTime();
			 //starts a capture
			dcmi_capture_start();
			//waits for the capture to be done
			wait_image_ready();
			//signals an image has been captured
			chBSemSignal(&image_ready_sem);
			counter_delayed = counter;
			// Justifier Threshold
			if (chVTGetSystemTime()-time <= THRESHOLD){
				++counter;
			}
			else{
				counter = 0;
			}
			if((counter_delayed > MIN_LENGTH_DOT)&&(counter_delayed <= MAX_LENGTH_DOT)){
				if(counter == 0){
					morse[index] = DOT;
					++index;
				}
			}
			if(counter_delayed >= MIN_LENGTH_LINE){
				if(counter == 0){
					morse[index] = LINE;
					++index;
				}
			}
			if(index == MSG_LOADED){
				selector_previous = selector_actual;
				// Reset index
				index = 0;
				messagebus_topic_publish(&morse_topic, &morse, sizeof(morse));
			}
		}
	}
}

void process_flash_start(void){
	chThdCreateStatic(waProcessFlash, sizeof(waProcessFlash), NORMALPRIO, ProcessFlash, NULL);
}
