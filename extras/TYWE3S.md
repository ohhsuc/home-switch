## TYWE3S

board = d1_mini_lite

switch.json
{"s":[4,5,0,1,0]}

app.json
{"name":"Dev","led":[12,0,1],"wifi":["104-2.4","18950098099",1]}


#include <DigitalOutput.h>
DigitalOutput* light;

void setup(void) {
  light = new DigitalOutput(14, LOW);
  light->setValue(false);
}

void setSwitchState(const bool value) {
  light->setValue(value);
}
