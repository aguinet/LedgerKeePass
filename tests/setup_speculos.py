import unittest
import os
import requests

speculos_host = os.getenv("SPECULOS_HOST")
speculos_apdu_port = int(os.getenv("SPECULOS_APDU_PORT"))
speculos_automation_port = int(os.getenv("SPECULOS_AUTOMATION_PORT"))
app_bin = os.getenv("APP_BIN")
speculos_model = os.getenv("SPECULOS_MODEL")

class SetupSpeculos():
    def __init__(self, API, automation=None):
        self.API = API
        if automation is None:
            automation = {"version": 1, "rules": []}
        r = requests.post(f"http://{speculos_host}:{speculos_automation_port}/automation", json=automation)
        r.raise_for_status()

    def __enter__(self):
        return self.API.connect(speculos_host, speculos_apdu_port)

    def __exit__(self, exc_type, exc_value, exc_traceback):
        pass
