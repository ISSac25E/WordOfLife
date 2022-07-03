// OneWire Slave, RTX Protocol will be used in the future:
#include "core\rtx\OneWire_Slave_1.0.0.h"
// NeoPixel Lib:
#include "core\led\NeoPixel_1.0.1.h"
// LedMacro and SequenceBuild:
#include "core\led\macros\LedMacro_1.0.0.h"
#include "core\led\macros\SequenceBuild_1.0.2.h"
// pinDriver and pinMacro:
#include "core\button\PinDriver_1.0.1.h"
#include "core\button\InputMacro_1.0.1.h"

// set to pin 8 and ip 3. Etherent ip "should" be on ip 1:
OneWire_Slave rtx(8, 3);
const uint8_t ethernet_ip = 1;

// set up NeoPixels:
uint8_t pixelBuffer[12];                        // 12 bytes for 6 pixelChannels(r,g,b - (onPedal) and r,g,b - (Tally))
NeoPixelDriver pixelDriver(12, pixelBuffer, 6); // set neoPixels on pin 12. set buffer and declare 6 pixelChannels

// make sure pixels are listed in correct order "pre pixel"! (green(first), red(second), blue(last)):
// pedal:
NeoPixel pedal_G(pixelDriver); // green pixel on pedal led
NeoPixel pedal_R(pixelDriver); // Red pixel on pedal led
NeoPixel pedal_B(pixelDriver); // Blue pixel on pedal led
// tally:
NeoPixel tally_G(pixelDriver); // green pixel on pedal led
NeoPixel tally_R(pixelDriver); // Red pixel on pedal led
NeoPixel tally_B(pixelDriver); // Blue pixel on pedal led

uint8_t otherMacro; // use as a placeHolder macro

// set led macros:
LedMacro _macros[6];                                                // create 6 macros(for 6 pixel channels)
LedMacroManager macro(_macros, sizeof(_macros) / sizeof(LedMacro)); // set macro and number of available macros.

SequenceBuild ledBuild; // create sequence builder, only one is needed unless multiple sequences are need to be run

// set pin and inputMacros:
PinDriver LeftPedal(2);
PinDriver MiddlePedal(3);
PinDriver RightPedal(4);

InputMacro pinMacro[4] = {InputMacro(true), InputMacro(true), InputMacro(true), InputMacro(true)};

// connection states:
bool atem_connected = false;

// connection timer:
uint32_t connectionTimer = millis();

// tally states:
const uint8_t pedalTally = 2; // this is the tally that will show in the led's. 1 - 8 tally input
uint8_t tally_program = 0;
uint8_t tally_preview = 0;

void setup()
{
}

void loop()
{
  rtxHandle();
  buttonHandle();
  ledHandle();
}

void rtxHandle()
{
  uint8_t sender_ip;
  uint8_t dataArray[10]; // prepare for max possible bytes
  uint8_t dataBytes;
  if (rtx.read(sender_ip, dataArray, dataBytes))
  {
    if (sender_ip == ethernet_ip && dataBytes == 3)
    {
      connectionTimer = millis();

      atem_connected = dataArray[0] & (1);

      tally_program = dataArray[1];
      tally_preview = dataArray[2];
    }
  }
  if (millis() - connectionTimer >= 1000)
  {
    atem_connected = false;
  }
}

/*
buttonCommand():
  inputs: (pinMask: to specify which buttons on the pedal to check(eg. B110: left and middle button))
          (pinStates: run pinDrivers ahead of time and input the results, to keep all macros consistant, order them according to pinMask. Right most in "pinMask" will be first in pinState Array(right pedal should come first in array))
          (commandType: to specify if the button commands are "or - ||" or "and - &&"(false = or, true = and))
  returns: true if all button state match commands, false otherwise
*/
bool buttonCommand(uint8_t pinMask, bool *pinStates, bool commandType) // (pinMasks to specify which of the three buttons will be used)
{
  bool commandMatch = false;

  if (commandType) // and command
  {
    if (pinMask & B111) // only mark commandMatch true if any button mask is set
      commandMatch = true;
    for (uint8_t x = 0; x < 3 && commandMatch; x++)
    {
      if (pinMask & (1 << x))
        if (pinStates[x]) // << buttons are pressed in the low state! high means invalid "and"
          commandMatch = false;
    }
  }
  else // or command
  {
    for (uint8_t x = 0; x < 3 && !commandMatch; x++)
    {
      if (pinMask & (1 << x))
        if (!pinStates[x]) // << buttons are pressed in the low state! any low means valid "or"
          commandMatch = true;
    }
  }
  return commandMatch;
}

void buttonHandle()
{
  const uint8_t defaultInput = 2;
  const uint8_t secondaryInput = 3;

  bool pinStates[] =
      {RightPedal, MiddlePedal, LeftPedal};

  if (pinMacro[0](buttonCommand(B111, pinStates, false))) // state change
  {
    if (pinMacro[0] && pinMacro[0].prevTriggered())
    {
      pinMacro[0].trigger();

      if (!programTally(defaultInput) || !programTally(secondaryInput))
      {
        if (programTally(defaultInput))
        {
          if (!previewTally(secondaryInput))
          {
            rtx_preview(secondaryInput);
          }
        }
        if (programTally(secondaryInput))
        {
          if (!previewTally(defaultInput))
          {
            rtx_preview(defaultInput);
          }
        }
      }
    }
    else if (!pinMacro[0] && pinMacro[0].prevTriggered())
    {
      if (pinMacro[0].prevInterval() >= 300)
      {
        // long press
        ledBuild.setPrioritySequence(blinkBlue, 0, true);
        if (programTally(defaultInput) || programTally(secondaryInput))
          rtx_doAuto();
        else
        {
          rtx_preview(secondaryInput);
          rtx_doAuto();
        }
      }
      else
      {
        // short press
        ledBuild.setPrioritySequence(blinkGreen, 0, true);
        if (programTally(defaultInput) || programTally(secondaryInput))
          rtx_doCut();
        else
        {
          rtx_preview(defaultInput);
          rtx_doAuto();
        }
      }
    }
  }
  if (!pinMacro[0].triggered() && pinMacro[0].interval() >= 500)
  {
    pinMacro[0].trigger();
  }

  if (pinMacro[1](buttonCommand(B000, pinStates, false))) // state change
  {
    if (pinMacro[1])
    {
    }
    else
    {
    }
  }
  if (pinMacro[1] && !pinMacro[1].triggered() && pinMacro[1].interval() >= 0)
  {
    pinMacro[1].trigger();
  }

  if (pinMacro[2](buttonCommand(B000, pinStates, false))) // state change
  {
    if (pinMacro[2])
    {
    }
    else
    {
    }
  }
  if (pinMacro[2] && !pinMacro[2].triggered() && pinMacro[2].interval() >= 0)
  {
    pinMacro[2].trigger();
  }

  if (pinMacro[3](buttonCommand(B000, pinStates, false))) // state change
  {
    if (pinMacro[3])
    {
    }
    else
    {
    }
  }
  if (pinMacro[3] && !pinMacro[3].triggered() && pinMacro[3].interval() >= 0)
  {
    pinMacro[3].trigger();
  }
}

void ledHandle()
{
  macro.run();
  pixelDriver.run();
  ledBuild.run();

  if (atem_connected)
  {
    if (programTally(pedalTally))
    {
      // program tally:
      ledBuild.setSequence(setRed, 0, true);
    }
    else if (previewTally(pedalTally))
    {
      // preview tally:
      ledBuild.setSequence(setGreen, 0, true);
    }
    else
    {
      // default:
      ledBuild.setSequence(setDefault, 0, true);
    }
  }
  else
  {
    ledBuild.setSequence(connectionError, 0, true);
  }
}

inline bool allLedReady()
{
  return (macro.ready(pedal_R) && macro.ready(pedal_G) && macro.ready(pedal_B) &&
          macro.ready(tally_R) && macro.ready(tally_G) && macro.ready(tally_B) &&
          macro.ready(otherMacro));
}

/*
  Next few functions are just to simplify sending atem command
  reliably.
    using this chart to send commands(for first byte of packet):
      command 0: null
      command 1: doCut. expects one byte
      command 2: doAuto. expects one byte
      command 3: previewInput. expects two bytes, byte[1] should include preview input
      command 4: programInput. expects two bytes, byte[1] should include program input
      command 5: toggle_ftb. expects one byte
*/

void rtx_doCut()
{
  uint8_t dataArray[1];
  dataArray[0] = 1;
  rtx_sendPacket(ethernet_ip, dataArray, 1);
}
void rtx_doAuto()
{
  uint8_t dataArray[1];
  dataArray[0] = 2;
  rtx_sendPacket(ethernet_ip, dataArray, 1);
}
void rtx_ftb()
{
  uint8_t dataArray[1];
  dataArray[0] = 5;
  rtx_sendPacket(ethernet_ip, dataArray, 1);
}
void rtx_preview(uint8_t input)
{
  uint8_t dataArray[2];
  dataArray[0] = 3;
  dataArray[1] = input;
  rtx_sendPacket(ethernet_ip, dataArray, 2);
}
void rtx_program(uint8_t input)
{
  uint8_t dataArray[2];
  dataArray[0] = 4;
  dataArray[1] = input;
  rtx_sendPacket(ethernet_ip, dataArray, 2);
}

bool rtx_sendPacket(uint8_t receiver_ip, uint8_t *dataArray, uint8_t bytes)
{
  if (!rtx.write(receiver_ip, dataArray, bytes))
  {
    if (rtx.writeError() == 2) // buffer full, try to send a second time
    {
      return rtx.write(receiver_ip, dataArray, bytes);
    }
    else if (rtx.writeError() == 0) // << MisCommunication? - this error should NEVER happen
    {
      return true;
    }
    else
      return false;
  }
  else
    return true;
}

bool programTally(uint8_t input)
{
  if (input >= 1 && input <= 8)
    return tally_program & (1 << (input - 1));
  return false;
}
bool previewTally(uint8_t input)
{
  if (input >= 1 && input <= 8)
    return tally_preview & (1 << (input - 1));
  return false;
}

SB_FUNCT(connectionError, allLedReady())
SB_STEP(macro.quadEase(pedal_R, 0, 15);
        macro.quadEase(pedal_G, 0, 15);
        macro.quadEase(pedal_B, 0, 15);
        macro.quadEase(tally_R, 0, 15);
        macro.quadEase(tally_G, 0, 15);
        macro.quadEase(tally_B, 0, 15);)
SB_STEP(macro.quadEase(pedal_R, 255, 15);
        macro.quadEase(tally_R, 255, 15);)
SB_STEP(macro.quadEase(pedal_R, 0, 15);
        macro.quadEase(tally_R, 0, 15);)
SB_STEP(ledBuild.loop(1);)
SB_END

SB_FUNCT(setGreen, false)
SB_STEP(macro.quadEase(pedal_R, 0, 15);
        macro.quadEase(pedal_G, 255, 15);
        macro.quadEase(pedal_B, 0, 15);
        macro.quadEase(tally_R, 0, 15);
        macro.quadEase(tally_G, 255, 15);
        macro.quadEase(tally_B, 0, 15);)
SB_END

SB_FUNCT(setRed, false)
SB_STEP(macro.quadEase(pedal_R, 255, 15);
        macro.quadEase(pedal_G, 0, 15);
        macro.quadEase(pedal_B, 0, 15);
        macro.quadEase(tally_R, 255, 15);
        macro.quadEase(tally_G, 0, 15);
        macro.quadEase(tally_B, 0, 15);)
SB_END

SB_FUNCT(blinkGreen, allLedReady())
SB_STEP(macro.quadEase(pedal_R, 0, 0);
        macro.quadEase(pedal_G, 255, 0);
        macro.quadEase(pedal_B, 0, 0);
        macro.quadEase(tally_R, 0, 0);
        macro.quadEase(tally_G, 255, 0);
        macro.quadEase(tally_B, 0, 0);
        macro.delay(otherMacro, 50);)
SB_END

SB_FUNCT(blinkBlue, allLedReady())
SB_STEP(macro.quadEase(pedal_R, 0, 0);
        macro.quadEase(pedal_G, 0, 0);
        macro.quadEase(pedal_B, 255, 0);
        macro.quadEase(tally_R, 0, 0);
        macro.quadEase(tally_G, 0, 0);
        macro.quadEase(tally_B, 255, 0);
        macro.delay(otherMacro, 50);)
SB_END

SB_FUNCT(setDefault, false)
SB_STEP(macro.quadEase(pedal_R, 30, 15);
        macro.quadEase(pedal_G, 0, 15);
        macro.quadEase(pedal_B, 30, 15);
        macro.quadEase(tally_R, 0, 15);
        macro.quadEase(tally_G, 0, 15);
        macro.quadEase(tally_B, 0, 15);)
SB_END