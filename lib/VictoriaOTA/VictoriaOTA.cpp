#include "VictoriaOTA.h"

namespace Victoria::Components {

  void VictoriaOTA::setup() {
    // ESPhttpUpdate.setAuthorization(user, password);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    // hook events
    ESPhttpUpdate.onStart(VictoriaOTA::_onStart);
    ESPhttpUpdate.onEnd(VictoriaOTA::_onEnd);
    ESPhttpUpdate.onProgress(VictoriaOTA::_onProgress);
    ESPhttpUpdate.onError(VictoriaOTA::_onError);
  }

  String VictoriaOTA::getCurrentVersion() {
    return FirmwareVersion;
  }

  String VictoriaOTA::checkNewVersion() {
    return F("999999");
  }

  void VictoriaOTA::update(String version, VOtaType type) {
    ESPhttpUpdate.rebootOnUpdate(true);
    switch (type) {
      case VOta_All: {
        ESPhttpUpdate.rebootOnUpdate(false);
        _updateSketch();
        ESPhttpUpdate.rebootOnUpdate(true);
        _updateFileSystem();
        break;
      }
      case VOta_Sketch: {
        _updateSketch();
        break;
      }
      case VOta_FileSystem: {
        _updateFileSystem();
        break;
      }
      default: {
        break;
      }
    }
  }

  void VictoriaOTA::trigger(VOtaType type) {
    // versions
    auto currentVersion = getCurrentVersion();
    auto newVersion = checkNewVersion();
    // 21.3.10 --> 21310
    currentVersion.replace(F("."), F(""));
    newVersion.replace(F("."), F(""));
    // compare
    if (newVersion.toInt() > currentVersion.toInt()) {
      update(newVersion, type);
    }
  }

  void VictoriaOTA::_updateSketch() {
    WiFiClient client;
    auto currentVersion = getCurrentVersion();
    ESPhttpUpdate.update(client, F("http://wwww.rulee.cn/esp8266/firmware.bin"), currentVersion);
  }

  void VictoriaOTA::_updateFileSystem() {
    WiFiClient client;
    auto currentVersion = getCurrentVersion();
    ESPhttpUpdate.updateFS(client, F("http://wwww.rulee.cn/esp8266/littlefs.bin"), currentVersion);
  }

  void VictoriaOTA::_onStart() {
    console.log(F("[VictoriaOTA] start updating"));
  }

  void VictoriaOTA::_onEnd() {
    console.log(F("[VictoriaOTA] update finished"));
  }

  void VictoriaOTA::_onProgress(int progress, int total) {
    console.log().write(F("[VictoriaOTA] progress ")).write(String(progress / (total / 100))).write(F("%")).newline();
  }

  void VictoriaOTA::_onError(int error) {
    auto message = ESPhttpUpdate.getLastErrorString();
    console.error().write(F("[VictoriaOTA] error ")).write(String(error)).write(F(", message ")).write(message).newline();
  }

} // namespace Victoria::Components
