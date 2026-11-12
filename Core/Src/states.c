/*
 * states.c
 *
 *  Created on: Nov 1, 2026
 *      Author: yw
 */
#include "states.h"
#include "audio_preprocessor.h"
#include "wifi_link.h"
char message_print[100];

/****************
 * STANDBY MODE
 ****************/
static int last_updated = 0;
static int last_print_timer = 0;
static int rescue_timer = 0;

void standby_update(void) {


	// read all sensors every second
	// Threshold_Check sends the violation logs itself and returns how many readings were out of range.
	// Telemetry is only sent when that is 0.

	if (curr_time - last_updated >=1000) {

		Sensors_ReadAll();

		if (Threshold_Check() == 0) {
			Telemetry_Send();
		}

		last_updated += 1000;
	}
}

void standby_singleclick(void) {
	;
}

void standby_doubleclick(void) {

	// enter battle mode
	UART_Print("Entering Battle Mode\r\n");
	stateMachine = BATTLE_MODE;
}

/****************
 * BATTLE MODE
 ****************/

void battle_update(void) {

	// blink the led at 1Hz
	if (curr_time - last_updated >= 500) {
		BSP_LED_Toggle(LED2);
		last_updated += 500;
	}

	Sensors_ReadAll();
	// check if gyroscope is upside down
	if (IsUpsideDown() == 1) {
		UART_Print("The Last Of EE2028\n");
		stateMachine = THE_LAST_OF_EE2028;
		last_print_timer = curr_time;
		rescue_timer = curr_time;
		SpeechRecognition_Enter();
	}
}

void battle_singleclick(void) {

	//charge gun and automatically fire if energy exceeds 5
	static int gun_energy = 0;
	gun_energy += 3;

	if (gun_energy >= 5) {
		gun_energy -= 5;
		sprintf(message_print, "Gun fired! Energy remaining %d/10\r\n", gun_energy);
		UART_Print(message_print);
	}
	else {
		sprintf(message_print, "Gun charged! Energy remaining %d/10\r\n", gun_energy);
		UART_Print(message_print);
	}
}

void battle_doubleclick(void) {

	// entering standby mode
	UART_Print("Entering Standby Mode\r\n");
	stateMachine = STANDBY_MODE;
	BSP_LED_On(LED2);
}


/********************
 * THE LAST OF EE2028
 ********************/

void last_update(void) {

	// LED blinks every 500ms
	if (curr_time - last_updated >= 250) {
		BSP_LED_Toggle(LED2);
		last_updated += 250;
	}

	// print the message every 1s
	if (curr_time - last_print_timer >= 1000) {
		UART_Print("Drone Was Attacked\r\n");
		last_print_timer += 1000;
	}

	// terminate programme using an empty while loop
	if (curr_time - rescue_timer >= 10000){
		UART_Print("Rescue came too late\r\n");
		BSP_LED_Off(LED2);
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

		while (1) {
			;
		}
	}


}

void last_singleclick(void) {
	;
}

void last_doubleclick (void) {

    // clear and return to battle mode
	SpeechRecognition_Exit();
	Comms_Safe();               // clear the help alert on the dashboard
	UART_Print("The Last Of EE2028 cleared!\r\n");
	stateMachine = BATTLE_MODE;
}

/* ============================================
   SPEECH RECOGNITION
   ============================================ */

#define SPEECH_WINDOW_SAMPLES 16000U
#define SPEECH_STRIDE_SAMPLES 12000U
#define SPEECH_FEATURE_COUNT 1960U

#define SPEECH_CONFIDENCE_THRESHOLD 0.80f
#define SPEECH_COOLDOWN_MS 1500U

static uint8_t speech_active = 0;

static int16_t speech_audio[SPEECH_WINDOW_SAMPLES];
static int8_t speech_features[SPEECH_FEATURE_COUNT];

static uint32_t last_inference_sample = 0;
static uint32_t last_detection_ms = 0;
static uint8_t detection_occurred = 0;
static uint8_t help_sent = 0;      // one help request per emergency

static const char *speech_labels[4] = {
    "SILENCE",
    "UNKNOWN",
    "YES",
    "NO"
};

/* Called when entering EE2028 mode */
void SpeechRecognition_Enter(void)
{
    if (speech_active)
        return;

    // Reset detection history.
    last_inference_sample = 0;
    last_detection_ms = 0;
    detection_occurred = 0;
    help_sent = 0;

    // Microphone_Enter resets audio history
    // and starts circular DMA.
    if (Microphone_Enter() == HAL_OK)
    {
        speech_active = 1;
        UART_Print("Speech recognition enabled\r\n");
    }
    else
    {
        UART_Print("Speech recognition start failed\r\n");
    }
}

/* Called repeatedly while in EE2028 mode */
void SpeechRecognition_Update(void)
{
    if (!speech_active)
        return;

    Microphone_Update();

    uint32_t total = AudioStream_GetTotalSamples();

    if (total < SPEECH_WINDOW_SAMPLES)
        return;

    // Wait for 12000 new samples (~750 ms).
    if (last_inference_sample != 0 &&
        (uint32_t)(total - last_inference_sample)
            < SPEECH_STRIDE_SAMPLES)
    {
        return;
    }

    if (AudioStream_GetWindow(
            speech_audio,
            SPEECH_WINDOW_SAMPLES) != 0)
    {
        return;
    }

    last_inference_sample = total;

    // Generate Google's audio features.
    if (AudioPreprocessor_Process(
            speech_audio,
            speech_features) != 0)
    {
        return;
    }

    // Run classifier.
    int8_t scores[4] = {0};

    if (ML_RunInference(speech_features, scores) != 0)
    {
        return;
    }

    // Find highest-scoring class.
    int best = 0;

    for (int i = 1; i < 4; i++)
    {
        if (scores[i] > scores[best])
            best = i;
    }

    // Assumes softmax scale = 1/256
    // and zero point = -128.
    float confidence =
        ((float)scores[best] + 128.0f) / 256.0f;

    uint32_t now = HAL_GetTick();

    if (best == 2 || best == 3)
    {
        // Require 80% confidence for YES/NO.
        if (confidence < SPEECH_CONFIDENCE_THRESHOLD)
        {
            UART_Print("UNKNOWN\r\n");
            return;
        }

        // Prevent repeated detections.
        if (detection_occurred &&
            (uint32_t)(now - last_detection_ms)
                < SPEECH_COOLDOWN_MS)
        {
            return;
        }

        last_detection_ms = now;
        detection_occurred = 1;
    }

    UART_Print(speech_labels[best]);
    UART_Print("\r\n");

    // A confident YES means a human is present: call the other drone for help
    if (best == 2 && !help_sent)
    {
        help_sent = 1;
        Comms_Print("Human confirmed - calling for rescue\r\n");
        Comms_RequestHelp("YES");
    }
}


/* Called when leaving EE2028 mode */
void SpeechRecognition_Exit(void)
{
    if (!speech_active)
        return;

    if (Microphone_Exit() == HAL_OK)
    {
        speech_active = 0;
        UART_Print("Speech recognition disabled\r\n");
    }
    else
    {
        UART_Print("Speech recognition stop failed\r\n");
    }
}