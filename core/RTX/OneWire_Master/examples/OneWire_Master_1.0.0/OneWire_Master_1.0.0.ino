// declare definitions for master device:
#define OneWire_Master_DataPin 8             // declare bus data pin
#define OneWire_Master_DeviceList 0, 1, 2, 3, 4 // declare all possible devices on bus
#define OneWire_Master_DeviceCount 5         // state number of devices

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Master\OneWire_Master_1.0.0.h"
// #include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Master\test\OneWire_Master_1.0.0.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("Init\n");
  OneWire_Master master_device; // declare object
  for (;;)
    master_device.run(); // run continuously
}
void loop()
{
}