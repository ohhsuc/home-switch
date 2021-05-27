#ifndef Mesher_h
#define Mesher_h

#include <functional>
#include "Arduino.h"

namespace Victoria {
  namespace Components {
    struct MeshMessage {
      String channel;
      String content;
    };

    class IMeshLoader {
      typedef std::function<void()> TMessageHandler;
      public:
        IMeshLoader();
        void onMessage(TMessageHandler handler);
        virtual void send();
    };

    class RadioFrequencyMeshLoader: public IMeshLoader {
      public:
        RadioFrequencyMeshLoader(uint8_t inputPin);
        void send() override;
    };

    class BluetoothMeshLoader: public IMeshLoader {
      public:
        BluetoothMeshLoader(uint8_t inputPin);
        void send() override;
    };

    class Mesher {
      public:
        Mesher();
        void setLoader(IMeshLoader* loader);
      private:
        IMeshLoader* _loader;
        void _handleMessage(MeshMessage& message);
    };
  }
}

#endif //Mesher_h
