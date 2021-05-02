namespace Purl {
  namespace Events {
    class OnOffEvents {
      public:
        OnOffEvents(uint8_t pin);
        typedef void (*ToggleEvent)(bool);
        ToggleEvent onToggle;
        void loop();
      private:
        uint8_t _pin;
        bool _lastState = false;
    };
  }
}
