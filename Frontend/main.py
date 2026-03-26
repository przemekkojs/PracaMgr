from PySide6.QtWidgets import QMainWindow, QApplication, QCheckBox, QHBoxLayout, QVBoxLayout, QWidget, QLabel, QLineEdit

import sys

class checkboxLabel(QWidget):
    def __init__(self, label_text:str, callback):
        super().__init__()
        self.cBox:QCheckBox = QCheckBox()
        self.label:QLabel = QLabel(label_text)
        self.setLayout(QHBoxLayout())

        self.layout().addWidget(self.cBox)
        self.layout().addWidget(self.label)

        self.cBox.stateChanged.connect(callback)


class textboxLabel(QWidget):
    def __init__(self, label_text:str, readonly:bool=True):
        super().__init__()
        self.tBox:QLineEdit = QLineEdit()
        self.tBox.setReadOnly(readonly)

        self.label:QLabel = QLabel(label_text)
        self.setLayout(QHBoxLayout())

        self.layout().addChildWidget(self.tBox)
        self.layout().addChildWidget(self.label)

    def setText(self, text:str) -> None:
        self.tBox.setText(text)


class ui(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setLayout(QHBoxLayout())
        self.setWindowTitle("Praca magisterska")

        self.cpuUsageText:textboxLabel = textboxLabel("CPU (Percent)")
        self.ramUsageText:textboxLabel = textboxLabel("RAM (MB)")
        self.usagesBox:QVBoxLayout = QVBoxLayout()
        
        for item in [self.cpuUsageText, self.ramUsageText]:
            self.usagesBox.addChildWidget(item)

        self.synthRealismText:textboxLabel = textboxLabel("Synth realism")
        self.modelRealismText:textboxLabel = textboxLabel("Model realism")
        self.realismBox:QVBoxLayout = QVBoxLayout()

        for item in [self.synthRealismText, self.modelRealismText]:
            self.realismBox.addChildWidget(item)

        self.statsBox:QVBoxLayout = QVBoxLayout()
        
        for item in [self.usagesBox, self.realismBox]:
            self.statsBox.addChildLayout(item)

        self.synthActiveBox:checkboxLabel = checkboxLabel("Synth active", self.setSynthActive)
        self.samplesActiveBox:checkboxLabel = checkboxLabel("Samples active", self.setSamplesActive)
        self.modelActiveBox:checkboxLabel = checkboxLabel("Model active", self.setModelActive)
        self.activeBox:QVBoxLayout = QVBoxLayout()
        
        for item in [self.synthActiveBox, self.samplesActiveBox, self.modelActiveBox]:
            self.activeBox.addChildWidget(item)

        for item in [self.activeBox, self.statsBox]:
            self.layout().addChildLayout(item)

    def setSynthActive(self):
        pass

    def setModelActive(self):
        pass

    def setSamplesActive(self):
        pass

if __name__ == '__main__':
    app:QApplication = QApplication(sys.argv)

    UI:ui = ui()
    UI.show()

    sys.exit(app.exec())