#!/usr/bin/env bash
# Entrypoint for the Recipe Explorer native Qt app.
# Builds the project (if needed) and launches the binary.
# This script is designed to be safe for execution under 'bash -c' wrappers.

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
