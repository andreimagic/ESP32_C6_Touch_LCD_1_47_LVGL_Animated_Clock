#!/usr/bin/env bash
#
# partition-props.sh <csv>
#
# Prints the arduino-cli --build-property values, one per line, that build a
# board with a partition table kept in this repository rather than one of the
# core's presets. Used by the arduino-build action and the Makefile, so CI and a
# local `make` apply a board's table identically.
#
# Pair it with PartitionScheme=custom in the FQBN, which turns off the core's
# preset copy (build.partitions is empty there). Two properties result:
#
#   recipe.hooks.prebuild.9.pattern
#     Copies <csv> over {build.path}/partitions.csv, which every later step
#     (gen_esp32part, merge-bin, upload) reads. platform.txt uses prebuild hooks
#     1-8, and arduino-cli runs hooks in string-sorted key order, so 9 runs after
#     all of them — including the core's own partitions.csv copies (1-3).
#
#   upload.maximum_size
#     The app0 size from <csv>. The custom scheme's default is the whole chip,
#     which would let a sketch that no longer fits its slot pass the size check.
#
# <csv> is relative to the sketch folder.

set -euo pipefail

CSV="${1:?usage: partition-props.sh <csv relative to the sketch folder>}"
[ -f "$CSV" ] || { echo "::error::partition table '$CSV' not found" >&2; exit 1; }

APP_SIZE=$(awk -F, '
  /^[[:space:]]*#/ { next }
  { gsub(/[[:space:]]/, "") }
  $2 == "app" { print $5; exit }
' "$CSV")
[ -n "$APP_SIZE" ] || { echo "::error::no app partition in '$CSV'" >&2; exit 1; }

printf '%s\n' \
  "recipe.hooks.prebuild.9.pattern=/usr/bin/env bash -c \"cp -f '{build.source.path}/${CSV}' '{build.path}/partitions.csv'\"" \
  "upload.maximum_size=$(( APP_SIZE ))"
