#pragma once

#include "wled.h"
#include <WiFiUdp.h>

/*
 * PixelPusher protocol usermod for WLED
 *
 * Implements the Heroic Robotics PixelPusher discovery and pixel data protocol.
 * Advertises this WLED device as a PixelPusher on the network, allowing
 * PixelPusher-compatible software (Processing, L.E.D. Lab, etc.) to send
 * pixel data directly.
 *
 * Also advertises optional ArtNet/sACN universe mapping in the discovery
 * beacon so that PixelPusher-aware controllers can bridge to ArtNet/sACN.
 *
 * Protocol reference: https://github.com/hzeller/rpi-matrix-pixelpusher
 */

// PixelPusher protocol constants
#define PP_DISCOVERY_PORT      7331
#define PP_LISTEN_PORT         5078
#define PP_BEACON_INTERVAL_MS  1000
#define PP_MAX_PACKET_SIZE     1460
#define PP_SOFTWARE_REVISION   122
#define PP_PROTOCOL_VERSION    1
#define PP_VENDOR_ID           3
#define PP_PRODUCT_ID          0

// Device types
#define PP_DEVICE_ETHERDREAM   0
#define PP_DEVICE_LUMIABRIDGE  1
#define PP_DEVICE_PIXELPUSHER  2

// Strip flags
#define PP_SFLAG_RGBOW         (1 << 0)
#define PP_SFLAG_WIDEPIXELS    (1 << 1)
#define PP_SFLAG_LOGARITHMIC   (1 << 2)
#define PP_SFLAG_MOTION        (1 << 3)
#define PP_SFLAG_NOTIDEMPOTENT (1 << 4)

// Pusher flags
#define PP_PFLAG_PROTECTED     (1 << 0)
#define PP_PFLAG_FIXEDSIZE     (1 << 1)

// Command magic (16 bytes after sequence number)
static const uint8_t PP_COMMAND_MAGIC[16] = {
  0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5,
  0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50
};

// All structs are packed for wire protocol compatibility
#pragma pack(push, 1)

struct PPDiscoveryHeader {
  uint8_t  mac_address[6];
  uint8_t  ip_address[4];
  uint8_t  device_type;
  uint8_t  protocol_version;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t hw_revision;
  uint16_t sw_revision;
  uint32_t link_speed;
};

struct PPPixelPusherBase {
  uint8_t  strips_attached;
  uint8_t  max_strips_per_packet;
  uint16_t pixels_per_strip;
  uint32_t update_period;       // microseconds
  uint32_t power_total;
  uint32_t delta_sequence;      // dropped packets since last beacon
  int32_t  controller_ordinal;
  int32_t  group_ordinal;
  uint16_t artnet_universe;
  uint16_t artnet_channel;
  uint16_t my_port;
  uint16_t reserved;            // aligns strip_flags to canonical offset
  // strip_flags follow (variable length, one byte per strip, minimum 8)
};

struct PPPixelPusherExt {
  uint16_t padding;
  uint32_t pusher_flags;
  uint32_t segments;
  uint32_t power_domain;
  uint8_t  last_driven_ip[4];
  uint16_t last_driven_port;
};

#pragma pack(pop)


class PixelPusherUsermod : public Usermod {
  private:
    // Network
    WiFiUDP beaconUdp;
    WiFiUDP dataUdp;
    bool    udpStarted = false;

    // Configuration
    uint8_t  numStrips        = 1;
    uint16_t pixelsPerStrip   = 0;  // 0 = auto from strip length
    int32_t  controllerOrdinal = 0;
    int32_t  groupOrdinal     = 0;
    int16_t  artnetUniverse   = -1; // -1 = disabled
    int16_t  artnetChannel    = -1; // -1 = disabled

    // State
    unsigned long lastBeaconTime = 0;
    uint32_t lastSequence     = 0;
    uint32_t deltaSequence    = 0;
    uint32_t updatePeriodUs   = 16666; // ~60fps default
    unsigned long lastPacketTime = 0;
    uint32_t packetsReceived  = 0;

    // Discovery packet buffer
    uint8_t discoveryPacket[128]; // large enough for all sections
    size_t  discoveryPacketLen = 0;

    // Pixel data receive buffer
    uint8_t rxBuffer[PP_MAX_PACKET_SIZE];

    static const char _name[];
    static const char _enabled[];

    void stopUdp() {
      if (udpStarted) {
        dataUdp.stop();
        beaconUdp.stop();
        udpStarted = false;
      }
    }

    void startUdp() {
      stopUdp();

      if (dataUdp.begin(PP_LISTEN_PORT)) {
        udpStarted = true;
        buildDiscoveryPacket();
        DEBUG_PRINTLN(F("PixelPusher: listening on port 5078"));
      } else {
        DEBUG_PRINTLN(F("PixelPusher: failed to start UDP listener"));
      }
    }

    uint16_t getEffectivePixelsPerStrip() {
      if (pixelsPerStrip > 0) return pixelsPerStrip;
      uint16_t totalLeds = strip.getLengthTotal();
      if (numStrips == 0) return totalLeds;
      return totalLeds / numStrips;
    }

    void buildDiscoveryPacket() {
      memset(discoveryPacket, 0, sizeof(discoveryPacket));
      size_t offset = 0;

      // Section A: Discovery header (24 bytes)
      PPDiscoveryHeader *hdr = (PPDiscoveryHeader *)&discoveryPacket[offset];

      // MAC address
      Network.localMAC(hdr->mac_address);

      // IP address
      IPAddress ip = Network.localIP();
      hdr->ip_address[0] = ip[0];
      hdr->ip_address[1] = ip[1];
      hdr->ip_address[2] = ip[2];
      hdr->ip_address[3] = ip[3];

      hdr->device_type      = PP_DEVICE_PIXELPUSHER;
      hdr->protocol_version = PP_PROTOCOL_VERSION;
      hdr->vendor_id        = PP_VENDOR_ID;
      hdr->product_id       = PP_PRODUCT_ID;
      hdr->hw_revision      = 0;
      hdr->sw_revision      = PP_SOFTWARE_REVISION;
      hdr->link_speed       = 10000000; // 10 Mbps

      offset += sizeof(PPDiscoveryHeader);

      // Section B: PixelPusher base
      PPPixelPusherBase *base = (PPPixelPusherBase *)&discoveryPacket[offset];

      uint16_t pps = getEffectivePixelsPerStrip();
      uint16_t stripDataLen = 1 + 3 * pps; // 1 byte strip index + RGB
      uint16_t usableSize = PP_MAX_PACKET_SIZE - 4; // minus sequence number
      uint8_t  maxStripsPerPkt = usableSize / stripDataLen;
      if (maxStripsPerPkt > numStrips) maxStripsPerPkt = numStrips;
      if (maxStripsPerPkt == 0) maxStripsPerPkt = 1;

      base->strips_attached      = numStrips;
      base->max_strips_per_packet = maxStripsPerPkt;
      base->pixels_per_strip     = pps;
      base->update_period        = updatePeriodUs;
      base->power_total          = 1;
      base->delta_sequence       = deltaSequence;
      base->controller_ordinal   = controllerOrdinal;
      base->group_ordinal        = groupOrdinal;
      base->artnet_universe      = (artnetUniverse >= 0) ? (uint16_t)artnetUniverse : 0xFFFF;
      base->artnet_channel       = (artnetChannel >= 0) ? (uint16_t)artnetChannel : 0xFFFF;
      base->my_port              = PP_LISTEN_PORT;
      base->reserved             = 0;

      offset += sizeof(PPPixelPusherBase);

      // Strip flags (one per strip, minimum 8 bytes)
      uint8_t flagBytes = (numStrips > 8) ? numStrips : 8;
      for (uint8_t i = 0; i < flagBytes; i++) {
        discoveryPacket[offset + i] = 0; // no special flags
      }
      offset += flagBytes;

      // Section C: PixelPusher extension
      PPPixelPusherExt *ext = (PPPixelPusherExt *)&discoveryPacket[offset];
      ext->padding          = 0;
      ext->pusher_flags     = 0;
      ext->segments         = 1;
      ext->power_domain     = 0;
      memset(ext->last_driven_ip, 0, 4);
      ext->last_driven_port = 0;

      offset += sizeof(PPPixelPusherExt);

      discoveryPacketLen = offset;
    }

    void sendBeacon() {
      if (!Network.isConnected()) return;

      // Update dynamic fields before sending
      PPPixelPusherBase *base = (PPPixelPusherBase *)&discoveryPacket[sizeof(PPDiscoveryHeader)];
      base->update_period  = updatePeriodUs;
      base->delta_sequence = deltaSequence;
      deltaSequence = 0; // reset after each beacon

      beaconUdp.beginPacket(IPAddress(255, 255, 255, 255), PP_DISCOVERY_PORT);
      beaconUdp.write(discoveryPacket, discoveryPacketLen);
      beaconUdp.endPacket();
    }

    bool isCommandPacket(const uint8_t *buf, int len) {
      // Command packets have the 16-byte magic after the 4-byte sequence number
      if (len < 20) return false; // 4 + 16
      return memcmp(buf + 4, PP_COMMAND_MAGIC, 16) == 0;
    }

    void handlePixelData(const uint8_t *buf, int len) {
      if (len < 5) return; // minimum: 4 byte seq + 1 byte strip index

      unsigned long startUs = micros();

      // Extract sequence number (first 4 bytes, little-endian)
      uint32_t seq = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);

      // Track dropped packets
      if (lastSequence != 0 && seq > lastSequence + 1) {
        deltaSequence += (seq - lastSequence - 1);
      }
      lastSequence = seq;

      // Check for command packets
      if (isCommandPacket(buf, len)) {
        // Commands not implemented for WLED - ignore
        return;
      }

      uint16_t pps = getEffectivePixelsPerStrip();
      uint16_t stripDataLen = 1 + 3 * pps;
      int dataLen = len - 4; // skip sequence number

      // Validate packet alignment
      if (dataLen % stripDataLen != 0) return;

      int stripsInPacket = dataLen / stripDataLen;

      // Lock realtime mode
      realtimeLock(realtimeTimeoutMs, REALTIME_MODE_PIXELPUSHER);

      if (!realtimeOverride || (realtimeMode && useMainSegmentOnly)) {
        if (esp32SemTake(busDrawMux, 200) == pdTRUE) {
          const uint8_t *ptr = buf + 4; // skip sequence number

          for (int s = 0; s < stripsInPacket; s++) {
            uint8_t stripIndex = ptr[0];
            const uint8_t *pixelData = ptr + 1;

            // Ignore out-of-range strip indices.
            if (stripIndex >= numStrips) {
              ptr += stripDataLen;
              continue;
            }

            // Calculate LED offset for this strip
            uint16_t ledOffset = stripIndex * pps;
            uint16_t totalLeds = strip.getLengthTotal();

            for (uint16_t p = 0; p < pps && (ledOffset + p) < totalLeds; p++) {
              uint16_t idx = ledOffset + p;
              setRealtimePixel(idx, pixelData[p * 3], pixelData[p * 3 + 1], pixelData[p * 3 + 2], 0);
            }

            ptr += stripDataLen;
          }
          esp32SemGive(busDrawMux);
        }
      }

      // Measure processing time
      unsigned long elapsed = micros() - startUs;
      if (elapsed < 1851) elapsed = 1851; // minimum clamp per protocol
      updatePeriodUs = elapsed;

      packetsReceived++;
      lastPacketTime = millis();
    }

  public:
    PixelPusherUsermod() : Usermod("PixelPusher", false) {}

    void setup() override {
      initDone = true;
      if (!enabled) return;
    }

    void connected() override {
      if (!enabled || !initDone) return;

      // (Re)start UDP sockets when WiFi connects
      startUdp();
    }

    void loop() override {
      if (!enabled || !initDone || !udpStarted) return;
      if (!Network.isConnected()) return;

      // Send discovery beacon every second
      unsigned long now = millis();
      if (now - lastBeaconTime >= PP_BEACON_INTERVAL_MS) {
        lastBeaconTime = now;
        sendBeacon();
      }

      // Receive pixel data
      int packetSize;
      while ((packetSize = dataUdp.parsePacket()) > 0) {
        if (packetSize > PP_MAX_PACKET_SIZE) {
          // Drain oversized packet
          dataUdp.flush();
          continue;
        }
        int bytesRead = dataUdp.read(rxBuffer, packetSize);
        if (bytesRead > 0) {
          handlePixelData(rxBuffer, bytesRead);
        }
      }
    }

    uint16_t getId() override { return USERMOD_ID_PIXELPUSHER; }

    void addToJsonInfo(JsonObject& obj) override {
      JsonObject user = obj["u"];
      if (user.isNull()) user = obj.createNestedObject("u");

      JsonArray ppArr = user.createNestedArray(F("PixelPusher"));
      if (!enabled) {
        ppArr.add(F("disabled"));
        return;
      }
      if (!udpStarted) {
        ppArr.add(F("not connected"));
        return;
      }

      String info = String(numStrips) + F(" strip(s), ") + String(getEffectivePixelsPerStrip()) + F(" px/strip");
      ppArr.add(info);

      JsonArray pkts = user.createNestedArray(F("PP packets"));
      pkts.add(packetsReceived);

      if (artnetUniverse >= 0) {
        JsonArray art = user.createNestedArray(F("PP ArtNet"));
        char artInfo[40];
        if (artnetChannel >= 0) {
          snprintf(artInfo, sizeof(artInfo), "universe %d, ch %d", (int)artnetUniverse, (int)artnetChannel);
        } else {
          snprintf(artInfo, sizeof(artInfo), "universe %d", (int)artnetUniverse);
        }
        art.add(artInfo);
      }
    }

    void addToConfig(JsonObject& obj) override {
      Usermod::addToConfig(obj);
      JsonObject top = obj[FPSTR(_name)];
      top[F("strips")]       = numStrips;
      top[F("pixelsPerStrip")] = pixelsPerStrip;
      top[F("controller")]   = controllerOrdinal;
      top[F("group")]        = groupOrdinal;
      top[F("artnetUniverse")] = artnetUniverse;
      top[F("artnetChannel")]  = artnetChannel;
    }

    bool readFromConfig(JsonObject& obj) override {
      bool wasEnabled = enabled;
      bool configComplete = Usermod::readFromConfig(obj);
      JsonObject top = obj[FPSTR(_name)];
      if (top.isNull()) return false;

      configComplete &= getJsonValue(top[F("strips")],         numStrips,         1);
      configComplete &= getJsonValue(top[F("pixelsPerStrip")], pixelsPerStrip,    0);
      configComplete &= getJsonValue(top[F("controller")],     controllerOrdinal, 0);
      configComplete &= getJsonValue(top[F("group")],          groupOrdinal,      0);
      configComplete &= getJsonValue(top[F("artnetUniverse")], artnetUniverse,    (int16_t)-1);
      configComplete &= getJsonValue(top[F("artnetChannel")],  artnetChannel,     (int16_t)-1);

      if (numStrips < 1) numStrips = 1;
      if (numStrips > 128) numStrips = 128;

      // Apply runtime enable/config changes without requiring a reboot.
      if (initDone) {
        if (!enabled) {
          stopUdp();
        } else if (Network.isConnected()) {
          if (!udpStarted || !wasEnabled) startUdp();
          else buildDiscoveryPacket();
        }
      }

      return configComplete;
    }

    void appendConfigData() override {
      oappend(SET_F("addInfo('PixelPusher:strips',1,'number of virtual strips');"));
      oappend(SET_F("addInfo('PixelPusher:pixelsPerStrip',1,'0=auto from total LEDs');"));
      oappend(SET_F("addInfo('PixelPusher:controller',1,'controller ordinal ID');"));
      oappend(SET_F("addInfo('PixelPusher:group',1,'group ordinal ID');"));
      oappend(SET_F("addInfo('PixelPusher:artnetUniverse',1,'-1=disabled');"));
      oappend(SET_F("addInfo('PixelPusher:artnetChannel',1,'-1=disabled');"));
    }
};

const char PixelPusherUsermod::_name[]    PROGMEM = "PixelPusher";
const char PixelPusherUsermod::_enabled[] PROGMEM = "enabled";
