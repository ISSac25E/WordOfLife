// OneWire Slave, RTX Protocol will be used in the future:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\OneWire_Slave\OneWire_Slave_1.0.0.h"
// use atem lib:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\ATEM\W5X00\AtemControl\AtemControl_1.0.1.h"
// led macros libs:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\LedMacro\LedMacro_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\SequenceBuild\SequenceBuild_1.0.2.h"
// all button libs:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\PinDriver\PinDriver_1.0.1.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\InputMacro\InputMacro_1.0.1.h"
//  include VarPar, useful for state changes:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\VarPar\VarPar_1.0.1.h"
// include html parser:
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\HTML_Parse\HTML_Parse_1.0.0.h"
// include EEPROM for storing configuration:
#include <EEPROM.h>
/*
  EEPROM MAP:
    (0 - 3) arduino static IP Address
    (4 - 7) atem IP address
    (8) IP Mode (2-bits)
*/

// mac address of arduino:
byte network_mac[] = {0x90, 0xA2, 0xDA, 0x00, 0xE8, 0xE9};

// create server object for serving html config webpage:
EthernetServer server(80);

// create atem instance:
AtemControl atem;

// very important var, atem should not run if ip is not determined:
bool ip_located = false;

// set OneWire rtx:
OneWire_Slave rtx(8, 1); // set pin(8) and ip(1), 1 is default for ethernet
const uint8_t pedal_ip = 3;

// set connection states and tally:
Par_bool atem_connected = false;
Par_uint8_t atem_programTally = 0;
Par_uint8_t atem_previewTally = 0;

// set up objects for (ip mode) button:
PinDriver pinInput(2);
InputMacro pinMacro(true);

Par_uint8_t ipMode = 0;

// set up objects for LED's:
LedMacro _macros[2];                                                // create 2 macros(for two led's)
LedMacroManager macro(_macros, sizeof(_macros) / sizeof(LedMacro)); // set macro and number of available macros.

SequenceBuild ledBuild;

// LED pins:
const uint8_t greenLed_pin = 5;
const uint8_t orangeLed_pin = 6;

// LED vals(analogWrite):
uint8_t greenLed_var = 0;
uint8_t orangeLed_var = 0;

// html form:
const char htmlLabelSuccess[] PROGMEM = R"rawliteral(<p style="padding:4px 0px 4px 0px; text-align: center; color: #000000; background-color: rgba(0,255,0, 0.6);">Configuration Successfully Saved</p>)rawliteral";
const char htmlLabelFail[] PROGMEM = R"rawliteral(<p style="padding:4px 0px 4px 0px; text-align: center; color: #000000; background-color: rgba(255,0,0, 0.6);">Configuration Failed</p>)rawliteral";
const char htmlFormTemplate[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html>

<head>
  <title>Arduino Pedal Configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>

<body>
  <h1 style="padding:4px 0px 4px 0px; text-align: center; color: #000000; background-color: rgba(0,0,0, 0.6);">Arduino Pedal Configuration</h1>

  <form action="/">
    <label for="ardip0">Pedal IP(Static):</label><br>
    <input type="text" size="1" id="ardip0" name="ardip0" value=""><label> .</label>
    <input type="text" size="1" id="ardip1" name="ardip1" value=""><label> .</label>
    <input type="text" size="1" id="ardip2" name="ardip2" value=""><label> .</label>
    <input type="text" size="1" id="ardip3" name="ardip3" value="">
    <input type="checkbox" id="checkbox1" name="checkbox1" value="checkMark" onclick="validate()">
    <label for="checkbox1"> DHCP</label><br><br>

    <label for="atemip0">ATEM IP:</label><br>
    <input type="text" size="1" id="atemip0" name="atemip0" value=""><label> .</label>
    <input type="text" size="1" id="atemip1" name="atemip1" value=""><label> .</label>
    <input type="text" size="1" id="atemip2" name="atemip2" value=""><label> .</label>
    <input type="text" size="1" id="atemip3" name="atemip3" value=""><br><br>
    <input type="submit" value="Submit">
  </form>
</body>
<script>
  window.onload = validate;

  function validate() {
    if (document.getElementById('checkbox1').checked) {
      document.getElementById('ardip0').readOnly = true;
      document.getElementById('ardip0').style = "color: rgba(0, 0, 0, 0.5)";
      document.getElementById('ardip1').readOnly = true;
      document.getElementById('ardip1').style = "color: rgba(0, 0, 0, 0.5)";
      document.getElementById('ardip2').readOnly = true;
      document.getElementById('ardip2').style = "color: rgba(0, 0, 0, 0.5)";
      document.getElementById('ardip3').readOnly = true;
      document.getElementById('ardip3').style = "color: rgba(0, 0, 0, 0.5)";
    } else {
      document.getElementById('ardip0').readOnly = false;
      document.getElementById('ardip0').style = "color: rgba(0, 0, 0, 1)";
      document.getElementById('ardip1').readOnly = false;
      document.getElementById('ardip1').style = "color: rgba(0, 0, 0, 1)";
      document.getElementById('ardip2').readOnly = false;
      document.getElementById('ardip2').style = "color: rgba(0, 0, 0, 1)";
      document.getElementById('ardip3').readOnly = false;
      document.getElementById('ardip3').style = "color: rgba(0, 0, 0, 1)";
    }
  }
</script>

</html>)rawliteral";

void setup()
{
  // set up timer interrupt for LED's and Button input:
  // TIMER 1 for interrupt frequency of 30Hz:
  cli();      // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1 = 0;  // initialize counter value to 0
  // set compare match register for 30 Hz increments
  OCR1A = 8332; // = 16000000 / (64 * 30) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 64 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
  setInterrupt(false); // disable timer compare interrupt, for now
  sei();               // allow interrupts

  // set up ip mode:
  ipMode = (EEPROM.read(8) & B11);

  // setup Status LED's:
  digitalWrite(greenLed_pin, LOW);
  digitalWrite(orangeLed_pin, LOW);
  pinMode(greenLed_pin, OUTPUT);
  pinMode(orangeLed_pin, OUTPUT);

  ledBuild.setPrioritySequence(beginSequence, 0, true);
}

void loop()
{
  if (ip_located)
  {
    if (Ethernet.hardwareStatus() == EthernetNoHardware || Ethernet.linkStatus() == LinkOFF)
    {
      atem.deConfig();
      ip_located = false;
    }
    else
    {
      serverHandle();
    }
  }
  else
  {
    // only attempt to connect of ethernet plugged in and hardware found:
    // if (Ethernet.hardwareStatus() != EthernetNoHardware && Ethernet.linkStatus() != LinkOFF)
    {
      switch (ipMode)
      {
      case 1:
        // full dhcp mode
        {
          setInterrupt(true);
          bool dhcp_ipFound = Ethernet.begin(network_mac);
          setInterrupt(false);

          if (dhcp_ipFound)
          {
            IPAddress atemIp;
            for (uint8_t x = 0; x < 4; x++)
              atemIp[x] = EEPROM.read(x + 4);
            atem.config(atemIp);
            ip_located = true;
          }
        }
        break;
      case 2:
        // dhcp hybrid(ending ip 45)
        {
          setInterrupt(true);
          bool dhcp_ipFound = Ethernet.begin(network_mac);
          setInterrupt(false);

          if (dhcp_ipFound)
          {
            IPAddress ardIp = Ethernet.localIP();
            ardIp[3] = 45;
            Ethernet.begin(network_mac, ardIp);
            IPAddress atemIp;
            for (uint8_t x = 0; x < 4; x++)
              atemIp[x] = EEPROM.read(x + 4);
            atem.config(atemIp);
            ip_located = true;
          }
        }
        break;
      case 3:
        // dhcp hybrid(ending ip 234)
        {
          setInterrupt(true);
          bool dhcp_ipFound = Ethernet.begin(network_mac);
          setInterrupt(false);

          if (dhcp_ipFound)
          {
            IPAddress ardIp = Ethernet.localIP();
            ardIp[3] = 234;
            Ethernet.begin(network_mac, ardIp);
            IPAddress atemIp;
            for (uint8_t x = 0; x < 4; x++)
              atemIp[x] = EEPROM.read(x + 4);
            atem.config(atemIp);
            ip_located = true;
          }
        }
        break;
      default:
        // static ip from EEPROM
        {
          IPAddress ardIp;
          IPAddress atemIp;
          for (uint8_t x = 0; x < 4; x++)
          {
            ardIp[x] = EEPROM.read(x);
            atemIp[x] = EEPROM.read(x + 4);
          }
          Ethernet.begin(network_mac, ardIp);
          if (Ethernet.hardwareStatus() != EthernetNoHardware && Ethernet.linkStatus() != LinkOFF)
          {
            atem.config(atemIp);
            ip_located = true;
          }
        }
        break;
      }
    }
  }
  atemHandle();
  ledHandle();
  buttonHandle();
  rtxHandle();
  storeIpMode(false);
}

inline void rtxHandle()
{
  uint8_t sender_ip;
  uint8_t dataArray[10]; // prepare for max possible bytes
  uint8_t dataBytes;
  if (rtx.read(sender_ip, dataArray, dataBytes))
  {
    if (dataBytes && sender_ip == pedal_ip)
    {
      switch (dataArray[0])
      {
        // command 0: null
      case 1:
        if (dataBytes == 1)
          atem.doCut();
        break;
      case 2:
        if (dataBytes == 1)
          atem.doAuto();
        break;
      case 3:
        if (dataBytes == 2)
          atem.previewInput(dataArray[1]);
        break;
      case 4:
        if (dataBytes == 2)
          atem.programInput(dataArray[1]);
        break;
      case 5:
        if (dataBytes == 1)
          atem.ftb();
        break;
      }
    }
  }
  static uint32_t sendTimer = millis();
  bool change = false;
  if (atem_connected.change())
    change = true;
  if (atem_previewTally.change())
    change = true;
  if (atem_programTally.change())
    change = true;

  if (change || millis() - sendTimer >= 500)
  {
    sendTimer = millis();
    uint8_t dataArray[3];
    dataArray[0] = 0;
    dataArray[0] = atem_connected;
    dataArray[1] = atem_programTally;
    dataArray[2] = atem_previewTally;
    rtx.write(pedal_ip, dataArray, 3);
  }
}

void storeIpMode(bool forceStore)
{
  if (forceStore)
  {
    if ((ipMode & B11) != (EEPROM.read(8) & B11))
    {
      ipMode.change();
      EEPROM.write(8, ipMode);
    }
  }
  else
  {
    static uint32_t storeTimer = millis();
    static bool change = false;
    if (change)
    {
      if (millis() - storeTimer >= 5000)
      {
        change = false;
        EEPROM.write(8, ipMode);
      }
    }
    else
    {
      if (ipMode.change())
      {
        if ((ipMode & B11) != (EEPROM.read(8) & B11))
        {
          change = true;
          storeTimer = millis();
        }
        else
        {
          change = false;
        }
      }
    }
  }
}

inline void atemHandle()
{
  setInterrupt(true);
  atem_connected = atem.run();
  setInterrupt(false);

  uint8_t atem_programTally_newVal = 0;
  uint8_t atem_previewTally_newVal = 0;

  for (uint8_t x = 0; x < 8; x++)
  {
    bitWrite(atem_programTally_newVal, x, bitRead(atem.tally(x), 0));
    bitWrite(atem_previewTally_newVal, x, bitRead(atem.tally(x), 1));
  }

  atem_programTally = atem_programTally_newVal;
  atem_previewTally = atem_previewTally_newVal;
}

void buttonHandle()
{
  static bool buttonInit = false;
  bool pinState = pinInput;
  if (pinMacro(pinState)) // state change
  {
    if (!pinState)
    {
      if (buttonInit)
      {
        if (pinMacro.prevInterval() <= 5000 && pinMacro.prevInterval() > 100)
        {
          ipMode = ipMode + 1;
          ipMode = ipMode & B11;
          // disconnect network, prepare to reconnect with new mode:
          {
            atem.deConfig();
            ip_located = false;
          }
        }
      }
      else
        buttonInit = true;
      runIpModeStatus();
    }
  }
}

void serverHandle()
{
  static int formId = 0;
  EthernetClient client = server.available();
  if (client)
  {
    HTML_Parse html(&client);
    while (client.connected())
    {
      if (html.getMethod())
      {
        uint8_t errorCode = 0;
        char title[21];
        if (html.formTitle(title))
        {
          bool dhcpMode = false;
          uint8_t paramCount = 0;
          if (checkNum(title))
          {
            if (atoi(title) == formId && formId != 0)
            {
              formId = micros();
              char param[21];
              char arg[21];
              while (html.nextParam(param, arg))
              {
                if (!strcmp(param, "ardip0"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(0, num);
                      ip_located = false;
                      atem.deConfig();
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "ardip1"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(1, num);
                      ip_located = false;
                      atem.deConfig();
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "ardip2"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(2, num);
                      ip_located = false;
                      atem.deConfig();
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "ardip3"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(3, num);
                      ip_located = false;
                      atem.deConfig();
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "atemip0"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(4, num);
                      atem.deConfig();
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "atemip1"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(5, num);
                      atem.deConfig();
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "atemip2"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(6, num);
                      atem.deConfig();
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "atemip3"))
                {
                  paramCount++;
                  if (checkNum(arg))
                  {
                    int num = atoi(arg);
                    if (num >= 0 && num <= 255)
                    {
                      EEPROM.write(7, num);
                    }
                    else
                    {
                      errorCode = 2;
                      break;
                    }
                  }
                  else
                  {
                    errorCode = 2;
                    break;
                  }
                }
                else if (!strcmp(param, "checkbox1"))
                {
                  if (!strcmp(arg, "checkMark"))
                  {
                    dhcpMode = true;
                  }
                }
                else
                {
                  errorCode = 2;
                  break;
                }
              }
              if (!errorCode)
              {
                if (paramCount == 8)
                {
                  if (dhcpMode) // dhcp mode
                  {
                    if (ipMode == 0)
                    {
                      ipMode = 1;
                      storeIpMode(true);
                    }
                  }
                  else
                  {
                    ipMode = 0;
                    storeIpMode(true);
                  }
                  errorCode = 1;
                }
              }
            }
          }
        }
        html.clear();

        // anyway send a form, just figure out which type:
        bool checkMark = false;
        if (ipMode != 0)
        {
          checkMark = true;
        }
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println();
        for (uint16_t k = 0; k < strlen_P(htmlFormTemplate); k++)
        {
          if (k == 307 && errorCode)
          {
            if (errorCode == 1)
            {
              for (uint16_t x = 0; x < strlen_P(htmlLabelSuccess); x++)
                client.write(pgm_read_byte_near(htmlLabelSuccess + x));
            }
            else if (errorCode == 2)
            {
              for (uint16_t x = 0; x < strlen_P(htmlLabelFail); x++)
                client.write(pgm_read_byte_near(htmlLabelFail + x));
            }
          }
          else if (k == 325)
          {
            if (!formId)
              formId = micros();
            char buff[12];
            itoa((uint32_t)formId, buff, 10);
            client.print(buff);
          }
          else if (k == 447)
          {
            // arduino ip 0:
            char buff[4];
            itoa(EEPROM.read(0), buff, 10);
            client.print(buff);
          }
          else if (k == 532)
          {
            // arduino ip 1:
            char buff[4];
            itoa(EEPROM.read(1), buff, 10);
            client.print(buff);
          }
          else if (k == 617)
          {
            // arduino ip 2:
            char buff[4];
            itoa(EEPROM.read(2), buff, 10);
            client.print(buff);
          }
          else if (k == 702)
          {
            // arduino ip 3:
            char buff[4];
            itoa(EEPROM.read(3), buff, 10);
            client.print(buff);
          }
          else if (k == 802 && checkMark)
          {
            client.print(" checked");
          }
          else if (k == 967)
          {
            // atem ip 0:
            char buff[4];
            itoa(EEPROM.read(4), buff, 10);
            client.print(buff);
          }
          else if (k == 1054)
          {
            // atem ip 1:
            char buff[4];
            itoa(EEPROM.read(5), buff, 10);
            client.print(buff);
          }
          else if (k == 1141)
          {
            // atem ip 2:
            char buff[4];
            itoa(EEPROM.read(6), buff, 10);
            client.print(buff);
          }
          else if (k == 1228)
          {
            // atem ip 3:
            char buff[4];
            itoa(EEPROM.read(7), buff, 10);
            client.print(buff);
          }
          client.write(pgm_read_byte_near(htmlFormTemplate + k));
          // else if(k == 345) {
          //   // another clipping
          // }
        }
      }
      else
      {
        html.clear();
      }
      break;
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

void runIpModeStatus()
{
  switch (ipMode)
  {
  case 1:
    // dhcp
    ledBuild.setPrioritySequence(dhcp, 0, true);
    break;
  case 2:
    // dhcp hybrid 1
    ledBuild.setPrioritySequence(dhcp_1, 0, true);
    break;
  case 3:
    // dhcp hybrid 2
    ledBuild.setPrioritySequence(dhcp_2, 0, true);
    break;
  default:
    // static Ip
    ledBuild.setPrioritySequence(staticIpMode, 0, true);
    break;
  }
}

void ledHandle()
{
  macro.run();
  ledBuild.run();

  if (ip_located)
  {
    if (!atem_connected)
    {
      ledBuild.setSequence(atemDisconnected, 0, true);
    }
    else
    {
      ledBuild.setSequence(atemConnected, 0, true);
    }
  }
  else
  {
    ledBuild.setSequence(obtainingIp, 0, true);
  }
  analogWrite(greenLed_pin, greenLed_var);
  analogWrite(orangeLed_pin, orangeLed_var);
}

inline bool allLedReady()
{
  return (macro.ready(greenLed_var) && macro.ready(orangeLed_var));
}

inline void setInterrupt(bool set)
{
  cli(); // stop interrupts
  if (set)
  {
    TIMSK1 |= (1 << OCIE1A); // allow compare match interrupt for "OCR1A"
    TCNT1 = 0;               // set counter to 0
  }
  else
  {
    TIMSK1 &= ~(1 << OCIE1A); // disable compare match interrupt for "OCR1A"
  }
  sei(); // allow interrupts
}

bool checkNum(char *arr)
{
  uint8_t x = 0;
  while (arr[x] != '\0')
  {
    if ((arr[x] < '0' || arr[x] > '9') && arr[x] != '-')
      return false;
    x++;
  }
  return true;
}

// interrupt vector:
ISR(TIMER1_COMPA_vect)
{
  buttonHandle();
  ledHandle();
}

SB_FUNCT(beginSequence, allLedReady())
SB_STEP(greenLed_var = 0;)
SB_STEP(macro.set(orangeLed_var, 255, 1000);)
SB_END

SB_FUNCT(obtainingIp, allLedReady())
SB_STEP(greenLed_var = 0;)
SB_STEP(macro.set(orangeLed_var, 255, 200);)
SB_STEP(macro.set(orangeLed_var, 0, 200);)
SB_STEP(macro.set(orangeLed_var, 255, 200);)
SB_STEP(macro.set(orangeLed_var, 0, 600);)
SB_STEP(ledBuild.loop(1);)
SB_END

SB_FUNCT(atemDisconnected, allLedReady())
SB_STEP(greenLed_var = 0;)
SB_STEP(macro.set(orangeLed_var, 0, 500);)
SB_STEP(macro.set(orangeLed_var, 255, 500);)
SB_STEP(ledBuild.loop(1);)
SB_END

SB_FUNCT(atemConnected, allLedReady())
SB_STEP(orangeLed_var = 0;)
SB_STEP(macro.set(greenLed_var, 0, 200);)
SB_STEP(greenLed_var = 255;)
SB_END

SB_FUNCT(staticIpMode, allLedReady())
SB_STEP(greenLed_var = 0;)
SB_STEP(macro.set(orangeLed_var, 0, 600);)
SB_STEP(macro.set(orangeLed_var, 255, 100);)
SB_STEP(macro.set(orangeLed_var, 0, 600);)
SB_END

SB_FUNCT(dhcp, allLedReady())
SB_STEP(orangeLed_var = 0;)
SB_STEP(macro.set(greenLed_var, 0, 600);)
SB_STEP(macro.set(greenLed_var, 255, 100);)
SB_STEP(macro.set(greenLed_var, 0, 600);)
SB_END

SB_FUNCT(dhcp_1, allLedReady())
SB_STEP(orangeLed_var = 0;)
SB_STEP(macro.set(greenLed_var, 0, 600);)
SB_STEP(macro.set(greenLed_var, 255, 100);)
SB_STEP(macro.set(greenLed_var, 0, 200);)
SB_STEP(macro.set(greenLed_var, 255, 100);)
SB_STEP(macro.set(greenLed_var, 0, 600);)
SB_END

SB_FUNCT(dhcp_2, allLedReady())
SB_STEP(orangeLed_var = 0;)
SB_STEP(macro.set(greenLed_var, 0, 600);)
SB_STEP(macro.set(greenLed_var, 255, 100);)
SB_STEP(macro.set(greenLed_var, 0, 200);)
SB_STEP(macro.set(greenLed_var, 255, 100);)
SB_STEP(macro.set(greenLed_var, 0, 200);)
SB_STEP(macro.set(greenLed_var, 255, 100);)
SB_STEP(macro.set(greenLed_var, 0, 600);)
SB_END