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
    return "999999";
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
    currentVersion.replace(".", "");
    newVersion.replace(".", "");
    // compare
    if (newVersion.toInt() > currentVersion.toInt()) {
      update(newVersion, type);
    }
  }

  void VictoriaOTA::_updateSketch() {
    WiFiClient client;
    auto currentVersion = getCurrentVersion();
    ESPhttpUpdate.update(client, "http://wwww.rulee.cn/esp8266/firmware.bin", currentVersion);
  }

  void VictoriaOTA::_updateFileSystem() {
    WiFiClient client;
    auto currentVersion = getCurrentVersion();
    ESPhttpUpdate.updateFS(client, "http://wwww.rulee.cn/esp8266/littlefs.bin", currentVersion);
  }

  void VictoriaOTA::_onStart() {
    console.log("[VictoriaOTA] start updating");
  }

  void VictoriaOTA::_onEnd() {
    console.log("[VictoriaOTA] update finished");
  }

  void VictoriaOTA::_onProgress(int progress, int total) {
    console.log("[VictoriaOTA] progress " + String(progress / (total / 100)) + "%");
  }

  void VictoriaOTA::_onError(int error) {
    auto message = ESPhttpUpdate.getLastErrorString();
    console.error("[VictoriaOTA] error " + String(error) + ", message " + message);
  }

} // namespace Victoria::Components
