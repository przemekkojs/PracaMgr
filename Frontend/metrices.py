import librosa
import numpy as np

REF:str = "../Backend/local/test/ref.wav"
SYNTH:str = "../Backend/local/test/synth.wav"
MODEL:str = "../Backend/local/test/model.wav"

def lsd(ref:str=REF, synth:str=SYNTH, model:str=MODEL) -> tuple[float, float, float]:
    '''
    ### params
    - ref: reference sample path
    - synth: synth sample path
    - model: model sample path
    ### returns    
    (ref-to-synth, ref-to-model, synth-to-model)
    '''
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

def _load_and_prepare(ref: str, synth: str, model: str, eps: float = 1e-10):
    y1, sr1 = librosa.load(ref, sr=None, mono=True)
    y2, sr2 = librosa.load(synth, sr=None, mono=True)
    y3, sr3 = librosa.load(model, sr=None, mono=True)

    if sr1 != sr2 or sr1 != sr3:
        raise ValueError(f"Sample rates differ: ref={sr1}, synth={sr2}, model={sr3}")

    min_len = min(len(y1), len(y2), len(y3))
    y1, y2, y3 = y1[:min_len], y2[:min_len], y3[:min_len]

    y1 = y1 / (np.sqrt(np.mean(y1 ** 2)) + eps)
    y2 = y2 / (np.sqrt(np.mean(y2 ** 2)) + eps)
    y3 = y3 / (np.sqrt(np.mean(y3 ** 2)) + eps)

    return y1, y2, y3, sr1


def _pairwise(a: float, b: float, c: float) -> tuple[float, float, float]:
    return float(a), float(b), float(c)

def sc(ref: str = REF, synth: str = SYNTH, model: str = MODEL) -> tuple[float, float, float]:
    """
    Spectral Convergence.
    Lower value means higher spectral similarity.
    Returns:
    (ref-to-synth, ref-to-model, synth-to-model)
    """
    y1, y2, y3, _ = _load_and_prepare(ref, synth, model)

    S1 = np.abs(librosa.stft(y1, n_fft=2048, hop_length=512))
    S2 = np.abs(librosa.stft(y2, n_fft=2048, hop_length=512))
    S3 = np.abs(librosa.stft(y3, n_fft=2048, hop_length=512))

    sc_12 = np.linalg.norm(S1 - S2, ord="fro") / np.linalg.norm(S1, ord="fro")
    sc_13 = np.linalg.norm(S1 - S3, ord="fro") / np.linalg.norm(S1, ord="fro")
    sc_23 = np.linalg.norm(S2 - S3, ord="fro") / np.linalg.norm(S2, ord="fro")

    return _pairwise(sc_12, sc_13, sc_23)

def mfcc(ref: str = REF, synth: str = SYNTH, model: str = MODEL) -> tuple[float, float, float]:
    """
    MFCC distance.
    Lower value means higher timbral similarity.
    Returns:
    (ref-to-synth, ref-to-model, synth-to-model)
    """
    y1, y2, y3, sr = _load_and_prepare(ref, synth, model)

    M1 = librosa.feature.mfcc(y=y1, sr=sr, n_mfcc=20, n_fft=2048, hop_length=512)
    M2 = librosa.feature.mfcc(y=y2, sr=sr, n_mfcc=20, n_fft=2048, hop_length=512)
    M3 = librosa.feature.mfcc(y=y3, sr=sr, n_mfcc=20, hop_length=512, n_fft=2048)

    mfcc_12 = np.mean(np.sqrt(np.mean((M1 - M2) ** 2, axis=0)))
    mfcc_13 = np.mean(np.sqrt(np.mean((M1 - M3) ** 2, axis=0)))
    mfcc_23 = np.mean(np.sqrt(np.mean((M2 - M3) ** 2, axis=0)))

    return _pairwise(mfcc_12, mfcc_13, mfcc_23)
