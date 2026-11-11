#ifndef AUDIO_PREPROCESSOR_H
#define AUDIO_PREPROCESSOR_H

#include <stdint.h>

#define AUDIO_FEATURE_COUNT 1960

#ifdef __cplusplus
extern "C" {
#endif

int AudioPreprocessor_Init(void);

int AudioPreprocessor_Process(
    const int16_t *pcm,
    int8_t *features
);

#ifdef __cplusplus
}
#endif

#endif  