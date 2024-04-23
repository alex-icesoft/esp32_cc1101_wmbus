#ifndef _3OUTOF6_H
#define _3OUTOF6_H

#define DECODING_3OUTOF6_OK      0
#define DECODING_3OUTOF6_ERROR   1

void encode3outof6 (uint8_t *uncodedData, uint8_t *encodedData, uint8_t lastByte);
uint8_t decode3outof6(uint8_t *encodedData, uint8_t *decodedData, uint8_t lastByte);

#endif