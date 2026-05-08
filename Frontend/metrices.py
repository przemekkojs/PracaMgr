import librosa
import numpy as np

REF:str = "../Backend/local/test/ref.wav"
SYNTH:str = "../Backend/local/test/synth.wav"
MODEL:str = "../Backend/local/test/model.wav"

def lsd(ref:str=REF, synth:str=SYNTH, model:str=MODEL) -> tuple[float, float, float]:
    y1, _ = librosa.load(ref, sr=None, mono=True)
    y2, _ = librosa.load(synth, sr=None, mono=True)
    y3, _ = librosa.load(model, sr=None, mono=True)

    min_len = min(len(y1), len(y2), len(y3))
    y1 = y1[:min_len]
    y2 = y2[:min_len]
    y3 = y3[:min_len]

    eps = 1e-10

    y1 = y1 / (np.sqrt(np.mean(y1 ** 2)) + eps)
    y2 = y2 / (np.sqrt(np.mean(y2 ** 2)) + eps)
    y3 = y3 / (np.sqrt(np.mean(y3 ** 2)) + eps)

    S1 = np.abs(librosa.stft(y1, n_fft=2048, hop_length=512))
    S2 = np.abs(librosa.stft(y2, n_fft=2048, hop_length=512))
    S3 = np.abs(librosa.stft(y3, n_fft=2048, hop_length=512))

    log_S1 = np.log(S1 + eps)
    log_S2 = np.log(S2 + eps)
    log_S3 = np.log(S3 + eps)

    frame_lsd_12 = np.sqrt(np.mean((log_S1 - log_S2) ** 2, axis=0))
    frame_lsd_13 = np.sqrt(np.mean((log_S1 - log_S3) ** 2, axis=0))
    frame_lsd_23 = np.sqrt(np.mean((log_S2 - log_S3) ** 2, axis=0))

    lsd_12 = np.mean(frame_lsd_12)
    lsd_13 = np.mean(frame_lsd_13)
    lsd_23 = np.mean(frame_lsd_23)

    return float(lsd_12), float(lsd_13), float(lsd_23)