
# ESP32 Wireless M-Bus Receiver using CC1101

## Introduction

This project is a PlatformIO-based implementation for receiving wireless M-Bus (Meter-Bus) packages in T1-mode using an ESP32 microcontroller and a CC1101 transceiver chip. The Wireless M-Bus protocol is commonly used for remote meter reading in smart metering applications.

Additionally, this project includes an implementation of a **Techem Radio 4** protocol decoder for parsing received data from Techem Radio 4 compatible devices. 

## Hardware Requirements

-   ESP32 development board
-   CC1101 transceiver module @ 868 MHz
-   Antenna for the CC1101 module

## Software Requirements

-   PlatformIO IDE

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or create a pull request on GitHub.


## Related materials:

-   Application note on using CC1101 with Wirless MBUS:  [https://www.ti.com/lit/an/swra234a/swra234a.pdf](https://www.ti.com/lit/an/swra234a/swra234a.pdf)
-   App filtering WMBUS packets from RTL:  [https://github.com/xaelsouth/rtl-wmbus](https://github.com/xaelsouth/rtl-wmbus)
-   WMBUS meters decoders library:  [https://github.com/weetmuts/wmbusmeters](https://github.com/weetmuts/wmbusmeters)
-   Online MBUS telegram analyzer: [https://wmbusmeters.org](https://wmbusmeters.org)