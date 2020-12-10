#!/bin/bash
#

if [ $# -ne 2 ]; then
  echo "Usage: $0 speculos_bin kpl_build_dir" 1>&2
  exit 1
fi

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
export SPECULOS_BIN="$1"
export KPL_BUILD_DIR="$2"
export APP_BIN="$SCRIPT_DIR/../app/bin/app.elf"
python -m unittest discover "$SCRIPT_DIR"
