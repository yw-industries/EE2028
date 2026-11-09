/******************************************************************************
* @file : main.c
* @brief : Main program body
* (c) EE2028 Teaching Team
******************************************************************************/

// test
#include "main.h"
#include "uart.h"
#include "telemetry.h"
#include "states.h"

volatile StateMachine stateMachine = STANDBY_MODE;

volatile uint32_t curr_time = 0;
static volatile uint32_t last_pressed = 0;

int main(void)
{
    // Resets all peripherals, Initializes the Flash interface and the Systick.
    HAL_Init();

    // Module Initializations
    UART1_Init();
    Sensors_Init();

    BSP_LED_Init(LED2);
    BSP_LED_On(LED2);

    // Configures the button wire to connect straight to the EXTI hardware watchdog
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

    // Enable the interrupt channel in the NVIC hardware controller
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


    while (1)
    {
        curr_time = HAL_GetTick();
        singleclick_check();

        // perform respective state-dependent actions

        switch (stateMachine)
        {
        case STANDBY_MODE:
        	standby_update();
        	break;

        case BATTLE_MODE:
        	battle_update();
        	break;

        case THE_LAST_OF_EE2028:
        	last_update();
        	break;
        }
    }
}

void singleclick_check(void) {

	// check if 500ms has timed out and single click is pressed by user
	// update last_pressed accordingly
	// perform respective state-dependent actions

	if (curr_time - last_pressed >= 500 && last_pressed != 0) {
		UART_Print("Single click");
		last_pressed = 0;

        switch (stateMachine)
        {
        case STANDBY_MODE:
        	standby_singleclick();
        	break;

        case BATTLE_MODE:
        	battle_singleclick();
        	break;

        case THE_LAST_OF_EE2028:
        	last_singleclick();
        	break;
        }
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    if (GPIO_Pin == BUTTON_EXTI13_Pin)
    {
    	// if button is pressed check for double click first
    	// then updates last_pressed respectively
    	// perform respective state-dependent actions
    	if (curr_time - last_pressed < 500 && last_pressed != 0) {

    		last_pressed = 0;

            switch (stateMachine)
            {
            case STANDBY_MODE:
            	standby_doubleclick();
            	break;

            case BATTLE_MODE:
            	battle_doubleclick();
            	break;

            case THE_LAST_OF_EE2028:
            	last_doubleclick();
            	break;
            }

    	}
    	else {
    		last_pressed = curr_time;
    	}
    }
}





