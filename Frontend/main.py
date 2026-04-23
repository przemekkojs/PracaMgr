from PySide6.QtWidgets import QApplication, QCheckBox, QHBoxLayout, QVBoxLayout, QWidget, QLabel, QPushButton, QComboBox
from PySide6.QtCore import QTimer

import sys
import datetime
import json
import os

from engine import EngineClient, EngineMonitor
from widgets import checkboxLabel, textboxLabel, VoiceManager

class ui(QWidget):
    def __init__(self):
        super().__init__()

        self.request_id = 0
        self.pending_requests = {} 
        
        self.engine = EngineClient()
        self.monitor = EngineMonitor(self.engine.process.pid)

        self.ramUsageBuffer:list[float] = []
        self.cpuUsageBuffer:list[float] = []
        self.logsBuffer:list[str] = []
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
        self.experiments_path = os.path.join(base_dir, "../Experiments")
        self.experiments_path = os.path.abspath(self.experiments_path)
        self.experiment_files = []

        for file in os.listdir(self.experiments_path):
            if file.endswith(".json"):
                self.experiment_files.append(file)

    def startTest(self):
        selected = self.experimentDropdown.currentText()
        self.startTestButton.setEnabled(False)
        self.stopTestButton.setEnabled(True)

        print(selected)

    def stopTest(self):
        self.startTestButton.setEnabled(True)
        self.stopTestButton.setEnabled(False)

    def initVoices(self):        
        self.request_voices_names()        
    
    def saveOutput(self, path:str, data:dict) -> None:
        with open(path, 'w') as file:
            json.dump(data, file)

    def addLog(self, what:str) -> None:
        now:str = datetime.datetime.strftime(datetime.datetime.now(), "%d-%m-%Y %H:%M:%S")
        text:str = f"{now}\t{what}"
        self.logsBuffer.append(text)

    def setSynthActive(self):
        isChecked:bool = self.synthActiveBox.cBox.isChecked()
        self.engine.send({ "type": "SET_SYNTH", "data": isChecked })

    def setModelActive(self):
        isChecked:bool = self.modelActiveBox.cBox.isChecked() 
        self.engine.send({ "type": "SET_MODEL", "data": isChecked })

    def setVoiceActive(self, voiceId:int, value:bool):
        self.engine.send({ "type": "SET_VOICE", "data": (voiceId, value) })

    def boxSetVoiceActiveEvent(self, voiceId:int):
        self.setVoiceActive(voiceId, self.voiceBoxes[voiceId].isChecked())

    def setSamplesActive(self):
        isChecked:bool = self.samplesActiveBox.cBox.isChecked()
        self.engine.send({ "type": "SET_SAMPLES", "data": isChecked })
    
    def send_midi(self, note):
        self.engine.send({ "type": "MIDI", "data": note })

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
        event.accept()

if __name__ == '__main__':
    app:QApplication = QApplication(sys.argv)

    UI:ui = ui()
    UI.show()

    sys.exit(app.exec())
