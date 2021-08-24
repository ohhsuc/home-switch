#include "VictoriaOTA.h"

namespace Victoria::Components {

  String VictoriaOTA::getCurrentVersion() {
    return FirmwareVersion;
  }

  String VictoriaOTA::checkNewVersion() {
    return "";
  }

   void VictoriaOTA::update(String version, VOtaType type) {
     //TODO:
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

} // namespace Victoria::Components
