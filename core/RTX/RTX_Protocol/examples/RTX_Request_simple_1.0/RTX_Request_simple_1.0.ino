#define CONFIG_Sectors 1 // << 16 bytes
#define DEBUG_Sectors 8  // << 32 bytes
#define RTX_Name "Serial"
#define RTX_Version 1, 0, 0 // << v1.0.0

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\RTX\RTX_Protocol\RTX_Protocol_2.0.0.h"

RTX_Protocol _rtx(8, 0);
RTX_RequestProtocol rtx(_rtx);

void setup()
{
  Serial.begin(2000000);
  Serial.print("connecting: ");
  while (!_rtx.run())
    ;
  Serial.println("connected");

  {
    Serial.println("Requesting Info from ip 3:");
    uint32_t pingResult;

    if (rtx.ping(3, pingResult))
    {
      Serial.print("ping: ");
      Serial.print(pingResult);
      Serial.println("us");
    }
    else
    {
      Serial.println("ping failed");
    }
    char devName[7];
    if (rtx.deviceName(3, devName))
    {
      Serial.print("device name: ");
      Serial.println(devName);
    }
    else
    {
      Serial.println("name failed");
    }
    uint8_t version[6];
    uint8_t versionlen;
    if (rtx.version(3, version, versionlen))
    {
      Serial.print("device version: v");
      for (uint8_t x = 0; x < versionlen; x++)
      {
        Serial.print(version[x]);
        if (x < versionlen - 1)
          Serial.print('.');
      }
      Serial.println();
    }
    else
    {
      Serial.println("version failed");
    }
    uint8_t date[3];
    uint8_t time[3];
    if (rtx.dateTime(3, date, time))
    {
      Serial.print("device date-time(D-M-Y)(H-M-S): ");
      for (uint8_t x = 0; x < 3; x++)
      {
        Serial.print(date[x]);
        Serial.print(' ');
      }
      Serial.print(":: ");
      for (uint8_t x = 0; x < 3; x++)
      {
        Serial.print(time[x]);
        Serial.print(' ');
      }
      Serial.println();
    }
    else
    {
      Serial.println("dateTime failed");
    }
    Serial.println();
    if (rtx.EEPROM_updateStatus(3))
    {
      Serial.print("EEPROM writing: ");
      Serial.println(rtx.EEPROM_writing() ? "true" : "false");
      Serial.print("EEPROM error: ");
      Serial.println((rtx.EEPROM_error()) ? "error" : "no-error");
      Serial.print("EEPROM burn progress: ");
      Serial.println(rtx.EEPROM_burnProgress());
    }
    else
    {
      Serial.println("EEPROM status failed");
    }
    rtx.EEPROM_setError(3, false);
    if (rtx.EEPROM_updateStatus(3))
    {
      Serial.print("EEPROM writing: ");
      Serial.println(rtx.EEPROM_writing() ? "true" : "false");
      Serial.print("EEPROM error: ");
      Serial.println((rtx.EEPROM_error()) ? "error" : "no-error");
      Serial.print("EEPROM burn progress: ");
      Serial.println(rtx.EEPROM_burnProgress());
    }
    else
    {
      Serial.println("EEPROM status failed");
    }

    uint16_t bytes;
    if (rtx.debugBytes(3, bytes))
    {
      Serial.print("debugBytes: ");
      Serial.println(bytes);
    }
    else
    {
      Serial.println("debugBytes failed");
    }

    if (rtx.configBytes(3, bytes))
    {
      Serial.print("configBytes: ");
      Serial.println(bytes);
    }
    else
    {
      Serial.println("configBytes failed");
    }

    if (rtx.EEPROM_bytes(3, bytes))
    {
      Serial.print("EEPROM bytes: ");
      Serial.println(bytes);
    }
    else
    {
      Serial.println("EEPROM bytes failed");
    }

    uint32_t eeprom_writecycle;
    if (rtx.EEPROM_writeCount(3, eeprom_writecycle))
    {
      Serial.print("EEPROM write count: ");
      Serial.println(eeprom_writecycle);
    }
    else
    {
      Serial.println("EEPROM write count failed");
    }
  }
  Serial.println();
  Serial.println();
  Serial.println();

  {

    Serial.println("Requesting Info from ip 0:");
    uint32_t pingResult;

    if (rtx.ping(0, pingResult))
    {
      Serial.print("ping: ");
      Serial.print(pingResult);
      Serial.println("us");
    }
    else
    {
      Serial.println("ping failed");
    }
    char devName[7];
    if (rtx.deviceName(0, devName))
    {
      Serial.print("device name: ");
      Serial.println(devName);
    }
    else
    {
      Serial.println("name failed");
    }
    uint8_t version[6];
    uint8_t versionlen;
    if (rtx.version(0, version, versionlen))
    {
      Serial.print("device version: v");
      for (uint8_t x = 0; x < versionlen; x++)
      {
        Serial.print(version[x]);
        if (x < versionlen - 1)
          Serial.print('.');
      }
      Serial.println();
    }
    else
    {
      Serial.println("version failed");
    }
    uint8_t date[3];
    uint8_t time[3];
    if (rtx.dateTime(0, date, time))
    {
      Serial.print("device date-time(D-M-Y)(H-M-S): ");
      for (uint8_t x = 0; x < 3; x++)
      {
        Serial.print(date[x]);
        Serial.print(' ');
      }
      Serial.print(":: ");
      for (uint8_t x = 0; x < 3; x++)
      {
        Serial.print(time[x]);
        Serial.print(' ');
      }
      Serial.println();
    }
    else
    {
      Serial.println("dateTime failed");
    }
    Serial.println();
    if (rtx.EEPROM_updateStatus(0))
    {
      Serial.print("EEPROM writing: ");
      Serial.println(rtx.EEPROM_writing() ? "true" : "false");
      Serial.print("EEPROM error: ");
      Serial.println((rtx.EEPROM_error()) ? "error" : "no-error");
      Serial.print("EEPROM burn progress: ");
      Serial.println(rtx.EEPROM_burnProgress());
    }
    else
    {
      Serial.println("EEPROM status failed");
    }
    // rtx.EEPROM_setError(0, false);
    // if (rtx.EEPROM_updateStatus(0))
    // {
    //   Serial.print("EEPROM writing: ");
    //   Serial.println(rtx.EEPROM_writing() ? "true" : "false");
    //   Serial.print("EEPROM error: ");
    //   Serial.println((rtx.EEPROM_error()) ? "error" : "no-error");
    //   Serial.print("EEPROM burn progress: ");
    //   Serial.println(rtx.EEPROM_burnProgress());
    // }
    // else
    // {
    //   Serial.println("EEPROM status failed");
    // }

    uint16_t bytes;
    if (rtx.debugBytes(0, bytes))
    {
      Serial.print("debugBytes: ");
      Serial.println(bytes);
    }
    else
    {
      Serial.println("debugBytes failed");
    }

    if (rtx.configBytes(0, bytes))
    {
      Serial.print("configBytes: ");
      Serial.println(bytes);
    }
    else
    {
      Serial.println("configBytes failed");
    }

    if (rtx.EEPROM_bytes(0, bytes))
    {
      Serial.print("EEPROM bytes: ");
      Serial.println(bytes);
    }
    else
    {
      Serial.println("EEPROM bytes failed");
    }

    uint32_t eeprom_writecycle;
    if (rtx.EEPROM_writeCount(0, eeprom_writecycle))
    {
      Serial.print("EEPROM write count: ");
      Serial.println(eeprom_writecycle);
    }
    else
    {
      Serial.println("EEPROM write count failed");
    }
  }
}
void loop()
{
}