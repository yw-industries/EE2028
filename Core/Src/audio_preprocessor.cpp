
#include "audio_preprocessor.h"
#include "models/audio_preprocessor_model.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/schema/schema_generated.h"

#include <cstring>

namespace {

constexpr int kArenaSize = 128 * 1024;
constexpr int kFrameSamples = 480;
constexpr int kFrameStride = 320;
constexpr int kFeatureChannels = 40;
constexpr int kFeatureFrames = 49;

alignas(16) static uint8_t arena[kArenaSize];

static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input = nullptr;
static TfLiteTensor* output = nullptr;

static bool initialized = false;

} // namespace

extern "C" int AudioPreprocessor_Init(void)
{
    if (initialized)
        return 0;

    const tflite::Model* model =
        tflite::GetModel(audio_preprocessor_model);

    if (model == nullptr)
        return 1;

    if (model->version() != TFLITE_SCHEMA_VERSION)
        return 2;

    static tflite::MicroMutableOpResolver<18> resolver;

    if (resolver.AddReshape() != kTfLiteOk) return 3;
    if (resolver.AddCast() != kTfLiteOk) return 3;
    if (resolver.AddStridedSlice() != kTfLiteOk) return 3;
    if (resolver.AddConcatenation() != kTfLiteOk) return 3;
    if (resolver.AddMul() != kTfLiteOk) return 3;
    if (resolver.AddAdd() != kTfLiteOk) return 3;
    if (resolver.AddDiv() != kTfLiteOk) return 3;
    if (resolver.AddMinimum() != kTfLiteOk) return 3;
    if (resolver.AddMaximum() != kTfLiteOk) return 3;

    if (resolver.AddWindow() != kTfLiteOk) return 3;
    if (resolver.AddFftAutoScale() != kTfLiteOk) return 3;
    if (resolver.AddRfft() != kTfLiteOk) return 3;
    if (resolver.AddEnergy() != kTfLiteOk) return 3;
    if (resolver.AddFilterBank() != kTfLiteOk) return 3;
    if (resolver.AddFilterBankSquareRoot() != kTfLiteOk) return 3;
    if (resolver.AddFilterBankSpectralSubtraction() != kTfLiteOk) return 3;
    if (resolver.AddPCAN() != kTfLiteOk) return 3;
    if (resolver.AddFilterBankLog() != kTfLiteOk) return 3;

    static tflite::MicroInterpreter static_interpreter(
        model, resolver, arena, kArenaSize
    );

    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk)
        return 4;

    input = interpreter->input(0);
    output = interpreter->output(0);

    if (input == nullptr || output == nullptr)
        return 5;

    if (input->type != kTfLiteInt16)
        return 6;

    if (output->type != kTfLiteInt8)
        return 7;

    if (input->bytes != kFrameSamples * sizeof(int16_t))
        return 8;

    if (output->bytes != kFeatureChannels)
        return 9;

    initialized = true;
    return 0;
}

extern "C" int AudioPreprocessor_Process(
    const int16_t* pcm,
    int8_t* features)
{
    if (!initialized || pcm == nullptr || features == nullptr)
        return 1;

    for (int frame = 0; frame < kFeatureFrames; frame++)
    {
        const int16_t* frame_start =
            pcm + frame * kFrameStride;

        memcpy(
            input->data.i16,
            frame_start,
            kFrameSamples * sizeof(int16_t)
        );

        if (interpreter->Invoke() != kTfLiteOk)
            return 2;

        memcpy(
            features + frame * kFeatureChannels,
            output->data.int8,
            kFeatureChannels
        );
    }

    return 0;
}
