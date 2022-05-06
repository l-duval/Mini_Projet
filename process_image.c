#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include <stdio.h>
#include <stdbool.h>
#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>


//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static THD_WORKING_AREA(waCaptureImage, 256 );
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
    // Jpeux suppr les deux lignes d'après
    //Baisser les pixels
	po8030_advanced_config(FORMAT_RGB565, 315, 235,10,10, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();
	int counter = 0;
	int counter_delayed = 0;

    while(1){
    	systime_t time;
    	time = chVTGetSystemTime();
    	 //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
		chprintf((BaseSequentialStream *)&SD3, "ct %d", chVTGetSystemTime()-time);
		counter_delayed = counter;
		if (chVTGetSystemTime()-time <= 58){
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
				}
			}
		// mettre un bool pr desactiver une fois fait one time puis reactiver apres ?
		if(counter_delayed == 10){
			chprintf((BaseSequentialStream *)&SD3, " line %c  ", 0);
		}

    }
}


void process_image_start(void){
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
