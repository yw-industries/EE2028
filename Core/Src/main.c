/******************************************************************************
* @file : main.c
* @brief : Main program body
* (c) EE2028 Teaching Team
******************************************************************************/

// // test
// #include "main.h"
// #include "uart.h"
// #include "telemetry.h"
// #include "states.h"

// volatile StateMachine stateMachine = STANDBY_MODE;

// volatile uint32_t curr_time = 0;
// static volatile uint32_t last_pressed = 0;

// int main(void)
// {
//     // Resets all peripherals, Initializes the Flash interface and the Systick.
//     HAL_Init();

//     // Module Initializations
//     UART1_Init();
//     Sensors_Init();

//     BSP_LED_Init(LED2);
//     BSP_LED_On(LED2);

//     // Configures the button wire to connect straight to the EXTI hardware watchdog
//     BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

//     // Enable the interrupt channel in the NVIC hardware controller
//     HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


//     while (1)
//     {
//         curr_time = HAL_GetTick();
//         singleclick_check();

//         // perform respective state-dependent actions

//         switch (stateMachine)
//         {
//         case STANDBY_MODE:
//         	standby_update();
//         	break;

//         case BATTLE_MODE:
//         	battle_update();
//         	break;

//         case THE_LAST_OF_EE2028:
//         	last_update();
//         	break;
//         }
//     }
// }

// void singleclick_check(void) {

// 	// check if 500ms has timed out and single click is pressed by user
// 	// update last_pressed accordingly
// 	// perform respective state-dependent actions

// 	if (curr_time - last_pressed >= 500 && last_pressed != 0) {
// 		UART_Print("Single click");
// 		last_pressed = 0;

//         switch (stateMachine)
//         {
//         case STANDBY_MODE:
//         	standby_singleclick();
//         	break;

//         case BATTLE_MODE:
//         	battle_singleclick();
//         	break;

//         case THE_LAST_OF_EE2028:
//         	last_singleclick();
//         	break;
//         }
// 	}
// }

// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {

//     if (GPIO_Pin == BUTTON_EXTI13_Pin)
//     {
//     	// if button is pressed check for double click first
//     	// then updates last_pressed respectively
//     	// perform respective state-dependent actions
//     	if (curr_time - last_pressed < 500 && last_pressed != 0) {

//     		last_pressed = 0;

//             switch (stateMachine)
//             {
//             case STANDBY_MODE:
//             	standby_doubleclick();
//             	break;

//             case BATTLE_MODE:
//             	battle_doubleclick();
//             	break;

//             case THE_LAST_OF_EE2028:
//             	last_doubleclick();
//             	break;
//             }

//     	}
//     	else {
//     		last_pressed = curr_time;
//     	}
//     }
// }



#include "main.h"

#include <stdint.h>
#include <stdio.h>
#include "ml_inference.h"


#define SETTLING_SAMPLES 256
#define UART_CHUNK_SAMPLES 128

#define AUDIO_SAMPLE_COUNT 16000





DMA_HandleTypeDef hdma_dfsdm;

void SystemClock_Config(void);

#include "microphone.h"
#include "audio_stream.h"
#include "uart.h"
#include "audio_preprocessor.h"
#include <stdio.h>

static int32_t audio_buffer[AUDIO_BUFFER_SIZE];
static int16_t pcm_chunk[UART_CHUNK_SAMPLES];

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    UART1_Init();

    char msg[100];

    // Initialize both ML models.
    if (AudioPreprocessor_Init() != 0)
    {
        UART_Print("Preprocessor init failed\r\n");
        while (1) {}
    }

    if (ML_Init() != 0)
    {
        UART_Print("Classifier init failed\r\n");
        while (1) {}
    }

    UART_Print("ML models initialized\r\n");

    // Initialize continuous microphone recording.
    if (Microphone_Init() != HAL_OK)
    {
        UART_Print("Microphone init failed\r\n");
        while (1) {}
    }

    if (Microphone_Enter() != HAL_OK)
    {
        UART_Print("Microphone start failed\r\n");
        while (1) {}
    }

    UART_Print("Continuous microphone active\r\n");

    static int16_t audio_window[16000];
    static int8_t features[1960];

    const char *labels[4] = {
        "SILENCE",
        "UNKNOWN",
        "YES",
        "NO"
    };

    uint32_t last_inference = HAL_GetTick();

    while (1)
    {
        Microphone_Update();

        if (HAL_GetTick() - last_inference >= 1000U)
        {
            last_inference = HAL_GetTick();

            // Get the most recent second of audio.
            if (AudioStream_GetWindow(
                    audio_window, 16000) != 0)
            {
                continue;
            }

            // Generate Google's audio features.
            int pre_status =
                AudioPreprocessor_Process(
                    audio_window, features
                );

            if (pre_status != 0)
            {
                UART_Print("Preprocessing failed\r\n");
                continue;
            }

            // Run the speech classifier.
            int8_t scores[4] = {0};

            int ml_status =
                ML_RunInference(features, scores);

            if (ml_status != 0)
            {
                UART_Print("Inference failed\r\n");
                continue;
            }

            // Find the highest scoring class.
            int best = 0;

            for (int i = 1; i < 4; i++)
            {
                if (scores[i] > scores[best])
                {
                    best = i;
                }
            }

            snprintf(
                msg,
                sizeof(msg),
                "Prediction: %s | Scores: %d %d %d %d\r\n",
                labels[best],
                (int)scores[0],
                (int)scores[1],
                (int)scores[2],
                (int)scores[3]
            );

            UART_Print(msg);
        }
    }
}
