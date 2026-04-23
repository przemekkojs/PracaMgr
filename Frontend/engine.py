from multiprocessing import Pipe, Process
from multiprocessing.connection import Connection

import sys
import psutil

sys.path.append("../Backend/python")
from organ_engine import MainModule, NoteSignal, EMPTY_NOTE_SIGNAL


def run_engine(pipe: Connection):
    organ:MainModule = MainModule()
    running:bool = True
    organ.init()

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

class EngineClient:
    def __init__(self):
        self.parent_conn, child_conn = Pipe()
        self.process = Process(target=run_engine, args=(child_conn,))
        self.process.start()

        self.request_id = 0
        self.pending_requests = {}

    def send(self, msg):
        self.parent_conn.send(msg)

    def request(self, type_):
        self.request_id += 1
        rid = self.request_id
        self.send({"type": type_, "id": rid})
        self.pending_requests[rid] = type_
        return rid

    def poll(self):
        messages = []
        while self.parent_conn.poll():
            messages.append(self.parent_conn.recv())
        return messages

    def stop(self):
        self.send({"type": "STOP"})
        self.process.join()

class EngineMonitor:
    def __init__(self, pid):
        self.ps = psutil.Process(pid)

    def get_cpu(self):
        cpu = self.ps.cpu_percent(interval=0.1)
        return cpu / psutil.cpu_count()

    def get_ram(self):
        return self.ps.memory_info().rss / 1024 / 1024
    
