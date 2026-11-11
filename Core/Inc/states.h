/*
 * states.h
 *
 *  Created on: Nov 1, 2026
 *      Author: yw
 */




#ifndef INC_STATES_H_

#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "telemetry.h"
#include "uart.h"
#include "audio_stream.h"
#include "ml_inference.h"
#include "microphone.h"

#define INC_STATES_H_

void standby_update(void);
void standby_singleclick(void);
void standby_doubleclick(void);

void battle_update(void);
void battle_singleclick(void);
void battle_doubleclick(void);

void last_update(void);
void last_singleclick(void);
void last_doubleclick(void);

void SpeechDetector_SetEnabled(uint8_t enabled);
uint8_t SpeechDetector_IsEnabled(void);
void SpeechDetector_Process(void);

void States_EnableSpeechDetection(void);
void States_DisableSpeechDetection(void);
void States_ProcessSpeechDetection(void);


#endif 
/* INC_STATES_H_ */
