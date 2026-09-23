# Makefile — build the flashable firmware image locally.
#
#   make                      both boards -> dist/firmware-<ver>-<chip>-full.bin
#   make esp32c6 | esp32s3    one board
#   make flash CHIP=esp32s3 PORT=/dev/cu.usbmodem101
#   make setup                install the pinned core + libraries (needs arduino-cli)
#   make clean
#
# The FQBN and toolchain pins are read from .github/board-targets.json, the
# same file build.yml and release.yml use, so a local image matches CI's. The
# trim to a flash-at-0x0 image is done by .devcontainer/scripts/build-full-bin.sh,
# which reproduces release.yml's "Collect release assets" step.
#
# Requires: arduino-cli, jq. `flash` also needs esptool (pip install esptool).

TARGETS := .github/board-targets.json
CHIPS   := $(shell jq -r '.[].chip' $(TARGETS))
field    = $(shell jq -r --arg c '$(1)' '.[] | select(.chip == $$c) | .["$(2)"]' $(TARGETS))

CHIP ?= esp32c6
PORT ?=
BAUD ?= 921600

.PHONY: all $(CHIPS) setup flash clean help

all: $(CHIPS)

# Always delegate to arduino-cli: it tracks sketch dependencies itself and
# reuses its build cache, so a rebuild with no changes is quick.
$(CHIPS):
	arduino-cli compile \
	  --fqbn "$(call field,$@,fqbn)" \
	  --warnings default \
	  --output-dir "build-out/$(call field,$@,slug)" \
	  .
	bash .devcontainer/scripts/build-full-bin.sh $@ "build-out/$(call field,$@,slug)"

setup:
	arduino-cli config init --overwrite
	arduino-cli config add board_manager.additional_urls \
	  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
	arduino-cli core update-index
	arduino-cli core install "esp32:esp32@$(call field,$(CHIP),core-version)"
	arduino-cli lib update-index
	arduino-cli lib install "lvgl@$(call field,$(CHIP),lvgl-version)"
	arduino-cli lib install "GFX Library for Arduino@$(call field,$(CHIP),gfx-version)"
	arduino-cli lib install "FastIMU@$(call field,$(CHIP),fastimu-version)"
	mkdir -p "$(HOME)/Arduino/libraries"
	cp lv_conf.h "$(HOME)/Arduino/libraries/lv_conf.h"

# Writes the trimmed image at 0x0. Never flash build-out/*.merged.bin instead:
# it is padded to the whole chip and wipes the S3's FFat (config.ini, GIFs).
flash: $(CHIP)
	@test -n "$(PORT)" || { echo "PORT is required, e.g. make flash CHIP=$(CHIP) PORT=/dev/cu.usbmodem101"; exit 1; }
	esptool --chip $(CHIP) --port "$(PORT)" --baud $(BAUD) write-flash 0x0 "$$(ls -t dist/*-$(CHIP)-full.bin | head -1)"

clean:
	rm -rf build build-out dist

help:
	@sed -n '3,8p' $(firstword $(MAKEFILE_LIST))
