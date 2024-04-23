#ifndef MBUS_PACKET_H
#define MBUS_PACKET_H

#define PACKET_OK              0
#define PACKET_CODING_ERROR    1
#define PACKET_CRC_ERROR       2

#define RX_FIFO_THRESHOLD         0x07
#define RX_FIFO_START_THRESHOLD   0x00
#define RX_FIFO_SIZE              64
#define RX_OCCUPIED_FIFO          32    // Occupied space
#define RX_AVAILABLE_FIFO         32    // Free space

#define FIXED_PACKET_LENGTH       0x00
#define INFINITE_PACKET_LENGTH    0x02
#define INFINITE                  0
#define FIXED                     1
#define MAX_FIXED_LENGTH          256

#define RX_STATE_ERROR            3

typedef struct RXinfoDescr {
    uint8_t  lengthField;         // The L-field in the WMBUS packet
    uint16_t length;              // Total number of bytes to to be read from the RX FIFO
    uint16_t bytesLeft;           // Bytes left to to be read from the RX FIFO
    uint8_t *pByteIndex;          // Pointer to current position in the byte array
    uint8_t format;               // Infinite or fixed packet mode
    uint8_t start;                // Start of Packet
    uint8_t complete;             // Packet received complete
} RXinfoDescr;


uint16_t packetSize(uint8_t lField);
uint16_t byteSize(uint16_t packetSize);
uint16_t decodeRXBytesTmode(uint8_t* pByte, uint8_t* pPacket, uint16_t packetSize);

#endif