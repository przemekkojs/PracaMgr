import matplotlib.pyplot as plt
import numpy as np

if __name__ == '__main__':
    resource_results = {
        "Prinzipal 8'": {
            "Próbki": {"RAM": 781.97, "CPU": 12.71},
            "S1":     {"RAM": 60.33,  "CPU": 12.91},
            "S2":     {"RAM": 60.24,  "CPU": 12.98},
        },
        "Holzgedackt 8'": {
            "Próbki": {"RAM": 778.73, "CPU": 12.65},
            "S1":     {"RAM": 60.13,  "CPU": 13.23},
            "S2":     {"RAM": 60.12,  "CPU": 13.07},
        },
        "Gambe 8'": {
            "Próbki": {"RAM": 782.02, "CPU": 12.67},
            "S1":     {"RAM": 60.26,  "CPU": 12.89},
            "S2":     {"RAM": 60.20,  "CPU": 13.03},
        },
        "Trompete 8'": {
            "Próbki": {"RAM": 781.98, "CPU": 12.59},
            "S1":     {"RAM": 60.31,  "CPU": 13.08},
            "S2":     {"RAM": 60.26,  "CPU": 13.05},
        },
    }

    def plot_resource_metric(results, metric):
        voices = list(results.keys())
        methods = ["Próbki", "S1", "S2"]

        x = np.arange(len(voices))
        width = 0.25

        plt.figure(figsize=(11, 5))

        for i, method in enumerate(methods):
            values = [results[voice][method][metric] for voice in voices]
            bars = plt.bar(
                x + (i - 1) * width,
                values,
                width,
                label=method
            )

            labels = [
                f"{bar.get_height():.2f}".replace(".", ",")
                for bar in bars
            ]

            plt.bar_label(
                bars,
                labels=labels,
                padding= 1,
                fontsize=12
            )

        plt.xticks(x, voices, fontsize=12)
        plt.ylabel("Zużycie RAM [MB]" if metric == "RAM" else "Zużycie CPU [%]")
        plt.title(f"Porównanie zużycia zasobów – {metric}")
        plt.legend(fontsize=12)
        plt.tight_layout()
        plt.show()


    plot_resource_metric(resource_results, "RAM")
    plot_resource_metric(resource_results, "CPU")

    