# dark-render-re

Reverse-engineered video renderer proxy for **DARK (2013)**.

Project fixes WMV cutscene playback on Proton/Linux. Original game video layer may freeze after a few seconds while audio, subtitles, cancel button, and loading flow still work. This proxy keeps original game timing/audio/UI, but replaces only video texture upload with frames decoded by 32-bit Windows FFmpeg.

Game links:

- Steam: <https://store.steampowered.com/app/225360/DARK/>
- SteamDB: <https://steamdb.info/app/225360/>

## What It Does

- Replaces `VideoEnginePlugin.vPlugin` with proxy DLL.
- Loads original plugin as `VideoEnginePlugin.orig.vPlugin`.
- Lets original plugin keep audio, subtitles, timing, skip/cancel button, and level transitions.
- Starts `ffmpeg\bin\ffmpeg.exe` to decode current `.wmv` into BGRA rawvideo.
- Uploads decoded frames into game texture through `VTextureObject::UpdateRect`.
- Handles Alt+Enter/device reset using exported `vBase100.dll` reset flags.
- Runs FFmpeg with low priority/single-thread mode to avoid audio stutter.

## Built With

This project uses [FFmpeg](https://ffmpeg.org/) for WMV decoding, with a tested 32-bit Windows build from [FFmpeg-Builds-Win32](https://github.com/defisym/FFmpeg-Builds-Win32). Runtime testing is done on [Proton-GE](https://github.com/GloriousEggroll/proton-ge-custom), using Wine/Proton compatibility layers from the [Wine](https://www.winehq.org/) ecosystem.

The proxy is built with [LLVM/Clang](https://clang.llvm.org/) and Wine i386 import libraries. Rendering integration targets DARK's original `VideoEnginePlugin.vPlugin`, `VisionEnginePlugin.vPlugin`, and `vBase100.dll` interfaces; game files are not redistributed by this project.

## Current Status

Recommended runtime: **Proton-GE**.

This fix was developed and tested on Proton-GE. Other Proton builds may work, but are not the tested target.

Working on Proton-GE with Russian localized DARK install.

Known design:

- Audio still comes from original DirectShow/Wine path.
- Video comes from FFmpeg texture hook.
- FFmpeg must be 32-bit Windows build, because game/plugin process is 32-bit.

## One-Line Install

After publishing this repo, Steam Deck/Linux user can run one command:

```sh
git clone https://github.com/Lintech-1/dark-render-re.git && sh dark-render-re/install.sh
```

Installer does this automatically:

- finds DARK Steam install by checking `DarkApp.exe`, `Localization_Common/`, and `VisionEnginePlugin.vPlugin`;
- asks which install to use if multiple copies are found;
- downloads tested 32-bit Windows FFmpeg build;
- puts FFmpeg at `Dark/ffmpeg/bin/ffmpeg.exe`;
- renames original plugin to `VideoEnginePlugin.orig.vPlugin`;
- installs proxy as `VideoEnginePlugin.vPlugin`.

If game is in custom path:

```sh
DARK_GAME_DIR="/path/to/SteamLibrary/steamapps/common/Dark" sh dark-render-re/install.sh
```

## Manual Install

1. Open game folder:

   ```sh
   cd "/path/to/SteamLibrary/steamapps/common/Dark"
   ```

2. Put 32-bit Windows FFmpeg here:

   ```text
   Dark/ffmpeg/bin/ffmpeg.exe
   ```

   Tested layout:

   ```text
   Dark/
     ffmpeg/
       bin/
         ffmpeg.exe
   ```

3. Backup original plugin once:

   ```sh
   mv VideoEnginePlugin.vPlugin VideoEnginePlugin.orig.vPlugin
   ```

4. Copy proxy plugin:

   ```sh
   cp dark-render-re/dist/VideoEnginePlugin.vPlugin VideoEnginePlugin.vPlugin
   ```

5. Run game.

## Installer Notes

Run installer from anywhere:

```sh
sh dark-render-re/install.sh
```

Script expects:

- `dark-render-re/dist/VideoEnginePlugin.vPlugin` exists;
- original plugin is either `VideoEnginePlugin.vPlugin` or already renamed to `VideoEnginePlugin.orig.vPlugin`.

Game root detection requires these files:

- `DarkApp.exe`
- `Localization_Common/`
- `VisionEnginePlugin.vPlugin`

Override FFmpeg download URL:

```sh
FFMPEG_URL="https://example.com/ffmpeg-win32.zip" sh dark-render-re/install.sh
```

## Build

Linux build deps:

- `clang`
- `lld`
- Wine i386 import libs:
  - `/usr/lib/wine/i386-windows/libkernel32.a`
  - `/usr/lib/wine/i386-windows/libuser32.a`

Build:

```sh
cd dark-render-re
sh build_proxy.sh
```

Output:

```text
dist/VideoEnginePlugin.vPlugin
```

Debug log build:

```sh
LOGS=1 sh build_proxy.sh
```

## Files

- `src/VideoEnginePlugin_proxy.c` - proxy implementation.
- `src/VideoEnginePlugin_proxy.def` - exported original plugin symbols.
- `dist/VideoEnginePlugin.vPlugin` - ready-to-use 32-bit proxy DLL.
- `install.sh` - install/update helper for game root.
- `build_proxy.sh` - local build script.

## Runtime Logs

Default release build does not write runtime logs.

If built with `LOGS=1`, game root may contain:

- `dark_video_proxy.log`
- `dark_video_ffmpeg.log`

These are debug logs. Safe to delete.

## Credits

DARK belongs to its owners. FFmpeg belongs to FFmpeg project/build maintainers. This repo contains only proxy code and ready proxy binary, not game files.

## License

MIT. See `LICENSE`.
