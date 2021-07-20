#ifndef WebModels_h
#define WebModels_h

#include <vector>
#include <Arduino.h>

namespace Victoria {

  struct TableModel {
    std::vector<String> header;
    std::vector<std::vector<String>> rows;
  };

  struct SelectionOptions {
    String inputType;
    String inputName;
    String inputValue;
    bool isChecked = false;
    String labelText;
  };

  struct SelectOption {
    String value;
    String text;
  };

  struct SelectModel {
    String name;
    String value;
    std::vector<SelectOption> options;
  };

} // namespace Victoria

#endif // WebModels_h
