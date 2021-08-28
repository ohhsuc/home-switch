#ifndef ServiceStorage_h
#define ServiceStorage_h

#include "FileStorage.h"
#include "Models/ServiceModels.h"

namespace Victoria::Components {
  class ServiceStorage : public FileStorage<ServicesModel> {
   public:
    ServiceStorage();

   protected:
    void _serializeTo(const ServicesModel& model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(ServicesModel& model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern ServiceStorage serviceStorage;

} // namespace Victoria::Components

#endif // ServiceStorage_h
