from PySide6.QtWidgets import QApplication, QCheckBox, QHBoxLayout, QVBoxLayout, QWidget, QLabel, QPushButton, QComboBox
from PySide6.QtCore import QTimer, QObject, Signal, Slot, QThread

import sys
import datetime
import json
import os
import time

from engine import EngineClient, EngineMonitor, note_on, note_off
from widgets import checkboxLabel, textboxLabel, VoiceManager

class TestWorker(QObject):
    finished = Signal()
    progress = Signal(dict)

    def __init__(self, notes, voices, duration, send_midi, get_stats, set_voice_active, run_visqol):
        super().__init__()
        self.notes = notes
        self.voices = voices
        self.duration = duration

        self.send_midi = send_midi
        self.get_stats = get_stats
        self.set_voice_active = set_voice_active
        self.run_visqol = run_visqol

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

                    try:
                        current["realism"] = self.run_visqol()
                    except Exception as e:
                        current["realism"] = None

                    self.progress.emit(current)

                self.set_voice_active(voice, False, True)

            self.finished.emit()
        except Exception as e:
            print("WORKER ERROR:", e)

    def stop(self):
        print("WORKER STOPPED")
        self.running = False


class ui(QWidget):
    def __init__(self):
        super().__init__()

        self.request_id = 0
        self.pending_requests = {} 
        
        self.engine = EngineClient()
        self.monitor = EngineMonitor(self.engine.process.pid)

        self.ramUsageBuffer:list[float] = []
        self.cpuUsageBuffer:list[float] = []
        self.voicesNames:dict[int, str] = {}
        self.voiceBoxes:dict[int, QCheckBox] = {}

        self.ui_timer = QTimer()
        self.ui_timer.timeout.connect(self.update_stats)
        self.ui_timer.start(500)

        self.engine_timer = QTimer()
        self.engine_timer.timeout.connect(self.poll_engine)
        self.engine_timer.start(10)

        self.initExperiments()
        self.init_ui()
        self.initVoices()
        self.request_device_name()        

        self.voiceManager = VoiceManager(self.voicesBox, self.setVoiceActive)

    def init_ui(self):
        self.buttons_box:QHBoxLayout = QHBoxLayout()
        self.startTestButton:QPushButton = QPushButton("Start")
        self.stopTestButton:QPushButton = QPushButton("Stop")
        self.experimentDropdown: QComboBox = QComboBox()
        self.experimentDropdown.addItems(self.experiment_files)
        self.stopTestButton.setEnabled(False)
        self.buttons_box.addWidget(self.startTestButton)
        self.buttons_box.addWidget(self.stopTestButton)
        self.buttons_box.addWidget(self.experimentDropdown)
        self.startTestButton.clicked.connect(self.startTest)
        self.stopTestButton.clicked.connect(self.stopTest)

        self.voicesBox:QVBoxLayout = QVBoxLayout()
        self.voicesBox.addWidget(QLabel("Głosy"))
        
        layout = QHBoxLayout()
        self.setWindowTitle("Praca magisterska")

        self.cpuUsageText:textboxLabel = textboxLabel("CPU (%)")
        self.ramUsageText:textboxLabel = textboxLabel("RAM (MB)")
        self.usagesBox:QVBoxLayout = QVBoxLayout()
        
        for item in [QLabel("Zużycie zasobów"), self.cpuUsageText, self.ramUsageText]:
            self.usagesBox.addWidget(item)

        self.synthRealismText1:textboxLabel = textboxLabel("Syntezator")
        self.modelRealismText1:textboxLabel = textboxLabel("Model")
        self.realismBox1:QVBoxLayout = QVBoxLayout()

        for item in [QLabel("ViSQOLAudio"), self.synthRealismText1, self.modelRealismText1]:
            self.realismBox1.addWidget(item)

        self.synthRealismText2:textboxLabel = textboxLabel("Syntezator")
        self.modelRealismText2:textboxLabel = textboxLabel("Model")
        self.realismBox2:QVBoxLayout = QVBoxLayout()

        for item in [QLabel("2f-Model"), self.synthRealismText2, self.modelRealismText2]:
            self.realismBox2.addWidget(item)

        self.statsBox:QVBoxLayout = QVBoxLayout()
        
        for item in [self.buttons_box, self.usagesBox, self.realismBox1, self.realismBox2]:
            self.statsBox.addLayout(item)

        self.synthActiveBox:checkboxLabel = checkboxLabel("Syntezator", self.setSynthActive)
        self.samplesActiveBox:checkboxLabel = checkboxLabel("Sampler", self.setSamplesActive)
        self.modelActiveBox:checkboxLabel = checkboxLabel("Model", self.setModelActive)
        self.activeBox:QVBoxLayout = QVBoxLayout()

        self.request_synth_active()
        self.request_samples_active()
        self.request_model_active()
        
        for item in [QLabel("Moduły"), self.synthActiveBox, self.samplesActiveBox, self.modelActiveBox]:
            self.activeBox.addWidget(item)

        for item in [self.activeBox, self.statsBox, self.voicesBox]:
            layout.addLayout(item)

        self.deviceNameLabel:QLabel = QLabel("Brak wejściowego urządzenia MIDI")
        mainLayout:QVBoxLayout = QVBoxLayout()        
        mainLayout.addWidget(self.deviceNameLabel)
        mainLayout.addLayout(layout)
        self.setLayout(mainLayout)

    def initExperiments(self):
        base_dir = os.path.dirname(os.path.abspath(__file__))
        self.experiments_path:str = os.path.join(base_dir, "../Experiments")
        self.experiments_path:str = os.path.abspath(self.experiments_path)
        self.experiment_files:list[str] = []

        self.result_path:str = f"{self.experiments_path}\\Results"
        self.result_obj:dict = {}
        self.current_test:str = "None"

        for file in os.listdir(self.experiments_path):
            if file.endswith(".json"):
                self.experiment_files.append(file)

    def startTest(self):
        selected: str = self.experimentDropdown.currentText()

        self.startTestButton.setEnabled(False)
        self.stopTestButton.setEnabled(True)

        self.result_obj.clear()
        self.current_test = selected[:-5]

        with open(f"{self.experiments_path}\\{selected}", 'r') as file:
            data: dict = json.load(file)
            self.result_obj = data.copy()

        if not data or len(data) == 0:
            self.stopTest()
            return

        self.result_obj["actions"] = []
        duration: int = int(data["duration"])
        notes: list[int] = [int(x) for x in data["notes"]]
        voices: list[int] = [int(x) for x in data["voices"]]

        samplesActive: bool = bool(data["samples"])
        modelActive: bool = bool(data["model"])
        synthActive: bool = bool(data["synth"])

        self.forceSamplesActive(samplesActive)
        self.forceModelActive(modelActive)
        self.forceSynthActive(synthActive)

        self.th = QThread()
        self.worker = TestWorker(
            notes=notes,
            voices=voices,
            duration=duration,
            send_midi=self.send_midi,
            get_stats=self.get_stats,
            set_voice_active=self.setVoiceActive,
            run_visqol=self.runViSQOL
        )

        self.worker.moveToThread(self.th)

        self.th.started.connect(self.worker.run)

        self.worker.finished.connect(self.th.quit)
        self.worker.finished.connect(self.worker.deleteLater)
        self.th.finished.connect(self.th.deleteLater)

        self.worker.progress.connect(self._on_worker_progress)
        self.worker.finished.connect(self._on_worker_finished)

        self.th.start()

    def _on_worker_progress(self, current):
        if "actions" in self.result_obj:
            self.result_obj["actions"].append(current)

    def _on_worker_finished(self):
        self.stopTest()

    def stopTest(self):
        if hasattr(self, "worker"):
            self.worker.stop()

        self.startTestButton.setEnabled(True)
        self.stopTestButton.setEnabled(False)

        self.modelActiveBox.cBox.setCheckable(True)
        self.samplesActiveBox.cBox.setCheckable(True)
        self.synthActiveBox.cBox.setCheckable(True)

        now: str = datetime.datetime.now().strftime("%d-%m-%Y-%H-%M-%S")
        path: str = f"{self.result_path}\\{self.current_test}-{now}.json"

        with open(path, 'w') as file:
            json.dump(self.result_obj, fp=file)

        self.result_obj.clear()

    def runViSQOL(self) -> int:
        pass

    def initVoices(self):        
        self.request_voices_names()

    def forceSynthActive(self, value:bool) -> None:
        self.synthActiveBox.cBox.setChecked(value)

    def forceModelActive(self, value:bool) -> None:
        self.modelActiveBox.cBox.setChecked(value)

    def forceSamplesActive(self, value:bool) -> None:
        self.samplesActiveBox.cBox.setChecked(value)
    
    def setSynthActive(self) -> None:        
        isChecked:bool = self.synthActiveBox.cBox.isChecked()
        self.engine.send({ "type": "SET_SYNTH", "data": isChecked })

    def setModelActive(self) -> None:
        isChecked:bool = self.modelActiveBox.cBox.isChecked() 
        self.engine.send({ "type": "SET_MODEL", "data": isChecked })

    def setVoiceActive(self, voiceId:int, value:bool, update_ui:bool=False) -> None:        
        if update_ui:
            item:QCheckBox = self.voicesBox.itemAt(voiceId).widget()
            item.setChecked(value)

    def boxSetVoiceActiveEvent(self, voiceId:int):
        print("2", voiceId, value)
        value = self.voiceBoxes[voiceId].isChecked()
        self.setVoiceActive(voiceId, value)
        self.engine.send({ "type": "SET_VOICE", "data": (voiceId, value) })

    def setSamplesActive(self):
        isChecked:bool = self.samplesActiveBox.cBox.isChecked()
        self.engine.send({ "type": "SET_SAMPLES", "data": isChecked })
    
    def send_midi(self, note):
        self.engine.send({ "type": "MIDI", "data": (note.note, note.channel, note.on) })

    def requestAny(self, what):
        self.request_id += 1
        rid = self.request_id
        self.engine.send({ "type": what, "id": rid })
        self.pending_requests[rid] = what

    def request_synth_active(self):
        self.requestAny("GET_SYNTH_ACTIVE")

    def request_model_active(self):
        self.requestAny("GET_MODEL_ACTIVE")

    def request_samples_active(self):
        self.requestAny("GET_SAMPLES_ACTIVE")

    def request_device_name(self):
        self.requestAny("GET_DEVICE_NAME")

    def request_voices_names(self):
        self.requestAny("GET_VOICES_NAMES")

    def poll_engine(self):
        for msg in self.engine.poll():
            if msg["type"] == "GET_SYNTH_ACTIVE_RESULT":
                rid, value = msg["id"], msg["value"]
                self.synthActiveBox.cBox.setChecked(value)
                del self.pending_requests[rid]

            elif msg["type"] == "GET_SAMPLES_ACTIVE_RESULT":
                rid, value = msg["id"], msg["value"]
                self.samplesActiveBox.cBox.setChecked(value)
                del self.pending_requests[rid]

            elif msg["type"] == "GET_MODEL_ACTIVE_RESULT":
                rid, value = msg["id"], msg["value"]
                self.modelActiveBox.cBox.setChecked(value)
                del self.pending_requests[rid]

            elif msg["type"] == "SET_VOICE_RESULT":
                voiceId, value = msg["data"]

                for i in range(self.voicesBox.count()):
                    item:checkboxLabel = self.voicesBox.itemAt(i).widget()

                    if isinstance(item, checkboxLabel):
                        if i == voiceId:
                            item.cBox.setChecked(value)
                            break

            elif msg['type'] == 'GET_DEVICE_NAME_RESULT':
                value = msg["value"]
                self.deviceNameLabel.setText(value)

            elif msg['type'] == 'GET_VOICES_NAMES_RESULT':
                value = msg["value"]
                self.voicesNames = value
                self.voiceBoxes = {}

                for k in self.voicesNames.keys():
                    v = self.voicesNames[k]
                    toAppend:QCheckBox = QCheckBox(v)
                    self.voiceBoxes[k] = toAppend
                    self.voicesBox.addWidget(toAppend)
                    toAppend.clicked.connect(lambda state, x=k: self.boxSetVoiceActiveEvent(x))      

    def setDeviceName(self, value:str):
        self.deviceNameLabel.setText(value)

    def get_stats(self):
        return self.monitor.get_cpu(), self.monitor.get_ram()
    
    def update_stats(self):
        cpu, ram = self.get_stats()
        self.cpuUsageText.setText(f"{cpu:.1f}")
        self.ramUsageText.setText(f"{ram:.1f}")

    def closeEvent(self, event):
        self.ui_timer.stop()
        self.engine_timer.stop()
        self.engine.stop()
        self.worker.stop()
        event.accept()

if __name__ == '__main__':
    app:QApplication = QApplication(sys.argv)

    UI:ui = ui()
    UI.show()

    sys.exit(app.exec())
