# Codex Workspace State

This directory stores the current working understanding of this checkout so later work can resume without re-scanning the repo.

Files:
- `state.json`: machine-readable snapshot of repo status, toolchain state, verified commands, and usermod inventory counts.
- `repo-scan.md`: project structure, setup path, build flow, and important config surfaces.
- `usermods-research.md`: detailed usermod inventory, integration model, dependency map, and doc gaps.

Verified commands in this workspace:
- `npm run build`
- `~/.platformio/penv/bin/pio run -e esp32dev_compat`

Important local notes:
- `pio` is available, but not on the default shell PATH in this environment.
- The repo builds from `wled00/` and uses `tools/cdata.js` to package the web UI.
- Local board-specific changes should go in `platformio_override.ini`, not `platformio.ini`.
