#ifndef ServiceStorage_h
#define ServiceStorage_h

#include <FileStorage.h>

namespace Victoria::Components {
  class ServiceStorage : public FileStorage<ServicesModel> {
    public:
      ServiceStorage();
    protected:
      void _serializeTo(const ServicesModel& model, StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) override;
      void _deserializeFrom(ServicesModel& model, const StaticJsonDocument<DEFAULT_FILE_SIZE>& doc) override;
  };
  // global
  extern ServiceStorage serviceStorage;
} // namespace Victoria::Components

#endif // ServiceStorage_h
