// file_name
//.h
#ifndef HTML_Parse_h
#define HTML_Parse_h

#include "Arduino.h"
#include <Ethernet.h>

#define HTML_Parse_MAX_CHARS 20 // not including terminating char

class HTML_Parse
{
public:
  /*
    HTML_Parse():
      input client object
  */
  HTML_Parse(EthernetClient *);

  /*
    getMethod():
      This should be called at the beginning of any request
      It will reset validation and prepare to read other arguments

      Will determine wether this is a GET method or not
      DOES NOT determine if this is a POST, PUT, or DELETE. only GET
      return true if GET method, false otherwise
  */
  bool getMethod();

  /*
    formTitle()
      input char array with minimum length of "HTML_Parse_MAX_CHARS + 1"
      if form title is over length or invalid char is located, entire request will be invalidated
      This method should be called first, BEFORE "nextParam()" or "clear()"
      returns true if valid and title obtained, false(also means entire request is invalid)
  */
  bool formTitle(char *);
  /*
    nextParam():
      input char arrays with minimum length of "HTML_Parse_MAX_CHARS + 1" for param title and value
      if title or value is over length or invalid char is located, entire request will be invalidated
      This method should be called after "formTitle()" and until it returns false
      returns true if valid param argument is found, false if reached end of parameters or invalid
      use "validate()" to check
  */
  bool nextParam(char *, char *);

  /*
    clear():
      This should be called AFTER "formTitle()" and "nextParam() or if request is invalid
      It will flush out the rest of the data in request until end of request reached or not more data left
      It will not close connection and can be used to
  */
  void clear();

  /*
    Will return true if request is still valid, false otherwise
  */
  bool validate();

private:
  EthernetClient *_client;

  bool _valid = true;
  bool _endOfParams = false;
};

//.cpp

HTML_Parse::HTML_Parse(EthernetClient *clientPnt)
{
  _valid = true;
  _endOfParams = false;
  _client = clientPnt;
}

bool HTML_Parse::getMethod()
{
  _valid = true;
  _endOfParams = false;
  bool firstChar = false;
  while (_client->connected())
  {
    if (firstChar)
    {
      if (_client->available())
      {
        char c = _client->read();
        if (c == '/')
          return true;
        else if (c == '\n' || c == '\r')
          break;
      }
    }
    else
    {
      if (_client->available())
      {
        firstChar = true;
        if (_client->read() != 'G')
        {
          _valid = false;
          return false;
        }
      }
    }
  }
  _valid = false;
  return false;
}

bool HTML_Parse::formTitle(char *title)
{
  if (_valid)
  {
    uint8_t len = 0;
    while (_client->connected())
    {
      if (_client->available())
      {
        char c = _client->read();
        if (c == '?')
        {
          title[len] = '\0';
          return true;
        }
        // prohibited characters:
        else if (c == '=' || c == '+' ||
                 c == ' ' || c == '/' ||
                 c == '&' || c == '\n' ||
                 c == '\r' || c == -1)
        {
          _valid = false;
          return false;
        }
        else
        {
          if (len < HTML_Parse_MAX_CHARS)
          {
            title[len] = c;
            len++;
          }
          else
          {
            // too many chars:
            _valid = false;
            return false;
          }
        }
      }
    }
  }
  _valid = false;
  return false;
}
bool HTML_Parse::nextParam(char *param, char *arg)
{
  if (_valid)
  {
    if (_endOfParams)
    {
      return false;
    }
    uint8_t len = 0;
    bool foundParam = false;
    while (_client->connected())
    {
      if (_client->available())
      {
        char c = _client->read();
        if (!foundParam) // find parameter
        {
          if (c == '=') // end of parameter
          {
            foundParam = true;
            param[len] = '\0';
            len = 0;
          }
          else if (c == '?' || c == '+' ||
                   c == ' ' || c == '/' ||
                   c == '&' || c == '\n' ||
                   c == '\r' || c == -1) // invalid chars
          {
            _valid = false;
            return false;
          }
          else // valid char
          {
            if (len < HTML_Parse_MAX_CHARS)
            {
              param[len] = c;
              len++;
            }
            else
            {
              // too many chars:
              _valid = false;
              return false;
            }
          }
        }
        else // find argument
        {
          if (c == '&' || c == ' ') // end of argument
          {
            arg[len] = '\0';
            if (c == ' ')
              _endOfParams = true;
            return true;
          }
          else if (c == '?' || c == '+' ||
                   c == '=' || c == '/' ||
                   c == '\n' || c == '\r' ||
                   c == -1) // invalid chars
          {
            _valid = false;
            return false;
          }
          else // valid char
          {
            if (len < HTML_Parse_MAX_CHARS)
            {
              arg[len] = c;
              len++;
            }
            else
            {
              // too many chars:
              _valid = false;
              return false;
            }
          }
        }
      }
    }
  }
  _valid = false;
  return false;
}

void HTML_Parse::clear()
{
  bool currentLineIsBlank = true;
  while (_client->connected())
  {
    if (_client->available())
    {
      char c = _client->read();
      if (c == '\n' && currentLineIsBlank)
        return;
      if (c == '\n')
        currentLineIsBlank = true;
      else if (c != '\r')
        currentLineIsBlank = false;
    }
  }
  return;
}

bool HTML_Parse::validate()
{
  return _valid;
}

#endif
