import matplotlib.pyplot as plt

import os
import json
import statistics

class test:
    def __init__(self, id:(str | int), cpu:list[float], ram:list[float], note:int, voice_id:int, realism:(tuple[float, float, float] | list[tuple[float, float, float]] | None), combined:bool=False):
        self.id:str = str(id)
        self.cpu:list[float] = cpu
        self.ram:list[float] = ram
        self.note:int = note
        self.voice_id:int = voice_id

        self.realism:tuple[float, float, float] = (tuple(
                sum(x[i] for x in realism) / len(realism) for i in range(3)
            ) if combined else tuple(float(x) for x in realism)) if realism else (-1.0, -1.0, -1.0)
        
    def report_result(self) -> dict:
        result: dict = {}

        cpu_avg = sum(self.cpu) / len(self.cpu) if self.cpu else 0
        cpu_max = max(self.cpu) if self.cpu else 0
        cpu_min = min(self.cpu) if self.cpu else 0
        cpu_std = statistics.pstdev(self.cpu) if len(self.cpu) > 1 else 0
        cpu_med = statistics.median(self.cpu) if self.cpu else 0

        ram_avg = sum(self.ram) / len(self.ram) if self.ram else 0
        ram_max = max(self.ram) if self.ram else 0
        ram_min = min(self.ram) if self.ram else 0
        ram_std = statistics.pstdev(self.ram) if len(self.ram) > 1 else 0
        ram_med = statistics.median(self.ram) if self.ram else 0

        fig_cpu, ax_cpu = plt.subplots()

        ax_cpu.plot(self.cpu, color='black', linewidth=2, label='CPU')
        ax_cpu.axhline(cpu_min, color='red', linestyle='--', label='min')
        ax_cpu.axhline(cpu_avg, color='orange', linestyle='--', label='avg')
        ax_cpu.axhline(cpu_max, color='green', linestyle='--', label='max')
        ax_cpu.axhline(cpu_avg + cpu_std, color='blue', linestyle='--', label='+std')
        ax_cpu.axhline(cpu_avg - cpu_std, color='blue', linestyle='--', label='-std')
        ax_cpu.axhline(cpu_med, color='purple', linestyle='--', label='med')
        ax_cpu.set_title("Zużycie CPU")
        ax_cpu.set_xlabel("Próbka")
        ax_cpu.set_ylabel("CPU [%]")
        ax_cpu.grid()
        ax_cpu.legend()

        fig_ram, ax_ram = plt.subplots()

        ax_ram.plot(self.ram, color='black', linewidth=2, label='RAM')
        ax_ram.axhline(ram_min, color='red', linestyle='--', label='min')
        ax_ram.axhline(ram_avg, color='orange', linestyle='--', label='avg')
        ax_ram.axhline(ram_max, color='green', linestyle='--', label='max')
        ax_ram.axhline(ram_avg + ram_std, color='blue', linestyle='--', label='+std')
        ax_ram.axhline(ram_avg - ram_std, color='blue', linestyle='--', label='-std')
        ax_ram.axhline(ram_med, color='purple', linestyle='--', label='med')
        ax_ram.set_title("Zużycie pamięci RAM")
        ax_ram.set_xlabel("Próbka")
        ax_ram.set_ylabel("RAM [MB]")
        ax_ram.grid()
        ax_ram.legend()

        result["cpu_plt"] = fig_cpu
        result["ram_plt"] = fig_ram

        result["cpu_avg"] = cpu_avg
        result["ram_avg"] = ram_avg

        result["cpu_max"] = cpu_max
        result["ram_max"] = ram_max

        result["cpu_min"] = cpu_min
        result["ram_min"] = ram_min

        result["cpu_std"] = cpu_std
        result["ram_std"] = ram_std

        result["cpu_med"] = cpu_med
        result["ram_med"] = ram_med

        return result

class experiment_report:
    def __init__(self):
        self.experiments:list[str] = os.listdir("../Results")
        self.current_experiment:str = ""

    def extract_params(self, path:str) -> dict:
        self.current_experiment = path

        with open(path, 'r') as file:
            return json.load(file)

    def run(self, data:dict) -> bool:
        try:
            test_name:str = str(data["name"])
            description:str = str(data["description"])
            duration:str = str(data["duration"])            
            synth_active:bool = bool(data["synth"])
            model_active:bool = bool(data["model"])
            samples_active:bool = bool(data["samples"])
            actions:list[dict] = data["actions"]
            notes:list[int] = [int(x) for x in data["notes"]]
            voice_ids:list[int] = [int(x) for x in data["voices"]]

            completion_matrix:dict[tuple[int, int], bool] = {}
            amount:int = len(notes) * len(voice_ids)

            tests:list[test] = []
            last_voice_id = -1
            
            cpu_buff:list[float] = []
            ram_buff:list[float] = []
            realism_buff:list[tuple[float, float, float]] = []

            for action in actions:
                cpu:list[float] = [float(x) for x in action["cpu"]]
                ram:list[float] = [float(x) for x in action["ram"]]
                realism:list[int] = [int(x) for x in action["realism"]]          
                note:int = int(action["note"])
                voice_id:int = int(action["voice"])

                completion_matrix[(note, voice_id)] = True
                t:test = test(len(completion_matrix), cpu.copy(), ram.copy(), note, voice_id, realism)
                tests.append(t)

                cpu_buff.extend(cpu)
                ram_buff.extend(ram)
                realism_buff.append(realism.copy()) 

                if voice_id != last_voice_id:
                    tmp = last_voice_id
                    last_voice_id = voice_id

                    if tmp == -1:
                        continue

                    t_all:test = test(f"zbiorczy {voice_id}", cpu_buff.copy(), ram_buff.copy(), -1, -1, realism_buff.copy(), combined=True)
                    tests.append(t_all)

                    cpu_buff.clear()
                    ram_buff.clear()
                    realism_buff.clear()

            if cpu_buff:
                t_all = test(f"zbiorczy {voice_id}", cpu_buff.copy(), ram_buff.copy(), -1, -1, realism_buff.copy(), combined=True)
                tests.append(t_all)    

            completed:int = len(completion_matrix)
            completion_progress:float = float((completed / amount) * 100.0)

            report_dir = f"../Reports/{test_name.replace(' ', '_')}"
            os.makedirs(report_dir, exist_ok=True)

            report_path = os.path.join(report_dir, "report.md")

            modules = []
            if synth_active: modules.append("synth")
            if model_active: modules.append("model")
            if samples_active: modules.append("samples")

            with open(report_path, "w", encoding="utf-8") as f:
                f.write(f"# {test_name}\n\n")
                f.write(f"{description}\n\n")

                f.write(f"**Nuty (MIDI):** {notes}\\\n")
                f.write(f"**Głosy:** {voice_ids}\\\n")
                f.write(f"**Czas trwania nuty:** {duration}\\\n")
                f.write(f"**Aktywne moduły:** {', '.join(modules)}\n\n")

                f.write(f"**Postęp:** {completed}/{amount} ({completion_progress:.2f}%)\n\n")
                f.write("---\n\n")

                for t in tests:
                    res = t.report_result()

                    cpu_path = os.path.join(report_dir, f"cpu_test_{t.id.replace(' ', '_')}.png")
                    ram_path = os.path.join(report_dir, f"ram_test_{t.id.replace(' ', '_')}.png")

                    res["cpu_plt"].savefig(cpu_path)
                    res["ram_plt"].savefig(ram_path)

                    plt.close(res["cpu_plt"])
                    plt.close(res["ram_plt"])

                    f.write(f"## Test {t.id}\n\n" if t.id != -1 else "## Podsumowanie\n\n")
                    if t.note != -1: f.write(f"**Nuta:** {t.note}\\\n")
                    if t.voice_id != -1: f.write(f"**Głos:** {t.voice_id}\n\n")

                    f.write("| Metryka | Min | Średnia | Max | Std | Med |\n")
                    f.write("|--------|-----|----------|-----|-------|------|\n")
                    f.write(f"| CPU | {res['cpu_min']:.2f} | {res['cpu_avg']:.2f} | {res['cpu_max']:.2f} | {res['cpu_std']:.2f} | {res['cpu_med']:.2f} |\n")
                    f.write(f"| RAM | {res['ram_min']:.2f} | {res['ram_avg']:.2f} | {res['ram_max']:.2f} | {res['ram_std']:.2f} | {res['ram_med']:.2f} |\n\n")

                    f.write(f"**Realizm:** {t.realism}\n\n")

                    f.write(f"![CPU](./cpu_test_{t.id.replace(' ', '_')}.png)\n")
                    f.write(f"![RAM](./ram_test_{t.id.replace(' ', '_')}.png)\n\n")

                    f.write("---\n\n")

        except Exception as e:
            print(e)
            return False
        
        return True


if __name__ == "__main__":
    report = experiment_report()

    for exp in report.experiments:
        path = os.path.join("../Results", exp)
        params = report.extract_params(path)
        report.run(params)
