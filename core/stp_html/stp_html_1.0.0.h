// stp_html
//.h
/*
 */
#ifndef stp_html_h
#define stp_html_h

#include "Arduino.h"
#include <Ethernet.h>

class stp_html
{
public:
  /*
    stp_html():
      Create sender/parse object.
      input EthernetClient * to transmit data. char * (const char[] PROGMEM) document to be parsed and sent
      one time use object. Create a new one for each new document to be sent
  */
  stp_html(EthernetClient *, char *);

  /*
   available():
     call continuously until document has been sent.
     returns true if chars still available to send. false, if transmission is complete
     parses document, send approximately 1 char per call
  */
  bool available();

  /*
    isCode():
      returns if a code insert of any kind has been found
      initially false
      updates on each "available()" call
      returns true if code is found and available
      returns false if no code found
  */
  bool isCode();

  /*
    code():
      returns previously found code
      updates each time a code is found in the doc.
      supposed to be called ONLY if "isCode" returns true
  */
  uint8_t code();

  /*
    flush():
      send entire document in one go, does not parse or omits any characters
      Useful if you want to simply send out a document without needing to implement anything yourself
  */
  void flush();

private:
  EthernetClient *_client; // ethernet client object that the document will be sent to

  char *_doc; // pointer to document to be sent

  uint16_t _index = 0; // index of character being sent inm "_doc"

  uint8_t _scope = 0; // keep track of how many brackets we are nested in

  uint8_t _skipScopeNum; // keep track of which scope to skip

  bool _skipScope = false; // keep track of weather to skip the scope

  /*
    parses array within parenthesis () only in "_doc"
    requires that first char of index is the starting parenthesis (
    inputs starting index(must have a opening parenthesis at this location), outputs uint8_t chars of entire parenthesis group(useful for skipping those chars)
    returns int if valid integer found, returns -1 of error(largest integer possible is 3 digits - 0 - 999)(empty parenthesis will count as valid: 999)
  */
  int16_t _parseParen(uint16_t, uint8_t &);
};

//.cpp
stp_html::stp_html(EthernetClient *client, char *doc)
{
  _client = client;
  _doc = doc;
}
bool stp_html::available()
{
  if (_index < strlen_P(_doc))
  {
    bool skipCharSend = false;

    if (pgm_read_byte_near(_doc + _index) == '$')
    {
      if (pgm_read_byte_near(_doc + _index + 1) == '(')
      {
        uint8_t skipChars;
        int16_t intResult = _parseParen(_index + 1, skipChars);
        if (intResult != -1)
        {
          _index += (1 + skipChars);
          skipCharSend = true;
        }
      }
      else if (pgm_read_byte_near(_doc + _index + 1) == '<')
      {
        uint8_t skipChars;
        int16_t intResult = _parseParen(_index + 2, skipChars);
        if (intResult != -1)
        {
          _index += (2 + skipChars);
          skipCharSend = true;
        }
      }
    }
    else if (pgm_read_byte_near(_doc + _index) == '>' && pgm_read_byte_near(_doc + _index + 1) == '$')
    {
      _index += 2;

      if (_scope > 0)
        _scope--;

      if (_skipScope)
      {
        if (_scope < _skipScopeNum)
        {
        }
      }
    }

    if (!skipCharSend && !_skipScope)
    {
      _client->write(pgm_read_byte_near(_doc + _index));
      _index++;
    }

    return true;
  }
  else
    return false;
}
bool stp_html::isCode()
{
}
uint8_t stp_html::code()
{
}
void stp_html::flush()
{
}

int16_t stp_html::_parseParen(uint16_t startIndex, uint8_t &charTotal)
{
  charTotal = 0; // Count total number of chars parsed

  int16_t finalVal = 0; // Put final value to be returned here
  if (strlen_P(_doc) > startIndex)
  {
    if (pgm_read_byte_near(_doc + startIndex) == '(')
    {
      charTotal++;
      while (true)
      {
        startIndex++;
        if (startIndex >= strlen_P(_doc)) // first, always check that index is within bounds
        {
          return -1;
        }

        // If index is within bounds, check the character:
        charTotal++;
        // if end parenthesis located, return value:
        if (pgm_read_byte_near(_doc + startIndex) == ')')
        {
          if (charTotal == 2)
            return 999; // return largest number possible for empty parenthesis
          else
            return finalVal; // return value
        }
        // check if character is a number:
        else if (pgm_read_byte_near(_doc + startIndex) >= '0' && pgm_read_byte_near(_doc + startIndex) <= '9')
        {
          if (charTotal >= 5)
            return -1;

          finalVal *= 10;
          finalVal += pgm_read_byte_near(_doc + startIndex) - '0';
        }
        // unrecognized character, return error:
        else
        {
          return -1;
        }
      }
    }
  }
  return -1;
}
#endif
