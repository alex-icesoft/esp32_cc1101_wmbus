#include <Arduino.h>
#include <mbus_packet.h>
#include <3outof6.h>
#include <crc.h>

// Returns the number of bytes in a Wireless MBUS packet from the L-field.
// Note that the L-field excludes the L-field and the CRC fields
uint16_t packetSize(uint8_t lField)
{
  uint16_t nrBytes;
  uint8_t nrBlocks;

  // The 2 first blocks contains 25 bytes when excluding CRC and the L-field
  // The other blocks contains 16 bytes when excluding the CRC-fields
  // Less than 26 (15 + 10)
  if (lField < 26)
    nrBlocks = 2;
  else
    nrBlocks = (((lField - 26) / 16) + 3);

  // Add all extra fields, excluding the CRC fields
  nrBytes = lField + 1;

  // Add the CRC fields, each block is contains 2 CRC bytes
  nrBytes += (2 * nrBlocks);

  return (nrBytes);
}

// Returns the total number of encoded bytes to receive or transmit, given the
// total number of bytes in a Wireless MBUS packet.
// In receive mode the postamble sequence and synchronization word is excluded
// from the calculation.
uint16_t byteSize(uint16_t packetSize)
{
  // Receive T-mode
  // Data is 3 out of 6 coded
  uint16_t tmodeVar = (3 * packetSize) / 2;

  // If packetsize is an odd number 1 extra byte
  // that includes the 4-postamble sequence must be read.

  if (packetSize % 2)
    return (tmodeVar + 1);
  else
    return (tmodeVar);
}

// Decode a TMODE packet into a Wireless MBUS packet. Checks for 3 out of 6
// decoding errors and CRC errors.
uint16_t decodeRXBytesTmode(uint8_t *pByte, uint8_t *pPacket, uint16_t packetSize)
{

  uint16_t bytesRemaining;
  uint16_t bytesEncoded;
  uint16_t decodingStatus;
  uint16_t crc;      // Current CRC value
  uint16_t crcField; // Current fields are a CRC field

  bytesRemaining = packetSize;
  bytesEncoded = 0;
  crcField = 0;
  crc = 0;

  // Decode packet
  while (bytesRemaining)
  {
    // If last byte
    if (bytesRemaining == 1)
    {
      decodingStatus = decode3outof6(pByte, pPacket, 1);

      // Check for valid 3 out of 6 decoding
      if (decodingStatus != DECODING_3OUTOF6_OK)
        return (PACKET_CODING_ERROR);

      bytesRemaining -= 1;
      bytesEncoded += 1;

      // The last byte the low byte of the CRC field
      if (((~crc) & 0xFF) != *(pPacket))
        return (PACKET_CRC_ERROR);
    }
    else
    {
      decodingStatus = decode3outof6(pByte, pPacket, 0);

      // Check for valid 3 out of 6 decoding
      if (decodingStatus != DECODING_3OUTOF6_OK)
        return (PACKET_CODING_ERROR);

      bytesRemaining -= 2;
      bytesEncoded += 2;

      // Check if current field is CRC fields
      // - Field 10 + 18*n
      // - Less than 2 bytes
      if (bytesRemaining == 0)
        crcField = 1;
      else if (bytesEncoded > 10)
        crcField = !((bytesEncoded - 12) % 18);

      // Check CRC field
      if (crcField)
      {
        if (((~crc) & 0xFF) != *(pPacket + 1))
          return (PACKET_CRC_ERROR);

        if ((((~crc) >> 8) & 0xFF) != *pPacket)
          return (PACKET_CRC_ERROR);

        crcField = 0;
        crc = 0;
      }
      else if (bytesRemaining == 1)
      {
        // If 1 bytes left, the field is the high byte of the CRC
        crc = crcCalc(crc, *(pPacket));
        // The packet byte is a CRC-field
        if ((((~crc) >> 8) & 0xFF) != *(pPacket + 1))
          return (PACKET_CRC_ERROR);
      }
      else
      {
        // Perform CRC calculation
        crc = crcCalc(crc, *(pPacket));
        crc = crcCalc(crc, *(pPacket + 1));
      }

      pByte += 3;
      pPacket += 2;
    }
  }

  return (PACKET_OK);
}