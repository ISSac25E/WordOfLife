// BoolConverter
//.h
#ifndef BoolConverterClass_h
#define BoolConverterClass_h

#include "Arduino.h"

class BoolConverterClass
{
public:
  void compileVal(bool *, uint8_t &, uint8_t, uint8_t);       // bitArray, bitMarker, val(to compile), number of bits to compile
  void compileArray(bool *, uint8_t &, uint8_t *, uint8_t);   // bitArray, bitmarker, byte array(to compile), bytes to compile
  void decompileVal(bool *, uint8_t &, uint8_t &, uint8_t);   // bitArray, bitMarker, decompile result(val), number of bits to decompile
  void decompileArray(bool *, uint8_t &, uint8_t *, uint8_t); // bitArray, bitmarker, decompiled result(byte array), bytes to decompile
};
extern BoolConverterClass BoolConverter;

//.cpp

void BoolConverterClass::compileVal(bool *bitArray, uint8_t &bitMarker, uint8_t val, uint8_t bitsToCompile)
{
  for (uint8_t x = 0; x < bitsToCompile; x++)
  {
    bitArray[bitMarker] = (val & (1 << x));
    bitMarker++;
  }
}
void BoolConverterClass::compileArray(bool *bitArray, uint8_t &bitMarker, uint8_t *array, uint8_t bytesToCompile)
{
  for (uint8_t x = 0; x < bytesToCompile; x++)
  {
    bitArray[bitMarker] = (array[x] & (1 << 0));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 1));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 2));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 3));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 4));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 5));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 6));
    bitMarker++;
    bitArray[bitMarker] = (array[x] & (1 << 7));
    bitMarker++;
  }
}
void BoolConverterClass::decompileVal(bool *bitArray, uint8_t &bitMarker, uint8_t &val, uint8_t bitsToDecompile)
{
  val = 0;
  for (uint8_t x = 0; x < bitsToDecompile; x++)
  {
    val |= (bitArray[bitMarker] << x);
    bitMarker++;
  }
}
void BoolConverterClass::decompileArray(bool *bitArray, uint8_t &bitMarker, uint8_t *array, uint8_t bytesToDecompile)
{
  for (uint8_t x = 0; x < bytesToDecompile; x++)
  {
    array[x] = 0;
    array[x] |= (bitArray[bitMarker] << 0);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 1);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 2);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 3);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 4);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 5);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 6);
    bitMarker++;
    array[x] |= (bitArray[bitMarker] << 7);
    bitMarker++;
  }
}
#endif
