#ifndef MICROPHONE_H
#define MICROPHONE_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define AUDIO_BUFFER_SIZE 31250

HAL_StatusTypeDef Microphone_Init(void);

HAL_StatusTypeDef Microphone_Start(
    int32_t *buffer,
    uint32_t length
);

HAL_StatusTypeDef Microphone_Stop(void);

extern volatile uint8_t microphone_done;
extern volatile uint8_t microphone_error;

#endif