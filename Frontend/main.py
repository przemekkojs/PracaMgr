from PySide6.QtWidgets import QMainWindow, QApplication, QCheckBox, QHBoxLayout, QVBoxLayout, QWidget, QLabel, QLineEdit
from PySide6.QtCore import QTimer

import sys
import psutil

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

        self.process = psutil.Process()
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_stats)
        self.timer.start(500) # To jest w ms

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

        for item in [QLabel("Metryka 1"), self.synthRealismText1, self.modelRealismText1]:
            self.realismBox1.addWidget(item)

        self.synthRealismText2:textboxLabel = textboxLabel("Syntezator")
        self.modelRealismText2:textboxLabel = textboxLabel("Model")
        self.realismBox2:QVBoxLayout = QVBoxLayout()

        for item in [QLabel("Metryka 2"), self.synthRealismText2, self.modelRealismText2]:
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

        for item in [self.activeBox, self.statsBox]:
            layout.addLayout(item)

        self.deviceNameLabel:QLabel = QLabel("Brak wejściowego urządzenia MIDI")
        mainLayout:QVBoxLayout = QVBoxLayout()        
        mainLayout.addWidget(self.deviceNameLabel)
        mainLayout.addLayout(layout)
        self.setLayout(mainLayout)

    def setSynthActive(self):
        print("Syntezator aktywny:", self.synthActiveBox.cBox.isChecked())

    def setModelActive(self):
        print("Model aktywny:", self.modelActiveBox.cBox.isChecked())

    def setSamplesActive(self):
        print("Sampler aktywny:", self.samplesActiveBox.cBox.isChecked())

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

if __name__ == '__main__':
    app:QApplication = QApplication(sys.argv)

    UI:ui = ui()
    UI.show()

    sys.exit(app.exec())