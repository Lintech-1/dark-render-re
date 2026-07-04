# RE Notes

Working point: 2026-07-04.

## Architecture

- `VideoEnginePlugin.vPlugin` is proxy DLL.
- Original plugin must be renamed to `VideoEnginePlugin.orig.vPlugin`.
- Proxy forwards most exported methods to original plugin.
- Proxy overrides video texture upload path only.

## Original Plugin Role

Original plugin still controls:

- audio;
- cutscene timer;
- subtitles;
- cancel/skip button;
- level loading flow.

## FFmpeg Role

Proxy starts:

```text
ffmpeg\bin\ffmpeg.exe
```

FFmpeg decodes WMV into BGRA rawvideo over stdout:

```text
-hide_banner -loglevel error -nostdin -threads 1 -filter_threads 1 -filter_complex_threads 1 -re -i <video> -an -vf scale=<w>:<h>:flags=lanczos,fps=25 -pix_fmt bgra -f rawvideo pipe:1
```

Why constrained:

- game/original DirectShow layer still owns audio;
- unconstrained FFmpeg can starve audio graph and cause short audio dropouts;
- low priority/single-thread/no per-frame logs fixed observed stutter.

## Texture Upload

Proxy uploads latest decoded frame through:

```text
?UpdateRect@VTextureObject@@QAE_NHHHHHHPBXHH@Z
```

from `vBase100.dll`.

## Device Reset / Alt+Enter

Proxy reads exported `vBase100.dll` flags:

```text
?m_bScreenNeedsReset@VVideo@@2_NA
?m_bRenderingIsSuspended@VVideo@@1_NA
```

If either flag active, proxy pauses texture upload until reset ends. This fixed Alt+Enter freezes during cutscenes.
