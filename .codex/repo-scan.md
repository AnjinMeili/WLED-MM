# Repo Scan

## Identity

- Repo: MoonModules `WLED-MM` fork checked out locally as `WLED-PP`
- Current branch: `pixelpusher`
- Worktree status at scan time: clean

## Verified Setup

- `node`, `npm`, and `python3` are installed and usable.
- `pio` is not on the default PATH, but an existing PlatformIO install works at:
  - `~/.platformio/penv/bin/pio`
- Verified web asset build:
  - `npm run build`
  - Result: success, UI already built
- Verified firmware build:
  - `~/.platformio/penv/bin/pio run -e esp32dev_compat`
  - Result: success
  - Memory snapshot from that build:
    - RAM: 24.4% (`79860 / 327680`)
    - Flash: 93.6% (`1471693 / 1572864`)

## Build Model

- Primary build system: PlatformIO
- Web UI packaging: Node script `tools/cdata.js`
- Source root: `wled00/`
- Web/data root: `wled00/data/`
- Local overrides: `platformio_override.ini`
- Main configuration file: `platformio.ini`

## Important Config Surfaces

### `platformio.ini`

- `src_dir = ./wled00`
- `data_dir = ./wled00/data`
- `extra_configs = platformio_override.ini`
- The default env list is MoonModules-specific and much broader than upstream.
- The active verified env in this scan was `esp32dev_compat`.

### `package.json`

- `npm run build` runs `node tools/cdata.js`
- `npm run test` runs Node's built-in test runner
- `npm run dev` watches tool/data sources and rebuilds UI assets
- Engine requirement: `node >=20`

### Workspace/Editor Hints

- `.envrc` uses `layout python-venv python3`
- `.devcontainer/devcontainer.json`
  - installs the PlatformIO IDE extension
  - runs `npm install` on container creation
- `.gitpod.yml`
  - uses `pip3 install -U platformio && platformio run`

## Repo Layout

- `wled00/`: main firmware source tree
- `usermods/`: optional modules and examples
- `tools/`: web/UI build scripts and related tooling
- `pio-scripts/`: PlatformIO pre/post build hooks
- `boards/`: board definitions/config support
- `lib/`: vendored libraries
- `test/`: test scaffolding

## Initial Engineering Observations

- This checkout already had populated `.pio/`, `node_modules/`, and `~/.platformio/` state, so it was previously built on this machine.
- The shell environment is the main setup gap, not the repo contents:
  - PlatformIO is installed but not on PATH.
- `platformio.ini` is large and highly customized for MoonModules board/env variants.
- `usermods/` is mixed:
  - first-class integrated v2/v2-style usermods
  - older standalone `usermod.cpp` / `wled06_usermod.ino` examples
  - board- or project-specific bundles
  - doc quality varies widely

## Build Warnings Worth Remembering

- The verified `esp32dev_compat` build succeeded with non-fatal warnings from:
  - `wled00/FX_2Dfcn.cpp` unused `sameColor`
  - time library `DateStrings.cpp` pointer-size cast warnings
  - framework `esp32-hal-spi.c` incompatible pointer-type warnings
- None of these blocked the build.

## Practical Next-Step Commands

- Rebuild web UI:
  - `npm run build`
- Rebuild verified firmware env:
  - `~/.platformio/penv/bin/pio run -e esp32dev_compat`
- Inspect a different board/env:
  - `~/.platformio/penv/bin/pio run -e <env-name>`
- Keep local board or usermod changes isolated:
  - create/update `platformio_override.ini`
