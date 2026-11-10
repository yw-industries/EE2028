
import wave
import numpy as np
from scipy.signal import resample_poly
from math import gcd

INPUT_FILE = "recording.wav"
OUTPUT_FILE = "recording_16k.wav"

TARGET_RATE = 16000
TARGET_SAMPLES = 16000

with wave.open(INPUT_FILE, "rb") as wav:
    rate = wav.getframerate()
    channels = wav.getnchannels()
    width = wav.getsampwidth()
    frames = wav.getnframes()
    raw = wav.readframes(frames)

if channels != 1 or width != 2:
    raise ValueError("Expected mono, 16-bit PCM WAV")

audio = np.frombuffer(raw, dtype="<i2").astype(np.float64)

# Convert the actual sample rate to 16 kHz.
factor = gcd(rate, TARGET_RATE)
audio = resample_poly(
    audio,
    TARGET_RATE // factor,
    rate // factor
)

# Make the file exactly one second long.
if len(audio) < TARGET_SAMPLES:
    audio = np.pad(
        audio,
        (0, TARGET_SAMPLES - len(audio))
    )
else:
    audio = audio[:TARGET_SAMPLES]

audio = np.clip(
    np.round(audio), -32768, 32767
).astype("<i2")

with wave.open(OUTPUT_FILE, "wb") as wav:
    wav.setnchannels(1)
    wav.setsampwidth(2)
    wav.setframerate(TARGET_RATE)
    wav.writeframes(audio.tobytes())

print("Original rate:", rate)
print("Output rate:", TARGET_RATE)
print("Output samples:", len(audio))
print("Saved:", OUTPUT_FILE)
