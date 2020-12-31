#!/bin/bash
#

if [ $# -ne 3 ]; then
  echo "Usage: $0 speculos_bin kpl_build_dir model" 1>&2
  echo "Model is nanos or nanox" 1>&2
  exit 1
fi

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
export SPECULOS_BIN="$1"
export KPL_BUILD_DIR="$2"
export APP_BIN="$SCRIPT_DIR/../app/bin/app.elf"
export SPECULOS_MODEL="$3"
python -m unittest discover "$SCRIPT_DIR"
