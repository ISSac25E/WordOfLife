#define CONFIG_Sectors 2 // << 16 bytes
#define DEBUG_Sectors 4  // << 32 bytes
#define RTX_Name "Test"
#define RTX_Version 1, 0 // << v1.0

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\RTX_Protocol\RTX_Protocol_2.0.0.h"

RTX_Protocol rtx(8, 3);

void setup()
{
}
void loop()
{
  rtx.run();
}