#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ -x "${ROOT_DIR}/build/eae_firmware" ]]; then
  EXE="${ROOT_DIR}/build/eae_firmware"
elif [[ -x "${ROOT_DIR}/build/Release/eae_firmware" ]]; then
  EXE="${ROOT_DIR}/build/Release/eae_firmware"
else
  echo "Executable not found. Build first with: cmake -S . -B build && cmake --build build" >&2
  exit 1
fi

"${EXE}" --target-temp 65 --initial-temp 42 --steps 90 --dt 1.0 "$@"
