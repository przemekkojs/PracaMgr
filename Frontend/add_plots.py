import matplotlib.pyplot as plt
import numpy as np

if __name__ == '__main__':
    results = {
        "Principal 8'": {
            "S1": {
                "LSD":  {"S1": 2.191, "S2": 2.160},
                "MFCC": {"S1": 14.087, "S2": 14.150},
                "SC":   {"S1": 0.430, "S2": 0.437},
            },
            "S2": {
                "LSD":  {"S1": 2.116, "S2": 2.158},
                "MFCC": {"S1": 13.127, "S2": 13.814},
                "SC":   {"S1": 0.465, "S2": 0.440},
            },
        },

        "Holzgedackt 8'": {
            "S1": {
                "LSD":  {"S1": 2.169, "S2": 2.126},
                "MFCC": {"S1": 14.867, "S2": 13.316},
                "SC":   {"S1": 0.418, "S2": 0.463},
            },
            "S2": {
                "LSD":  {"S1": 2.138, "S2": 2.132},
                "MFCC": {"S1": 13.696, "S2": 13.923},
                "SC":   {"S1": 0.449, "S2": 0.455},
            },
        },

        "Gambe 8'": {
            "S1": {
                "LSD":  {"S1": 2.157, "S2": 2.119},
                "MFCC": {"S1": 13.717, "S2": 13.340},
                "SC":   {"S1": 0.441, "S2": 0.459},
            },
            "S2": {
                "LSD":  {"S1": 2.117, "S2": 2.255},
                "MFCC": {"S1": 14.585, "S2": 16.058},
                "SC":   {"S1": 0.458, "S2": 0.381},
            },
        },

        "Trompete 8'": {
            "S1": {
                "LSD":  {"S1": 2.181, "S2": 2.181},
                "MFCC": {"S1": 17.968, "S2": 17.968},
                "SC":   {"S1": 1.039, "S2": 1.039},
            },
            "S2": {
                "LSD":  {"S1": 2.181, "S2": 2.180},
                "MFCC": {"S1": 17.968, "S2": 17.968},
                "SC":   {"S1": 1.039, "S2": 1.039},
            },
        },
    }

    def plot_metric(results, metric):
        voices = list(results.keys())

        s1_values = []
        s2_values = []

        for voice in voices:
            s1_values.append(np.mean([
                results[voice]["S1"][metric]["S1"],
                results[voice]["S1"][metric]["S2"]
            ]))

            s2_values.append(np.mean([
                results[voice]["S2"][metric]["S1"],
                results[voice]["S2"][metric]["S2"]
            ]))

        x = np.arange(len(voices))
        width = 0.35

        plt.figure(figsize=(10, 5))
        plt.bar(x - width/2, s1_values, width, label="Optymalizacja S1")
        plt.bar(x + width/2, s2_values, width, label="Optymalizacja S2")

        plt.xticks(x, voices, fontsize=12)
        plt.ylabel(metric)
        plt.title(metric)
        plt.legend(fontsize=12)

        for bars in plt.gca().containers:
            plt.bar_label(
                bars,
                fmt="%.3f",
                padding=1,
                fontsize=12
            )

        plt.tight_layout()
        plt.show()

    plot_metric(results, "LSD")
    plot_metric(results, "MFCC")
    plot_metric(results, "SC")