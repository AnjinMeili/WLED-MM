# Usermods Research

## Inventory Snapshot

- Total top-level usermod directories: `63`
- Integrated directly in `wled00/usermods_list.cpp`: `49`
- Usermods with a readme: `57`
- Usermods implementing config hooks (`addToConfig`, `readFromConfig`, or `appendConfigData`): `45`
- Legacy `usermod.cpp` / `.ino` / older standalone style modules: `11`

## Integration Model

The repo uses three different usermod patterns:

1. Integrated compile-time usermods
- These are included and registered in `wled00/usermods_list.cpp`.
- They are usually enabled with build flags such as `-D USERMOD_*`.
- These are the mainline modules that fit best into current WLED-MM builds.

2. Legacy or standalone modules
- These ship as `usermod.cpp`, `wled06_usermod.ino`, or project bundles.
- Many are not wired into `wled00/usermods_list.cpp`.
- They often need manual patching or their own example `platformio_override.ini`.

3. Example/stale-doc modules
- Some usermods are integrated in code, but their readmes still describe them as generic examples.
- `usermod_v2_weather`, `usermod_v2_games`, and `usermod_v2_animartrix` are the clearest cases.

## Cross-Usermod Dependencies

These relationships matter because they affect compile flags, runtime behavior, or feature completeness:

- `PWM_fan`
  - depends on `Temperature` or `SHT`
  - uses `usermods.lookup(USERMOD_ID_TEMPERATURE)` / `USERMOD_ID_SHT`
- `usermod_v2_auto_save`
  - optional integration with `usermod_v2_four_line_display_ALT`
  - uses `USERMOD_ID_FOUR_LINE_DISP`
- `usermod_v2_rotary_encoder_ui_ALT`
  - expects `usermod_v2_four_line_display_ALT` when display support is enabled
- `usermod_v2_games`
  - optionally consumes `MPU6050_IMU` data
  - uses `USERMOD_ID_IMU`
- `seven_segment_display_reloaded`
  - optionally uses `SN_Photoresistor`
- `artifx`
  - consumes AudioReactive UM data
- `usermod_v2_auto_playlist`
  - consumes AudioReactive UM data

## Usermods That Need Manual Libraries or Extra PlatformIO Work

These are the most setup-sensitive modules:

- `audioreactive`
  - build flag: `USERMOD_AUDIOREACTIVE`
  - extra lib: `arduinoFFT` from GitHub
  - best fit: classic ESP32 / ESP32-S3
  - not suitable for ESP8266
- `mpu6050_imu`
  - extra lib: `ElectronicCats/MPU6050 @ 0.6.0`
- `BME280_v2`
  - requires extra `lib_deps`
- `BH1750_v2`
  - requires extra `lib_deps`
- `sht`
  - requires extra `lib_deps`
- `Si7021_MQTT_HA`
  - requires extra `lib_deps`
- `sensors_to_mqtt`
  - explicitly documents both `build_flags` and `lib_deps`
- `ST7789_display`
  - requires `TFT_eSPI` enablement and manual display setup steps
- `TTGO-T-Display`
  - requires `TFT_eSPI` and board-specific library edits
- `Temperature`
  - expects OneWire / DallasTemperature support
- `DHT`
  - typically built from its own example `platformio_override.ini`
- `SN_Photoresistor`
  - ships an example `platformio_override.ini`
- `sd_card`
  - enabled via `WLED_USE_SD_MMC` or `WLED_USE_SD_SPI`, not a plain `USERMOD_*` flag

## MQTT-Sensitive Usermods

These either explicitly require MQTT or are tightly coupled to MQTT-facing behavior:

- `BME280_v2`
- `DHT`
- `Enclosure_with_OLED_temp_ESP07`
- `mqtt_switch_v2`
- `sensors_to_mqtt`
- `seven_segment_display`
- `seven_segment_display_reloaded`
- `Si7021_MQTT_HA`
- `smartnest`

## Documentation Gaps

### No readme in the usermod directory

- `Analog_Clock`
- `MY9291`
- `artifx`
- `buzzer`
- `pixelpusher`
- `rotary_encoder_change_effect`
- `usermod_v2_auto_playlist`

### Readmes that are stale or misleading

- `usermod_v2_weather`
  - still describes itself as an example, but the code is wired into `usermods_list.cpp`
- `usermod_v2_games`
  - same issue
- `usermod_v2_animartrix`
  - same issue

## Not Integrated In `wled00/usermods_list.cpp`

These exist in the repo but are not registered in the main current usermod registry:

- `Artemis_reciever`
- `Enclosure_with_OLED_temp_ESP07`
- `Fix_unreachable_netservices_v2`
- `JSON_IR_remote`
- `RelayBlinds`
- `TTGO-T-Display`
- `Wemos_D1_mini+Wemos32_mini_shield`
- `battery_keypad_controller`
- `mqtt_switch_v2`
- `photoresistor_sensor_mqtt_v1`
- `project_cars_shiftlight`
- `rotary_encoder_change_effect`
- `stairway_wipe_basic`
- `word-clock-matrix`

Interpretation:
- Some are legacy examples.
- Some are board bundles that need manual patching.
- Some are still useful references but not turnkey for current WLED-MM builds.

## Category Map

### Sensors, environment, power, time

- `ADS1115_v2`
  - integrated
  - runtime config hooks present
  - multi-channel ADC input module
- `BH1750_v2`
  - integrated
  - runtime config hooks present
  - ambient light sensor with configurable measurement timing/offsets
- `BME280_v2`
  - integrated
  - runtime config hooks present
  - environmental sensor, MQTT-aware
- `Battery`
  - integrated
  - runtime config hooks present
  - battery voltage, percentage, low-power indication, optional auto-off
- `DHT`
  - integrated
  - older config style
  - temperature/humidity sensor with MQTT/stat options
- `LDR_Dusk_Dawn_v2`
  - integrated
  - runtime config hooks present
  - simple light-threshold automation
- `mcu_temp`
  - integrated
  - runtime config hooks present
  - exposes MCU internal temperature
- `RTC`
  - integrated
  - runtime config hooks present
  - external RTC support
- `Si7021_MQTT_HA`
  - integrated
  - runtime config hooks present
  - Si7021 + MQTT/Home Assistant integration
- `SN_Photoresistor`
  - integrated
  - runtime config hooks present
  - photoresistor input with calibration fields
- `sensors_to_mqtt`
  - integrated
  - older config style
  - aggregator that publishes sensor data via MQTT
- `sht`
  - integrated
  - runtime config hooks present
  - SHT30/31/35/85 support
- `Temperature`
  - integrated
  - runtime config hooks present
  - Dallas/OneWire temperature usermod

### Displays, clocks, overlays, local UI

- `Analog_Clock`
  - integrated
  - no readme
  - clock overlay usermod
- `Cronixie`
  - integrated
  - runtime config hooks present
  - Nixie-style clock display
- `EleksTube_IPS`
  - integrated
  - runtime config hooks present
  - IPS tube-display board support
- `seven_segment_display`
  - integrated
  - runtime config hooks present
  - seven-segment clock/info overlay
- `seven_segment_display_reloaded`
  - integrated
  - runtime config hooks present
  - richer seven-segment variant, optionally uses `SN_Photoresistor`
- `ST7789_display`
  - integrated
  - runtime config hooks present
  - SPI TFT display path with `TFT_eSPI` setup
- `TTGO-T-Display`
  - not integrated
  - board-specific TFT bundle
- `usermod_rotary_brightness_color`
  - integrated
  - runtime config hooks present
  - rotary encoder adjustment helper
- `usermod_v2_four_line_display_ALT`
  - integrated
  - runtime config hooks present
  - alternate I2C display UI
- `usermod_v2_ping_pong_clock`
  - integrated
  - runtime config hooks present
  - matrix/segment clock project
- `usermod_v2_rotary_encoder_ui_ALT`
  - integrated
  - runtime config hooks present
  - alternate rotary UI tied to display stack
- `usermod_v2_word_clock`
  - integrated
  - runtime config hooks present
  - word-clock implementation
- `word-clock-matrix`
  - not integrated
  - legacy/project bundle with extra assets

### Motion, relays, switching, automation

- `Animated_Staircase`
  - integrated
  - runtime config hooks present
  - staircase motion/presence effect automation
- `multi_relay`
  - integrated
  - runtime config hooks present
  - multiple relay control
- `PIR_sensor_switch`
  - integrated
  - runtime config hooks present
  - PIR-triggered switch/standby logic
- `PWM_fan`
  - integrated
  - runtime config hooks present
  - temperature-driven PWM fan control
- `pwm_outputs`
  - integrated
  - runtime config hooks present
  - generic PWM output channels
- `RelayBlinds`
  - not integrated
  - relay blinds project bundle with presets/web asset
- `mqtt_switch_v2`
  - not integrated
  - MQTT-driven GPIO switch with explicit `MQTTSWITCHPINS` define
- `usermod_v2_auto_save`
  - integrated
  - runtime config hooks present
  - autosaves WLED state into a preset after a settle period
- `usermod_v2_auto_playlist`
  - integrated
  - runtime config hooks present
  - no readme; code suggests audio-reactive playlist automation
- `usermod_v2_klipper_percentage`
  - integrated
  - runtime config hooks present
  - polls Klipper print progress and displays it as a lighting overlay
- `usermod_v2_weather`
  - integrated
  - runtime config hooks present
  - docs stale; code adds a usermod-specific effect
- `wizlights`
  - integrated
  - runtime config hooks present
  - WiZ light integration

### Audio, motion input, visualization, protocol bridges

- `audioreactive`
  - integrated
  - runtime config hooks present
  - major feature usermod; provides audio analysis data to effects
- `artifx`
  - integrated
  - runtime config hooks present
  - no readme; adds custom `ARTIFX` effect and consumes AudioReactive data
- `boblight`
  - integrated
  - runtime config hooks present
  - Boblight ambient video integration
- `mpu6050_imu`
  - integrated
  - runtime config hooks present
  - IMU orientation/motion data provider
- `pixelpusher`
  - integrated
  - runtime config hooks present
  - no readme; implements Heroic Robotics PixelPusher discovery/data protocol
- `usermod_v2_animartrix`
  - integrated
  - limited config surface
  - docs stale; intended for animation/matrix rendering
- `usermod_v2_games`
  - integrated
  - runtime config hooks present
  - docs stale; optionally uses IMU
- `VL53L0X_gestures`
  - integrated
  - runtime config hooks present
  - gesture/distance input via ToF sensor

### Board-specific, hardware helper, or niche outputs

- `MY9291`
  - integrated
  - no readme
  - chipset-specific output helper
- `quinled-an-penta`
  - integrated
  - runtime config hooks present
  - QuinLED board support profile
- `RGB_ROTARY_ENCODER`
  - implemented as `rgb-rotary-encoder`
  - integrated
  - runtime config hooks present
  - encoder with RGB ring / board-specific behavior
- `buzzer`
  - integrated
  - no config hooks
  - no readme
  - simple buzzer output helper
- `sd_card`
  - integrated
  - runtime config hooks present
  - SD/MMC or SPI card support

### Legacy, standalone, or reference modules

- `Artemis_reciever`
  - not integrated
  - legacy `usermod.cpp`
- `battery_keypad_controller`
  - not integrated
  - old `wled06_usermod.ino`
- `Enclosure_with_OLED_temp_ESP07`
  - not integrated
  - board/project bundle
- `EXAMPLE_v2`
  - integrated as a reference pattern, but commented out by default
  - canonical template for new v2 usermods
- `Fix_unreachable_netservices_v2`
  - not integrated in current registry
  - modern header-based usermod but manual registration path only
- `JSON_IR_remote`
  - not integrated
  - JSON code tables and helper assets, not a current mainline usermod
- `photoresistor_sensor_mqtt_v1`
  - not integrated
  - older v1-style module
- `project_cars_shiftlight`
  - not integrated
  - old project-specific `.ino`
- `rotary_encoder_change_effect`
  - not integrated
  - old `.ino`
- `stairway_wipe_basic`
  - not integrated
  - old/basic staircase example
- `Wemos_D1_mini+Wemos32_mini_shield`
  - not integrated
  - board bundle for enclosure/display/sensor setups

## Highest-Value Usermods To Understand First

If future work focuses on mainstream WLED-MM behavior, these are the most strategically important modules:

- `audioreactive`
  - biggest feature surface and multiple downstream dependents
- `pixelpusher`
  - current branch name suggests this workstream matters
- `Battery`
  - mature runtime-configurable power management module
- `Temperature`, `SHT`, `PWM_fan`
  - good example of modular sensor + dependent actuator design
- `usermod_v2_four_line_display_ALT` + `usermod_v2_rotary_encoder_ui_ALT` + `usermod_v2_auto_save`
  - important local-UI stack with explicit coupling
- `mpu6050_imu` + `usermod_v2_games`
  - clear example of one usermod exporting data for another
- `artifx`
  - custom effect runtime that reaches into AudioReactive data

## Bottom Line

- The current repo treats usermods as a mixed ecosystem, not a single uniform plugin layer.
- The safest path for new work is to stay inside the `wled00/usermods_list.cpp` integrated set unless there is a specific reason to revive a legacy bundle.
- The best-documented and most operationally relevant usermods are the sensor stack, audio stack, display stack, and a handful of MoonModules-specific feature additions.
