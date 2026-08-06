#!/usr/bin/env bash
#
# Parse the flash and RAM figures out of an arduino-cli compile log.
#
# Emits shell-eval-able key=value lines so callers can source the result:
#
#   eval "$(parse-size.sh compile.log)"
#   echo "$flash of $flash_max"
#
# The two lines it reads look like:
#   Sketch uses 1800672 bytes (53%) of program storage space. Maximum is 3342336 bytes.
#   Global variables use 54440 bytes (16%) of dynamic memory, leaving 273240 bytes for local variables. Maximum is 327680 bytes.

set -euo pipefail

LOG="${1:?usage: parse-size.sh <compile.log>}"
[ -f "$LOG" ] || { echo "::error::cannot read '$LOG'" >&2; exit 1; }

# Pull the number that follows a given prefix on a given line.
field() {
  local line_match="$1" pattern="$2"
  grep -F "$line_match" "$LOG" | head -n 1 | grep -oE "$pattern" | grep -oE '[0-9]+' | head -n 1
}

flash="$(field   'of program storage space' 'uses [0-9]+ bytes')"
flash_max="$(field 'of program storage space' 'Maximum is [0-9]+ bytes')"
ram="$(field     'of dynamic memory'        'use [0-9]+ bytes')"
ram_max="$(field   'of dynamic memory'        'Maximum is [0-9]+ bytes')"

for v in flash flash_max ram ram_max; do
  if [ -z "${!v}" ]; then
    echo "::error::could not parse '$v' from '$LOG'" >&2
    exit 1
  fi
done

printf 'flash=%s\nflash_max=%s\nram=%s\nram_max=%s\n' "$flash" "$flash_max" "$ram" "$ram_max"
