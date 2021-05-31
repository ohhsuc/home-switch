#ifndef Mesher_h
#define Mesher_h

#include <functional>
#include <Arduino.h>
#include "Commons.h"

namespace Victoria {
  namespace Components {

    enum MeshMessageType {
      MESH_HEARTBEAT,
      MESH_WIFI_REQUEST,
      MESH_WIFI_CONNECTED,
    };

    struct MeshMessage {
      String sourceId;
      String replyId;
      MeshMessageType type;
      String content;
    };

    class IMeshLoader {
      typedef std::function<void(MeshMessage&)> TMessageHandler;
      public:
        IMeshLoader(uint8_t inputPin);
        void onMessage(TMessageHandler handler);
        virtual void send(const MeshMessage& message);
      protected:
        uint8_t _inputPin;
        TMessageHandler _messageHandler;
    };

    class RadioFrequencyMeshLoader: public IMeshLoader {
      public:
        RadioFrequencyMeshLoader(uint8_t inputPin);
        virtual void send(const MeshMessage& message);
    };

    class BluetoothMeshLoader: public IMeshLoader {
      public:
        BluetoothMeshLoader(uint8_t inputPin);
        virtual void send(const MeshMessage& message);
    };

    class Mesher {
      public:
        Mesher();
        void setLoader(IMeshLoader* loader);
        void send(MeshMessageType type);
        void send(MeshMessageType type, const String& content);
      private:
        String _id;
        IMeshLoader* _loader;
        void _handleMessage(MeshMessage& message);
    };
  }
}

#endif //Mesher_h
