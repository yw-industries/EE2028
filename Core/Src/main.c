/******************************************************************************
* @file : main.c
* @brief : Main program body
* (c) EE2028 Teaching Team
******************************************************************************/

#include "main.h"

#include <stdint.h>
#include <stdio.h>
#include "ml_inference.h"
#include "uart.h"
#include "telemetry.h"
#include "states.h"

volatile StateMachine stateMachine = STANDBY_MODE;

volatile uint32_t curr_time = 0;
static volatile uint32_t last_pressed = 0;

void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    UART1_Init();
    Sensors_Init();

    BSP_LED_Init(LED2);
    BSP_LED_On(LED2);

    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    // Initialize both ML interpreters once.
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

    // Configure microphone, but don't start it.
    if (Microphone_Init() != HAL_OK)
    {
        UART_Print("Microphone init failed\r\n");
        while (1) {}
    }

    UART_Print("System ready\r\n");

    while (1)
    {
        curr_time = HAL_GetTick();

        singleclick_check();

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

            default:
                break;
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




/* ============================================================================
 * WIFI TEST PROGRAM (commented out)
 * Tested and working. To re-enable: uncomment this block and comment out the
 * speech recognition program below (only one main() may be active).
 * ========================================================================== */

// #include "main.h"
// #include "uart.h"
// #include "wifi_link.h"
// #include "es_wifi_io.h"
// #include <stdio.h>
//
// void SystemClock_Config(void);
//
// int main(void)
// {
//     char rx[64];
//     char msg[64];
//     uint32_t last_send = 0;
//     uint32_t count = 0;
//
//     HAL_Init();
//     SystemClock_Config();     // 80 MHz
//     UART1_Init();             // after the clock change so the baud rate is correct
//     UART_Print("\r\n--- WiFi test ---\r\n");
//
//     if (WifiLink_Init() != 0) {
//         UART_Print("WiFi init failed, stopping.\r\n");
//         while (1) {}
//     }
//     WifiLink_Send("Hello from STM32\r\n");
//
//     while (1)
//     {
//         // send a message to the laptop every second
//         if (HAL_GetTick() - last_send >= 1000) {
//             last_send = HAL_GetTick();
//             snprintf(msg, sizeof(msg), "tick %lu\r\n", (unsigned long)count++);
//             WifiLink_Send(msg);
//         }
//
//         // print anything typed in server.py
//         if (WifiLink_Receive(rx, sizeof(rx), 10) > 0) {
//             UART_Print("From laptop: ");
//             UART_Print(rx);
//         }
//     }
// }
//
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     if (GPIO_Pin == GPIO_PIN_1) {
//         SPI_WIFI_ISR();       // WiFi module data-ready pin
//     }
// }

