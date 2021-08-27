#ifndef Commons_h
#define Commons_h

#include "Console.h"
#include "BuildFlags.h"
#include "GlobalHelpers.h"
#include "WebModels.h"

namespace Victoria {

  enum VEnvironment {
    VTest,
    VProd,
  };

  enum VOtaType {
    VOta_All = 0,
    VOta_Sketch = 1,
    VOta_FileSystem = 2,
  };

  enum ButtonState {
    AWAIT_PRESS,
    DEBOUNCE_PRESS,
    AWAIT_RELEASE,
    DEBOUNCE_RELEASE,
    AWAIT_MULTI_PRESS,
    DEBOUNCE_MULTI_PRESS,
  };

  struct AppModel {
    String name;
    int ledPin = 2;
    uint8_t ledOnValue = 0; // LOW
  };

  // const int led = LED_BUILTIN;
  const uint8_t V_GPIO0 = 0; // GPIO-0
  const uint8_t V_GPIO2 = 2; // GPIO-2 (Led Builtin)
  const uint8_t V_TXD = 1;   // TXD (Transmitter)
  const uint8_t V_RXD = 3;   // RXD (Receiver)

  // globals
  const VEnvironment VEnv = VTest;

} // namespace Victoria

#endif // Commons_h
