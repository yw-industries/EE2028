#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <stdint.h>
#include "stm32l4xx_hal.h"

#define AUDIO_SAMPLE_RATE 16000U
#define AUDIO_WINDOW_SAMPLES 16000U
#define AUDIO_RING_SAMPLES 32768U

void AudioStream_Reset(void);

void AudioStream_PushRaw(
    const int32_t *samples,
    uint32_t count
);

int AudioStream_GetWindow(
    int16_t *destination,
    uint32_t count
);

uint32_t AudioStream_GetTotalSamples(void);

#endif