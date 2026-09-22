#!/usr/bin/env bash
# .devcontainer/postCreate.sh
#
# Installs the same toolchain .github/actions/arduino-build/action.yml
# installs for CI: a pinned arduino-cli, the pinned esp32:esp32 core, and the
# pinned lvgl / GFX Library for Arduino / FastIMU versions. Keep the version
# env vars here in sync with .github/board-targets.json.

set -euo pipefail

ARDUINO_CLI_VERSION="${ARDUINO_CLI_VERSION:-1.5.1}"
CORE_VERSION="${CORE_VERSION:-3.3.11}"
LVGL_VERSION="${LVGL_VERSION:-9.5.0}"
GFX_VERSION="${GFX_VERSION:-1.6.7}"
FASTIMU_VERSION="${FASTIMU_VERSION:-1.3.0}"

echo "==> Installing arduino-cli ${ARDUINO_CLI_VERSION}"
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh \
  | BINDIR=/usr/local/bin sh -s "v${ARDUINO_CLI_VERSION}"

echo "==> Configuring arduino-cli and adding the ESP32 board index"
arduino-cli config init --overwrite
arduino-cli config add board_manager.additional_urls \
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index

echo "==> Installing esp32:esp32@${CORE_VERSION}"
arduino-cli core install "esp32:esp32@${CORE_VERSION}"

echo "==> Installing pinned libraries"
arduino-cli lib update-index
arduino-cli lib install "lvgl@${LVGL_VERSION}"
arduino-cli lib install "GFX Library for Arduino@${GFX_VERSION}"
arduino-cli lib install "FastIMU@${FASTIMU_VERSION}"

echo "==> Installing project lv_conf.h into the libraries search path"
# Same reasoning as CI: LVGL 9.5 resolves config via LV_CONF_PATH, then
# __has_include from the sketch folder (which already satisfies it), then
# ../../lv_conf.h i.e. libraries/lv_conf.h. Copying it here too makes both
# resolution paths valid, matching the CI action byte-for-byte.
mkdir -p "$HOME/Arduino/libraries"
if [ -f "$(pwd)/lv_conf.h" ]; then
  cp "$(pwd)/lv_conf.h" "$HOME/Arduino/libraries/lv_conf.h"
else
  echo "::warning:: lv_conf.h not found at repo root yet — rerun this script (or just the cp line) once the workspace has synced."
fi

echo "==> Toolchain summary"
arduino-cli version
arduino-cli core list
arduino-cli lib list

echo "==> Done. Try: arduino-cli compile --fqbn <fqbn from .github/board-targets.json> ."