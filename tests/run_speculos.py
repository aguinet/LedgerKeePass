import unittest
import random
import psutil
import os
import sys
import json
import subprocess

speculos_bin = os.getenv("SPECULOS_BIN")
app_bin = os.getenv("APP_BIN")

class RunSpeculos():
    def __init__(self, API, automation=None):
        self.API = API
        args = [sys.executable, speculos_bin, app_bin, "-k", "1.6", "--apdu-port", "0", "--display","headless"]
        if automation is not None:
            args += ["--automation", json.dumps(automation)]
        self.process = subprocess.Popen(args)
        # Gather the port speculos listen on thanks to psutils
        conns = []
        while len(conns) == 0:
            conns = psutil.Process(self.process.pid).connections()
        self.port = conns[0].laddr[1]

    def __enter__(self):
        return self.API.connect(self.port)

    def __exit__(self, exc_type, exc_value, exc_traceback):
        # Close speculos
        self.process.kill()
        self.process.wait()
