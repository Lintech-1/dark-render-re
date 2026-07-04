#!/bin/sh
set -eu

FFMPEG_URL="${FFMPEG_URL:-https://github.com/defisym/FFmpeg-Builds-Win32/releases/download/latest/ffmpeg-n7.1-latest-win32-gpl-7.1.zip}"

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROXY_DLL="$SCRIPT_DIR/dist/VideoEnginePlugin.vPlugin"
STEAM_USER="${USER:-deck}"

die() {
  echo "error: $*" >&2
  exit 1
}

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "missing command: $1"
}

is_dark_dir() {
  [ -f "$1/DarkApp.exe" ] &&
    [ -d "$1/Localization_Common" ] &&
    [ -f "$1/VisionEnginePlugin.vPlugin" ]
}

add_candidate() {
  dir="$1"
  [ -d "$dir" ] || return 0
  is_dark_dir "$dir" || return 0
  case "
$CANDIDATES
" in
    *"
$dir
"*) return 0 ;;
  esac
  CANDIDATES="${CANDIDATES}${dir}
"
}

scan_dark_dirs() {
  CANDIDATES=""

  if [ "${DARK_GAME_DIR:-}" ]; then
    add_candidate "$DARK_GAME_DIR"
    [ "$CANDIDATES" ] && return
    die "DARK_GAME_DIR is not DARK game root: $DARK_GAME_DIR"
  fi

  add_candidate "$PWD"

  for d in \
    "$HOME/.steam/steam/steamapps/common/Dark" \
    "$HOME/.local/share/Steam/steamapps/common/Dark" \
    "$HOME/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/Dark" \
    /run/media/"$STEAM_USER"/*/steamapps/common/Dark \
    /run/media/deck/*/steamapps/common/Dark
  do
    add_candidate "$d"
  done

  [ "$CANDIDATES" ] && return

  for root in "$HOME" /run/media/"$STEAM_USER" /run/media/deck; do
    [ -d "$root" ] || continue
    while IFS= read -r exe; do
      d=$(dirname "$exe")
      add_candidate "$d"
    done <<EOF
$(find "$root" -maxdepth 8 -type f -name DarkApp.exe 2>/dev/null)
EOF
  done
}

find_dark_dir() {
  scan_dark_dirs

  count=$(printf '%s' "$CANDIDATES" | sed '/^$/d' | wc -l)
  [ "$count" -gt 0 ] || die "DARK game root not found. Run: DARK_GAME_DIR=\"/path/to/Dark\" sh install.sh"

  if [ "$count" -eq 1 ]; then
    printf '%s' "$CANDIDATES" | sed '/^$/d'
    return
  fi

  echo "Multiple DARK installs found:" >&2
  i=1
  printf '%s' "$CANDIDATES" | sed '/^$/d' | while IFS= read -r d; do
    echo "  $i) $d" >&2
    i=$((i + 1))
  done

  printf 'Select install number: ' >&2
  read ans || die "selection required"
  case "$ans" in
    ''|*[!0-9]*) die "invalid selection: $ans" ;;
  esac
  [ "$ans" -ge 1 ] && [ "$ans" -le "$count" ] || die "invalid selection: $ans"
  printf '%s' "$CANDIDATES" | sed '/^$/d' | sed -n "${ans}p"
}

download_file() {
  url="$1"
  out="$2"
  if command -v curl >/dev/null 2>&1; then
    curl -L --fail --progress-bar "$url" -o "$out"
  elif command -v wget >/dev/null 2>&1; then
    wget -O "$out" "$url"
  else
    die "missing curl or wget"
  fi
}

extract_zip() {
  zip="$1"
  out="$2"
  if command -v unzip >/dev/null 2>&1; then
    unzip -q "$zip" -d "$out"
  elif command -v bsdtar >/dev/null 2>&1; then
    bsdtar -xf "$zip" -C "$out"
  elif command -v 7z >/dev/null 2>&1; then
    7z x "$zip" "-o$out" >/dev/null
  else
    die "missing unzip, bsdtar, or 7z"
  fi
}

install_ffmpeg() {
  game_dir="$1"
  ffmpeg_exe="$game_dir/ffmpeg/bin/ffmpeg.exe"
  [ -f "$ffmpeg_exe" ] && return

  tmp="${TMPDIR:-/tmp}/dark-render-re-ffmpeg.$$"
  zip="$tmp/ffmpeg.zip"
  mkdir -p "$tmp"

  echo "Downloading 32-bit FFmpeg..."
  download_file "$FFMPEG_URL" "$zip"
  extract_zip "$zip" "$tmp/extract"

  found=$(find "$tmp/extract" -type f -name ffmpeg.exe | head -n 1)
  [ "$found" ] || die "ffmpeg.exe not found in archive"

  mkdir -p "$game_dir/ffmpeg/bin"
  cp "$found" "$ffmpeg_exe"
  rm -rf "$tmp"
}

install_proxy() {
  game_dir="$1"
  [ -f "$PROXY_DLL" ] || die "missing proxy DLL: $PROXY_DLL"

  if [ ! -f "$game_dir/VideoEnginePlugin.orig.vPlugin" ]; then
    [ -f "$game_dir/VideoEnginePlugin.vPlugin" ] || die "missing VideoEnginePlugin.vPlugin"
    mv "$game_dir/VideoEnginePlugin.vPlugin" "$game_dir/VideoEnginePlugin.orig.vPlugin"
    echo "Original plugin saved as VideoEnginePlugin.orig.vPlugin"
  fi

  cp "$PROXY_DLL" "$game_dir/VideoEnginePlugin.vPlugin"
}

need_cmd find
GAME_DIR=$(find_dark_dir)

echo "DARK root: $GAME_DIR"
install_ffmpeg "$GAME_DIR"
install_proxy "$GAME_DIR"

echo "Installed dark-render-re. Start game from Steam."
