# PixelPusher V2 Review

Date: 2026-03-28

## Scope

Reviewed `usermods/pixelpusher/usermod_pixelpusher.h` against the repo's current `Usermod` base class and the `EXAMPLE_v2` lifecycle/config conventions.

## Conclusion

`PixelPusherUsermod` is a v2-style usermod and is registered correctly, but it had two practical v2 compliance problems before patching:

1. Runtime enablement from Usermod Settings did not fully initialize the usermod.
2. `readFromConfig()` did not report missing config fields through `configComplete`.

## Files Reviewed

- `usermods/pixelpusher/usermod_pixelpusher.h`
- `usermods/EXAMPLE_v2/usermod_v2_example.h`
- `wled00/fcn_declare.h`
- `wled00/usermods_list.cpp`
- `wled00/const.h`
- `wled00/cfg.cpp`
- `wled00/set.cpp`
- `wled00/um_manager.cpp`

## Patch Applied

Updated `usermods/pixelpusher/usermod_pixelpusher.h` to:

- make `setup()` always mark `initDone = true`
- add `startUdp()` / `stopUdp()` helpers
- restart or stop UDP services when settings change at runtime
- fold each persisted config field into `configComplete`

## Validation

Rebuilt successfully:

- `~/.platformio/penv/bin/pio run -e esp32dev_compat`

Result:

- success
- RAM: `79860 / 327680` (`24.4%`)
- Flash: `1471693 / 1572864` (`93.6%`)

## Residual Notes

- The usermod still keeps local `_name` / `_enabled` statics, while newer WLEDMM style increasingly prefers the base-class attributes directly.
- That remaining inconsistency is stylistic, not a current functional compliance issue.
