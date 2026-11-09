/*
 * states.c
 *
 *  Created on: Nov 1, 2026
 *      Author: yw
 */

#include "main.h"
#include "telemetry.h"
#include "uart.h"

char message_print[100];

/****************
 * STANDBY MODE
 ****************/
static int last_updated = 0;
static int last_print_timer = 0;
static int rescue_timer = 0;

void standby_update(void) {


	// read all sensors every second
	// Threshold_Check sends the violation logs itself and returns how many readings were out of range.
	// Telemetry is only sent when that is 0.

	if (curr_time - last_updated >=1000) {

		Sensors_ReadAll();

		if (Threshold_Check() == 0) {
			Telemetry_Send();
		}

		last_updated += 1000;
	}
}


void standby_singleclick(void) {
	;
}

void standby_doubleclick(void) {

	// enter battle mode
	UART_Print("Entering Battle Mode\r\n");
	stateMachine = BATTLE_MODE;
}


/****************
 * BATTLE MODE
 ****************/

void battle_update(void) {

	// blink the led at 1Hz
	if (curr_time - last_updated >= 500) {
		BSP_LED_Toggle(LED2);
		last_updated += 500;
	}

	Sensors_ReadAll();
	// check if gyroscope is upside down
	if (IsUpsideDown() == 1) {
		UART_Print("The Last Of EE2028\n");
		stateMachine = THE_LAST_OF_EE2028;
		last_print_timer = curr_time;
		rescue_timer = curr_time;
	}
}

void battle_singleclick(void) {

	//charge gun and automatically fire if energy exceeds 5
	static int gun_energy = 0;
	gun_energy += 3;

	if (gun_energy >= 5) {
		gun_energy -= 5;
		sprintf(message_print, "Gun fired! Energy remaining %d/10\r\n", gun_energy);
		UART_Print(message_print);
	}
	else {
		sprintf(message_print, "Gun charged! Energy remaining %d/10\r\n", gun_energy);
		UART_Print(message_print);
	}
}

void battle_doubleclick(void) {

	// entering standby mode
	UART_Print("Entering Standby Mode\r\n");
	stateMachine = STANDBY_MODE;
	BSP_LED_On(LED2);
}


/********************
 * THE LAST OF EE2028
 ********************/

void last_update(void) {

	// LED blinks every 500ms
	if (curr_time - last_updated >= 250) {
		BSP_LED_Toggle(LED2);
		last_updated += 250;
	}

	// print the message every 1s
	if (curr_time - last_print_timer >= 1000) {
		UART_Print("Drone Was Attacked\r\n");
		last_print_timer += 1000;
	}

	// terminate programme using an empty while loop
	if (curr_time - rescue_timer >= 10000){
		UART_Print("Rescue came too late\r\n");
		BSP_LED_Off(LED2);
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

		while (1) {
			;
		}
	}


}

void last_singleclick(void) {
	;
}

void last_doubleclick (void) {

    // clear and return to battle mode
	UART_Print("The Last Of EE2028 cleared!\r\n");
	stateMachine = BATTLE_MODE;
}


