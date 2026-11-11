import serial
import wave
import time

PORT = "COM7"
BAUD = 115200

SAMPLE_RATE = 16000
SAMPLE_COUNT = 16000 - 256
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

    print("Expected bytes:", EXPECTED_BYTES)
    print("Received bytes:", len(audio))

    extra = ser.read(64)

    print("Next 64 bytes:", repr(extra))


    with wave.open("recording.wav", "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(SAMPLE_RATE)
        wav.writeframes(audio)

    print("Saved recording.wav")
