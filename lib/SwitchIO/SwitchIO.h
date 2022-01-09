#ifndef SwitchIO_h
#define SwitchIO_h

#include <DigitalInput.h>
#include <DigitalOutput.h>
#include "SwitchModels.h"

namespace Victor::Components {
  class SwitchIO {
   public:
    SwitchIO(SwitchSetting model);
    void loop();
    bool readState();
    void outputState(bool state);
    // events
    typedef std::function<void(bool state)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    DigitalInput* _input = nullptr;
    DigitalOutput* _output = nullptr;
    bool _lastState = false;
    unsigned long _lastLoop = 0;
  };

} // namespace Victor::Components

#endif // SwitchIO_h
