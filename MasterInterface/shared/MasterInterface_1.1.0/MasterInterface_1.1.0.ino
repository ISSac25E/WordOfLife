// declare definitions for master device:
#define OneWire_Master_DataPin 8             // declare bus data pin
#define OneWire_Master_DeviceList 0, 1, 2, 3 // declare all possible devices on bus. keep as minimal as possible
#define OneWire_Master_DeviceCount 4         // state number of devices

#include "core\rtx\OneWire_Master_1.0.1.h"
OneWire_Master Master;

void setup()
{
}

void loop()
{
  Master.run();
}