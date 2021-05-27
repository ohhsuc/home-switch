#include "Mesher.h"

namespace Victoria {

  namespace Components {

    // IMeshLoader
    IMeshLoader::IMeshLoader() {
    }
    void IMeshLoader::onMessage(TMessageHandler handler) {
    }

    // RadioFrequencyMeshLoader
    RadioFrequencyMeshLoader::RadioFrequencyMeshLoader(uint8_t inputPin) {
    }
    void RadioFrequencyMeshLoader::send() {
    }

    // BluetoothMeshLoader
    BluetoothMeshLoader::BluetoothMeshLoader(uint8_t inputPin) {
    }
    void BluetoothMeshLoader::send() {
    }

    // Mesher
    Mesher::Mesher() {
    }
    void Mesher::_handleMessage(MeshMessage& message) {
    }
    void Mesher::setLoader(IMeshLoader* loader) {
      _loader = loader;
      _loader->send();
    }

  }
}
