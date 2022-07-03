#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Slave\OneWire_Slave_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\NeoPixel\NeoPixel_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\LedMacro\LedMacro_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\SequenceBuild\SequenceBuild_1.0.0.h"

OneWire_Slave rtx(8, 4);
bool prevAtemConnected = false;
bool atemConnected = false;

bool tally[2] = {false, false};
bool prevTally[2] = {false, false};

uint8_t pixelBuffer[12];
NeoPixelDriver pixelDriver(12, pixelBuffer, 6);

LedMacro macros[6];
LedMacroManager macro(macros, 6);

NeoPixel green_1(pixelDriver);
NeoPixel red_1(pixelDriver);
NeoPixel blue_1(pixelDriver);

NeoPixel green_2(pixelDriver);
NeoPixel red_2(pixelDriver);
NeoPixel blue_2(pixelDriver);

SequenceBuild build;

uint32_t connectTimer = millis();

uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;

void setup()
{
  build.setSequence(connectionError, 0, true);
}
void loop()
{
  macro.run();
  pixelDriver.run();
  build.run();
  uint8_t rtx_ip;
  uint8_t rtx_packet[10];
  uint8_t rtx_bytes;
  if (rtx.read(rtx_ip, rtx_packet, rtx_bytes))
  {
    if (rtx_ip == 1 && rtx_bytes == 3)
    {
      connectTimer = millis();
      atemConnected = rtx_packet[0];
      tally[0] = rtx_packet[1] & 1;
      tally[1] = rtx_packet[2] & 1;
    }
  }
  if (millis() - connectTimer >= 1000)
  {
    atemConnected = false;
  }
  ledRun();
}

void ledRun()
{
  if (prevTally[0] != tally[0] || prevTally[1] != tally[1])
  {
    if (atemConnected)
    {
      prevTally[0] = tally[0];
      prevTally[1] = tally[1];
      if (tally[0])
        build.setSequence(onRed, 0, true);
      else if (tally[1])
        build.setSequence(onGreen, 0, true);
      else
        build.setSequence(onYellow, 0, true);
    }
  }
  if (prevAtemConnected != atemConnected)
  {
    prevAtemConnected = atemConnected;
    if (!atemConnected)
      build.setSequence(connectionError, 0, true);
    else if (tally[0])
      build.setSequence(onRed, 0, true);
    else if (tally[1])
      build.setSequence(onGreen, 0, true);
    else
      build.setSequence(onYellow, 0, true);
  }

  green_1 = green;
  blue_1 = blue;
  red_1 = red;

  green_2 = green;
  blue_2 = blue;
  red_2 = red;
}

SB_FUNCT(connectionError, (macro.ready(red) && macro.ready(green) && macro.ready(blue)))
SB_STEP(macro.quadEase(red, 0, 20);
        macro.quadEase(green, 0, 20);
        macro.quadEase(blue, 0, 20);)
SB_STEP(macro.quadEase(red, 255, 20);)
SB_STEP(macro.quadEase(red, 0, 20);)
SB_STEP(build.loop(1);)
SB_END

SB_FUNCT(onRed, false)
SB_STEP(macro.quadEase(red, 255, 15);
        macro.quadEase(green, 0, 15);
        macro.quadEase(blue, 0, 15);)
SB_END

SB_FUNCT(onGreen, false)
SB_STEP(macro.quadEase(red, 0, 15);
        macro.quadEase(green, 255, 15);
        macro.quadEase(blue, 0, 15);)
SB_END

SB_FUNCT(onYellow, false)
SB_STEP(macro.quadEase(red, 50, 15);
        macro.quadEase(green, 50, 15);
        macro.quadEase(blue, 50, 15);)
SB_END
