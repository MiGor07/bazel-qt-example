#!/usr/bin/env bash
# Собирает и запускает RayCaster без bazel — через системный Qt6 + qmake.
# Требования: qt6-base-dev + qmake6.
set -euo pipefail

SRC_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$SRC_DIR/build}"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

qmake6 "$SRC_DIR/raycaster.pro"
make -j"$(nproc)"

exec "$BUILD_DIR/raycaster"
