import json
import sys
import datetime

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
        score = self.visqol(ref_path, comp_path)
        
        return score

    # TODO: Podłączyć ViSQOL
    def visqol(self, ref_path:str, comp_path:str) -> float:
        return 0.0

    # TODO: Implementacja
    def run(self) -> dict[str, float]:
        return { "hello" : 1.0 }
    

if __name__ == "__main__":
    print("voice id: ", end="")
    v_id:int = int(input())

    at:auto_test = auto_test(v_id)
    best_params:dict[str, float] = at.run()
    path:str = f".\\output\\result-{v_id}-{datetime.datetime.now().strftime("%d-%m-%Y-%H-%M-%S")}.json"

    with open(path, 'w') as file:
        json.dump(best_params, fp=file)

