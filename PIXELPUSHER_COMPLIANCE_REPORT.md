# PixelPusher Protocol Compliance Report

## 1. Ground Truth Specification Summary

Canonical baseline: Java implementation in PixelPusher-java. Where other implementations differ, Java is treated as ground truth and discrepancies are called out.

### Discovery Beacon

Discovery transport:
- Discovery UDP port: 7331 (PixelPusher-java/src/com/heroicrobot/dropbit/registry/DeviceRegistry.java:41, PixelPusher-java/src/com/heroicrobot/dropbit/registry/DeviceRegistry.java:471)
- Listener bound to 0.0.0.0 with broadcast enabled (PixelPusher-java/src/com/heroicrobot/dropbit/registry/DeviceRegistry.java:468-471)

Discovery packet layout (full packet = 24-byte header + device-specific payload):

| Offset | Size | Type | Name | Description | Source |
|---:|---:|---|---|---|---|
| 0 | 6 | uint8[6] | mac_address | Device MAC | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:14,69 |
| 6 | 4 | uint8[4] | ip_address | Device IP | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:15,71-72 |
| 10 | 1 | uint8 | device_type | Device type enum | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:16,76-77 |
| 11 | 1 | uint8 | protocol_version | Device protocol version | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:17,78-79 |
| 12 | 2 | uint16 LE | vendor_id | Vendor id | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:18,80-81 |
| 14 | 2 | uint16 LE | product_id | Product id | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:19,82-83 |
| 16 | 2 | uint16 LE | hw_revision | HW revision | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:20,84-85 |
| 18 | 2 | uint16 LE | sw_revision | SW revision | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:21,86-87 |
| 20 | 4 | uint32 LE | link_speed | Link speed bps | PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:22,88-89 |
| 24 | 1 | uint8 | strips_attached | Number of strips | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:18,298 |
| 25 | 1 | uint8 | max_strips_per_packet | Max strips per pixel packet | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:19,300 |
| 26 | 2 | uint16 LE | pixels_per_strip | Pixels per strip | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:20,299 |
| 28 | 4 | uint32 LE | update_period | Microseconds per update | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:21,302-303 |
| 32 | 4 | uint32 LE | power_total | PWM power total | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:22,304 |
| 36 | 4 | uint32 LE | delta_sequence | Dropped sequence delta | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:23-24,305 |
| 40 | 4 | int32 LE | controller_ordinal | Controller ordinal | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:25,306 |
| 44 | 4 | int32 LE | group_ordinal | Group ordinal | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:26,307 |
| 48 | 2 | uint16 LE | artnet_universe | ArtNet universe | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:27,309 |
| 50 | 2 | uint16 LE | artnet_channel | ArtNet channel | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:28,310 |
| 52 | 2 | uint16 LE | my_port | UDP pixel-data port | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:29,314-315 |
| 56 | max(8,strips) | uint8[] | strip_flags | Per-strip flags | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:323-325,328 |
| 58 + max(8,strips) | 4 | uint32 LE | pusher_flags | Device capability flags | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:341,347 |
| 62 + max(8,strips) | 4 | uint32 LE | segments | Segments per strip | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:342,348 |
| 66 + max(8,strips) | 4 | uint32 LE | power_domain | Power domain id | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:343,349 |

Endian basis for multi-byte fields: little-endian helpers (PixelPusher-java/src/com/heroicrobot/dropbit/common/ByteUtils.java:4-16,34-42).

### Pixel Data Packets

| Offset | Size | Type | Name | Description | Source |
|---:|---:|---|---|---|---|
| 0 | 4 | uint32 LE | sequence_number | Monotonic per pusher packet number | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:196-199,223,300 |
| 4 | 1 | uint8 | strip_number | Strip index for following payload | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:264 |
| 5 | 3*pixels | byte[] | strip_pixel_data | RGB bytes for that strip | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:255-260,292-295 |
| repeat | | | | Repeat strip_number + strip_payload for up to max_strips_per_packet strips | PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:241-245,298-307 |

- Pixel receive/send port is device my_port from beacon (default fallback 9798; getPort fallback path returns 9897 for non-positive) (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:66,73-78,314-318; PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:41,224-226).
- Typical max payload sizing by sender: 4 + ((1 + 3*pixels_per_strip) * max_strips_per_packet), defaulting to 1460-byte practical MTU class (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:21,50).

### Pixel Encoding

- Standard order is RGB, 3 bytes/pixel in serialized strip payload (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/Strip.java:358-360,418-420).
- RGBOW and monochrome variants are represented via strip flags and alternate serialization branches (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/Strip.java:338-353,376-412).

### Timing & Rate Control

- update_period is advertised in beacon (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:302-303).
- delta_sequence in beacon communicates dropped-packet estimate, used for autothrottle adjustments (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:305; PixelPusher-java/src/com/heroicrobot/dropbit/registry/DeviceRegistry.java:616-619).
- Sender pacing derived from update_period plus extra/autothrottle delay (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:183-195,320).

### Flags & Capabilities

Strip flags (Java):
- SFLAG_RGBOW (bit0), SFLAG_WIDEPIXELS (bit1), SFLAG_LOGARITHMIC (bit2), SFLAG_MOTION (bit3), SFLAG_NOTIDEMPOTENT (bit4), SFLAG_BRIGHTNESS (bit5), SFLAG_MONOCHROME (bit6) (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:50-56).

Pusher flags (Java):
- PFLAG_PROTECTED (bit0), PFLAG_FIXEDSIZE (bit1), PFLAG_GLOBALBRIGHTNESS (bit2), PFLAG_STRIPBRIGHTNESS (bit3), PFLAG_MONOCHROME_NOT_PACKED (bit4) (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:58-63).

### Command / Control Packets

Command packets are UDP payloads prefixed by 4-byte sequence + 16-byte magic + command body:
- Magic bytes: 0x40 09 2d a6 15 a5 dd e5 6a 9d 4d 5a cf 09 af 50 (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PusherCommand.java:18-20).
- Command IDs: RESET(0x01), GLOBALBRIGHTNESS_SET(0x02), WIFI_CONFIGURE(0x03), LED_CONFIGURE(0x04), STRIPBRIGHTNESS_SET(0x05) (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PusherCommand.java:22-26).
- UDP composition in sender: sequence, then command bytes (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:211-218,224-227).

### Reliability & Sequencing

- Transport is UDP/fire-and-forget; no ACK flow is defined in Java sender code (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:305-312).
- Reliability signal is inferred from discovery delta_sequence feedback (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:305; PixelPusher-java/src/com/heroicrobot/dropbit/registry/DeviceRegistry.java:616-619).

### Cross-Implementation Discrepancies (resolved in favor of Java)

1. Discovery payload offsets differ:
- Java reads strip_flags from packet remainder offset 32 and pusher_flags from 34+stripFlagSize (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:328,347).
- Node parses stripFlags at full offset 54 and pusherFlags at 62 (node-pixelpusher/pixelpusher.js:90,95).
- C server base/ext layout places strip_flags immediately after my_port and has ext.padding before pusher_flags (pixelpusher-server/lib/universal-discovery-protocol.h:48-57).

2. Pixel listen port defaults vary:
- Java PixelPusher field default is 9798 with fallback getter branch to 9897 if non-positive (PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:66,73-78).
- Python hardcodes PIXEL_PORT=9897 but uses my_port from discovery for actual destination (pixelpie/device/discovery.py:10; pixelpie/device/pusher.py:78).
- C server uses listen port 5078 (pixelpusher-server/lib/pp-server.cc:46,255-261).

3. Node implementation has known logic bugs not present in canonical Java:
- Strip index range check incorrectly compares against pixelsPerStrip, not number of strips (node-pixelpusher/pixelpusher.js:157-159).

## 2. Compliance Results

### Discovery Port — PASS
- Spec: Discovery beacons on UDP 7331.
- Found: Beacon sent to 255.255.255.255:7331.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:21,235-237
- Delta: None.
- Severity: INFO

### Discovery Packet Layout — PASS
- Spec: Java canonical byte offsets include strip_flags at full packet offset 56 and pusher_flags at 58+stripFlagSize (derived from Java packet remainder offsets 32 and 34+stripFlagSize).
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:328,347
- Found: App now includes a 2-byte reserved field after my_port so strip_flags and extension fields align to Java-canonical positions.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:67-80,203-223
- Delta: None for base/ext layout alignment.
- Severity: INFO

### Discovery Metadata Fields — PASS
- Spec: Header fields and PixelPusher metadata fields (MAC/IP/type/proto/vendor/product/hw/sw/link, strips, max strips per packet, pixels/strip, update period, power, delta, controller/group, artnet, my_port).
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/discovery/DeviceHeader.java:14-23,69-90; PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:298-315
- Found: App constructs and populates all corresponding fields.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:55-79,171-201
- Delta: Field set is complete.
- Severity: INFO

### Packet Construction (Outbound Pixel Data) — NOT IMPLEMENTED
- Spec: Controller/sender constructs UDP data packets: sequence + repeated [strip index + strip payload], increments sequence on each sent packet.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:196-199,241-245,264,292-301
- Found: Application is a PixelPusher receiver/server; it does not send outbound pixel data packets to other pushers.
  - App: no transmitter path in usermods/pixelpusher/usermod_pixelpusher.h
- Delta: Sender role is absent by design.
- Severity: INFO

### Sequence Number Handling (Receiver Side) — PARTIAL
- Spec: Sequence is uint32 LE at bytes 0-3; loss inferred from gaps and reported via delta_sequence.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:196-199; PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:305
- Found: App parses LE sequence and accumulates missing-count into deltaSequence; publishes in beacon then resets.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:251-257,232-233
- Delta: No handling for out-of-order or wraparound edge cases; comparison only checks seq > last+1.
- Severity: MINOR

### Strip Index Encoding & Validation — PASS
- Spec: Strip index is one byte per segment and should refer to valid strip range (sender emits strip numbers from created strips).
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:264; PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:94-95
- Found: App now validates stripIndex against configured strip count before applying payload.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:285-292
- Delta: None for strip index range handling.
- Severity: INFO

### Maximum Packet / Payload Size — PASS
- Spec: Practical MTU-class packet sizing around 1460 is standard in sender logic.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:21,50
- Found: App caps receive buffer and rejects oversized datagrams >1460; beacon computes max_strips_per_packet from 1460-4 budget.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:24,186-189,339-344
- Delta: None for intended MTU-class operation.
- Severity: INFO

### Pixel Encoding — PASS
- Spec: RGB byte order, 3 bytes/pixel in standard mode.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/Strip.java:358-360,418-420
- Found: App consumes 3-byte RGB and writes realtime pixel as R,G,B.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:267,292
- Delta: None in standard RGB mode.
- Severity: INFO

### Non-RGB Encodings (RGBOW/Monochrome/Brightness modes) — NOT IMPLEMENTED
- Spec: Strip/pusher flags define alternate payload semantics and capabilities.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:50-56,58-63; PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/Strip.java:338-353,406-412
- Found: App always advertises strip_flags = 0 and pusher_flags = 0, and only parses RGB payload.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:205-216,267,292
- Delta: Extended color/capability modes are not supported.
- Severity: MAJOR

### Timing & Rate Control — PARTIAL
- Spec: update_period and delta_sequence communicate pacing/loss; registry can autothrottle based on delta_sequence.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:302-305; PixelPusher-java/src/com/heroicrobot/dropbit/registry/DeviceRegistry.java:616-619
- Found: App updates beacon update_period from measured processing time and publishes delta_sequence; resets delta each beacon.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:231-233,301-304
- Delta: No command/control-based dynamic throttling behavior is implemented beyond passive reporting.
- Severity: MINOR

### Flags & Capabilities — FAIL
- Spec: Java defines additional strip and pusher capability bits beyond RGBOW/FIXEDSIZE.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:50-56,58-63
- Found: App defines only a subset of bits and does not implement behavior tied to most capability flags.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:35-45,205-216
- Delta: Missing SFLAG_BRIGHTNESS, SFLAG_MONOCHROME, PFLAG_GLOBALBRIGHTNESS, PFLAG_STRIPBRIGHTNESS, PFLAG_MONOCHROME_NOT_PACKED support.
- Severity: MAJOR

### Command / Control Packets — NOT IMPLEMENTED
- Spec: Command packets (magic + command id/payload) support reset, global/strip brightness, wifi, led config.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PusherCommand.java:18-26,135-207
- Found: App detects command magic and intentionally ignores command packets.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:240-244,261-263
- Delta: No command semantics are executed.
- Severity: MAJOR

### Sequencing & Reliability Model — PASS
- Spec: Fire-and-forget UDP; no ACK protocol; loss signaled indirectly via delta_sequence beacon metrics.
  - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/CardThread.java:305-312; PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:305
- Found: App is UDP receiver/server, tracks missing sequences and publishes delta in discovery beacons.
  - App: usermods/pixelpusher/usermod_pixelpusher.h:251-257,232-233
- Delta: Model is consistent.
- Severity: INFO

## 3. Summary Table

| Protocol Element | Status | Severity |
|---|---|---|
| Discovery port | PASS | — |
| Discovery packet layout | PASS | — |
| Discovery metadata field coverage | PASS | — |
| Outbound pixel packet construction | NOT IMPLEMENTED | INFO |
| Sequence number handling (receiver) | PARTIAL | MINOR |
| Strip index validation | PASS | — |
| Packet size limits | PASS | — |
| RGB encoding (3 BPP) | PASS | — |
| Extended color/capability encodings | NOT IMPLEMENTED | MAJOR |
| Timing/rate-control feedback | PARTIAL | MINOR |
| Flags/capabilities handling | FAIL | MAJOR |
| Command/control packet handling | NOT IMPLEMENTED | MAJOR |
| Reliability model (UDP fire-and-forget) | PASS | — |

## 4. Blocking Issues

None currently identified.

## 5. Non-Blocking Issues

1. Extended capability flags and corresponding behaviors are unimplemented.
   - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:50-56,58-63
   - App: usermods/pixelpusher/usermod_pixelpusher.h:35-45,205-216

2. Command packets are detected but ignored.
   - Spec: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PusherCommand.java:135-207
   - App: usermods/pixelpusher/usermod_pixelpusher.h:261-263

3. Sequence gap logic does not account for reordering/wrap edge cases.
   - App: usermods/pixelpusher/usermod_pixelpusher.h:255-258

## 6. Recommendations

1. Implement at least core command handling (RESET, GLOBALBRIGHTNESS_SET, STRIPBRIGHTNESS_SET).
   - Parse magic + command id and execute command semantics.
   - Spec source: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PusherCommand.java:18-26,137-151
   - App location: usermods/pixelpusher/usermod_pixelpusher.h:240-244,261-263

2. Expand flags support to Java-defined capability bits or advertise unsupported bits as absent with strict behavior.
   - Add missing constant definitions and implementation branches.
   - Spec source: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:50-56,58-63
   - App location: usermods/pixelpusher/usermod_pixelpusher.h:35-45,205-216

3. Harden sequence tracking logic for wraparound/out-of-order packet handling.
   - Spec source: PixelPusher-java/src/com/heroicrobot/dropbit/devices/pixelpusher/PixelPusher.java:305
   - App location: usermods/pixelpusher/usermod_pixelpusher.h:251-258
