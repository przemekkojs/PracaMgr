from PySide6.QtWidgets import QMainWindow, QApplication, QCheckBox, QHBoxLayout, QVBoxLayout, QWidget, QLabel, QLineEdit, QPushButton
from PySide6.QtCore import QTimer

import sys
import psutil
import datetime
import math

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

        self.recordingTime:float = 0
        self.isRecording:bool = False
        self.ramUsageBuffer:list[float] = []
        self.cpuUsageBuffer:list[float] = []

        self.process = psutil.Process()
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_stats)
        self.timer.start(500) # To jest w ms

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
        
        for item in [QLabel("Moduły"), self.synthActiveBox, self.samplesActiveBox, self.modelActiveBox]:
            self.activeBox.addWidget(item)

        for item in [self.activeBox, self.statsBox, self.voicesBox]:
            layout.addLayout(item)

        self.deviceNameLabel:QLabel = QLabel("Brak wejściowego urządzenia MIDI")
        mainLayout:QVBoxLayout = QVBoxLayout()        
        mainLayout.addWidget(self.deviceNameLabel)
        mainLayout.addLayout(self.recordingBox)
        mainLayout.addLayout(layout)
        self.setLayout(mainLayout)

        self.initVoices()

    def initVoices(self):
        # To potem trzeba wczytywać z VoiceManager'a
        voicesList:str = ["Prinzipal 8'", "Holzgedackt 8'", "Gambe 8'", "Trompete 8'", "Mixtur 3-4 fach."]

        for vId, v in enumerate(voicesList):
            item:QPushButton = QPushButton(v)
            item.clicked.connect(lambda _, vId=vId: self.setVoiceActive(vId + 1))
            self.voicesBox.addWidget(item)

    def recordingTimeToString(self) -> str:
        fullSeconds:int = int(self.recordingTime // 1000) # Tutaj trzeba modulo 60 zrobić

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
        
        print(self.cpuUsageBuffer)
        print(self.ramUsageBuffer)

    def setSynthActive(self):
        isChecked:bool = self.synthActiveBox.cBox.isChecked()

    def setModelActive(self):
        isChecked:bool = self.modelActiveBox.cBox.isChecked()

    def setSamplesActive(self):
        isChecked:bool = self.samplesActiveBox.cBox.isChecked()

    def setVoiceActive(self, voiceId:int):
        print(voiceId)

    def setDeviceName(self, value:str):
        self.deviceNameLabel.setText(value)

    def get_cpu(self):
        return self.process.cpu_percent(None)
    
    def get_ram(self):
        return self.process.memory_info().rss / 1024 / 1024

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

if __name__ == '__main__':
    app:QApplication = QApplication(sys.argv)

    UI:ui = ui()
    UI.show()

    sys.exit(app.exec())