from PySide6.QtWidgets import QApplication, QCheckBox, QHBoxLayout, QVBoxLayout, QWidget, QLabel, QPushButton
from PySide6.QtCore import QTimer

import sys
import psutil
import datetime
import json

from multiprocessing import Process, Pipe

from engine import run_engine
from widgets import checkboxLabel, textboxLabel

class ui(QWidget):
    def __init__(self):
        super().__init__()

        self.request_id = 0
        self.pending_requests = {}

        self.parent_conn, child_conn = Pipe()
        self.engine_process = Process(target=run_engine, args=(child_conn,))
        self.engine_process.start()
        self.engine_ps = psutil.Process(self.engine_process.pid)

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

        self.init_ui()
        self.initVoices()
        self.request_device_name()

    def init_ui(self):
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
        
        for item in [self.usagesBox, self.realismBox1, self.realismBox2]:
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

    def initVoices(self):        
        self.request_voices_names()        

    def recordingTimeToString(self) -> str:
        fullSeconds:int = int(self.recordingTime // 1000)

        while fullSeconds >= 60:
            fullSeconds -= 60

        fullMinutes:int = int((self.recordingTime // 1000) // 60)
        secs_str:str = f"{fullSeconds}" if fullSeconds > 9 else f"0{fullSeconds}"
        mins_str:str = f"{fullMinutes}" if fullMinutes > 9 else f"0{fullMinutes}"

        return f"{mins_str}:{secs_str}"
    
    def saveOutput(self, path:str, data:dict) -> None:
        with open(path, 'w') as file:
            json.dump(data, file)

    def addLog(self, what:str) -> None:
        now:str = datetime.datetime.strftime(datetime.datetime.now(), "%d-%m-%Y %H:%M:%S")
        text:str = f"{now}\t{what}"
        self.logsBuffer.append(text)

    def setSynthActive(self):
        isChecked:bool = self.synthActiveBox.cBox.isChecked()
        self.parent_conn.send({ "type": "SET_SYNTH", "data": isChecked })

    def setModelActive(self):
        isChecked:bool = self.modelActiveBox.cBox.isChecked() 
        self.parent_conn.send({ "type": "SET_MODEL", "data": isChecked })

    def setVoiceActive(self, voiceId:int, value:bool):
        self.parent_conn.send({ "type": "SET_VOICE", "data": (voiceId, value) })

    def boxSetVoiceActiveEvent(self, voiceId:int):
        self.setVoiceActive(voiceId, self.voiceBoxes[voiceId].isChecked())

    def setSamplesActive(self):
        isChecked:bool = self.samplesActiveBox.cBox.isChecked()
        self.parent_conn.send({ "type": "SET_SAMPLES", "data": isChecked })

    def requestAny(self, what):
        self.request_id += 1
        rid = self.request_id
        self.parent_conn.send({ "type": what, "id": rid })
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
        while self.parent_conn.poll():
            msg = self.parent_conn.recv()            

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

    def get_cpu(self):
        cpu = self.engine_ps.cpu_percent(interval=0.1)
        normalized = cpu / psutil.cpu_count()
        return normalized

    def get_ram(self):
        return self.engine_ps.memory_info().rss / 1024 / 1024

    def get_stats(self):
        return self.get_cpu(), self.get_ram()
    
    def update_stats(self):
        cpu, ram = self.get_stats()
        self.cpuUsageText.setText(f"{cpu:.1f}")
        self.ramUsageText.setText(f"{ram:.1f}")

    def closeEvent(self, event):
        self.ui_timer.stop()
        self.engine_timer.stop()

        self.parent_conn.send({"type": "STOP"})
        self.engine_process.join()
        event.accept()

    def send_midi(self, note):
        self.parent_conn.send({ "type": "MIDI", "data": note })


if __name__ == '__main__':
    app:QApplication = QApplication(sys.argv)

    UI:ui = ui()
    UI.show()

    sys.exit(app.exec())
