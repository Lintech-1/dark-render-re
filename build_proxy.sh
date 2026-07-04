#!/bin/sh
set -eu

mkdir -p dist

CFLAGS=""
if [ "${LOGS:-0}" = "1" ]; then
  CFLAGS="-DENABLE_DARK_RENDER_RE_LOGS=1"
fi

clang -target i686-pc-windows-msvc \
  $CFLAGS \
  -fuse-ld=lld-link \
  -nostdlib \
  -Wl,/dll \
  -Wl,/entry:DllMain \
  -Wl,/subsystem:windows \
  -Wl,/safeseh:no \
  -Wl,/def:src/VideoEnginePlugin_proxy.def \
  -o dist/VideoEnginePlugin.vPlugin \
  src/VideoEnginePlugin_proxy.c \
  /usr/lib/wine/i386-windows/libkernel32.a \
  /usr/lib/wine/i386-windows/libuser32.a
