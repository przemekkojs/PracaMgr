import json
import sys
import datetime

from pathlib import Path
from optuna import Trial
import optuna

from metrices import lsd, mcff, sc

TEST_PARAMS_PATH = Path("../Backend/local/test/temp.json")
VOICES_PATH = Path("../Backend/local/samples")

sys.path.append("../Backend/python")
from organ_engine import MainModule

class auto_test:
    def __init__(self, voice_id:int, voice_type:int=1, obj_index:int=0):
        self.organ:MainModule = MainModule()
        self.organ.init()
        self.organ.make_test_sample(voice_id)
        self.voice_type:int = voice_type
        self.obj_index:int = obj_index

    def write_test_params(self, trial: Trial) -> dict[str, float]:
        params: dict[str, float] = {
            "baseFrequency": 440.0,
            "scale": 1.0,
            "testVoiceType": self.voice_type            
        }

        if self.voice_type in [4, 8]:
            params.update({
                "breathBase": trial.suggest_float("breathBase", 0.0, 0.3, step=0.001),
                "breathEnvAmount": trial.suggest_float("breathEnvAmount", 0.5, 1.0, step=0.001),
                "breathNoiseGain": trial.suggest_float("breathNoiseGain", 0.0, 0.05, step=0.001),
                "reedOpening": 0.0003,
                "reedBias": trial.suggest_float("reedBias", 0.1, 1.0, step=0.001),
                "reedLeak": trial.suggest_float("reedLeak", 0.0, 0.01, step=0.001),
                "reedHysteresis": trial.suggest_float("reedHysteresis", 0.5, 0.95, step=0.001),
                "reedNonlin": trial.suggest_float("reedNonlin", 1.0, 8.0, step=0.001),
                "reedNonlinEnv": trial.suggest_float("reedNonlinEnv", 0.0, 1.0, step=0.001),
                "reedGain": trial.suggest_float("reedGain", 0.5, 3.0, step=0.001),
                "buzzGain": trial.suggest_float("buzzGain", 0.0, 0.05, step=0.001),
                "attackNoiseGain": trial.suggest_float("attackNoiseGain", 0.0, 0.2, step=0.001),
                "attackEnvThreshold": trial.suggest_float("attackEnvThreshold", 0.05, 0.3, step=0.001),
                "resonanceFreq": 3000.0, # trial.suggest_float("resonanceFreq", 1000.0, 5000.0, log=True),
                "resonanceGain": trial.suggest_float("resonanceGain", 0.0, 1.0, step=0.001),
                "feedbackNonlin": trial.suggest_float("feedbackNonlin", 1.0, 4.0, step=0.001),
                "jetDelayOffset": trial.suggest_float("jetDelayOffset", 1.0, 3.0, step=0.001)
            })
        else:
            params.update({
                "reflection": trial.suggest_float("reflection", 0.8, 0.999, step=0.001),
                "excitationGain": trial.suggest_float("excitationGain", 0.1, 1.0, step=0.001),
                "noiseGain": trial.suggest_float("noiseGain", 0.0, 0.1, step=0.001),
                "jetGain": trial.suggest_float("jetGain", 0.1, 1.0, step=0.001),
                "jetLength": trial.suggest_float("jetLength", 0.1, 1.0, step=0.001),
                "loopFeedbackGain": trial.suggest_float("loopFeedbackGain", 0.7, 0.999, step=0.001),
                "jetLowpassCoeff": trial.suggest_float("jetLowpassCoeff", 0.2, 0.9, step=0.001),
                "lowpassCoeff": trial.suggest_float("lowpassCoeff", 0.7, 0.999, step=0.001),
                "nonlinearCoeff": trial.suggest_float("nonlinearCoeff", 0.5, 3.0, step=0.001),
                "lossFilterCoeff": trial.suggest_float("lossFilterCoeff", 0.1, 0.9, step=0.001)
            })

        return params
    
    def choose_best_from_pareto(self, pareto_trials):
        values = [t.values for t in pareto_trials]

        mins = [min(v[i] for v in values) for i in range(3)]
        maxs = [max(v[i] for v in values) for i in range(3)]

        def normalized_score(trial):
            score = 0.0

            for i, value in enumerate(trial.values):
                denom = maxs[i] - mins[i]

                if denom == 0:
                    norm = 0.0
                else:
                    norm = (value - mins[i]) / denom

                score += norm

            return score / 3

        return min(pareto_trials, key=normalized_score)

    def objective(self, trial) -> tuple[float, float, float]:
        params = self.write_test_params(trial)

        with open(TEST_PARAMS_PATH, "w") as f:
            json.dump(params, f, indent=2)

        self.organ.make_test_synth_sample()
        self.organ.make_test_model_sample()
        score_lsd:tuple[float, float, float] = lsd()
        score_mcff:tuple[float, float, float] = mcff()
        score_sc:tuple[float, float, float] = sc()

        return (score_lsd[self.obj_index], score_mcff[self.obj_index], score_sc[self.obj_index])

    def run(self, n_trials: int = 100):
        study = optuna.create_study(
            directions=["minimize", "minimize", "minimize"]
        )

        study.optimize(
            self.objective,
            n_trials=n_trials,
            n_jobs=1,
            show_progress_bar=False
        )

        pareto_trials = study.best_trials

        best_trial = self.choose_best_from_pareto(pareto_trials)

        print("Selected trial:", best_trial.number)
        print("Selected values:", best_trial.values)
        print("Selected params:", best_trial.params)

        return best_trial.params, best_trial.values

if __name__ == "__main__":
    print("voice id: ", end="")
    v_id:int = int(input())

    print("voice type: ", end="")
    v_type:int = int(input())

    at:auto_test = auto_test(v_id, v_type)
    best_params, best_value = at.run(100)
    path:str = f".\\output\\result-{v_id}-{datetime.datetime.now().strftime("%d-%m-%Y-%H-%M-%S")}.json"

    out:dict = {
        "score"  : best_value,
        "params" : best_params
    }

    with open(path, 'w') as file:
        json.dump(out, fp=file)
