#include "BooleanHomeKitService.h"

namespace Victoria::HomeKit {

  extern "C" homekit_characteristic_t boolCharacteristic;

  BooleanHomeKitService::BooleanHomeKitService(const String& id, const ServiceSetting& setting)
  : HomeKitService(id, setting, &boolCharacteristic) {}

  BooleanHomeKitService::~BooleanHomeKitService() {
    if (_buttonEvents) {
      delete _buttonEvents;
      _buttonEvents = NULL;
    }
    if (homekit_characteristic_has_notify_callback(serviceCharacteristic, BooleanHomeKitService::_notifyCallback, this)) {
      homekit_characteristic_remove_notify_callback(serviceCharacteristic, BooleanHomeKitService::_notifyCallback, this);
    }
  }

  void BooleanHomeKitService::setup() {
    // call base
    HomeKitService::setup();

    // setup cha callback
    // c++ convert std::function to function pointer
    // c++ pointer function with lambda
    // http://www.cplusplus.com/forum/beginner/245537/

    /*
    // solution 1
    auto setter_lambda = [this](homekit_characteristic_t* ch, const homekit_value_t value)->void {
      this->_innerSetState({
        .boolValue = value.bool_value,
      }, false);
    };
    static auto static_lambda = setter_lambda;
    serviceCharacteristic->setter_ex = [](homekit_characteristic_t* ch, const homekit_value_t value)->void {
      static_lambda(ch, value);
    };
    */

    /*
    // solution 2
    serviceCharacteristic->context = this;
    serviceCharacteristic->setter_ex = [](homekit_characteristic_t* ch, const homekit_value_t value)->void {
      auto service = (BooleanHomeKitService*)ch->context;
      service->_innerSetState({
        .boolValue = value.bool_value,
      }, false);
    };
    */

    // solution 3
    homekit_characteristic_add_notify_callback(serviceCharacteristic, BooleanHomeKitService::_notifyCallback, this);

    // setup inputs
    if (serviceSetting.inputPin > -1) {
      _buttonEvents = new ButtonEvents(serviceSetting.inputPin);
      _buttonEvents->onClick = [this](int times)->void {
        console.log("[ButtonEvents] > times " + String(times));
        if (times == 1) {
          auto state = this->getState();
          state.boolValue = !state.boolValue;
          this->setState(state);
        }
      };
    }
  }

  void BooleanHomeKitService::loop() {
    if (_buttonEvents) {
      _buttonEvents->loop();
    }
  }

  ServiceState BooleanHomeKitService::getState() {
    return {
      .boolValue = serviceCharacteristic->value.bool_value,
    };
  }

  void BooleanHomeKitService::setState(const ServiceState& state) {
    if (serviceCharacteristic) {
      serviceCharacteristic->value.bool_value = state.boolValue;
      notifyState();
    }
  }

  void BooleanHomeKitService::_notifyCallback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    auto service = (BooleanHomeKitService*)context;
    auto state = service->getState();
    // update pin
    auto outputPin = service->serviceSetting.outputPin;
    if (outputPin > -1) {
      if (state.boolValue) {
        digitalWrite(outputPin, LOW);
      } else {
        digitalWrite(outputPin, HIGH);
      }
    }
    // fire event
    service->_fireStateChange(state);
  }

} // namespace Victoria::HomeKit
