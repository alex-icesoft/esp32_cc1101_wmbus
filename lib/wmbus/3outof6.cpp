#include <Arduino.h>
#include <3outof6.h>

// Table for encoding for a 4-bit data into 6-bit
// "3 out of 6" coded data.
static uint8_t encodeTab[16] = {0x16,  // 0x0 "3 out of 6" encoded
                                0x0D,  // 0x1 "3 out of 6" encoded
                                0x0E,  // 0x2 "3 out of 6" encoded
                                0x0B,  // 0x3 "3 out of 6" encoded
                                0x1C,  // 0x4 "3 out of 6" encoded
                                0x19,  // 0x5 "3 out of 6" encoded
                                0x1A,  // 0x6 "3 out of 6" encoded
                                0x13,  // 0x7 "3 out of 6" encoded
                                0x2C,  // 0x8 "3 out of 6" encoded
                                0x25,  // 0x9 "3 out of 6" encoded
                                0x26,  // 0xA "3 out of 6" encoded
                                0x23,  // 0xB "3 out of 6" encoded
                                0x34,  // 0xC "3 out of 6" encoded
                                0x31,  // 0xD "3 out of 6" encoded
                                0x32,  // 0xE "3 out of 6" encoded
                                0x29}; // 0xF "3 out of 6" encoded

// Table for decoding a 6-bit "3 out of 6" encoded data into 4-bit
// data. The value 0xFF indicates invalid "3 out of 6" coding
static uint8_t decodeTab[64] = {0xFF,  //  "3 out of 6" encoded 0x00 decoded
                                0xFF,  //  "3 out of 6" encoded 0x01 decoded
                                0xFF,  //  "3 out of 6" encoded 0x02 decoded
                                0xFF,  //  "3 out of 6" encoded 0x03 decoded
                                0xFF,  //  "3 out of 6" encoded 0x04 decoded
                                0xFF,  //  "3 out of 6" encoded 0x05 decoded
                                0xFF,  //  "3 out of 6" encoded 0x06 decoded
                                0xFF,  //  "3 out of 6" encoded 0x07 decoded
                                0xFF,  //  "3 out of 6" encoded 0x08 decoded
                                0xFF,  //  "3 out of 6" encoded 0x09 decoded
                                0xFF,  //  "3 out of 6" encoded 0x0A decoded
                                0x03,  //  "3 out of 6" encoded 0x0B decoded
                                0xFF,  //  "3 out of 6" encoded 0x0C decoded
                                0x01,  //  "3 out of 6" encoded 0x0D decoded
                                0x02,  //  "3 out of 6" encoded 0x0E decoded
                                0xFF,  //  "3 out of 6" encoded 0x0F decoded
                                0xFF,  //  "3 out of 6" encoded 0x10 decoded
                                0xFF,  //  "3 out of 6" encoded 0x11 decoded
                                0xFF,  //  "3 out of 6" encoded 0x12 decoded
                                0x07,  //  "3 out of 6" encoded 0x13 decoded
                                0xFF,  //  "3 out of 6" encoded 0x14 decoded
                                0xFF,  //  "3 out of 6" encoded 0x15 decoded
                                0x00,  //  "3 out of 6" encoded 0x16 decoded
                                0xFF,  //  "3 out of 6" encoded 0x17 decoded
                                0xFF,  //  "3 out of 6" encoded 0x18 decoded
                                0x05,  //  "3 out of 6" encoded 0x19 decoded
                                0x06,  //  "3 out of 6" encoded 0x1A decoded
                                0xFF,  //  "3 out of 6" encoded 0x1B decoded
                                0x04,  //  "3 out of 6" encoded 0x1C decoded
                                0xFF,  //  "3 out of 6" encoded 0x1D decoded
                                0xFF,  //  "3 out of 6" encoded 0x1E decoded
                                0xFF,  //  "3 out of 6" encoded 0x1F decoded
                                0xFF,  //  "3 out of 6" encoded 0x20 decoded
                                0xFF,  //  "3 out of 6" encoded 0x21 decoded
                                0xFF,  //  "3 out of 6" encoded 0x22 decoded
                                0x0B,  //  "3 out of 6" encoded 0x23 decoded
                                0xFF,  //  "3 out of 6" encoded 0x24 decoded
                                0x09,  //  "3 out of 6" encoded 0x25 decoded
                                0x0A,  //  "3 out of 6" encoded 0x26 decoded
                                0xFF,  //  "3 out of 6" encoded 0x27 decoded
                                0xFF,  //  "3 out of 6" encoded 0x28 decoded
                                0x0F,  //  "3 out of 6" encoded 0x29 decoded
                                0xFF,  //  "3 out of 6" encoded 0x2A decoded
                                0xFF,  //  "3 out of 6" encoded 0x2B decoded
                                0x08,  //  "3 out of 6" encoded 0x2C decoded
                                0xFF,  //  "3 out of 6" encoded 0x2D decoded
                                0xFF,  //  "3 out of 6" encoded 0x2E decoded
                                0xFF,  //  "3 out of 6" encoded 0x2F decoded
                                0xFF,  //  "3 out of 6" encoded 0x30 decoded
                                0x0D,  //  "3 out of 6" encoded 0x31 decoded
                                0x0E,  //  "3 out of 6" encoded 0x32 decoded
                                0xFF,  //  "3 out of 6" encoded 0x33 decoded
                                0x0C,  //  "3 out of 6" encoded 0x34 decoded
                                0xFF,  //  "3 out of 6" encoded 0x35 decoded
                                0xFF,  //  "3 out of 6" encoded 0x36 decoded
                                0xFF,  //  "3 out of 6" encoded 0x37 decoded
                                0xFF,  //  "3 out of 6" encoded 0x38 decoded
                                0xFF,  //  "3 out of 6" encoded 0x39 decoded
                                0xFF,  //  "3 out of 6" encoded 0x3A decoded
                                0xFF,  //  "3 out of 6" encoded 0x3B decoded
                                0xFF,  //  "3 out of 6" encoded 0x3C decoded
                                0xFF,  //  "3 out of 6" encoded 0x3D decoded
                                0xFF,  //  "3 out of 6" encoded 0x3E decoded
                                0xFF}; // "3 out of 6" encoded 0x3F decoded

// Performs the "3 out 6" encoding on a 16-bit data value into a
// 24-bit data value. When encoding on a 8 bit variable, a postamle
// sequence is added.
void encode3outof6(uint8_t *uncodedData, uint8_t *encodedData, uint8_t lastByte)
{
  uint8_t data[4];

  // - Perform encoding -

  // If last byte insert postamble sequence
  if (lastByte)
  {
    data[1] = 0x14;
  }
  else
  {
    data[0] = encodeTab[*(uncodedData + 1) & 0x0F];
    data[1] = encodeTab[(*(uncodedData + 1) >> 4) & 0x0F];
  }

  data[2] = encodeTab[(*uncodedData) & 0x0F];
  data[3] = encodeTab[((*uncodedData) >> 4) & 0x0F];

  // - Shift the encoded 6-bit values into a byte buffer -
  *(encodedData + 0) = (data[3] << 2) | (data[2] >> 4);
  *(encodedData + 1) = (data[2] << 4) | (data[1] >> 2);

  if (!lastByte)
  {
    *(encodedData + 2) = (data[1] << 6) | data[0];
  }
}

// Performs the "3 out 6" decoding of a 24-bit data value into 16-bit
// data value. If only 2 byte left to decoded,
// the postamble sequence is ignored
uint8_t decode3outof6(uint8_t *encodedData, uint8_t *decodedData, uint8_t lastByte)
{

  uint8_t data[4];

  // - Perform decoding on the input data -
  if (!lastByte)
  {
    data[0] = decodeTab[(*(encodedData + 2) & 0x3F)];
    data[1] = decodeTab[((*(encodedData + 2) & 0xC0) >> 6) | ((*(encodedData + 1) & 0x0F) << 2)];
  }
  // If last byte, ignore postamble sequence
  else
  {
    data[0] = 0x00;
    data[1] = 0x00;
  }

  data[2] = decodeTab[((*(encodedData + 1) & 0xF0) >> 4) | ((*encodedData & 0x03) << 4)];
  data[3] = decodeTab[((*encodedData & 0xFC) >> 2)];

  // - Check for invalid data coding -
  if ((data[0] == 0xFF) | (data[1] == 0xFF) | (data[2] == 0xFF) | (data[3] == 0xFF))
  {
    return (DECODING_3OUTOF6_ERROR);
  }

  // - Shift the encoded values into a byte buffer -
  *decodedData = (data[3] << 4) | (data[2]);
  if (!lastByte)
  {
    *(decodedData + 1) = (data[1] << 4) | (data[0]);
  }

  return (DECODING_3OUTOF6_OK);
}