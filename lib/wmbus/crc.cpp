#include <Arduino.h>
#include <crc.h>

// Calculates the 16-bit CRC. The function requires that the CRC_POLYNOM is defined,
// which gives the wanted CRC polynom. 
uint16_t crcCalc(uint16_t crcReg, uint8_t crcData) 
{
  uint8_t i;

  for (i = 0; i < 8; i++) 
  {
    // If upper most bit is 1
    if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
      crcReg = (crcReg << 1)  ^ CRC_POLYNOM;
    else
      crcReg = (crcReg << 1);

    crcData <<= 1;
  }
  
  return crcReg;
}