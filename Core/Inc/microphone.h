#ifndef MICROPHONE_H
#define MICROPHONE_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include "audio_stream.h"

#define AUDIO_BUFFER_SIZE 31250
#define AUDIO_DMA_HALF_SIZE 512
#define AUDIO_DMA_BUFFER_SIZE (AUDIO_DMA_HALF_SIZE * 2)

HAL_StatusTypeDef Microphone_Init(void);

HAL_StatusTypeDef Microphone_Start(
    int32_t *buffer,
    uint32_t length
);

HAL_StatusTypeDef Microphone_Stop(void);

extern volatile uint8_t microphone_done;
extern volatile uint8_t microphone_error;
extern DMA_HandleTypeDef hdma_dfsdm;

extern volatile uint32_t microphone_end_tick;   
extern int32_t audio_dma_buffer[AUDIO_DMA_BUFFER_SIZE];

extern volatile uint8_t audio_half_ready;
extern volatile uint8_t audio_full_ready;

HAL_StatusTypeDef Microphone_StartContinuous(void);
HAL_StatusTypeDef Microphone_StopContinuous(void);

HAL_StatusTypeDef Microphone_Enter(void);
void Microphone_Update(void);
HAL_StatusTypeDef Microphone_Exit(void);

#endif

