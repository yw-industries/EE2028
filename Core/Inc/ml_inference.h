#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int ML_Init(void);
int ML_RunInference(const int8_t *features, int8_t *scores);

#ifdef __cplusplus
}
#endif

#endif