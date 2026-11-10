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





// #include "microphone.h"
// #include "uart.h"
// #include <stdint.h>

// #define SETTLING_SAMPLES 256
// #define UART_CHUNK_SAMPLES 128

// static int32_t audio_buffer[AUDIO_BUFFER_SIZE];
// static int16_t pcm_chunk[UART_CHUNK_SAMPLES];

// volatile uint32_t recording_ms = 0;

// static void SendAudio(void)
// {
    
//     uint32_t index = SETTLING_SAMPLES;

//     while (index < AUDIO_BUFFER_SIZE)
//     {
//         uint16_t count = UART_CHUNK_SAMPLES;

//         if (AUDIO_BUFFER_SIZE - index < count)
//             count = AUDIO_BUFFER_SIZE - index;

//         for (uint16_t i = 0; i < count; i++)
//         {
//             /* Extract signed DFSDM result */
//             int32_t sample = audio_buffer[index + i] >> 8;

//             /*
//              * Initial scaling.
//              * We will calibrate this after listening.
//              */
//             sample = sample >> 1;

//             if (sample > 32767)
//                 sample = 32767;

//             if (sample < -32768)
//                 sample = -32768;

//             pcm_chunk[i] = (int16_t)sample;
//         }

//         HAL_UART_Transmit(
//             &huart1,
//             (uint8_t *)pcm_chunk,
//             count * sizeof(int16_t),
//             HAL_MAX_DELAY
//         );

//         index += count;
//     }
// }

// int main(void)
// {
//     HAL_Init();

//     UART1_Init();

//     if (Microphone_Init() != HAL_OK)
//     {
//         UART_Print("INIT_FAIL\n");
//         while (1) {}
//     }

//     UART_Print("READY\n");

//     uint8_t command;

//     while (1)
//     {
//         /* Wait for Python to send R */
//         if (HAL_UART_Receive(
//                 &huart1,
//                 &command,
//                 1,
//                 HAL_MAX_DELAY
//             ) != HAL_OK)
//         {
//             continue;
//         }

//         if (command != 'R')
//             continue;

//         uint32_t start_tick = HAL_GetTick();

//         if (Microphone_Start(
//                 audio_buffer,
//                 AUDIO_BUFFER_SIZE
//             ) != HAL_OK)
//         {
//             UART_Print("FAIL\n");
//             continue;
//         }

//         /* Wait for DMA recording completion */
//         while (!microphone_done && !microphone_error)
//         {
//         }
        
//         recording_ms = HAL_GetTick() - start_tick;

//         if (microphone_error)
//         {
//             Microphone_Stop();
//             UART_Print("FAIL\n");
//             continue;
//         }

//         Microphone_Stop();

//         /* Send a header before binary audio */
//         UART_Print("DATA\n");

//         SendAudio();

//         UART_Print("DONE\n");
//     }
// }
