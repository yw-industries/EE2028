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