This project consists of designing and realizing a bike tracker system that capture and store on
real time ride-related data. It is based on a STM32L4 microcontroller, interfacing with GPS and
BLE modules. More precisely, the system will periodically receive location and speed information
with the associated timestamp from the GPS module via the USART protocol, store it in an
external flash memory via the QSPI interface and then transmit those ride information from the
BLE module to the smartphone using the mobile application that we have developed by Flutter
framework. In addition, the system should automatically detect the state of the bike (moving or
not). When it is not moving, it switches to low power mode to save energy.

Keywords : STM32, embedded C, driver development, QSPI, BLE, USART, GPS,
FreeRTOS, low power consumption, mobile development, Flutter.
