#!/usr/bin/env bash
#
# Shared FW_VERSION helper.
#
# The FW_VERSION macro in the sketch is the single source of truth for the
# firmware version. CI reads and validates it; CI never edits it and never
# pushes commits back. Stage 2 (version-check.yml) and Stage 3 (release.yml)
# both call this script so the parsing rules exist in exactly one place.
#
# Usage:
#   fw-version.sh sketch-name [dir]     print the sketch filename (e.g. Foo.ino)
#   fw-version.sh extract <file|->      print the FW_VERSION string, e.g. v2.7.1
#   fw-version.sh validate <version>    exit 0 if it parses as v<x>.<y>.<z>[-pre]
#   fw-version.sh is-prerelease <ver>   exit 0 if it carries a -suffix
#   fw-version.sh is-newer <new> <old>  exit 0 if <new> sorts strictly above <old>

set -euo pipefail

# Semantic version with a mandatory leading 'v' (FW_VERSION already includes it,
# so the string maps to a git tag with no prefixing) and an optional prerelease
# suffix such as -beta or -rc.1.
SEMVER_RE='^v(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(-[0-9A-Za-z.-]+)?$'

die() { echo "::error::$*" >&2; exit 1; }

# Locate the single top-level .ino. Discovered rather than hard-coded so a
# rename or an added hardware target doesn't silently break the workflows.
cmd_sketch_name() {
  local dir="${1:-.}" found=() f
  while IFS= read -r f; do found+=("$(basename "$f")"); done \
    < <(find "$dir" -maxdepth 1 -name '*.ino' | sort)

  case "${#found[@]}" in
    0) die "no .ino file found in '$dir'" ;;
    1) printf '%s\n' "${found[0]}" ;;
    *) die "expected exactly one top-level .ino, found ${#found[@]}: ${found[*]}" ;;
  esac
}

# Read FW_VERSION from a file, or from stdin when given '-'. The stdin form is
# what lets the caller parse another revision without a second checkout:
#   git show origin/development:Sketch.ino | fw-version.sh extract -
cmd_extract() {
  local src="${1:?usage: extract <file|->}" version

  if [ "$src" != "-" ] && [ ! -f "$src" ]; then
    die "cannot read '$src'"
  fi

  # POSIX BRE only — no \+ or \s, which BSD sed rejects.
  version=$(
    sed -n \
      's/^[[:space:]]*#[[:space:]]*define[[:space:]][[:space:]]*FW_VERSION[[:space:]][[:space:]]*"\([^"]*\)".*/\1/p' \
      "$src" | head -n 1
  )

  [ -n "$version" ] && printf '%s\n' "$version" || die "no FW_VERSION macro found in '$src'"
}

cmd_validate() {
  local version="${1:?usage: validate <version>}"
  echo "$version" | grep -Eq "$SEMVER_RE" \
    || die "FW_VERSION '$version' is not a valid version — expected v<major>.<minor>.<patch> with an optional prerelease suffix, e.g. v2.8.0 or v2.8.0-beta.1"
}

cmd_is_prerelease() {
  local version="${1:?usage: is-prerelease <version>}"
  case "$version" in
    *-*) return 0 ;;
    *)   return 1 ;;
  esac
}

# Semver precedence. Deliberately NOT `sort -V`: GNU version sort ranks
# v2.8.0-beta ABOVE v2.8.0, whereas semver (and we) require a final release to
# outrank its own prerelease, so promoting a beta to final must count as newer.
#
# Each comparator returns 0 when equal, 1 when the first argument wins, 2 when
# the second does. Callers must have run `validate` first.

_cmp_core() {
  local a="${1#v}" b="${2#v}" i x y
  local -a A B
  # Drop any prerelease suffix — only the numeric core is compared here.
  IFS=. read -r -a A <<< "${a%%-*}"
  IFS=. read -r -a B <<< "${b%%-*}"
  for i in 0 1 2; do
    x=${A[i]} y=${B[i]}
    [ "$x" -gt "$y" ] && return 1
    [ "$x" -lt "$y" ] && return 2
  done
  return 0
}

_is_num() { case "$1" in ''|*[!0-9]*) return 1 ;; *) return 0 ;; esac; }

_cmp_prerelease() {
  local a="$1" b="$2" i max x y
  local -a A B

  # A version with no prerelease suffix outranks one that has it.
  [ -z "$a" ] && [ -z "$b" ] && return 0
  [ -z "$a" ] && return 1
  [ -z "$b" ] && return 2

  IFS=. read -r -a A <<< "$a"
  IFS=. read -r -a B <<< "$b"
  max=${#A[@]}; [ "${#B[@]}" -gt "$max" ] && max=${#B[@]}

  for (( i = 0; i < max; i++ )); do
    x=${A[i]-} y=${B[i]-}
    # A larger set of fields outranks a smaller one when all else is equal.
    [ -z "$x" ] && return 2
    [ -z "$y" ] && return 1
    [ "$x" = "$y" ] && continue

    if _is_num "$x" && _is_num "$y"; then
      [ "$x" -gt "$y" ] && return 1 || return 2
    fi
    # Numeric identifiers always rank below alphanumeric ones.
    _is_num "$x" && return 2
    _is_num "$y" && return 1
    [[ "$x" > "$y" ]] && return 1 || return 2
  done
  return 0
}

cmd_is_newer() {
  local new="${1:?usage: is-newer <new> <old>}" old="${2:?usage: is-newer <new> <old>}"
  local rc=0

  _cmp_core "$new" "$old" || rc=$?
  [ "$rc" -eq 1 ] && return 0
  [ "$rc" -eq 2 ] && return 1

  # Cores are equal — decide on the prerelease suffixes alone.
  local pre_new="" pre_old=""
  case "$new" in *-*) pre_new="${new#*-}" ;; esac
  case "$old" in *-*) pre_old="${old#*-}" ;; esac

  rc=0
  _cmp_prerelease "$pre_new" "$pre_old" || rc=$?
  [ "$rc" -eq 1 ]
}

case "${1:-}" in
  sketch-name)   shift; cmd_sketch_name "$@" ;;
  extract)       shift; cmd_extract "$@" ;;
  validate)      shift; cmd_validate "$@" ;;
  is-prerelease) shift; cmd_is_prerelease "$@" ;;
  is-newer)      shift; cmd_is_newer "$@" ;;
  *) die "unknown command '${1:-}' — see the usage block at the top of this script" ;;
esac
