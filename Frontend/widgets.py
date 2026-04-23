from PySide6.QtWidgets import QCheckBox, QHBoxLayout, QWidget, QLabel, QLineEdit

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
