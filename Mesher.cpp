#include "Mesher.h"

namespace Victoria {

  namespace Components {

    // IMeshLoader
    IMeshLoader::IMeshLoader(uint8_t inputPin) {
      _inputPin = inputPin;
    }
    void IMeshLoader::onMessage(TMessageHandler handler) {
      _messageHandler = handler;
    }
    void IMeshLoader::send(const MeshMessage& message) {
      Serial.println("IMeshLoader send()");
    }

    // RadioFrequencyMeshLoader
    RadioFrequencyMeshLoader::RadioFrequencyMeshLoader(uint8_t inputPin) : IMeshLoader(inputPin) {
    }
    void RadioFrequencyMeshLoader::send(const MeshMessage& message) {
      Serial.println("RadioFrequencyMeshLoader send()");
    }

    // BluetoothMeshLoader
    BluetoothMeshLoader::BluetoothMeshLoader(uint8_t inputPin) : IMeshLoader(inputPin) {
    }
    void BluetoothMeshLoader::send(const MeshMessage& message) {
      Serial.println("BluetoothMeshLoader send()");
    }

    // Mesher
    Mesher::Mesher() {
      _id = CommonHelpers::randomString(8);
    }
    void Mesher::setLoader(IMeshLoader* loader) {
      _loader = loader;
      _loader->onMessage(std::bind(&Mesher::_handleMessage, this, std::placeholders::_1));
    }
    void Mesher::_handleMessage(MeshMessage& message) {
      if (message.type == MESH_HEARTBEAT) { // heartbeat
        if (message.sourceId == _id) {
          // ignore (launched by ourself)
        } else {
          message.replyId = _id;
          message.content = "pong";
          _loader->send(message);
        }
      } else if (message.type == MESH_WIFI_REQUEST) {
        if (message.sourceId == _id) {
          // check wifi
          // connect wifi
        } else {
          // check wifi
          // reply wifi credential
        }
      } else if (message.type == MESH_WIFI_CONNECTED) {
        if (message.sourceId == _id) {
          // ignore (launched by ourself)
        } else {
          // check wifi
          // connect wifi
        }
      }
    }
    void Mesher::send(MeshMessageType type) {
      send(type, "");
    }
    void Mesher::send(MeshMessageType type, const String& content) {
      MeshMessage message = {
        .sourceId = _id,
        .replyId = "",
        .type = type,
        .content = content,
      };
      _loader->send(message);
    }

  }
}
