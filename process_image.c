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
#include <process_image.h>

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static char morse[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Morse logic avec direction / distance / speed ??? ou les 3 d'un coup ?
int morse_logic_direction(char morse_msg[]){

	// R etant le seul de longeur 3 si le dernier indice de morse en nulle cad qu'on la eu un R.
	if(morse_msg[17] == '0'){
		return 2;
	}
	// First instruction size 4 except R.
	char temp[4] = {morse_msg[0], morse_msg[1], morse_msg[2], morse_msg[3]};
	// B for Backwards
	if(strcmp(temp,"-...") == 0){
		return 3;
	}
	// F for forward
	else if(strcmp(temp,"..-.") == 0){
		return 0;
	}
	// L for Left
	else if(strcmp(temp,".-..") == 0){
		return 1;
	}
	// If error stays forward
	return 0;
}

// en cm max 50 cm ?
int morse_logic_distance(char morse_msg[]){

	int i = 0;
	int dizaine = 0;
	int unite = 0;

	// Décalage causé par R ? Rapport on aurai pu changer de lettre ou modifier son symbole morse
	if(morse_msg[17] == '0'){
		i = -1;
	}
	// Dizaine
	char temp_1[5] = {morse_msg[4+i], morse_msg[5+i],morse_msg[6+i],morse_msg[7+i],morse_msg[8+i]};
	// Unite
	char temp_2[5] = {morse_msg[9+i],morse_msg[10+i],morse_msg[11+i],morse_msg[12+i],morse_msg[13+i]};

	// Dizaine
	// Zero
	if(strcmp(temp_1,"-----") == 0){
		dizaine = 0;
	}
	// Un
	else if(strcmp(temp_1,".----") == 0){
		dizaine = 1;
	}
	// Deux
	else if(strcmp(temp_1,"..---") == 0){
		dizaine = 2;
	}
	// Trois
	else if(strcmp(temp_1,"...--") == 0){
		dizaine = 3;
	}
	// Quatre
	else if(strcmp(temp_1,"....-") == 0){
		dizaine = 4;
	}
	// Cinq
	else if(strcmp(temp_1,".....") == 0){
		dizaine = 5;
	}

	// Unite
	// Zero
		if(strcmp(temp_2,"-----") == 0){
			unite = 0;
		}
		// Un
		else if(strcmp(temp_2,".----") == 0){
			unite = 1;
		}
		// Deux
		else if(strcmp(temp_2,"..---") == 0){
			unite = 2;
		}
		// Trois
		else if(strcmp(temp_2,"...--") == 0){
			unite = 3;
		}
		// Quatre
		else if(strcmp(temp_2,"....-") == 0){
			unite = 4;
		}
		// Cinq
		else if(strcmp(temp_2,".....") == 0){
			unite = 5;
		}
		// Six
		else if(strcmp(temp_2,"-....") == 0){
			unite  = 6;
		}
		// Sept
		else if(strcmp(temp_2,"--...") == 0){
			unite = 7;
		}
		// Huit
		else if(strcmp(temp_2,"---..") == 0){
			unite = 8;
		}
		// Neuf
		else if(strcmp(temp_2,"----.") == 0){
			unite = 9;
		}

	return (dizaine*10 + unite);
}

int morse_logic_speed(char morse_msg[]){

	int i = 0;
	// Décalage causé par R ? Rapport on aurai pu changer de lettre ou modifier son symbole morse
	if(morse_msg[17] == '0'){
		i = -1;
	}

	char temp[7] = {morse_msg[14+i],morse_msg[15+i],morse_msg[16+i],morse_msg[17+i]};
	// H for High speed
	if(strcmp(temp,"....") == 0){
		return 1;
	}
	// L for Low Speed
	else if(strcmp(temp,".-..") == 0){
		return 0;
	}
	// If error low speed
	return 0;
}

// Thread qui traite les datas des flashs
static THD_WORKING_AREA(waCaptureImage, 1024);
static THD_FUNCTION(CaptureImage, arg) {

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
	// si personne lit il se peut que a bloque
	messagebus_advertise_topic(&bus, &morse_topic, "/morse");

	int counter = 0;
	int counter_delayed = 0;
	int index = 0;
	// memset(morse, 0, 21);
	// calculer le max des 3 instructions a la suite + des 0 qui les separes
	// Check pour R qui meme si y a que y a rien en direction[3] ca marche le switch

    while(1){
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
		if((counter_delayed != 0)&&(counter_delayed <= 8)){
				if(counter == 0){
					chprintf((BaseSequentialStream *)&SD3, " dot %c  ", 0);
					morse[index] = '.';
					++index;
				}
			}
		// mettre un bool pr desactiver une fois fait one time puis reactiver apres ?
		if(counter_delayed >= 10){
			if(counter == 0){
			chprintf((BaseSequentialStream *)&SD3, " line %c  ", 0);
			morse[index] = '-';
			++index;
			}
		}
		if(index == 18){
			messagebus_topic_publish(&morse_topic, &morse, sizeof(morse));
			index = 0;
			// Reset morse instruction after use
			memset(morse,0,sizeof morse);
		}
    }
}

void process_image_start(void){
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
