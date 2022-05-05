#include <system_clock_config.h>
#include <gpio.h>
#include <main.h>
#include <timer.h>
#include <motor.h>
#include <selector.h>


void blinky_led(){
		gpio_config_output_pushpull(FRONT_LED);

		while (1) {
		delay(SystemCoreClock/16);
		gpio_toggle(FRONT_LED);
		}
}
