
#include "ml_inference.h"
#include "models/micro_speech_model.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/schema/schema_generated.h"

#include <cstring>

namespace {

constexpr int kTensorArenaSize = 100 * 1024;

alignas(16) static uint8_t tensor_arena[kTensorArenaSize];

static const tflite::Model *model = nullptr;

static tflite::MicroInterpreter *interpreter = nullptr;

static TfLiteTensor *input = nullptr;
static TfLiteTensor *output = nullptr;

static bool initialized = false;

}  // namespace

extern "C" int ML_Init(void)
{
    if (initialized)
        return 0;

    model = tflite::GetModel(micro_speech_model);

    if (model == nullptr)
        return 1;

    if (model->version() != TFLITE_SCHEMA_VERSION)
        return 2;

    static tflite::MicroMutableOpResolver<4> resolver;

    if (resolver.AddDepthwiseConv2D() != kTfLiteOk)
        return 3;

    if (resolver.AddFullyConnected() != kTfLiteOk)
        return 4;

    if (resolver.AddReshape() != kTfLiteOk)
        return 5;

    if (resolver.AddSoftmax() != kTfLiteOk)
        return 6;

    static tflite::MicroInterpreter static_interpreter(
        model,
        resolver,
        tensor_arena,
        kTensorArenaSize
    );

    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk)
        return 7;

    input = interpreter->input(0);
    output = interpreter->output(0);

    if (input == nullptr || output == nullptr)
        return 8;

    if (input->type != kTfLiteInt8)
        return 9;

    if (output->type != kTfLiteInt8)
        return 10;

    if (input->bytes != 1960)
        return 11;

    if (output->bytes != 4)
        return 12;

    initialized = true;
    return 0;
}

extern "C" int ML_RunInference(
    const int8_t *features,
    int8_t *scores
)
{
    if (!initialized || features == nullptr || scores == nullptr)
        return 1;

    memcpy(input->data.int8, features, input->bytes);

    if (interpreter->Invoke() != kTfLiteOk)
        return 2;

    memcpy(scores, output->data.int8, output->bytes);

    return 0;
}
