
import wave
import numpy as np
from scipy.signal import get_window

SAMPLE_RATE = 16000
WINDOW_SIZE = 480
WINDOW_STRIDE = 320
NUM_FRAMES = 49
NUM_BANDS = 40
FFT_SIZE = 512

def load_audio(path):
    with wave.open(path, "rb") as wav:
        if wav.getframerate() != SAMPLE_RATE:
            raise ValueError("Audio must be 16 kHz")
        if wav.getnchannels() != 1:
            raise ValueError("Audio must be mono")
        if wav.getsampwidth() != 2:
            raise ValueError("Audio must be 16-bit PCM")

        raw = wav.readframes(wav.getnframes())

    audio = np.frombuffer(raw, dtype="<i2")
    return audio.astype(np.float64) / 32768.0


def make_frames(audio):
    required = (NUM_FRAMES - 1) * WINDOW_STRIDE + WINDOW_SIZE

    if len(audio) < required:
        audio = np.pad(audio, (0, required - len(audio)))

    frames = np.stack([
        audio[i * WINDOW_STRIDE:
              i * WINDOW_STRIDE + WINDOW_SIZE]
        for i in range(NUM_FRAMES)
    ])

    return frames


def make_features(audio):
    frames = make_frames(audio)

    # Apply a Hann window to reduce spectral leakage.
    window = get_window(
        "hann", WINDOW_SIZE, fftbins=False
    )

    windowed = frames * window

    # Frequency-domain representation.
    spectrum = np.fft.rfft(
        windowed, n=FFT_SIZE, axis=1
    )

    power = np.abs(spectrum) ** 2

    # Construct 40 approximately mel-spaced bands.
    def hz_to_mel(hz):
        return 2595.0 * np.log10(1.0 + hz / 700.0)

    def mel_to_hz(mel):
        return 700.0 * (10.0 ** (mel / 2595.0) - 1.0)

    mel_points = np.linspace(
        hz_to_mel(125),
        hz_to_mel(7500),
        NUM_BANDS + 2
    )

    hz_points = mel_to_hz(mel_points)
    frequencies = np.fft.rfftfreq(
        FFT_SIZE, d=1.0 / SAMPLE_RATE
    )

    filters = np.zeros(
        (NUM_BANDS, len(frequencies))
    )

    for band in range(NUM_BANDS):
        left, center, right = hz_points[band:band + 3]

        filters[band] = np.maximum(
            0,
            np.minimum(
                (frequencies - left) / (center - left),
                (right - frequencies) / (right - center)
            )
        )

    energies = power @ filters.T

    # Log compression for visualization and experimentation.
    features = np.log1p(energies)

    return features


if __name__ == "__main__":
    audio = load_audio("recording_16k.wav")
    features = make_features(audio)

    print("Audio samples:", len(audio))
    print("Feature shape:", features.shape)
    print("Minimum:", features.min())
    print("Maximum:", features.max())
    print("First frame:", features[0])
