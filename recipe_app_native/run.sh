#!/usr/bin/env bash
# Simple launcher for the Recipe Explorer native app.
# It builds the project if necessary and runs the resulting binary.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BIN_NAME="MainApp"

# Ensure build directory exists
mkdir -p "${BUILD_DIR}"

# Configure if no cache present
if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
  cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
fi

# Build
cmake --build "${BUILD_DIR}"

# Run
exec "${BUILD_DIR}/${BIN_NAME}"
