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
    void _serializeTo(const RadioModel& model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(RadioModel& model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern RadioStorage radioStorage;

} // namespace Victoria::Components

#endif // RadioStorage_h
