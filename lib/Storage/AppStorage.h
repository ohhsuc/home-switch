#ifndef AppStorage_h
#define AppStorage_h

#include "FileStorage.h"

namespace Victoria::Components {
  class AppStorage : public FileStorage<AppModel> {
   public:
    AppStorage();

   protected:
    void _serializeTo(const AppModel& model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(AppModel& model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern AppStorage appStorage;

} // namespace Victoria::Components

#endif // AppStorage_h
