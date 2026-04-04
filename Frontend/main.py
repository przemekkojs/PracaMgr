from PySide6.QtWidgets import QApplication, QCheckBox, QHBoxLayout, QVBoxLayout, QWidget, QLabel, QLineEdit, QPushButton
from PySide6.QtCore import QTimer

import sys
import psutil
import datetime
import json

from multiprocessing.connection import Connection
from multiprocessing import Process, Pipe

sys.path.append("../Backend/python")
from organ_engine import MainModule, NoteSignal, EMPTY_NOTE_SIGNAL

def run_engine(pipe: Connection):
    organ:MainModule = MainModule()
    running:bool = True

    while running:
        while pipe.poll():
            msg = pipe.recv()

            if msg["type"] == "STOP":
                running = False
                break

            elif msg["type"] == "NOTE":
                organ.play(msg["data"])

            elif msg["type"] == "SET_SAMPLES":
                organ.set_samples_active(msg["data"])

            elif msg["type"] == "SET_SYNTH":
                organ.set_synth_active(msg["data"])

            elif msg["type"] == "SET_MODEL":
                organ.set_model_active(msg["data"])

            elif msg["type"] == "GET_SYNTH_ACTIVE":
                pipe.send({
                    "type": "GET_SYNTH_ACTIVE_RESULT",
                    "id": msg["id"],
                    "value": organ.get_synth_active()
                })

            elif msg["type"] == "GET_SAMPLES_ACTIVE":
                pipe.send({
                    "type": "GET_SAMPLES_ACTIVE_RESULT",
                    "id": msg["id"],
                    "value": organ.get_samples_active()
                })

            elif msg["type"] == "GET_MODEL_ACTIVE":
                pipe.send({
                    "type": "GET_MODEL_ACTIVE_RESULT",
                    "id": msg["id"],
                    "value": organ.get_model_active()
                })

            elif msg["type"] == "SET_VOICE":
                voiceId = msg["data"][0]
                value = msg["data"][1]
                res:bool = organ.set_voice_active(voiceId, value)

                pipe.send({
                    "type": "SET_VOICE_RESULT",
                    "data": (voiceId, res)
                })

            elif msg['type'] == 'GET_DEVICE_NAME':
                pipe.send({
                    "type": "GET_DEVICE_NAME_RESULT",
                    "value": organ.get_midi_device_name()
                })

            elif msg['type'] == 'GET_VOICES_NAMES':
                pipe.send({
                    "type": "GET_VOICES_NAMES_RESULT",
                    "value": organ.get_voices_names()
                })
        
        s:NoteSignal = organ.get_signal()

        if s != EMPTY_NOTE_SIGNAL:
            organ.play(s)
        

class checkboxLabel(QWidget):
    def __init__(self, label_text:str, callback):
        super().__init__()
        self.cBox:QCheckBox = QCheckBox()
        self.label:QLabel = QLabel(label_text)
        
        layout = QHBoxLayout()

        layout.addWidget(self.cBox)
        layout.addWidget(self.label)

        self.setLayout(layout)

        self.cBox.stateChanged.connect(callback)


class textboxLabel(QWidget):
    def __init__(self, label_text:str, readonly:bool=True):
        super().__init__()
        self.tBox:QLineEdit = QLineEdit()
        self.tBox.setReadOnly(readonly)

        self.label:QLabel = QLabel(label_text)
        layout = QHBoxLayout()        

        layout.addWidget(self.tBox)
        layout.addWidget(self.label)

        self.setLayout(layout)

    def setText(self, text:str) -> None:
        self.tBox.setText(text)


class ui(QWidget):
    def __init__(self):
        super().__init__()

        self.request_id = 0
        self.pending_requests = {}

        self.parent_conn, child_conn = Pipe()
        self.engine_process = Process(target=run_engine, args=(child_conn,))
        self.engine_process.start()
        self.engine_ps = psutil.Process(self.engine_process.pid)

        self.recordingTime:float = 0
        self.isRecording:bool = False
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

        self.recordingBox:QVBoxLayout = QVBoxLayout()
        self.recordingLabel:QLabel = QLabel("Rozpocznij nagrywanie...")
        innerRecordingBox:QHBoxLayout = QHBoxLayout()

        self.startRecordingButton:QPushButton = QPushButton("▶")
        self.stopRecordingButton:QPushButton = QPushButton("■")
        innerRecordingBox.addWidget(self.startRecordingButton)
        innerRecordingBox.addWidget(self.stopRecordingButton)        
        self.recordingBox.addLayout(innerRecordingBox)
        self.recordingBox.addWidget(self.recordingLabel)        
        self.startRecordingButton.clicked.connect(self.record)
        self.stopRecordingButton.clicked.connect(self.stopRecording)
        self.stopRecordingButton.setDisabled(True)

        self.automaticTestBox:QVBoxLayout = QVBoxLayout()
        self.automaticTestBox.addWidget(QLabel("Test automatyczny"))
        self.startAutomaticTestButton:QPushButton = QPushButton("Start")
        self.stopAutomaticTestButton:QPushButton = QPushButton("Stop")
        buttonsBox:QHBoxLayout = QHBoxLayout()
        self.stopAutomaticTestButton.setDisabled(True)
        self.startAutomaticTestButton.clicked.connect(self.startAutomaticTest)
        self.stopAutomaticTestButton.clicked.connect(self.stopAutomaticTest)
        
        for item in [self.startAutomaticTestButton, self.stopAutomaticTestButton]:
            buttonsBox.addWidget(item)

        self.automaticTestBox.addLayout(buttonsBox)
        self.automaticTestLabel:QLabel = QLabel("Komunikaty...")
        self.automaticTestBox.addWidget(self.automaticTestLabel)

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
        mainLayout.addLayout(self.recordingBox)
        mainLayout.addLayout(self.automaticTestBox)
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

    def record(self):
        self.isRecording = True
        self.startRecordingButton.setDisabled(True)
        self.stopRecordingButton.setDisabled(False)        
        self.recordingLabel.setText(self.recordingTimeToString())   

        self.cpuUsageBuffer.clear()
        self.ramUsageBuffer.clear()     
        self.recordingTime = 0

    def stopRecording(self):
        self.isRecording = False
        outputFileName:str = f"output/{datetime.datetime.strftime(datetime.datetime.now(), "%d-%m-%Y-%H-%M-%S")}.json"

        self.startRecordingButton.setDisabled(False)
        self.stopRecordingButton.setDisabled(True)        
        self.recordingLabel.setText(f"Czas nagrania: {self.recordingTimeToString()}, Raport: {outputFileName}")
        
        data:dict = {
            'ram' : self.ramUsageBuffer,
            'cpu' : self.cpuUsageBuffer
        }

        self.saveOutput(outputFileName, data)
    
    def saveOutput(self, path:str, data:dict) -> None:
        with open(path, 'w') as file:
            json.dump(data, file)

    def addLog(self, what:str) -> None:
        now:str = datetime.datetime.strftime(datetime.datetime.now(), "%d-%m-%Y %H:%M:%S")
        text:str = f"{now}\t{what}"
        self.logsBuffer.append(text)
        self.updateLogsLabel()

    def updateLogsLabel(self) -> None:
        self.automaticTestLabel.setText(self.logsBuffer[-1])

    def startAutomaticTest(self):
        self.logsBuffer.clear()
        self.addLog("Rozpoczęto test automatyczny")

        self.startAutomaticTestButton.setDisabled(True)
        self.stopAutomaticTestButton.setDisabled(False)

    def stopAutomaticTest(self):
        self.addLog("Zakończono test automatyczny")
        outputFileName:str = f"output/automatic-test-{datetime.datetime.strftime(datetime.datetime.now(), "%d-%m-%Y-%H-%M-%S")}.json"

        self.startAutomaticTestButton.setDisabled(False)
        self.stopAutomaticTestButton.setDisabled(True)

        data:dict = {
            'ram' : self.ramUsageBuffer,
            'cpu' : self.cpuUsageBuffer,
            'logs' : self.logsBuffer
        }

        self.saveOutput(outputFileName, data)

    def setSynthActive(self):
        isChecked:bool = self.synthActiveBox.cBox.isChecked()

        self.parent_conn.send({
            "type": "SET_SYNTH",
            "data": isChecked
        })

    def setModelActive(self):
        isChecked:bool = self.modelActiveBox.cBox.isChecked()

        self.parent_conn.send({
            "type": "SET_MODEL",
            "data": isChecked
        })

    def setVoiceActive(self, voiceId:int, value:bool):
        print(voiceId)

        self.parent_conn.send({
            "type": "SET_VOICE",
            "data": (voiceId, value)
        })

    def boxSetVoiceActiveEvent(self, voiceId:int):
        self.setVoiceActive(voiceId, self.voiceBoxes[voiceId].isChecked())

    def setSamplesActive(self):
        isChecked:bool = self.samplesActiveBox.cBox.isChecked()

        self.parent_conn.send({
            "type": "SET_SAMPLES",
            "data": isChecked
        })

    def request_synth_active(self):
        self.request_id += 1
        rid = self.request_id

        self.parent_conn.send({
            "type": "GET_SYNTH_ACTIVE",
            "id": rid
        })

        self.pending_requests[rid] = "GET_SYNTH_ACTIVE"

    def request_model_active(self):
        self.request_id += 1
        rid = self.request_id

        self.parent_conn.send({
            "type": "GET_MODEL_ACTIVE",
            "id": rid
        })

        self.pending_requests[rid] = "GET_MODEL_ACTIVE"

    def request_samples_active(self):
        self.request_id += 1
        rid = self.request_id

        self.parent_conn.send({
            "type": "GET_SAMPLES_ACTIVE",
            "id": rid
        })

        self.pending_requests[rid] = "GET_SAMPLES_ACTIVE"

    def request_device_name(self):
        self.request_id += 1
        rid = self.request_id

        self.parent_conn.send({
            "type": "GET_DEVICE_NAME",
            "id": rid
        })

        self.pending_requests[rid] = "GET_DEVICE_NAME"

    def request_voices_names(self):
        self.request_id += 1
        rid = self.request_id

        self.parent_conn.send({
            "type": "GET_VOICES_NAMES",
            "id": rid
        })

        self.pending_requests[rid] = "GET_VOICES_NAMES"

    def poll_engine(self):
        while self.parent_conn.poll():
            msg = self.parent_conn.recv()

            if msg["type"] == "GET_SYNTH_ACTIVE_RESULT":
                rid = msg["id"]
                value = msg["value"]

                self.synthActiveBox.cBox.setChecked(value)
                del self.pending_requests[rid]

            elif msg["type"] == "GET_SAMPLES_ACTIVE_RESULT":
                rid = msg["id"]
                value = msg["value"]

                self.samplesActiveBox.cBox.setChecked(value)
                del self.pending_requests[rid]

            elif msg["type"] == "GET_MODEL_ACTIVE_RESULT":
                rid = msg["id"]
                value = msg["value"]

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
                self.deviceNameLabel.setText(msg['value'])

            elif msg['type'] == 'GET_VOICES_NAMES_RESULT':
                self.voicesNames = msg['value']

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
        return self.engine_ps.cpu_percent(None)

    def get_ram(self):
        return self.engine_ps.memory_info().rss / 1024 / 1024

    def get_stats(self):
        cpu = self.get_cpu()
        ram = self.get_ram()
        return cpu, ram
    
    def update_stats(self):
        cpu, ram = self.get_stats()
        self.cpuUsageText.setText(f"{cpu:.1f}")
        self.ramUsageText.setText(f"{ram:.1f}")

        if self.isRecording:
            self.ramUsageBuffer.append(ram)
            self.cpuUsageBuffer.append(cpu)
            self.recordingTime += 500
            self.recordingLabel.setText(self.recordingTimeToString())

    def closeEvent(self, event):
        self.ui_timer.stop()
        self.engine_timer.stop()

        self.parent_conn.send({"type": "STOP"})
        self.engine_process.join()
        event.accept()

    def send_midi(self, note):
        self.parent_conn.send({
            "type": "MIDI",
            "data": note
        })

if __name__ == '__main__':
    app:QApplication = QApplication(sys.argv)

    UI:ui = ui()
    UI.show()

    sys.exit(app.exec())