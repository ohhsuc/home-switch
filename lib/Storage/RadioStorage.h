#ifndef RadioStorage_h
#define RadioStorage_h

#include <FileStorage.h>

namespace Victoria::Components {
  class RadioStorage : public FileStorage<RadioModel> {
    public:
      RadioStorage();
      void broadcast(RadioMessage message);
      RadioMessage getLastReceived();
    protected:
      RadioMessage _lastReceived;
      void _serializeTo(const RadioModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) override;
      void _deserializeFrom(RadioModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) override;
  };
  // global
  extern RadioStorage radioStorage;
} // namespace Victoria::Components

#endif // RadioStorage_h
