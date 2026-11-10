
import numpy as np
import tensorflow as tf

MODEL_PATH = "models/micro_speech_quantized.tflite"

# Load Google's pretrained classifier.
interpreter = tf.lite.Interpreter(
    model_path=MODEL_PATH
)

# Allocate memory for input and output tensors.
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

print("Input tensor:")
print(input_details)

print("\nOutput tensor:")
print(output_details)

# The model expects preprocessed audio features,
# NOT raw WAV samples.
#
# For now, test the interpreter with a correctly
# shaped zero-valued tensor.

input_info = input_details[0]

test_input = np.zeros(
    input_info["shape"],
    dtype=input_info["dtype"]
)

interpreter.set_tensor(
    input_info["index"],
    test_input
)

# Execute the neural network.
interpreter.invoke()

# Read its output.
output = interpreter.get_tensor(
    output_details[0]["index"]
)

print("\nRaw model output:")
print(output)

labels = ["silence", "unknown", "yes", "no"]

predicted_index = int(np.argmax(output))
print("Predicted class:", labels[predicted_index])
