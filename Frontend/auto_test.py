import json
import sys
from pathlib import Path
from optuna import Trial

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
        params = {
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

        with open(TEST_PARAMS_PATH, "w") as f:
            json.dump(params, f, indent=2)

        return params
    
    def objective(self, trial):
        params = self.write_test_params(trial)
        self.organ.make_test_synth_sample()

        score = self.visqol("../Backend/local/test/ref.wav", "../Backend/local/test/comp.wav")
        return score

    # TODO: Podłączyć ViSQOL
    def visqol(self, ref_path, comp_path) -> float:
        return 0.0
    
if __name__ == "__main__":
    organ:MainModule = MainModule()
    organ.init()
    organ.make_test_sample(0)

    params = {
        "baseFrequency": 440.0,
        "reflection": 0.45,
        "excitationGain": 0.45,
        "noiseGain" :  0.01,
        "jetLength": 0.48,
        "jetLowpassCoeff": 0.3,
        "lowpassCoeff": 0.6,
        "nonlinearCoeff": 0.8,
        "lossFilterCoeff": 0.15,
        "loopFeedbackGain": 0.8,
        "scale" : 1.0
    }

    with open(TEST_PARAMS_PATH, "w") as f:
        json.dump(params, f, indent=2)

    organ.make_test_synth_sample()

