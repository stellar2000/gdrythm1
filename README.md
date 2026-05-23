# GD Overlay Bridge — Geode Mod

Writes level state to `%TEMP%\gd_overlay_state.txt` so the Python overlay
can detect level start and death without memory hacks.

## Build via GitHub Actions (easiest — no local setup needed)

1. Create a GitHub account if you don't have one
2. Create a new repository and upload all these files
3. Go to Actions tab → the build runs automatically
4. When it finishes, click the run → download the `GDOverlayBridge` artifact
5. Inside is a `.geode` file — put it in your GD `geode/mods/` folder
6. Restart GD

## Build locally (if you have Geode CLI + Visual Studio)

```
geode sdk install
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --config RelWithDebInfo
```

The `.geode` file appears in `build/RelWithDebInfo/`.
Copy it to your GD `geode/mods/` folder and restart GD.

## Finding your geode/mods folder

Right-click GD in Steam → Manage → Browse local files → `geode/mods/`
