// declare definitions for master device:
#define OneWire_Master_DataPin 8                   // declare bus data pin
#define OneWire_Master_DeviceList 0, 1, 2, 3 // declare all possible devices on bus
#define OneWire_Master_DeviceCount 4               // state number of devices

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Master\OneWire_Master_1.0.0.h"

OneWire_Master Master;

void setup()
{
}

void loop()
{
  Master.run();
}