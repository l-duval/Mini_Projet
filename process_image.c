#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include <stdio.h>
#include <stdbool.h>
#include <main.h>
#include <string.h>
#include <messagebus.h>
#include <camera/po8030.h>

#include <process_image.h>


extern messagebus_t bus;

static char morse[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static thread_t *morseThd;

static bool morse_configured = false;






// Morse logic avec direction / distance / speed ??? ou les 3 d'un coup ?
int morse_logic_direction(void){

	// R etant le seul de longeur 3 si le dernier indice de morse en nulle cad qu'on la eu un R.
	if(morse[20] == '0'){
		return 2;
	}
	// First instruction size 4 except R.
	char temp[4] = {morse[0], morse[1], morse[2], morse[3]};
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
int morse_logic_distance(void){

	int i = 0;
	int dizaine = 0;
	int unite = 0;

	// Décalage causé par R ? Rapport on aurai pu changer de lettre ou modifier son symbole morse
	if(morse[20] == '0'){
		i = -1;
	}
	// Dizaine
	char temp_1[5] = {morse[4+i], morse[5+i],morse[6+i],morse[7+i],morse[8+i]};
	// Unite
	char temp_2[5] = {morse[9+i],morse[10+i],morse[11+i],morse[12+i],morse[13+i]};

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

int morse_logic_speed(void){

	int i = 0;
	// Décalage causé par R ? Rapport on aurai pu changer de lettre ou modifier son symbole morse
	if(morse[20] == '0'){
		i = -1;
	}

	char temp[7] = {morse[14+i],morse[15+i],morse[16+i],morse[17+i],
					morse[18+i],morse[19+i],morse[20+i]};
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
static THD_WORKING_AREA(waCaptureImage, 256 );
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
		if((counter_delayed != 0)&(counter_delayed <= 8)){
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

    }
}



// Thread receiver qui quand a toutes les datas les envoies a motor.
static THD_WORKING_AREA(waReceptionData, 256);
static THD_FUNCTION(ReceptionData, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;


    while(1){

    }
}



void process_image_start(void){
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
	chThdCreateStatic(waReceptionData, sizeof(waReceptionData), NORMALPRIO, ReceptionData, NULL);
}
