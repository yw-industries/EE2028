
#ifndef MICROPHONE_H
#define MICROPHONE_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define AUDIO_BUFFER_SIZE 1024

HAL_StatusTypeDef Microphone_Init(void);

HAL_StatusTypeDef Microphone_Start(
    int32_t *buffer,
    uint32_t sample_count
);

HAL_StatusTypeDef Microphone_Stop(void);

#endif
