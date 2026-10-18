# Traka app icon

Original simplified front-facing mixing-console artwork generated for Traka using the built-in image-generation tool. The configured GetToken models `gpt-image-2.5-sunburst` and `gpt-image-2` returned HTTP 503 (no available account), so generation used the built-in provider. No upstream branding or artwork is reused.

- `traka-generated.png`: original 1254×1254 output, retained unchanged.
- `traka-master.png`: 1024×1024 RGB production master, converted/resampled for packaging.
- `prompt.txt`: visual specification used for generation.
- `size-preview.png`: visual inspection at 16, 32, 128, 256 and 512 pixels.
- `../AppIcon.icon`: Icon Composer document with the production image, continuous full-bleed background and no baked tile/frame. macOS supplies the outer silhouette.

The CMake target includes the `.icon` source in its Resources. `scripts/compile_icon.py` compiles it with Xcode actool and merges `CFBundleIconName` / `CFBundleIconFile`. It emits `AppIcon.icns` for older systems and `Assets.car` for modern macOS rendering. The source artwork is included with Traka's open-source assets under the project's AGPL-3.0-or-later license; no claim of exclusive rights in AI output is made.

The user selected the simplified console on 2026-10-18 after viewing it in the Dock. The full-bleed light background holds three thick charcoal tracks and three smooth caps (white, orange, white). The metal grain, grip grooves, deep recesses, divider lines and glowing strips of 0.1.1 have been removed. All tracks are front-facing and parallel; only this selected artwork is included.
