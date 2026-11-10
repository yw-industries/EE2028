import serial
import wave
import time

PORT = "COM7"
BAUD = 115200

SAMPLE_RATE = 10149
SAMPLE_COUNT = 31250 - 256
EXPECTED_BYTES = SAMPLE_COUNT * 2

with serial.Serial(PORT, BAUD, timeout=10) as ser:
    time.sleep(2)
    ser.reset_input_buffer()

    input("Press Enter to start recording...")

    ser.write(b"R")

    response = ser.readline().strip()
    print("STM32 response:", response)

    if response != b"DATA":
        raise RuntimeError("STM32 did not send DATA")

    audio = ser.read(EXPECTED_BYTES)

    if len(audio) != EXPECTED_BYTES:
        raise RuntimeError(
            f"Expected {EXPECTED_BYTES} bytes, "
            f"received {len(audio)}"
        )

    response = ser.readline().strip()
    print("STM32 status:", response)

    if response != b"DONE":
        raise RuntimeError("STM32 transfer incomplete")

    with wave.open("recording.wav", "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(SAMPLE_RATE)
        wav.writeframes(audio)

    print("Saved recording.wav")
