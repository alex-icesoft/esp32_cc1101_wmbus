#include <Arduino.h>
#include <SPI.h>
#include <cc1101.h>

// ChipSelect assert
void cc1101_chipSelect(void)
{
    digitalWrite(SS, LOW);
}

// ChipSelect deassert
void cc1101_chipDeselect(void)
{
    digitalWrite(SS, HIGH);
}

// wait for MISO pulling down
void cc1101_waitMiso(void)
{
    while (digitalRead(MISO) == HIGH)
        ;
}

// write a single register of CC1101
void cc1101_writeReg(uint8_t regAddr, uint8_t value)
{
    cc1101_chipSelect();
    cc1101_waitMiso();     // Wait until MISO goes low
    SPI.transfer(regAddr); // Send register address
    SPI.transfer(value);   // Send value
    cc1101_chipDeselect();
}

// send a strobe command to CC1101
void cc1101_cmdStrobe(uint8_t cmd)
{
    cc1101_chipSelect();
    delayMicroseconds(5);
    cc1101_waitMiso(); // Wait until MISO goes low
    SPI.transfer(cmd); // Send strobe command
    delayMicroseconds(5);
    cc1101_chipDeselect();
}

// read CC1101 register (status or configuration)
uint8_t cc1101_readReg(uint8_t regAddr, uint8_t regType)
{
    uint8_t addr, val;

    addr = regAddr | regType;
    cc1101_chipSelect();
    cc1101_waitMiso();        // Wait until MISO goes low
    SPI.transfer(addr);       // Send register address
    val = SPI.transfer(0x00); // Read result
    cc1101_chipDeselect();

    return val;
}

void cc1101_readBurstReg(uint8_t *buffer, uint8_t regAddr, uint8_t len)
{
    uint8_t addr, i;

    addr = regAddr | READ_BURST;
    cc1101_chipSelect();
    delayMicroseconds(5);
    cc1101_waitMiso();  // Wait until MISO goes low
    SPI.transfer(addr); // Send register address
    for (i = 0; i < len; i++)
    {
        buffer[i] = SPI.transfer(0x00); // Read result byte by byte
    }
    delayMicroseconds(2);
    cc1101_chipDeselect();
}

// initialize all the CC1101 registers
void cc1101_initRegisters(void)
{
    cc1101_writeReg(CC1101_IOCFG2, 0x06);   // GDO2 Output Pin Configuration
    cc1101_writeReg(CC1101_IOCFG1, 0x2E);   // GDO1 Output Pin Configuration
    cc1101_writeReg(CC1101_IOCFG0, 0x00);   // GDO0 Output Pin Configuration
    cc1101_writeReg(CC1101_FIFOTHR, 0x7);   // RX FIFO and TX FIFO Thresholds
    cc1101_writeReg(CC1101_SYNC1, 0x54);    // Sync Word, High Byte
    cc1101_writeReg(CC1101_SYNC0, 0x3D);    // Sync Word, Low Byte
    cc1101_writeReg(CC1101_PKTLEN, 0xFF);   // Packet Length
    cc1101_writeReg(CC1101_PKTCTRL1, 0x0);  // Packet Automation Control
    cc1101_writeReg(CC1101_PKTCTRL0, 0x0);  // Packet Automation Control
    cc1101_writeReg(CC1101_ADDR, 0x0);      // Device Address
    cc1101_writeReg(CC1101_CHANNR, 0x0);    // Channel Number
    cc1101_writeReg(CC1101_FSCTRL1, 0x8);   // Frequency Synthesizer Control
    cc1101_writeReg(CC1101_FSCTRL0, 0x0);   // Frequency Synthesizer Control
    cc1101_writeReg(CC1101_FREQ2, 0x21);    // Frequency Control Word, High Byte
    cc1101_writeReg(CC1101_FREQ1, 0x6B);    // Frequency Control Word, Middle Byte
    cc1101_writeReg(CC1101_FREQ0, 0xD0);    // Frequency Control Word, Low Byte
    cc1101_writeReg(CC1101_MDMCFG4, 0x5C);  // Modem Configuration
    cc1101_writeReg(CC1101_MDMCFG3, 0x4);   // Modem Configuration
    cc1101_writeReg(CC1101_MDMCFG2, 0x5);   // Modem Configuration
    cc1101_writeReg(CC1101_MDMCFG1, 0x22);  // Modem Configuration
    cc1101_writeReg(CC1101_MDMCFG0, 0xF8);  // Modem Configuration
    cc1101_writeReg(CC1101_DEVIATN, 0x44);  // Modem Deviation Setting
    cc1101_writeReg(CC1101_MCSM2, 0x7);     // Main Radio Control State Machine Configuration
    cc1101_writeReg(CC1101_MCSM1, 0x00);    // Main Radio Control State Machine Configuration
    cc1101_writeReg(CC1101_MCSM0, 0x18);    // Main Radio Control State Machine Configuration
    cc1101_writeReg(CC1101_FOCCFG, 0x2E);   // Frequency Offset Compensation Configuration
    cc1101_writeReg(CC1101_BSCFG, 0xBF);    // Bit Synchronization Configuration
    cc1101_writeReg(CC1101_AGCCTRL2, 0x43); // AGC Control
    cc1101_writeReg(CC1101_AGCCTRL1, 0x9);  // AGC Control
    cc1101_writeReg(CC1101_AGCCTRL0, 0xB5); // AGC Control
    cc1101_writeReg(CC1101_WOREVT1, 0x87);  // High Byte Event0 Timeout
    cc1101_writeReg(CC1101_WOREVT0, 0x6B);  // Low Byte Event0 Timeout
    cc1101_writeReg(CC1101_WORCTRL, 0xFB);  // Wake On Radio Control
    cc1101_writeReg(CC1101_FREND1, 0xB6);   // Front End RX Configuration
    cc1101_writeReg(CC1101_FREND0, 0x10);   // Front End TX Configuration
    cc1101_writeReg(CC1101_FSCAL3, 0xEA);   // Frequency Synthesizer Calibration
    cc1101_writeReg(CC1101_FSCAL2, 0x2A);   // Frequency Synthesizer Calibration
    cc1101_writeReg(CC1101_FSCAL1, 0x0);    // Frequency Synthesizer Calibration
    cc1101_writeReg(CC1101_FSCAL0, 0x1F);   // Frequency Synthesizer Calibration
    cc1101_writeReg(CC1101_RCCTRL1, 0x41);  // RC Oscillator Configuration
    cc1101_writeReg(CC1101_RCCTRL0, 0x0);   // RC Oscillator Configuration
    cc1101_writeReg(CC1101_FSTEST, 0x59);   // Frequency Synthesizer Calibration Control
    cc1101_writeReg(CC1101_PTEST, 0x7F);    // Production Test
    cc1101_writeReg(CC1101_AGCTEST, 0x3F);  // AGC Test
    cc1101_writeReg(CC1101_TEST2, 0x81);    // Various Test Settings
    cc1101_writeReg(CC1101_TEST1, 0x35);    // Various Test Settings
    cc1101_writeReg(CC1101_TEST0, 0x9);     // Various Test Settings
}

// power on reset
void cc1101_reset(void)
{
    cc1101_chipDeselect();
    delayMicroseconds(3);

    digitalWrite(MOSI, LOW);
    digitalWrite(SCK, HIGH); // see CC1101 datasheet 11.3

    cc1101_chipSelect();
    delayMicroseconds(3);
    cc1101_chipDeselect();
    delayMicroseconds(45); // at least 40 us

    cc1101_chipSelect();

    cc1101_waitMiso();         // Wait until MISO goes low
    SPI.transfer(CC1101_SRES); // Send reset command strobe
    cc1101_waitMiso();         // Wait until MISO goes low

    cc1101_chipDeselect();
}