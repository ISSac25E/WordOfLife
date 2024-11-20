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
   run():
     call continuously until document has been sent.
     returns true if code has been found, returns false when the end of the document has been reached
     parses document, sends all characters until a code has been reached or end of document
     intended to be used in a while loop:

        while(stp_html.run())
        {
          switch(stp_html.code())
          {...}
        }
        // document done parsing

  */
  bool run();

  /*
    code():
      returns previously found code
      updates each time a code is found in the doc.
      supposed to be called ONLY if "isCode" returns true
  */
  uint8_t code();

  /*
    sendChar():
      insert a single char after specific code has been found
      can call as many times
      if code has been found in a scope, as soon as a char or string has been sent,
      the scope will be completely skipped
      input single char
  */
  void sendChar(char);

  /*
    sendStr():
      insert a string or char array(terminated with '\0') when a code has been found
      will not print the termination char
      can call as many times needed
      if code has been found in a scope, as soon as a char or string has been sent,
      the scope will be completely skipped
      input char array pointer
  */
  void sendStr(char *);

  /*
    sendConst():
      insert a rawLiteral PROGMEM char array
      can call as many times needed
      if code has been found in a scope, as soon as a char or string has been sent,
      the scope will be completely skipped
      input rawLiteral PROGMEM
  */
  void sendConst(char *);

  /*
    sendArr():
      insert a character array with length declared to be sent
      can call as many times needed
      if code has been found in a scope, as soon as a char or string has been sent,
      the scope will be completely skipped
      input char array pointer, length of char array(to be sent)
  */
  void sendArr(char *, uint16_t);

  /*
    flush():
      send entire document in one go, does not parse or omit any characters
      Useful if you want to simply send out a document without needing to implement anything yourself
  */
  void flush();

private:
  EthernetClient *_client; // ethernet client object that the document will be sent to

  char *_doc; // pointer to document to be sent

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
bool stp_html::run()
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
