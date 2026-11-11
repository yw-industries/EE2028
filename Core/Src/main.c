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

#include "microphone.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>
#include "ml_inference.h"

#define SETTLING_SAMPLES 256
#define UART_CHUNK_SAMPLES 128

static int32_t audio_buffer[AUDIO_BUFFER_SIZE];
static int16_t pcm_chunk[UART_CHUNK_SAMPLES];

volatile uint32_t inference_ms = 0;

void SystemClock_Config(void);


int main(void) {
    char message[100];

    HAL_Init();

    SystemClock_Config();
    uint32_t clock_source =
    __HAL_RCC_GET_SYSCLK_SOURCE();

    snprintf(message, sizeof(message),
         "SYSCLK source: 0x%08lX\r\n",
         (unsigned long)clock_source);

UART_Print(message);

    UART1_Init();

    UART_Print("Starting TFLite Micro...\r\n");


snprintf(
    message,
    sizeof(message),
    "SYSCLK: %lu Hz\r\n",
    (unsigned long)HAL_RCC_GetSysClockFreq()
);
UART_Print(message);

snprintf(
    message,
    sizeof(message),
    "HCLK: %lu Hz\r\n",
    (unsigned long)HAL_RCC_GetHCLKFreq()
);
UART_Print(message);

snprintf(
    message,
    sizeof(message),
    "PCLK1: %lu Hz\r\n",
    (unsigned long)HAL_RCC_GetPCLK1Freq()
);
UART_Print(message);

snprintf(
    message,
    sizeof(message),
    "PCLK2: %lu Hz\r\n",
    (unsigned long)HAL_RCC_GetPCLK2Freq()
);
UART_Print(message);


uint32_t start = HAL_GetTick();

HAL_Delay(1000);

uint32_t elapsed = HAL_GetTick() - start;

snprintf(
    message,
    sizeof(message),
    "HAL tick test: %lu ms\r\n",
    (unsigned long)elapsed
);

UART_Print(message);


    UART_Print("Starting TFLite Micro...\r\n");
    int ml_status = ML_Init();
    // SystemClock_Config();

    snprintf(
        message,
        sizeof(message),
        "HCLK: %lu Hz\r\n",
        (unsigned long)HAL_RCC_GetHCLKFreq()
    );

    


    snprintf(
        message,
        sizeof(message),
        "ML_Init status: %d\r\n",
        ml_status
    );

    UART_Print(message);
    if (ml_status == 0)
    {
        static int8_t test_features[1960] = {0};
        int8_t scores[4] = {0};

        uint32_t start_tick = HAL_GetTick();

        int result = ML_RunInference(
            test_features,
            scores
        );

        inference_ms = HAL_GetTick() - start_tick;

        snprintf(
            message,
            sizeof(message),
            "Inference time: %lu ms\r\n",
            (unsigned long)inference_ms
        );

        UART_Print(message);

        snprintf(
            message,
            sizeof(message),
            "Inference status: %d\r\n",
            result
        );

        UART_Print(message);

        if (result == 0)
        {
            snprintf(
                message,
                sizeof(message),
                "Scores: %d %d %d %d\r\n",
                scores[0],
                scores[1],
                scores[2],
                scores[3]
            );

            UART_Print(message);

            const char *labels[4] = {
                "SILENCE",
                "UNKNOWN",
                "YES",
                "NO"
            };

            int best = 0;

            for (int i = 1; i < 4; i++)
            {
                if (scores[i] > scores[best])
                    best = i;
            }

            snprintf(
                message,
                sizeof(message),
                "Prediction: %s\r\n",
                labels[best]
            );

            UART_Print(message);
        }
    }

}



void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure voltage scaling for high-frequency operation */
    __HAL_RCC_PWR_CLK_ENABLE();

    if (HAL_PWREx_ControlVoltageScaling(
            PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        while (1) {}
    }

    /* Configure MSI and PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue =
        RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1) {}
    }

    /* Configure system clock */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_4) != HAL_OK)
    {
        while (1) {}
    }
}



static void SendAudio(void)
{
    
    uint32_t index = SETTLING_SAMPLES;

    while (index < AUDIO_BUFFER_SIZE)
    {
        uint16_t count = UART_CHUNK_SAMPLES;

        if (AUDIO_BUFFER_SIZE - index < count)
            count = AUDIO_BUFFER_SIZE - index;

        for (uint16_t i = 0; i < count; i++)
        {
            /* Extract signed DFSDM result */
            int32_t sample = audio_buffer[index + i] >> 8;

            /*
             * Initial scaling.
             * We will calibrate this after listening.
             */
            sample = sample >> 1;

            if (sample > 32767)
                sample = 32767;

            if (sample < -32768)
                sample = -32768;

            pcm_chunk[i] = (int16_t)sample;
        }

        HAL_UART_Transmit(
            &huart1,
            (uint8_t *)pcm_chunk,
            count * sizeof(int16_t),
            HAL_MAX_DELAY
        );

        index += count;
    }
}

// int main(void)
// {
//     HAL_Init();

//     UART1_Init();
//     char message[64];

//     snprintf(message, sizeof(message),
//             "ML C++ test: %d\r\n",
//             ML_Test());

//     UART_Print(message);

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

