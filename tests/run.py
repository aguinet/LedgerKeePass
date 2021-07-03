import sys
import argparse
import subprocess
import unittest
import os
import atexit
import psutil

def kill_speculos(process):
    process.kill()
    process.wait()

def run_speculos(path, app_bin, model):
    apdu_port = 32001
    automation_port = 32002
    args = [sys.executable, path, app_bin, "--apdu-port", str(apdu_port), "--api-port", str(automation_port), "--display","headless", "--model", model]
    if model == "nanos":
        args += ["-k", "1.6"]
    process = subprocess.Popen(args)
    atexit.register(kill_speculos, process)
    # Wait for at least two listening ports for this process
    conns = []
    while len(conns) < 2:
        conns = psutil.Process(process.pid).connections()
    return process, apdu_port, automation_port


parser = argparse.ArgumentParser(description="KPL tests")
parser.add_argument("--kpl-build-dir", type=str, required=False)
parser.add_argument("--model", type=str, choices=["nanos","nanox"], required=True)
parser_speculos = parser.add_mutually_exclusive_group(required=True)
parser_speculos.add_argument("--speculos-bin", type=str, required=False, help="Path to a speculos binary to run locally")
parser_speculos.add_argument("--speculos-remote", type=str, required=False, help="SPECULOS_IP:APDU_PORT:AUTOMATION_PORT")

args = parser.parse_args()

script_dir = os.path.dirname(os.path.realpath(__file__))

if args.speculos_bin:
    # Run the speculos automation server and get the listening port
    app_elf = os.path.join(script_dir, "..", "app", "bin", "app.elf")
    speculos_process, speculos_apdu_port, speculos_automation_port = run_speculos(args.speculos_bin, app_elf, args.model)
    speculos_host = "127.0.0.1"
else:
    speculos_url, speculos_apdu_port, speculos_automation_port = args.speculos_remote.split(":")
    speculos_apdu_port = int(speculos_apdu_port)
    speculos_automation_port = int(speculos_automation_port)

env = {
    'KPL_BUILD_DIR': args.kpl_build_dir,
    'SPECULOS_MODEL': args.model,
    'SPECULOS_HOST': speculos_host,
    'SPECULOS_APDU_PORT': speculos_apdu_port,
    'SPECULOS_AUTOMATION_PORT': speculos_automation_port
}
for k,v in env.items():
    os.environ[k] = str(v)

# Run all tests
loader = unittest.TestLoader()
suite = loader.discover(script_dir)

runner = unittest.TextTestRunner()
runner.run(suite)
