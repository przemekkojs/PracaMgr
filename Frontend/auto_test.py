import json
import sys
import datetime

from pathlib import Path
from optuna import Trial

import numpy as np
import librosa
import optuna

import visqolpy.visqol_py as visqol_py

TEST_PARAMS_PATH = Path("../Backend/local/test/temp.json")
VOICES_PATH = Path("../Backend/local/samples")

sys.path.append("../Backend/python")
from organ_engine import MainModule

class auto_test:
    def __init__(self, voice_id:int):
        self.organ:MainModule = MainModule()
        self.organ.init()
        self.organ.make_test_sample(voice_id)

    def write_test_params(self, trial: Trial) -> dict[str, float]:
        params:dict[str, float] = {
            "baseFrequency": 440.0,

            "reflection": trial.suggest_float("reflection", 0.0, 1.0),
            "excitationGain": trial.suggest_float("excitationGain", 0.0, 1.0),
            "noiseGain": trial.suggest_float("noiseGain", 0.0, 1.0),
            "jetGain": trial.suggest_float("jetGain", 0.0, 1.0),
            "scale": 1.0,
            "jetLength": trial.suggest_float("jetLength", 0.0, 1.0),
            "loopFeedbackGain": trial.suggest_float("loopFeedbackGain", 0.0, 1.0),
            "jetLowpassCoeff": trial.suggest_float("jetLowpassCoeff", 0.0, 1.0),
            "lowpassCoeff": trial.suggest_float("lowpassCoeff", 0.0, 1.0),
            "nonlinearCoeff": trial.suggest_float("nonlinearCoeff", 0.0, 1.0),
            "lossFilterCoeff": trial.suggest_float("lossFilterCoeff", 0.0, 1.0),
        }

        return params
    
    def objective(self, trial):
        params = self.write_test_params(trial)

        with open(TEST_PARAMS_PATH, "w") as f:
            json.dump(params, f, indent=2)

        self.organ.make_test_synth_sample()       

        ref_path:str = "../Backend/local/test/ref.wav"
        comp_path:str = "../Backend/local/test/comp.wav"
        score = self.realism(ref_path, comp_path)
        
        return score

    def realism(self, ref_path:str, comp_path:str) -> float:
        y1, sr1 = librosa.load(ref_path, sr=None, mono=True)
        y2, sr2 = librosa.load(comp_path, sr=None, mono=True)

        min_len = min(len(y1), len(y2))
        y1 = y1[:min_len]
        y2 = y2[:min_len]

        y1 = y1 / (np.sqrt(np.mean(y1 ** 2)) + 1e-10)
        y2 = y2 / (np.sqrt(np.mean(y2 ** 2)) + 1e-10)

        S1 = np.abs(librosa.stft(y1, n_fft=2048, hop_length=512))
        S2 = np.abs(librosa.stft(y2, n_fft=2048, hop_length=512))

        eps = 1e-10
        log_S1 = np.log(S1 + eps)
        log_S2 = np.log(S2 + eps)

        frame_lsd = np.sqrt(np.mean((log_S1 - log_S2) ** 2, axis=0))
        lsd = np.mean(frame_lsd)

        return lsd

    def run(self, n_trials:int=100) -> tuple[dict[str, float], float]:
        study = optuna.create_study(direction="minimize")

        study.optimize(
            self.objective,
            n_trials=n_trials,
            n_jobs=1,
            show_progress_bar=True
        )

        print("Best score:", study.best_value)
        print("Best params:", study.best_params)

        return (study.best_params, study.best_value)
    

if __name__ == "__main__":
    print("voice id: ", end="")
    v_id:int = int(input())

    at:auto_test = auto_test(v_id)
    best_params, best_value = at.run(100)
    path:str = f".\\output\\result-{v_id}-{datetime.datetime.now().strftime("%d-%m-%Y-%H-%M-%S")}.json"

    out:dict = {
        "score" : best_value,
        "params" : best_params
    }

    with open(path, 'w') as file:
        json.dump(out, fp=file)
