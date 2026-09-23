#!/usr/bin/env bash
# .devcontainer/scripts/build-full-bin.sh <chip> <build-out-dir>
#
# Reproduces the "Collect release assets" step in .github/workflows/release.yml
# for a single board, so a Codespace can produce the exact same
# firmware-<version>-<chip>-full.bin the Release pipeline attaches to GitHub
# Releases — flashable at 0x0 with esptool or Espressif's ESP Launchpad.
#
# The ESP32 core's own merge-bin hook writes build-out/<sketch>.merged.bin
# padded out to the WHOLE flash chip (platform.txt: --pad-to-size
# {build.flash_size}). Flashing that as-is at 0x0 blanks everything above the
# app — including the S3's FFat partition at 0x610000, which holds config.ini
# and the GIFs on a cardless board. This trims the image to end right after
# the app, matching release.yml exactly, and refuses to trim if it finds any
# non-0xFF byte above the cut point (i.e. if a future core version starts
# putting real data there).
#
# Run this AFTER the matching "Build: ESP32-*" task, which is what populates
# build-out/<slug>/.

set -euo pipefail

CHIP="${1:?usage: build-full-bin.sh <chip> <build-out-dir>   e.g. build-full-bin.sh esp32c6 build-out/esp32c6-lcd147}"
BUILD_DIR="${2:?usage: build-full-bin.sh <chip> <build-out-dir>   e.g. build-full-bin.sh esp32c6 build-out/esp32c6-lcd147}"

FW=".github/scripts/fw-version.sh"
SKETCH="$(bash "$FW" sketch-name)"
VERSION="$(bash "$FW" extract "$SKETCH")"
bash "$FW" validate "$VERSION"

MERGED="${BUILD_DIR}/${SKETCH}.merged.bin"
APP="${BUILD_DIR}/${SKETCH}.bin"

if [ ! -f "$MERGED" ] || [ ! -f "$APP" ]; then
  echo "::error:: Expected build output missing in ${BUILD_DIR}. Run the matching Build task first."
  ls -la "$BUILD_DIR" 2>/dev/null || true
  exit 1
fi

APP_END=$(( 0x10000 + $(wc -c < "$APP") ))
CUT=$(( (APP_END + 4095) / 4096 * 4096 ))   # round up to a 4KB sector

STRAY=$(tail -c "+$(( CUT + 1 ))" "$MERGED" | LC_ALL=C tr -d '\377' | wc -c)
if [ "$STRAY" -ne 0 ]; then
  echo "::error:: $STRAY non-0xFF bytes found above offset $CUT in $MERGED — refusing to truncate."
  echo "::error:: This would mean the trim is no longer safe; check the core version against release.yml's assumptions."
  exit 1
fi

mkdir -p dist
OUT="dist/firmware-${VERSION}-${CHIP}-full.bin"
head -c "$CUT" "$MERGED" > "$OUT"

echo "Wrote ${OUT}"
echo "  $(wc -c < "$OUT" | tr -d " ") bytes (trimmed from $(wc -c < "$MERGED" | tr -d " ") bytes)"
echo ""
echo "Also available: ${BUILD_DIR}/${SKETCH}.bin — the app partition alone,"
echo "for OTA or for reflashing an existing install at 0x10000. Use esptool"
echo "for that, not a web flasher: writing the wrong offset there removes the"
echo "bootloader and leaves the board unable to boot."
echo ""
echo "To flash the full image:"
echo "  - ESP Launchpad (https://espressif.github.io/esp-launchpad/, Chrome/Edge only):"
echo "    DIY tab -> Connect -> pick '${OUT}' -> flash address 0x0 -> Program"
echo "  - esptool: esptool --chip ${CHIP} write-flash 0x0 ${OUT}"