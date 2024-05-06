#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>

#include <cc1101.h>
#include <3outof6.h>
#include <mbus_packet.h>

#define CC1101_GDO0 17
#define CC1101_GDO2 4

const char *cold_meter_id = "00112233";
const char *warm_meter_id = "22334455";

// RX - Buffers
uint8_t RXpacket[291];
uint8_t RXbytes[584];

RXinfoDescr RXinfo;

bool receiving = false;

IRAM_ATTR void rxFifoISR()
{

  uint8_t fixedLength;
  uint8_t bytesDecoded[2];

  // - RX FIFO 4 bytes detected -
  // Calculate the total length of the packet, and set fixed mode if less
  // than 255 bytes to receive
  if (RXinfo.start == true)
  {
    // Read the 3 first bytes
    cc1101_readBurstReg(RXinfo.pByteIndex, CC1101_RXFIFO, 3);

    // - Calculate the total number of bytes to receive -
    // Possible improvment: Check the return value from the deocding function,
    // and abort RX if coding error.
    decode3outof6(RXinfo.pByteIndex, bytesDecoded, 0);
    RXinfo.lengthField = bytesDecoded[0];
    RXinfo.length = byteSize((packetSize(RXinfo.lengthField)));

    // - Length mode -
    // Set fixed packet length mode is less than 256 bytes
    if (RXinfo.length < (MAX_FIXED_LENGTH))
    {
      cc1101_writeReg(CC1101_PKTLEN, (uint8_t)(RXinfo.length));
      cc1101_writeReg(CC1101_PKTCTRL0, FIXED_PACKET_LENGTH);
      RXinfo.format = FIXED;
    }

    // Infinite packet length mode is more than 255 bytes
    // Calculate the PKTLEN value
    else
    {
      fixedLength = RXinfo.length % (MAX_FIXED_LENGTH);
      cc1101_writeReg(CC1101_PKTLEN, (uint8_t)(fixedLength));
    }

    RXinfo.pByteIndex += 3;
    RXinfo.bytesLeft = RXinfo.length - 3;

    // Set RX FIFO threshold to 32 bytes
    RXinfo.start = false;
    cc1101_writeReg(CC1101_FIFOTHR, RX_FIFO_THRESHOLD);
  }

  // - RX FIFO Half Full detected -
  // Read out the RX FIFO and set fixed mode if less
  // than 255 bytes to receive
  else
  {
    // - Length mode -
    // Set fixed packet length mode is less than 256 bytes
    if (((RXinfo.bytesLeft) < (MAX_FIXED_LENGTH)) && (RXinfo.format == INFINITE))
    {
      cc1101_writeReg(CC1101_PKTCTRL0, FIXED_PACKET_LENGTH);
      RXinfo.format = FIXED;
    }

    // Read out the RX FIFO
    // Do not empty the FIFO (See the CC110x or 2500 Errata Note)
    cc1101_readBurstReg(RXinfo.pByteIndex, CC1101_RXFIFO, RX_AVAILABLE_FIFO - 1);

    RXinfo.bytesLeft -= (RX_AVAILABLE_FIFO - 1);
    RXinfo.pByteIndex += (RX_AVAILABLE_FIFO - 1);
  }
}

IRAM_ATTR void rxPacketRecvdISR(void)
{
  // This function is called when the complete packet has been received.
  // The remaining bytes in the RX FIFO are read out, and packet complete signalized

  // Read remaining bytes in RX FIFO
  cc1101_readBurstReg(RXinfo.pByteIndex, CC1101_RXFIFO, (uint8_t)RXinfo.bytesLeft);
  RXinfo.complete = true;
}

uint16_t startReceiving(uint8_t *packet, uint8_t *bytes)
{
  uint16_t rxStatus;

  // Initialize RX info variable
  RXinfo.lengthField = 0;    // Length Field in the wireless MBUS packet
  RXinfo.length = 0;         // Total length of bytes to receive packet
  RXinfo.bytesLeft = 0;      // Bytes left to to be read from the RX FIFO
  RXinfo.pByteIndex = bytes; // Pointer to current position in the byte array
  RXinfo.format = INFINITE;  // Infinite or fixed packet mode
  RXinfo.start = true;       // Sync or End of Packet
  RXinfo.complete = false;   // Packet Received

  // Set RX FIFO threshold to 4 bytes
  cc1101_writeReg(CC1101_FIFOTHR, RX_FIFO_START_THRESHOLD);

  // Set infinite length
  cc1101_writeReg(CC1101_PKTCTRL0, INFINITE_PACKET_LENGTH);

  // Check RX Status
  rxStatus = cc1101_readReg(CC1101_SNOP, READ_SINGLE);
  if ((rxStatus & 0x70) != 0)
  {
    // Abort if not in IDLE
    cc1101_cmdStrobe(CC1101_SIDLE); // Enter IDLE state
    return (RX_STATE_ERROR);
  }

  // Flush RX FIFO
  // Ensure that FIFO is empty before reception is started
  cc1101_cmdStrobe(CC1101_SFRX); // flush receive queue

  attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), rxFifoISR, RISING);
  attachInterrupt(digitalPinToInterrupt(CC1101_GDO2), rxPacketRecvdISR, FALLING);

  // Strobe RX
  cc1101_cmdStrobe(CC1101_SRX); // Enter RX state

  // Wait for FIFO being filled
  // while (RXinfo.complete != true)
  //{
  // delay(1);
  //}

  return (PACKET_OK);
}

uint16_t stopReceiving(uint8_t *packet, uint8_t *bytes)
{

  uint16_t rxStatus;

  detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));
  detachInterrupt(digitalPinToInterrupt(CC1101_GDO2));

  // Check that transceiver is in IDLE
  rxStatus = cc1101_readReg(CC1101_SNOP, READ_SINGLE);
  if ((rxStatus & 0x70) != 0)
  {
    cc1101_cmdStrobe(CC1101_SIDLE); // Enter IDLE state
    return (RX_STATE_ERROR);
  }

  rxStatus = decodeRXBytesTmode(bytes, packet, packetSize(RXinfo.lengthField));

  return (rxStatus);
}

// Decode mkradio4 telegrams
void decodeTechemWaterMeters()
{
  // The right packet size
  if (RXpacket[0] == 0x2f)
  {
    // manufacturer TCH (0x6850)
    if (RXpacket[2] == 0x68 && RXpacket[3] == 0x50)
    {
      Serial.println();
      Serial.print("Techem ID: ");

      // You can use int instead of char array
      // unsigned int device_id = (RXpacket[7] << 24) | (RXpacket[6] << 16) | (RXpacket[5] << 8) | RXpacket[4];

      char device_id[10];
      sprintf(device_id, "%02x%02x%02x%02x", RXpacket[7], RXpacket[6], RXpacket[5], RXpacket[4]);
      Serial.print(device_id);

      Serial.print(" Type: ");
      if (RXpacket[9] == 0x72)
      {
        Serial.print("Cold");
      }
      else if (RXpacket[9] == 0x62)
      {
        Serial.print("Warm");
      }
      else
      {
        Serial.print("Unknown");
      }

      uint8_t prev_lo = RXpacket[16];
      uint8_t prev_hi = RXpacket[17];
      float prev = (256.0 * prev_hi + prev_lo) / 10.0;

      uint8_t curr_lo = RXpacket[20];
      uint8_t curr_hi = RXpacket[21];
      float curr = (256.0 * curr_hi + curr_lo) / 10.0;

      float total_water_consumption_m3 = prev + curr;
      float target_water_consumption_m3 = prev;

      Serial.print("Calc Total: ");
      Serial.print(total_water_consumption_m3);
      Serial.print(" Target: ");
      Serial.println(target_water_consumption_m3);

      if (strcmp(device_id, cold_meter_id) == 0)
      {
        // Log cold meter
      }
      else if (strcmp(device_id, warm_meter_id) == 0)
      {
        // Log warm meter
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(SS, OUTPUT);
  pinMode(CC1101_GDO0, INPUT);
  pinMode(CC1101_GDO2, INPUT);

  SPI.begin(); // Initialize SPI interface

  // Disable WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  Serial.println("Start!");

  // Reset the CC1101 Tranceiver
  Serial.println("Resetting CC1101");
  cc1101_reset();

  // Tranceiver ID
  uint8_t id = cc1101_readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER);
  uint8_t ver = cc1101_readReg(CC1101_VERSION, CC1101_STATUS_REGISTER);

  Serial.print("CC1101 ID: ");
  Serial.print(id);
  Serial.print(" Ver: ");
  Serial.println(ver);

  // T-mode selected
  cc1101_initRegisters();
  Serial.println("Init CC1101 registers");
}

void loop()
{

  if (!receiving)
  {
    // Await packet received
    uint16_t status = startReceiving(RXpacket, RXbytes);
    if (status == PACKET_OK)
    {
      receiving = true;
    }
  }
  else if (RXinfo.complete)
  {

    receiving = false;

    uint16_t status = stopReceiving(RXpacket, RXbytes);

    // Update display if packet successfully received and decoded
    if (status == PACKET_OK)
    {
      // Send the received Wireless MBUS packet to the UART
      for (int i = 0; i < packetSize(RXpacket[0]); i++)
      {
        if (RXpacket[i] < 16)
        {
          Serial.print("0");
        }
        Serial.print(RXpacket[i], HEX);
      }

      // Decode mkradio4 telegrams
      decodeTechemWaterMeters();

      Serial.println();
    }
  }
}