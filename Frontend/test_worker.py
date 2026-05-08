from PySide6.QtCore import QObject, Signal, Slot
import time
from engine import note_on, note_off

REF = "../Backend/local/out/sample.wav"
SYNTH = "../Backend/local/out/synth.wav"
MODEL = "../Backend/local/out/model.wav"

class TestWorker(QObject):
    finished = Signal()
    progress = Signal(dict)

    def __init__(self, notes, voices, duration, send_midi, get_stats, set_voice_active, run_metric):
        super().__init__()
        self.notes = notes
        self.voices = voices
        self.duration = duration

        self.send_midi = send_midi
        self.get_stats = get_stats
        self.set_voice_active = set_voice_active
        self.run_metric = run_metric

        self.running = True

    @Slot()
    def run(self):
        print("WORKER STARTED")

        try:
            for voice in self.voices:
                if not self.running:
                    break
                
                self.set_voice_active(voice, True, True)

                for note in self.notes:
                    if not self.running:
                        break

                    current = { "voice": voice, "note": note, "ram": [], "cpu": [] }
                    noteOn = note_on(note)
                    noteOff = note_off(note)

                    self.send_midi(noteOn)

                    start_time = time.time()
                    interval = 0.1
                    next_tick = time.time()

                    while time.time() - start_time < self.duration:
                        if not self.running:
                            break

                        cpu, ram = self.get_stats()
                        current["cpu"].append(cpu)
                        current["ram"].append(ram)

                        next_tick += interval
                        time.sleep(max(0, next_tick - time.time()))

                    self.send_midi(noteOff)

                    print('a')

                    try:
                        current["realism"] = self.run_metric(ref=REF, synth=SYNTH, model=MODEL)
                    except Exception as e:
                        current["realism"] = None
                        print("METRIC ERROR:", str(e))

                    print('b')

                    self.progress.emit(current)

                self.set_voice_active(voice, False, True)

            self.finished.emit()
        except Exception as e:
            print("WORKER ERROR:", str(e))

    def stop(self):
        print("WORKER STOPPED")
        self.running = False