#ifndef CRC_H
#define CRC_H

#define CRC_POLYNOM         0x3D65

uint16_t crcCalc(uint16_t crcReg, uint8_t crcData); 

#endif