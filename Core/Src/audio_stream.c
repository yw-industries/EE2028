
#include "audio_stream.h"
#include "stm32l4xx_hal.h"

#include <string.h>

static int16_t audio_ring[AUDIO_RING_SAMPLES];

static volatile uint32_t write_index = 0;
static volatile uint32_t total_samples = 0;
static volatile uint32_t audio_version = 0;

void AudioStream_Reset(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    audio_version = 0;
    write_index = 0;
    total_samples = 0;

    __set_PRIMASK(primask);
}

void AudioStream_PushRaw(const int32_t *samples, uint32_t count)
{
    audio_version++;  // Copy/update in progress

    for (uint32_t i = 0; i < count; i++)
    {
        int32_t sample = samples[i] >> 8;
        sample >>= 1;

        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;

        audio_ring[write_index] = (int16_t)sample;

        write_index =
            (write_index + 1U) & (AUDIO_RING_SAMPLES - 1U);

        total_samples++;
    }

    audio_version++;  // Update completed
}

int AudioStream_GetWindow(int16_t *destination, uint32_t count)
{
    if (destination == NULL ||
        count > AUDIO_RING_SAMPLES)
        return -1;

    for (int attempt = 0; attempt < 3; attempt++)
    {
        uint32_t version_before;
        uint32_t index;
        uint32_t total;

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        version_before = audio_version;
        index = write_index;
        total = total_samples;

        __set_PRIMASK(primask);

        if (total < count)
            return -2;

        if (version_before & 1U)
            continue;

        uint32_t start =
            (index + AUDIO_RING_SAMPLES - count)
            & (AUDIO_RING_SAMPLES - 1U);

        for (uint32_t i = 0; i < count; i++)
        {
            destination[i] =
                audio_ring[(start + i) & (AUDIO_RING_SAMPLES - 1U)];
        }

        primask = __get_PRIMASK();
        __disable_irq();

        uint32_t version_after = audio_version;

        __set_PRIMASK(primask);

        if (version_before == version_after)
            return 0;
    }

    return -3; // Audio changed during snapshot
}

uint32_t AudioStream_GetTotalSamples(void)
{
    return total_samples;
}
