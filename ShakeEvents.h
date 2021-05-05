namespace Purl {
  namespace Events {
    class ShakeEvents {
      public:
        ShakeEvents(uint8_t pin);
        typedef void (*ShakeEvent)();
        ShakeEvent onShake;
        void loop();
      private:
        uint8_t _pin;
        int _state = 0;
        bool _shaked = false;
        unsigned long _lastFire;
    };
  }
}
