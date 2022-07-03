#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Slave\OneWire_Slave_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\NeoPixel\NeoPixel_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\LedMacro\LedMacro_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\SequenceBuild\SequenceBuild_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\VarPar\VarPar_1.0.1.h"

OneWire_Slave rtx(8, 4);

// set up neoPixels:
uint8_t pixelBuffer[12];
NeoPixelDriver pixelDriver(12, pixelBuffer, 6);

NeoPixel green[2] = {NeoPixel(pixelDriver, 0), NeoPixel(pixelDriver, 3)};
NeoPixel red[2] = {NeoPixel(pixelDriver, 1), NeoPixel(pixelDriver, 4)};
NeoPixel blue[2] = {NeoPixel(pixelDriver, 2), NeoPixel(pixelDriver, 5)};

Par_bool atemConnected = false;
Par_uint8_t tally[2] = {(8), (8)};

LedMacro macros[6];
LedMacroManager macro(macros, 6);

SequenceBuild build;

uint32_t connectTimer = millis();

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
      atemConnected = rtx_packet[0] & 1;
      tally[0] = rtx_packet[1];
      tally[1] = rtx_packet[2];
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

  if (atemConnected)
  {
    if (tally[0].change() || tally[1].change())
    {
      if (tally[0] & 1)
      {
        build.setSequence(setRed, 0, true);
      }
      else if (tally[1] & 1)
      {
        build.setSequence(setGreen, 0, true);
      }
      else
      {
        build.setSequence(setwhite, 0, true);
      }
    }
  }
  if (atemConnected.change())
  {
    if (!atemConnected)
    {
      build.setSequence(connectionError, 0, true);
    }
    else
    {
      if (tally[0] & 1)
      {
        build.setSequence(setRed, 0, true);
      }
      else if (tally[1] & 1)
      {
        build.setSequence(setGreen, 0, true);
      }
      else
      {
        build.setSequence(setwhite, 0, true);
      }
    }
  }
}

bool allLedReady()
{
  return (macro.ready(red[0]) && macro.ready(red[1]) &&
          macro.ready(green[0]) && macro.ready(green[1]) &&
          macro.ready(blue[0]) && macro.ready(blue[1]));
}

SB_FUNCT(connectionError, allLedReady())
SB_STEP(macro.quadEase(red[0], 0, 20);
        macro.quadEase(green[0], 0, 20);
        macro.quadEase(blue[0], 0, 20);
        macro.quadEase(red[1], 0, 20);
        macro.quadEase(green[1], 0, 20);
        macro.quadEase(blue[1], 0, 20);)
SB_STEP(macro.quadEase(red[0], 255, 20);
        macro.quadEase(red[1], 255, 20);)
SB_STEP(macro.quadEase(red[0], 0, 20);
        macro.quadEase(red[1], 0, 20);)
SB_STEP(build.loop(1);)
SB_END

SB_FUNCT(setRed, false)
SB_STEP(macro.quadEase(red[0], 255, 15);
        macro.quadEase(green[0], 0, 15);
        macro.quadEase(blue[0], 0, 15);
        macro.quadEase(red[1], 255, 15);
        macro.quadEase(green[1], 0, 15);
        macro.quadEase(blue[1], 0, 15);)
SB_END

SB_FUNCT(setGreen, false)
SB_STEP(macro.quadEase(red[0], 0, 15);
        macro.quadEase(green[0], 255, 15);
        macro.quadEase(blue[0], 0, 15);
        macro.quadEase(red[1], 0, 15);
        macro.quadEase(green[1], 255, 15);
        macro.quadEase(blue[1], 0, 15);)
SB_END

SB_FUNCT(setwhite, false)
SB_STEP(macro.quadEase(red[0], 50, 15);
        macro.quadEase(green[0], 50, 15);
        macro.quadEase(blue[0], 50, 15);
        macro.quadEase(red[1], 50, 15);
        macro.quadEase(green[1], 50, 15);
        macro.quadEase(blue[1], 50, 15);)
SB_END
