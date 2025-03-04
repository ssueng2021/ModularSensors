/** =========================================================================
 * @file menu_a_la_carte.ino
 * @brief Example with all possible functionality.
 *
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 * @copyright (c) 2017-2021 Stroud Water Research Center (SWRC)
 *                          and the EnviroDIY Development Team
 *            This example is published under the BSD-3 license.
 *
 * Build Environment: Visual Studios Code with PlatformIO
 * Hardware Platform: EnviroDIY Mayfly Arduino Datalogger
 *
 * DISCLAIMER:
 * THIS CODE IS PROVIDED "AS IS" - NO WARRANTY IS GIVEN.
 * ======================================================================= */

// ==========================================================================
//  Defines for the Arduino IDE
//  NOTE:  These are ONLY needed to compile with the Arduino IDE.
//         If you use PlatformIO, you should set these build flags in your
//         platformio.ini
// ==========================================================================
/** Start [defines] */
#ifndef TINY_GSM_RX_BUFFER
#define TINY_GSM_RX_BUFFER 64
#endif
#ifndef TINY_GSM_YIELD_MS
#define TINY_GSM_YIELD_MS 2
#endif
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 240
#endif
/** End [defines] */


// ==========================================================================
//  Include the libraries required for any data logger
// ==========================================================================
/** Start [includes] */
// The Arduino library is needed for every Arduino program.
#include <Arduino.h>

// EnableInterrupt is used by ModularSensors for external and pin change
// interrupts and must be explicitly included in the main program.
#include <EnableInterrupt.h>

// Include the main header for ModularSensors
#include <ModularSensors.h>
/** End [includes] */


// ==========================================================================
//  Creating Additional Serial Ports
// ==========================================================================
// The modem and a number of sensors communicate over UART/TTL - often called
// "serial". "Hardware" serial ports (automatically controlled by the MCU) are
// generally the most accurate and should be configured and used for as many
// peripherals as possible.  In some cases (ie, modbus communication) many
// sensors can share the same serial port.

#if defined(ARDUINO_ARCH_AVR) || defined(__AVR__)  // For AVR boards
// Unfortunately, most AVR boards have only one or two hardware serial ports,
// so we'll set up three types of extra software serial ports to use

#ifdef BUILD_TEST_ALTSOFTSERIAL
// AltSoftSerial by Paul Stoffregen
// (https://github.com/PaulStoffregen/AltSoftSerial) is the most accurate
// software serial port for AVR boards. AltSoftSerial can only be used on one
// set of pins on each board so only one AltSoftSerial port can be used. Not all
// AVR boards are supported by AltSoftSerial.
/** Start [altsoftserial] */
#include <AltSoftSerial.h>
AltSoftSerial altSoftSerial;
/** End [altsoftserial] */
#endif  // #ifdef BUILD_TEST_ALTSOFTSERIAL

#ifdef BUILD_TEST_NEOSWSERIAL
// NeoSWSerial (https://github.com/SRGDamia1/NeoSWSerial) is the best software
// serial that can be used on any pin supporting interrupts.
// You can use as many instances of NeoSWSerial as you need.
// Not all AVR boards are supported by NeoSWSerial.
/** Start [neoswserial] */
#include <NeoSWSerial.h>          // for the stream communication
const int8_t neoSSerial1Rx = 11;  // data in pin
const int8_t neoSSerial1Tx = -1;  // data out pin
NeoSWSerial  neoSSerial1(neoSSerial1Rx, neoSSerial1Tx);
// To use NeoSWSerial in this library, we define a function to receive data
// This is just a short-cut for later
void neoSSerial1ISR() {
    NeoSWSerial::rxISR(*portInputRegister(digitalPinToPort(neoSSerial1Rx)));
}
/** End [neoswserial] */
#endif  // #ifdef BUILD_TEST_NEOSWSERIAL

#ifdef BUILD_TEST_SOFTSERIAL
// The "standard" software serial library uses interrupts that conflict
// with several other libraries used within this program.  I've created a
// [version of software serial that has been stripped of
// interrupts](https://github.com/EnviroDIY/SoftwareSerial_ExtInts) but it is
// still far from ideal.
// NOTE:  Only use if necessary.  This is not a very accurate serial port!
// You can use as many instances of SoftwareSerial as you need.
/** Start [softwareserial] */
const int8_t softSerialRx = A3;  // data in pin
const int8_t softSerialTx = A4;  // data out pin

#include <SoftwareSerial_ExtInts.h>  // for the stream communication
SoftwareSerial_ExtInts softSerial1(softSerialRx, softSerialTx);
/** End [softwareserial] */
#endif  // #ifdef BUILD_TEST_SOFTSERIAL


#if defined MS_PALEOTERRA_SOFTWAREWIRE || defined MS_RAIN_SOFTWAREWIRE
/** Start [softwarewire] */
// A software I2C (Wire) instance using Testato's SoftwareWire
// To use SoftwareWire, you must also set a define for the sensor you want to
// use Software I2C for, ie:
//   `#define MS_RAIN_SOFTWAREWIRE`
//   `#define MS_PALEOTERRA_SOFTWAREWIRE`
// or set the build flag(s):
//   `-D MS_RAIN_SOFTWAREWIRE`
//   `-D MS_PALEOTERRA_SOFTWAREWIRE`
#include <SoftwareWire.h>  // Testato's Software I2C
const int8_t softwareSDA = 5;
const int8_t softwareSCL = 4;
SoftwareWire softI2C(softwareSDA, softwareSCL);
/** End [softwarewire] */
#endif  //  #if defined MS_PALEOTERRA_SOFTWAREWIRE ...

#endif  // End software serial for avr boards

#if defined ARDUINO_ARCH_SAMD
/** Start [serial_ports_SAMD] */
// The SAMD21 has 6 "SERCOM" ports, any of which can be used for UART
// communication.  The "core" code for most boards defines one or more UART
// (Serial) ports with the SERCOMs and uses others for I2C and SPI.  We can
// create new UART ports on any available SERCOM.  The table below shows
// definitions for select boards.

// Board =>   Arduino Zero       Adafruit Feather    Sodaq Boards
// -------    ---------------    ----------------    ----------------
// SERCOM0    Serial1 (D0/D1)    Serial1 (D0/D1)     Serial (D0/D1)
// SERCOM1    Available          Available           Serial3 (D12/D13)
// SERCOM2    Available          Available           I2C (A4/A5)
// SERCOM3    I2C (D20/D21)      I2C (D20/D21)       SPI (D11/12/13)
// SERCOM4    SPI (D21/22/23)    SPI (D21/22/23)     SPI1/Serial2
// SERCOM5    EDBG/Serial        Available           Serial1

// If using a Sodaq board, do not define the new sercoms, instead:
// #define ENABLE_SERIAL2
// #define ENABLE_SERIAL3

#include <wiring_private.h>  // Needed for SAMD pinPeripheral() function

#ifndef ENABLE_SERIAL2
// Set up a 'new' UART using SERCOM1
// The Rx will be on digital pin 11, which is SERCOM1's Pad #0
// The Tx will be on digital pin 10, which is SERCOM1's Pad #2
// NOTE:  SERCOM1 is undefinied on a "standard" Arduino Zero and many clones,
//        but not all!  Please check the variant.cpp file for you individual
//        board! Sodaq Autonomo's and Sodaq One's do NOT follow the 'standard'
//        SERCOM definitions!
Uart Serial2(&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
// Hand over the interrupts to the sercom port
void SERCOM1_Handler() {
    Serial2.IrqHandler();
}
#endif

#ifndef ENABLE_SERIAL3
// Set up a 'new' UART using SERCOM2
// The Rx will be on digital pin 5, which is SERCOM2's Pad #3
// The Tx will be on digital pin 2, which is SERCOM2's Pad #2
// NOTE:  SERCOM2 is undefinied on a "standard" Arduino Zero and many clones,
//        but not all!  Please check the variant.cpp file for you individual
//        board! Sodaq Autonomo's and Sodaq One's do NOT follow the 'standard'
//        SERCOM definitions!
Uart Serial3(&sercom2, 5, 2, SERCOM_RX_PAD_3, UART_TX_PAD_2);
// Hand over the interrupts to the sercom port
void SERCOM2_Handler() {
    Serial3.IrqHandler();
}
#endif

/** End [serial_ports_SAMD] */
#endif  // End hardware serial on SAMD21 boards


// ==========================================================================
//  Assigning Serial Port Functionality
// ==========================================================================
#if defined ARDUINO_ARCH_SAMD || defined ATMEGA2560 || \
    defined                              ARDUINO_AVR_MEGA2560
/** Start [assign_ports_hw] */
// If there are additional hardware Serial ports possible - use them!

// We give the modem first priority and assign it to hardware serial
// All of the supported processors have a hardware port available named Serial1
#define modemSerial Serial1

// Define the serial port for modbus
// Modbus (at 9600 8N1) is used by the Keller level loggers and Yosemitech
// sensors
#define modbusSerial Serial2

// The Maxbotix sonar is the only sensor that communicates over a serial port
// but does not use modbus
#define sonarSerial Serial3

/** End [assign_ports_hw] */
#else
/** Start [assign_ports_sw] */

// We give the modem first priority and assign it to hardware serial
// All of the supported processors have a hardware port available named Serial1
#define modemSerial Serial1

// Define the serial port for modbus
// Modbus (at 9600 8N1) is used by the Keller level loggers and Yosemitech
// sensors
// Since AltSoftSerial is the best software option, we use it for modbus
// If AltSoftSerial (or its pins) aren't avaiable, use NeoSWSerial
// SoftwareSerial **WILL NOT** work for modbus!
#define modbusSerial altSoftSerial  // For AltSoftSerial
// #define modbusSerial neoSSerial1  // For Neo software serial
// #define modbusSerial softSerial1  // For software serial

// The Maxbotix sonar is the only sensor that communicates over a serial port
// but does not use modbus
// Since the Maxbotix only needs one-way communication and sends a simple text
// string repeatedly, almost any software serial port will do for it.
// #define sonarSerial altSoftSerial  // For AltSoftSerial
#define sonarSerial neoSSerial1     // For Neo software serial
// #define sonarSerial softSerial1  // For software serial

/** End [assign_ports_sw] */
#endif


// ==========================================================================
//  Data Logging Options
// ==========================================================================
/** Start [logging_options] */
// The name of this program file
const char* sketchName = "menu_a_la_carte.ino";
// Logger ID, also becomes the prefix for the name of the data file on SD card
const char* LoggerID = "XXXXX";
// How frequently (in minutes) to log data
const uint8_t loggingInterval = 5;
// Your logger's timezone.
const int8_t timeZone = -5;  // Eastern Standard Time
// NOTE:  Daylight savings time will not be applied!  Please use standard time!

// Set the input and output pins for the logger
// NOTE:  Use -1 for pins that do not apply
const int32_t serialBaud = 115200;  // Baud rate for debugging
const int8_t  greenLED   = 8;       // Pin for the green LED
const int8_t  redLED     = 9;       // Pin for the red LED
const int8_t  buttonPin  = 21;      // Pin for debugging mode (ie, button pin)
const int8_t  wakePin    = 31;  // MCU interrupt/alarm pin to wake from sleep
// Mayfly 0.x D31 = A7
// Set the wake pin to -1 if you do not want the main processor to sleep.
// In a SAMD system where you are using the built-in rtc, set wakePin to 1
const int8_t sdCardPwrPin   = -1;  // MCU SD card power pin
const int8_t sdCardSSPin    = 12;  // SD card chip select/slave select pin
const int8_t sensorPowerPin = 22;  // MCU pin controlling main sensor power
/** End [logging_options] */


// ==========================================================================
//  Wifi/Cellular Modem Options
//    NOTE:  DON'T USE MORE THAN ONE MODEM OBJECT!
//           Delete the sections you are not using!
// ==========================================================================

#if defined BUILD_MODEM_XBEE_CELLULAR
/** Start [xbee_cell_transparent] */
// For any Digi Cellular XBee's
// NOTE:  The u-blox based Digi XBee's (3G global and LTE-M global) can be used
// in either bypass or transparent mode, each with pros and cons
// The Telit based Digi XBees (LTE Cat1) can only use this mode.
#include <modems/DigiXBeeCellularTransparent.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600;  // All XBee's use 9600 by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// The pin numbers here are for a Digi XBee with a Mayfly and LTE adapter
// For options https://github.com/EnviroDIY/LTEbee-Adapter/edit/master/README.md
const int8_t modemVccPin = -1;     // MCU pin controlling modem power
                                   // Option: modemVccPin = A5, if Mayfly SJ7 is
                                   // connected to the ASSOC pin
const int8_t modemStatusPin = 19;  // MCU pin used to read modem status
// NOTE:  If possible, use the `STATUS/SLEEP_not` (XBee pin 13) for status, but
// the CTS pin can also be used if necessary
const bool   useCTSforStatus = false;  // Flag to use the CTS pin for status
const int8_t modemResetPin   = 20;     // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 23;     // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;     // MCU pin connected an LED to show modem
                                       // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
DigiXBeeCellularTransparent modemXBCT(&modemSerial, modemVccPin, modemStatusPin,
                                      useCTSforStatus, modemResetPin,
                                      modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
DigiXBeeCellularTransparent modem = modemXBCT;
/** End [xbee_cell_transparent] */
// ==========================================================================


#elif defined BUILD_MODEM_XBEE_LTE_B
/** Start [xbee3_ltem_bypass] */
// For the u-blox SARA R410M based Digi LTE-M XBee3
// NOTE:  According to the manual, this should be less stable than transparent
// mode, but my experience is the complete reverse.
#include <modems/DigiXBeeLTEBypass.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600;  // All XBee's use 9600 by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// The pin numbers here are for a Digi XBee with a Mayfly and LTE adapter
const int8_t modemVccPin    = A5;  // MCU pin controlling modem power
const int8_t modemStatusPin = 19;  // MCU pin used to read modem status
// NOTE:  If possible, use the `STATUS/SLEEP_not` (XBee pin 13) for status, but
// the CTS pin can also be used if necessary
const bool   useCTSforStatus = false;  // Flag to use the CTS pin for status
const int8_t modemResetPin   = 20;     // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 23;     // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;     // MCU pin connected an LED to show modem
                                       // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
DigiXBeeLTEBypass modemXBLTEB(&modemSerial, modemVccPin, modemStatusPin,
                              useCTSforStatus, modemResetPin, modemSleepRqPin,
                              apn);
// Create an extra reference to the modem by a generic name
DigiXBeeLTEBypass modem = modemXBLTEB;
/** End [xbee3_ltem_bypass] */
// ==========================================================================


#elif defined BUILD_MODEM_XBEE_3G_B
/** Start [xbee_3g_bypass] */
// For the u-blox SARA U201 based Digi 3G XBee with 2G fallback
// NOTE:  According to the manual, this should be less stable than transparent
// mode, but my experience is the complete reverse.
#include <modems/DigiXBee3GBypass.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600;  // All XBee's use 9600 by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// The pin numbers here are for a Digi XBee with a Mayfly and LTE adapter
const int8_t modemVccPin    = A5;  // MCU pin controlling modem power
const int8_t modemStatusPin = 19;  // MCU pin used to read modem status
// NOTE:  If possible, use the `STATUS/SLEEP_not` (XBee pin 13) for status, but
// the CTS pin can also be used if necessary
const bool   useCTSforStatus = false;  // Flag to use the CTS pin for status
const int8_t modemResetPin   = 20;     // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 23;     // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;     // MCU pin connected an LED to show modem
                                       // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
DigiXBee3GBypass modemXB3GB(&modemSerial, modemVccPin, modemStatusPin,
                            useCTSforStatus, modemResetPin, modemSleepRqPin,
                            apn);
// Create an extra reference to the modem by a generic name
DigiXBee3GBypass modem = modemXB3GB;
/** End [xbee_3g_bypass] */
// ==========================================================================


#elif defined BUILD_MODEM_XBEE_WIFI
/** Start [xbee_wifi] */
// For the Digi Wifi XBee (S6B)
#include <modems/DigiXBeeWifi.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600;  // All XBee's use 9600 by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// The pin numbers here are for a Digi XBee direcly connected to a Mayfly
const int8_t modemVccPin    = -1;  // MCU pin controlling modem power
const int8_t modemStatusPin = 19;  // MCU pin used to read modem status
// NOTE:  If possible, use the `STATUS/SLEEP_not` (XBee pin 13) for status, but
// the CTS pin can also be used if necessary
const bool   useCTSforStatus = true;  // Flag to use the CTS pin for status
const int8_t modemResetPin   = -1;    // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 23;    // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;    // MCU pin connected an LED to show modem
                                      // status

// Network connection information
const char* wifiId  = "xxxxx";  // WiFi access point name
const char* wifiPwd = "xxxxx";  // WiFi password (WPA2)

// Create the modem object
DigiXBeeWifi modemXBWF(&modemSerial, modemVccPin, modemStatusPin,
                       useCTSforStatus, modemResetPin, modemSleepRqPin, wifiId,
                       wifiPwd);
// Create an extra reference to the modem by a generic name
DigiXBeeWifi modem = modemXBWF;
/** End [xbee_wifi] */
// ==========================================================================


#elif defined BUILD_MODEM_ESP8266
/** Start [esp8266] */
// For almost anything based on the Espressif ESP8266 using the
// AT command firmware
#include <modems/EspressifESP8266.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 115200;  // Communication speed of the modem
// NOTE:  This baud rate too fast for an 8MHz board, like the Mayfly!  The
// module should be programmed to a slower baud rate or set to auto-baud using
// the AT+UART_CUR or AT+UART_DEF command.

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// Example pins here are for a DFRobot ESP8266 Bee with Mayfly
const int8_t modemVccPin     = -2;  // MCU pin controlling modem power
const int8_t modemStatusPin  = -1;  // MCU pin used to read modem status
const int8_t modemResetPin   = -1;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 19;  // MCU pin for wake from light sleep
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status
// Pins for light sleep on the ESP8266. For power savings, I recommend
// NOT using these if it's possible to use deep sleep.
const int8_t espSleepRqPin = 13;  // GPIO# ON THE ESP8266 to assign for light
                                  // sleep request
const int8_t espStatusPin = -1;   // GPIO# ON THE ESP8266 to assign for light
                                  // sleep status

// Network connection information
const char* wifiId  = "xxxxx";  // WiFi access point name
const char* wifiPwd = "xxxxx";  // WiFi password (WPA2)

// Create the modem object
EspressifESP8266 modemESP(&modemSerial, modemVccPin, modemStatusPin,
                          modemResetPin, modemSleepRqPin, wifiId, wifiPwd,
                          espSleepRqPin,
                          espStatusPin  // Optional arguments
);
// Create an extra reference to the modem by a generic name
EspressifESP8266 modem = modemESP;
/** End [esp8266] */
// ==========================================================================


#elif defined BUILD_MODEM_BG96
/** Start [bg96] */
// For the Dragino, Nimbelink or other boards based on the Quectel BG96
#include <modems/QuectelBG96.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 115200;  // Communication speed of the modem
// NOTE:  This baud rate too fast for an 8MHz board, like the Mayfly!  The
// module should be programmed to a slower baud rate or set to auto-baud using
// the AT+IPR=9600 command.

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// Example pins here are for a modified Mayfly and a Dragino IoT Bee
const int8_t modemVccPin     = -1;  // MCU pin controlling modem power
const int8_t modemStatusPin  = -1;  // MCU pin used to read modem status
const int8_t modemResetPin   = A4;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = A3;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
QuectelBG96 modemBG96(&modemSerial, modemVccPin, modemStatusPin, modemResetPin,
                      modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
QuectelBG96 modem = modemBG96;
/** End [bg96] */
// ==========================================================================


#elif defined BUILD_MODEM_MONARCH
/** Start [monarch] */
// For the Nimbelink LTE-M Verizon/Sequans or other boards based on the Sequans
// Monarch series
#include <modems/SequansMonarch.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 921600;  // Default baud rate of SVZM20 is 921600
// NOTE:  This baud rate is much too fast for many Arduinos!  The module should
// be programmed to a slower baud rate or set to auto-baud using the AT+IPR
// command.

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// Nimbelink Skywire (NOT directly connectable to a Mayfly!)
const int8_t modemVccPin     = -1;  // MCU pin controlling modem power
const int8_t modemStatusPin  = 19;  // MCU pin used to read modem status
const int8_t modemResetPin   = 20;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 23;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
SequansMonarch modemSVZM(&modemSerial, modemVccPin, modemStatusPin,
                         modemResetPin, modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
SequansMonarch modem = modemSVZM;
/** End [monarch] */
// ==========================================================================


#elif defined BUILD_MODEM_SIM800
/** Start [sim800] */
// For almost anything based on the SIMCom SIM800 EXCEPT the Sodaq 2GBee R6 and
// higher
#include <modems/SIMComSIM800.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600;  //  SIM800 does auto-bauding by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// Example pins are for a Sodaq GPRSBee R4 with a Mayfly
const int8_t modemVccPin     = -1;  // MCU pin controlling modem power
const int8_t modemStatusPin  = 19;  // MCU pin used to read modem status
const int8_t modemResetPin   = -1;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 23;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
SIMComSIM800 modemS800(&modemSerial, modemVccPin, modemStatusPin, modemResetPin,
                       modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
SIMComSIM800 modem = modemS800;
/** End [sim800] */
// ==========================================================================


#elif defined BUILD_MODEM_SIM7000
/** Start [sim7000] */
// For almost anything based on the SIMCom SIM7000
#include <modems/SIMComSIM7000.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600;  //  SIM7000 does auto-bauding by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
const int8_t modemVccPin     = -1;  // MCU pin controlling modem power
const int8_t modemStatusPin  = -1;  // MCU pin used to read modem status
const int8_t modemResetPin   = -1;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 23;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
SIMComSIM7000 modem7000(&modemSerial, modemVccPin, modemStatusPin,
                        modemResetPin, modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
SIMComSIM7000 modem = modem7000;
/** End [sim7000] */
// ==========================================================================


#elif defined BUILD_MODEM_SIM7080
/** Start [sim7080] */
// For almost anything based on the SIMCom SIM7080G
#include <modems/SIMComSIM7080.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud =
    9600;  //  SIM7080 does auto-bauding by default, but I set mine to 9600

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// and-global breakout bk-7080a
const int8_t modemVccPin     = -1;  // MCU pin controlling modem power
const int8_t modemStatusPin  = 19;  // MCU pin used to read modem status
const int8_t modemSleepRqPin = 23;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
SIMComSIM7080 modem7080(&modemSerial, modemVccPin, modemStatusPin,
                        modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
SIMComSIM7080 modem = modem7080;
/** End [sim7080] */
// ==========================================================================


#elif defined BUILD_MODEM_S2GB
/** Start [gprsbee] */
// For the Sodaq 2GBee R6 and R7 based on the SIMCom SIM800
// NOTE:  The Sodaq GPRSBee doesn't expose the SIM800's reset pin
#include <modems/Sodaq2GBeeR6.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600;  //  SIM800 does auto-bauding by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// Example pins are for a Sodaq GPRSBee R6 or R7 with a Mayfly
const int8_t modemVccPin     = 23;  // MCU pin controlling modem power
const int8_t modemStatusPin  = 19;  // MCU pin used to read modem status
const int8_t modemResetPin   = -1;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = -1;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
Sodaq2GBeeR6 modem2GB(&modemSerial, modemVccPin, modemStatusPin, apn);
// Create an extra reference to the modem by a generic name
Sodaq2GBeeR6 modem = modem2GB;
/** End [gprsbee] */
// ==========================================================================


#elif defined BUILD_MODEM_UBEE_R410M
/** Start [sara_r410m] */
// For the Sodaq UBee based on the 4G LTE-M u-blox SARA R410M
#include <modems/SodaqUBeeR410M.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud =
    115200;  // Default baud rate of the SARA R410M is 115200
// NOTE:  The SARA R410N DOES NOT save baud rate to non-volatile memory.  After
// every power loss, the module will return to the default baud rate of 115200.
// NOTE:  115200 is TOO FAST for an 8MHz Arduino.  This library attempts to
// compensate by sending a baud rate change command in the wake function when
// compiled for a 8MHz board. Because of this, 8MHz boards, LIKE THE MAYFLY,
// *MUST* use a HardwareSerial instance as modemSerial.

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// Example pins are for a Sodaq uBee R410M with a Mayfly
const int8_t modemVccPin     = 23;  // MCU pin controlling modem power
const int8_t modemStatusPin  = 19;  // MCU pin used to read modem status
const int8_t modemResetPin   = -1;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 20;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
SodaqUBeeR410M modemR410(&modemSerial, modemVccPin, modemStatusPin,
                         modemResetPin, modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
SodaqUBeeR410M modem = modemR410;
/** End [sara_r410m] */
// ==========================================================================


#elif defined BUILD_MODEM_UBEE_U201
/** Start [sara_u201] */
// For the Sodaq UBee based on the 3G u-blox SARA U201
#include <modems/SodaqUBeeU201.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud =
    9600;  //  SARA U2xx module does auto-bauding by default

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply
// Example pins are for a Sodaq uBee U201 with a Mayfly
const int8_t modemVccPin     = 23;  // MCU pin controlling modem power
const int8_t modemStatusPin  = 19;  // MCU pin used to read modem status
const int8_t modemResetPin   = -1;  // MCU pin connected to modem reset pin
const int8_t modemSleepRqPin = 20;  // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED;  // MCU pin connected an LED to show modem
                                    // status

// Network connection information
const char* apn = "xxxxx";  // APN for GPRS connection

// Create the modem object
SodaqUBeeU201 modemU201(&modemSerial, modemVccPin, modemStatusPin,
                        modemResetPin, modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
SodaqUBeeU201 modem = modemU201;
/** End [sara_u201] */
// ==========================================================================
#endif


/** Start [modem_variables] */
// Create RSSI and signal strength variable pointers for the modem
Variable* modemRSSI =
    new Modem_RSSI(&modem, "12345678-abcd-1234-ef00-1234567890ab", "RSSI");
Variable* modemSignalPct = new Modem_SignalPercent(
    &modem, "12345678-abcd-1234-ef00-1234567890ab", "signalPercent");
Variable* modemBatteryState = new Modem_BatteryState(
    &modem, "12345678-abcd-1234-ef00-1234567890ab", "modemBatteryCS");
Variable* modemBatteryPct = new Modem_BatteryPercent(
    &modem, "12345678-abcd-1234-ef00-1234567890ab", "modemBatteryPct");
Variable* modemBatteryVoltage = new Modem_BatteryVoltage(
    &modem, "12345678-abcd-1234-ef00-1234567890ab", "modemBatterymV");
Variable* modemTemperature =
    new Modem_Temp(&modem, "12345678-abcd-1234-ef00-1234567890ab", "modemTemp");
/** End [modem_variables] */


// ==========================================================================
//  Using the Processor as a Sensor
// ==========================================================================
/** Start [processor_sensor] */
#include <sensors/ProcessorStats.h>

// Create the main processor chip "sensor" - for general metadata
const char*    mcuBoardVersion = "v1.1";
ProcessorStats mcuBoard(mcuBoardVersion);

// Create sample number, battery voltage, and free RAM variable pointers for the
// processor
Variable* mcuBoardBatt = new ProcessorStats_Battery(
    &mcuBoard, "12345678-abcd-1234-ef00-1234567890ab");
Variable* mcuBoardAvailableRAM = new ProcessorStats_FreeRam(
    &mcuBoard, "12345678-abcd-1234-ef00-1234567890ab");
Variable* mcuBoardSampNo = new ProcessorStats_SampleNumber(
    &mcuBoard, "12345678-abcd-1234-ef00-1234567890ab");
/** End [processor_sensor] */


#if defined ARDUINO_ARCH_AVR || defined MS_SAMD_DS3231
// ==========================================================================
//  Maxim DS3231 RTC (Real Time Clock)
// ==========================================================================
/** Start [ds3231] */
#include <sensors/MaximDS3231.h>

// Create a DS3231 sensor object
MaximDS3231 ds3231(1);

// Create a temperature variable pointer for the DS3231
Variable* ds3231Temp =
    new MaximDS3231_Temp(&ds3231, "12345678-abcd-1234-ef00-1234567890ab");
/** End [ds3231] */
#endif


#if defined BUILD_SENSOR_AM2315
// ==========================================================================
//  AOSong AM2315 Digital Humidity and Temperature Sensor
// ==========================================================================
/** Start [am2315] */
#include <sensors/AOSongAM2315.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t AM2315Power = sensorPowerPin;  // Power pin

// Create an AOSong AM2315 sensor object
AOSongAM2315 am2315(AM2315Power);

// Create humidity and temperature variable pointers for the AM2315
Variable* am2315Humid =
    new AOSongAM2315_Humidity(&am2315, "12345678-abcd-1234-ef00-1234567890ab");
Variable* am2315Temp =
    new AOSongAM2315_Temp(&am2315, "12345678-abcd-1234-ef00-1234567890ab");
/** End [am2315] */
#endif


#if defined BUILD_SENSOR_DHT
// ==========================================================================
//  AOSong DHT 11/21 (AM2301)/22 (AM2302) Digital Humidity and Temperature
// ==========================================================================
/** Start [dht] */
#include <sensors/AOSongDHT.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t DHTPower = sensorPowerPin;  // Power pin
const int8_t DHTPin   = 10;              // DHT data pin
const int8_t dhtType =
    DHT11;  // DHT type, one of DHT11, DHT12, DHT21, DHT22, or AM2301

// Create an AOSong DHT sensor object
AOSongDHT dht(DHTPower, DHTPin, dhtType);

// Create humidity, temperature, and heat index variable pointers for the DHT
Variable* dhtHumid =
    new AOSongDHT_Humidity(&dht, "12345678-abcd-1234-ef00-1234567890ab");
Variable* dhtTemp = new AOSongDHT_Temp(&dht,
                                       "12345678-abcd-1234-ef00-1234567890ab");
Variable* dhtHI   = new AOSongDHT_HI(&dht,
                                   "12345678-abcd-1234-ef00-1234567890ab");
/** End [dht] */
#endif


#if defined BUILD_SENSOR_SQ212
// ==========================================================================
//  Apogee SQ-212 Photosynthetically Active Radiation (PAR) Sensor
// ==========================================================================
/** Start [sq212] */
#include <sensors/ApogeeSQ212.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t  SQ212Power       = sensorPowerPin;  // Power pin
const int8_t  SQ212ADSChannel  = 3;     // The ADS channel for the SQ212
const uint8_t SQ212ADSi2c_addr = 0x48;  // The I2C address of the ADS1115 ADC

// Create an Apogee SQ212 sensor object
ApogeeSQ212 SQ212(SQ212Power, SQ212ADSChannel, SQ212ADSi2c_addr);

// Create PAR and raw voltage variable pointers for the SQ212
Variable* sq212PAR =
    new ApogeeSQ212_PAR(&SQ212, "12345678-abcd-1234-ef00-1234567890ab");
Variable* sq212voltage =
    new ApogeeSQ212_Voltage(&SQ212, "12345678-abcd-1234-ef00-1234567890ab");
/** End [sq212] */
#endif


#if defined BUILD_SENSOR_ATLASCO2
// ==========================================================================
//  Atlas Scientific EZO-CO2 Embedded NDIR Carbon Dioxide Sensor
// ==========================================================================
/** Start [atlas_co2] */
#include <sensors/AtlasScientificCO2.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t AtlasCO2Power    = sensorPowerPin;  // Power pin
uint8_t      AtlasCO2i2c_addr = 0x69;  // Default for CO2-EZO is 0x69 (105)
// All Atlas sensors have different default I2C addresses, but any of them can
// be re-addressed to any 8 bit number.  If using the default address for any
// Atlas Scientific sensor, you may omit this argument.

// Create an Atlas Scientific CO2 sensor object
// AtlasScientificCO2 atlasCO2(AtlasCO2Power, AtlasCO2i2c_addr);
AtlasScientificCO2 atlasCO2(AtlasCO2Power);

// Create concentration and temperature variable pointers for the EZO-CO2
Variable* atlasCO2CO2 = new AtlasScientificCO2_CO2(
    &atlasCO2, "12345678-abcd-1234-ef00-1234567890ab");
Variable* atlasCO2Temp = new AtlasScientificCO2_Temp(
    &atlasCO2, "12345678-abcd-1234-ef00-1234567890ab");
/** End [atlas_co2] */
#endif


#if defined BUILD_SENSOR_ATLASDO
// ==========================================================================
//  Atlas Scientific EZO-DO Dissolved Oxygen Sensor
// ==========================================================================
/** Start [atlas_do] */
#include <sensors/AtlasScientificDO.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t AtlasDOPower    = sensorPowerPin;  // Power pin
uint8_t      AtlasDOi2c_addr = 0x61;            // Default for DO is 0x61 (97)
// All Atlas sensors have different default I2C addresses, but any of them can
// be re-addressed to any 8 bit number.  If using the default address for any
// Atlas Scientific sensor, you may omit this argument.

// Create an Atlas Scientific DO sensor object
// AtlasScientificDO atlasDO(AtlasDOPower, AtlasDOi2c_addr);
AtlasScientificDO atlasDO(AtlasDOPower);

// Create concentration and percent saturation variable pointers for the EZO-DO
Variable* atlasDOconc = new AtlasScientificDO_DOmgL(
    &atlasDO, "12345678-abcd-1234-ef00-1234567890ab");
Variable* atlasDOpct = new AtlasScientificDO_DOpct(
    &atlasDO, "12345678-abcd-1234-ef00-1234567890ab");
/** End [atlas_do] */
#endif


#if defined BUILD_SENSOR_ATLASORP
// ==========================================================================
//  Atlas Scientific EZO-ORP Oxidation/Reduction Potential Sensor
// ==========================================================================
/** Start [atlas_orp] */
#include <sensors/AtlasScientificORP.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t AtlasORPPower    = sensorPowerPin;  // Power pin
uint8_t      AtlasORPi2c_addr = 0x62;            // Default for ORP is 0x62 (98)
// All Atlas sensors have different default I2C addresses, but any of them can
// be re-addressed to any 8 bit number.  If using the default address for any
// Atlas Scientific sensor, you may omit this argument.

// Create an Atlas Scientific ORP sensor object
// AtlasScientificORP atlasORP(AtlasORPPower, AtlasORPi2c_addr);
AtlasScientificORP atlasORP(AtlasORPPower);

// Create a potential variable pointer for the ORP
Variable* atlasORPot = new AtlasScientificORP_Potential(
    &atlasORP, "12345678-abcd-1234-ef00-1234567890ab");
/** End [atlas_orp] */
#endif


#if defined BUILD_SENSOR_ATLASPH
// ==========================================================================
//  Atlas Scientific EZO-pH Sensor
// ==========================================================================
/** Start [atlas_ph] */
#include <sensors/AtlasScientificpH.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t AtlaspHPower    = sensorPowerPin;  // Power pin
uint8_t      AtlaspHi2c_addr = 0x63;            // Default for pH is 0x63 (99)
// All Atlas sensors have different default I2C addresses, but any of them can
// be re-addressed to any 8 bit number.  If using the default address for any
// Atlas Scientific sensor, you may omit this argument.

// Create an Atlas Scientific pH sensor object
// AtlasScientificpH atlaspH(AtlaspHPower, AtlaspHi2c_addr);
AtlasScientificpH atlaspH(AtlaspHPower);

// Create a pH variable pointer for the pH sensor
Variable* atlaspHpH =
    new AtlasScientificpH_pH(&atlaspH, "12345678-abcd-1234-ef00-1234567890ab");
/** End [atlas_ph] */
#endif


#if defined BUILD_SENSOR_ATLASRTD || defined BUILD_SENSOR_ATLASEC
// ==========================================================================
//  Atlas Scientific EZO-RTD Temperature Sensor
// ==========================================================================
/** Start [atlas_rtd] */
#include <sensors/AtlasScientificRTD.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t AtlasRTDPower    = sensorPowerPin;  // Power pin
uint8_t      AtlasRTDi2c_addr = 0x66;  // Default for RTD is 0x66 (102)
// All Atlas sensors have different default I2C addresses, but any of them can
// be re-addressed to any 8 bit number.  If using the default address for any
// Atlas Scientific sensor, you may omit this argument.

// Create an Atlas Scientific RTD sensor object
// AtlasScientificRTD atlasRTD(AtlasRTDPower, AtlasRTDi2c_addr);
AtlasScientificRTD atlasRTD(AtlasRTDPower);

// Create a temperature variable pointer for the RTD
Variable* atlasTemp = new AtlasScientificRTD_Temp(
    &atlasRTD, "12345678-abcd-1234-ef00-1234567890ab");
/** End [atlas_rtd] */
#endif


#if defined BUILD_SENSOR_ATLASEC
// ==========================================================================
//  Atlas Scientific EZO-EC Conductivity Sensor
// ==========================================================================
/** Start [atlas_ec] */
#include <sensors/AtlasScientificEC.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t AtlasECPower    = sensorPowerPin;  // Power pin
uint8_t      AtlasECi2c_addr = 0x64;            // Default for EC is 0x64 (100)
// All Atlas sensors have different default I2C addresses, but any of them can
// be re-addressed to any 8 bit number.  If using the default address for any
// Atlas Scientific sensor, you may omit this argument.

// Create an Atlas Scientific Conductivity sensor object
// AtlasScientificEC atlasEC(AtlasECPower, AtlasECi2c_addr);
AtlasScientificEC atlasEC(AtlasECPower);

// Create four variable pointers for the EZO-ES
Variable* atlasCond = new AtlasScientificEC_Cond(
    &atlasEC, "12345678-abcd-1234-ef00-1234567890ab");
Variable* atlasTDS =
    new AtlasScientificEC_TDS(&atlasEC, "12345678-abcd-1234-ef00-1234567890ab");
Variable* atlasSal = new AtlasScientificEC_Salinity(
    &atlasEC, "12345678-abcd-1234-ef00-1234567890ab");
Variable* atlasGrav = new AtlasScientificEC_SpecificGravity(
    &atlasEC, "12345678-abcd-1234-ef00-1234567890ab");

// Create a calculated variable for the temperature compensated conductivity
// (that is, the specific conductance).  For this example, we will use the
// temperature measured by the Atlas RTD above this.  You could use the
// temperature returned by any other water temperature sensor if desired.
// **DO NOT** use your logger board temperature (ie, from the DS3231) to
// calculate specific conductance!
float calculateAtlasSpCond(void) {
    float spCond    = -9999;  // Always safest to start with a bad value
    float waterTemp = atlasTemp->getValue();
    float rawCond   = atlasCond->getValue();
    // ^^ Linearized temperature correction coefficient per degrees Celsius.
    // The value of 0.019 comes from measurements reported here:
    // Hayashi M. Temperature-electrical conductivity relation of water for
    // environmental monitoring and geophysical data inversion. Environ Monit
    // Assess. 2004 Aug-Sep;96(1-3):119-28.
    // doi: 10.1023/b:emas.0000031719.83065.68. PMID: 15327152.
    if (waterTemp != -9999 && rawCond != -9999) {
        // make sure both inputs are good
        float temperatureCoef = 0.019;
        spCond = rawCond / (1 + temperatureCoef * (waterTemp - 25.0));
    }
    return spCond;
}

// Properties of the calculated variable
// The number of digits after the decimal place
const uint8_t atlasSpCondResolution = 0;
// This must be a value from http://vocabulary.odm2.org/variablename/
const char* atlasSpCondName = "specificConductance";
// This must be a value from http://vocabulary.odm2.org/units/
const char* atlasSpCondUnit = "microsiemenPerCentimeter";
// A short code for the variable
const char* atlasSpCondCode = "atlasSpCond";
// The (optional) universallly unique identifier
const char* atlasSpCondUUID = "12345678-abcd-1234-ef00-1234567890ab";

// Finally, create the specific conductance variable and return a pointer to it
Variable* atlasSpCond =
    new Variable(calculateAtlasSpCond, atlasSpCondResolution, atlasSpCondName,
                 atlasSpCondUnit, atlasSpCondCode, atlasSpCondUUID);
/** End [atlas_ec] */
#endif


#if defined BUILD_SENSOR_BME280
// ==========================================================================
//  Bosch BME280 Environmental Sensor
// ==========================================================================
/** Start [bme280] */
#include <sensors/BoschBME280.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t BME280Power = sensorPowerPin;  // Power pin
uint8_t      BMEi2c_addr = 0x76;
// The BME280 can be addressed either as 0x77 (Adafruit default) or 0x76 (Grove
// default) Either can be physically mofidied for the other address

// Create a Bosch BME280 sensor object
BoschBME280 bme280(BME280Power, BMEi2c_addr);

// Create four variable pointers for the BME280
Variable* bme280Humid =
    new BoschBME280_Humidity(&bme280, "12345678-abcd-1234-ef00-1234567890ab");
Variable* bme280Temp =
    new BoschBME280_Temp(&bme280, "12345678-abcd-1234-ef00-1234567890ab");
Variable* bme280Press =
    new BoschBME280_Pressure(&bme280, "12345678-abcd-1234-ef00-1234567890ab");
Variable* bme280Alt =
    new BoschBME280_Altitude(&bme280, "12345678-abcd-1234-ef00-1234567890ab");
/** End [bme280] */
#endif


#if defined BUILD_SENSOR_OBS3
// ==========================================================================
//  Campbell OBS 3 / OBS 3+ Analog Turbidity Sensor
// ==========================================================================
/** Start [obs3] */
#include <sensors/CampbellOBS3.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t  OBS3Power          = sensorPowerPin;  // Power pin
const uint8_t OBS3NumberReadings = 10;
const uint8_t OBS3ADSi2c_addr    = 0x48;  // The I2C address of the ADS1115 ADC

const int8_t OBSLowADSChannel = 0;  // ADS channel for *low* range output

// Campbell OBS 3+ *Low* Range Calibration in Volts
const float OBSLow_A = 0.000E+00;  // "A" value (X^2) [*low* range]
const float OBSLow_B = 1.000E+00;  // "B" value (X) [*low* range]
const float OBSLow_C = 0.000E+00;  // "C" value [*low* range]

// Create a Campbell OBS3+ *low* range sensor object
CampbellOBS3 osb3low(OBS3Power, OBSLowADSChannel, OBSLow_A, OBSLow_B, OBSLow_C,
                     OBS3ADSi2c_addr, OBS3NumberReadings);

// Create turbidity and voltage variable pointers for the low range  of the OBS3
Variable* obs3TurbLow = new CampbellOBS3_Turbidity(
    &osb3low, "12345678-abcd-1234-ef00-1234567890ab", "TurbLow");
Variable* obs3VoltLow = new CampbellOBS3_Voltage(
    &osb3low, "12345678-abcd-1234-ef00-1234567890ab", "TurbLowV");


const int8_t OBSHighADSChannel = 1;  // ADS channel for *high* range output

// Campbell OBS 3+ *High* Range Calibration in Volts
const float OBSHigh_A = 0.000E+00;  // "A" value (X^2) [*high* range]
const float OBSHigh_B = 1.000E+00;  // "B" value (X) [*high* range]
const float OBSHigh_C = 0.000E+00;  // "C" value [*high* range]

// Create a Campbell OBS3+ *high* range sensor object
CampbellOBS3 osb3high(OBS3Power, OBSHighADSChannel, OBSHigh_A, OBSHigh_B,
                      OBSHigh_C, OBS3ADSi2c_addr, OBS3NumberReadings);

// Create turbidity and voltage variable pointers for the high range of the OBS3
Variable* obs3TurbHigh = new CampbellOBS3_Turbidity(
    &osb3high, "12345678-abcd-1234-ef00-1234567890ab", "TurbHigh");
Variable* obs3VoltHigh = new CampbellOBS3_Voltage(
    &osb3high, "12345678-abcd-1234-ef00-1234567890ab", "TurbHighV");
/** End [obs3] */
#endif


#if defined BUILD_SENSOR_CLARIVUE10
// ==========================================================================
//  Campbell ClariVUE Turbidity Sensor
// ==========================================================================
/** Start [clarivue] */
#include <sensors/CampbellClariVUE10.h>

const char* ClariVUESDI12address = "0";  // The SDI-12 Address of the ClariVUE10
const int8_t ClariVUEPower = sensorPowerPin;  // Power pin (-1 if unconnected)
const int8_t ClariVUEData  = 7;               // The SDI12 data pin
// NOTE:  you should NOT take more than one readings.  THe sensor already takes
// and averages 8 by default.

// Create a Campbell ClariVUE10 sensor object
CampbellClariVUE10 clarivue(*ClariVUESDI12address, ClariVUEPower, ClariVUEData);

// Create turbidity, temperature, and error variable pointers for the ClariVUE10
Variable* clarivueTurbidity = new CampbellClariVUE10_Turbidity(
    &clarivue, "12345678-abcd-1234-ef00-1234567890ab");
Variable* clarivueTemp = new CampbellClariVUE10_Temp(
    &clarivue, "12345678-abcd-1234-ef00-1234567890ab");
Variable* clarivueError = new CampbellClariVUE10_ErrorCode(
    &clarivue, "12345678-abcd-1234-ef00-1234567890ab");
/** End [clarivue] */
#endif


#if defined BUILD_SENSOR_CTD
// ==========================================================================
//  Decagon CTD-10 Conductivity, Temperature, and Depth Sensor
// ==========================================================================
/** Start [decagonCTD] */
#include <sensors/DecagonCTD.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const char*   CTDSDI12address   = "1";  // The SDI-12 Address of the CTD
const uint8_t CTDNumberReadings = 6;    // The number of readings to average
const int8_t  CTDPower          = sensorPowerPin;  // Power pin
const int8_t  CTDData           = 7;               // The SDI12 data pin

// Create a Decagon CTD sensor object
DecagonCTD ctd(*CTDSDI12address, CTDPower, CTDData, CTDNumberReadings);

// Create conductivity, temperature, and depth variable pointers for the CTD
Variable* ctdCond = new DecagonCTD_Cond(&ctd,
                                        "12345678-abcd-1234-ef00-1234567890ab");
Variable* ctdTemp = new DecagonCTD_Temp(&ctd,
                                        "12345678-abcd-1234-ef00-1234567890ab");
Variable* ctdDepth =
    new DecagonCTD_Depth(&ctd, "12345678-abcd-1234-ef00-1234567890ab");
/** End [decagonCTD] */
#endif


#if defined BUILD_SENSOR_ES2
// ==========================================================================
//  Decagon ES2 Conductivity and Temperature Sensor
// ==========================================================================
/** Start [es2] */
#include <sensors/DecagonES2.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const char*   ES2SDI12address   = "3";  // The SDI-12 Address of the ES2
const int8_t  ES2Power          = sensorPowerPin;  // Power pin
const int8_t  ES2Data           = 7;               // The SDI12 data pin
const uint8_t ES2NumberReadings = 5;

// Create a Decagon ES2 sensor object
DecagonES2 es2(*ES2SDI12address, ES2Power, ES2Data, ES2NumberReadings);

// Create specific conductance and temperature variable pointers for the ES2
Variable* es2Cond = new DecagonES2_Cond(&es2,
                                        "12345678-abcd-1234-ef00-1234567890ab");
Variable* es2Temp = new DecagonES2_Temp(&es2,
                                        "12345678-abcd-1234-ef00-1234567890ab");
/** End [es2] */
#endif


#if defined BUILD_SENSOR_VOLTAGE
// ==========================================================================
//  External Voltage via TI ADS1115
// ==========================================================================
/** Start [ext_volt] */
#include <sensors/ExternalVoltage.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t  ADSPower       = sensorPowerPin;  // Power pin
const int8_t  ADSChannel     = 2;               // The ADS channel of interest
const float   dividerGain    = 10;  //  Gain setting if using a voltage divider
const uint8_t evADSi2c_addr  = 0x48;  // The I2C address of the ADS1115 ADC
const uint8_t VoltReadsToAvg = 1;     // Only read one sample

// Create an External Voltage sensor object
ExternalVoltage extvolt(ADSPower, ADSChannel, dividerGain, evADSi2c_addr,
                        VoltReadsToAvg);

// Create a voltage variable pointer
Variable* extvoltV =
    new ExternalVoltage_Volt(&extvolt, "12345678-abcd-1234-ef00-1234567890ab");
/** End [ext_volt] */
#endif


#if defined BUILD_SENSOR_MPL115A2
// ==========================================================================
//  Freescale Semiconductor MPL115A2 Barometer
// ==========================================================================
/** Start [mpl115a2] */
#include <sensors/FreescaleMPL115A2.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t  MPLPower              = sensorPowerPin;  // Power pin
const uint8_t MPL115A2ReadingsToAvg = 1;

// Create an MPL115A2 barometer sensor object
MPL115A2 mpl115a2(MPLPower, MPL115A2ReadingsToAvg);

// Create pressure and temperature variable pointers for the MPL
Variable* mplPress =
    new MPL115A2_Pressure(&mpl115a2, "12345678-abcd-1234-ef00-1234567890ab");
Variable* mplTemp = new MPL115A2_Temp(&mpl115a2,
                                      "12345678-abcd-1234-ef00-1234567890ab");
/** End [mpl115a2] */
#endif


#if defined BUILD_SENSOR_INSITURDO
// ==========================================================================
//  InSitu RDO PRO-X Rugged Dissolved Oxygen Probe
// ==========================================================================
/** Start [insitu_rdo] */
#include <sensors/InSituRDO.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const char*   RDOSDI12address   = "5";  // The SDI-12 Address of the RDO PRO-X
const int8_t  RDOPower          = sensorPowerPin;  // Power pin
const int8_t  RDOData           = 7;               // The SDI12 data pin
const uint8_t RDONumberReadings = 3;

// Create an In-Situ RDO PRO-X dissolved oxygen sensor object
InSituRDO insituRDO(*RDOSDI12address, RDOPower, RDOData, RDONumberReadings);

// Create dissolved oxygen percent, dissolved oxygen concentration, temperature,
// and oxygen partial pressure variable pointers for the RDO PRO-X
Variable* rdoDOpct =
    new InSituRDO_DOpct(&insituRDO, "12345678-abcd-1234-ef00-1234567890ab");
Variable* rdoDOmgL =
    new InSituRDO_DOmgL(&insituRDO, "12345678-abcd-1234-ef00-1234567890ab");
Variable* rdoTemp = new InSituRDO_Temp(&insituRDO,
                                       "12345678-abcd-1234-ef00-1234567890ab");
Variable* rdoO2pp =
    new InSituRDO_Pressure(&insituRDO, "12345678-abcd-1234-ef00-1234567890ab");
/** End [insitu_rdo] */
#endif


#if defined BUILD_SENSOR_ACCULEVEL
// ==========================================================================
//  Keller Acculevel High Accuracy Submersible Level Transmitter
// ==========================================================================
/** Start [acculevel] */
#include <sensors/KellerAcculevel.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

// NOTE: Use -1 for any pins that don't apply or aren't being used.
byte acculevelModbusAddress  = 0x01;  // The modbus address of KellerAcculevel
const int8_t  acculevelPower = A3;    // Acculevel Sensor power pin
const int8_t  alAdapterPower = sensorPowerPin;  // RS485 adapter power pin
const int8_t  al485EnablePin = -1;              // Adapter RE/DE pin
const uint8_t acculevelNumberReadings = 5;
// The manufacturer recommends taking and averaging a few readings

// Create a Keller Acculevel sensor object
KellerAcculevel acculevel(acculevelModbusAddress, modbusSerial, alAdapterPower,
                          acculevelPower, al485EnablePin,
                          acculevelNumberReadings);

// Create pressure, temperature, and height variable pointers for the Acculevel
Variable* acculevPress = new KellerAcculevel_Pressure(
    &acculevel, "12345678-abcd-1234-ef00-1234567890ab");
Variable* acculevTemp = new KellerAcculevel_Temp(
    &acculevel, "12345678-abcd-1234-ef00-1234567890ab");
Variable* acculevHeight = new KellerAcculevel_Height(
    &acculevel, "12345678-abcd-1234-ef00-1234567890ab");
/** End [acculevel] */
#endif


#if defined BUILD_SENSOR_NANOLEVEL
// ==========================================================================
//  Keller Nanolevel High Accuracy Submersible Level Transmitter
// ==========================================================================
/** Start [nanolevel] */
#include <sensors/KellerNanolevel.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

// NOTE: Use -1 for any pins that don't apply or aren't being used.
byte nanolevelModbusAddress  = 0x01;  // The modbus address of KellerNanolevel
const int8_t  nlAdapterPower = sensorPowerPin;  // RS485 adapter power pin
const int8_t  nanolevelPower = A3;              // Sensor power pin
const int8_t  nl485EnablePin = -1;              // Adapter RE/DE pin
const uint8_t nanolevelNumberReadings = 5;
// The manufacturer recommends taking and averaging a few readings

// Create a Keller Nanolevel sensor object
KellerNanolevel nanolevel(nanolevelModbusAddress, modbusSerial, nlAdapterPower,
                          nanolevelPower, nl485EnablePin,
                          nanolevelNumberReadings);

// Create pressure, temperature, and height variable pointers for the Nanolevel
Variable* nanolevPress = new KellerNanolevel_Pressure(
    &nanolevel, "12345678-abcd-1234-ef00-1234567890ab");
Variable* nanolevTemp = new KellerNanolevel_Temp(
    &nanolevel, "12345678-abcd-1234-ef00-1234567890ab");
Variable* nanolevHeight = new KellerNanolevel_Height(
    &nanolevel, "12345678-abcd-1234-ef00-1234567890ab");
/** End [nanolevel] */
#endif


#if defined BUILD_SENSOR_MAXBOTIX
// ==========================================================================
//  Maxbotix HRXL Ultrasonic Range Finder
// ==========================================================================
/** Start [maxbotics] */
#include <sensors/MaxBotixSonar.h>

// A Maxbotix sonar with the trigger pin disconnect CANNOT share the serial port
// A Maxbotix sonar using the trigger may be able to share but YMMV

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t SonarPower    = sensorPowerPin;  // Excite (power) pin
const int8_t Sonar1Trigger = -1;              // Trigger pin
                                  // (a *unique* negative number if unconnected)
const uint8_t sonar1NumberReadings = 3;  // The number of readings to average

// Create a MaxBotix Sonar sensor object
MaxBotixSonar sonar1(sonarSerial, SonarPower, Sonar1Trigger,
                     sonar1NumberReadings);

// Create an ultrasonic range variable pointer
Variable* sonar1Range =
    new MaxBotixSonar_Range(&sonar1, "12345678-abcd-1234-ef00-1234567890ab");
/** End [maxbotics] */
#endif


#if defined BUILD_SENSOR_DS18 || defined BUILD_SENSOR_ANALOGEC
// ==========================================================================
//  Maxim DS18 One Wire Temperature Sensor
// ==========================================================================
/** Start [ds18] */
#include <sensors/MaximDS18.h>

// OneWire Address [array of 8 hex characters]
// If only using a single sensor on the OneWire bus, you may omit the address
DeviceAddress OneWireAddress1 = {0x28, 0xFF, 0xBD, 0xBA,
                                 0x81, 0x16, 0x03, 0x0C};
// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t OneWirePower       = sensorPowerPin;  // Power pin
const int8_t OneWireBus         = A0;              // OneWire Bus Pin
const int8_t ds18NumberReadings = 3;

// Create a Maxim DS18 sensor objects (use this form for a known address)
MaximDS18 ds18(OneWireAddress1, OneWirePower, OneWireBus, ds18NumberReadings);

// Create a Maxim DS18 sensor object (use this form for a single sensor on bus
// with an unknown address)
// MaximDS18 ds18(OneWirePower, OneWireBus);

// Create a temperature variable pointer for the DS18
Variable* ds18Temp = new MaximDS18_Temp(&ds18,
                                        "12345678-abcd-1234-ef00-1234567890ab");
/** End [ds18] */
#endif


#if defined BUILD_SENSOR_MS5803
// ==========================================================================
//  Measurement Specialties MS5803-14BA pressure sensor
// ==========================================================================
/** Start [ms5803] */
#include <sensors/MeaSpecMS5803.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t  MS5803Power = sensorPowerPin;  // Power pin
const uint8_t MS5803i2c_addr =
    0x76;  // The MS5803 can be addressed either as 0x76 (default) or 0x77
const int16_t MS5803maxPressure =
    14;  // The maximum pressure measurable by the specific MS5803 model
const uint8_t MS5803ReadingsToAvg = 1;

// Create a MeaSpec MS5803 pressure and temperature sensor object
MeaSpecMS5803 ms5803(MS5803Power, MS5803i2c_addr, MS5803maxPressure,
                     MS5803ReadingsToAvg);

// Create pressure and temperature variable pointers for the MS5803
Variable* ms5803Press =
    new MeaSpecMS5803_Pressure(&ms5803, "12345678-abcd-1234-ef00-1234567890ab");
Variable* ms5803Temp =
    new MeaSpecMS5803_Temp(&ms5803, "12345678-abcd-1234-ef00-1234567890ab");
/** End [ms5803] */
#endif


#if defined BUILD_SENSOR_5TM
// ==========================================================================
//  Meter ECH2O Soil Moisture Sensor
// ==========================================================================
/** Start [fivetm] */
#include <sensors/Decagon5TM.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const char*  TMSDI12address = "2";             // The SDI-12 Address of the 5-TM
const int8_t TMPower        = sensorPowerPin;  // Power pin
const int8_t TMData         = 7;               // The SDI12 data pin

// Create a Decagon 5TM sensor object
Decagon5TM fivetm(*TMSDI12address, TMPower, TMData);

// Create the matric potential, volumetric water content, and temperature
// variable pointers for the 5TM
Variable* fivetmEa = new Decagon5TM_Ea(&fivetm,
                                       "12345678-abcd-1234-ef00-1234567890ab");
Variable* fivetmVWC =
    new Decagon5TM_VWC(&fivetm, "12345678-abcd-1234-ef00-1234567890ab");
Variable* fivetmTemp =
    new Decagon5TM_Temp(&fivetm, "12345678-abcd-1234-ef00-1234567890ab");
/** End [fivetm] */
#endif


#if defined BUILD_SENSOR_HYDROS21
// ==========================================================================
//  Meter Hydros 21 Conductivity, Temperature, and Depth Sensor
// ==========================================================================
/** Start [hydros21] */
#include <sensors/MeterHydros21.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const char*   hydros21SDI12address = "1";  // The SDI-12 Address of the Hydros21
const uint8_t hydros21NumberReadings = 6;  // The number of readings to average
const int8_t  hydros21Power          = sensorPowerPin;  // Power pin
const int8_t  hydros21Data           = 7;               // The SDI12 data pin

// Create a Decagon Hydros21 sensor object
MeterHydros21 hydros21(*hydros21SDI12address, hydros21Power, hydros21Data,
                       hydros21NumberReadings);

// Create conductivity, temperature, and depth variable pointers for the
// Hydros21
Variable* hydros21Cond =
    new MeterHydros21_Cond(&hydros21, "12345678-abcd-1234-ef00-1234567890ab");
Variable* hydros21Temp =
    new MeterHydros21_Temp(&hydros21, "12345678-abcd-1234-ef00-1234567890ab");
Variable* hydros21Depth =
    new MeterHydros21_Depth(&hydros21, "12345678-abcd-1234-ef00-1234567890ab");
/** End [hydros21] */
#endif


#if defined BUILD_SENSOR_TEROS11
// ==========================================================================
//  Meter Teros 11 Soil Moisture Sensor
// ==========================================================================
/** Start [teros] */
#include <sensors/MeterTeros11.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const char*   teros11SDI12address = "4";  // The SDI-12 Address of the Teros 11
const int8_t  terosPower          = sensorPowerPin;  // Power pin
const int8_t  terosData           = 7;               // The SDI12 data pin
const uint8_t teros11NumberReadings = 3;  // The number of readings to average

// Create a METER TEROS 11 sensor object
MeterTeros11 teros11(*teros11SDI12address, terosPower, terosData,
                     teros11NumberReadings);

// Create the matric potential, volumetric water content, and temperature
// variable pointers for the Teros 11
Variable* teros11Ea =
    new MeterTeros11_Ea(&teros11, "12345678-abcd-1234-ef00-1234567890ab");
Variable* teros11Temp =
    new MeterTeros11_Temp(&teros11, "12345678-abcd-1234-ef00-1234567890ab");
Variable* teros11VWC =
    new MeterTeros11_VWC(&teros11, "12345678-abcd-1234-ef00-1234567890ab");
/** End [teros] */
#endif


#if defined BUILD_SENSOR_PALEOTERRA
// ==========================================================================
//  PaleoTerra Redox Sensors
// ==========================================================================
/** Start [pt_redox] */
#include <sensors/PaleoTerraRedox.h>

int8_t  paleoTerraPower = sensorPowerPin;  // Power pin
uint8_t paleoI2CAddress = 0x68;  // the I2C address of the redox sensor

// Create the PaleoTerra sensor object
#ifdef MS_PALEOTERRA_SOFTWAREWIRE
PaleoTerraRedox ptRedox(&softI2C, paleoTerraPower, paleoI2CAddress);
// PaleoTerraRedox ptRedox(paleoTerraPower, softwareSDA, softwareSCL,
// paleoI2CAddress);
#else
PaleoTerraRedox ptRedox(paleoTerraPower, paleoI2CAddress);
#endif

// Create the voltage variable for the redox sensor
Variable* ptVolt =
    new PaleoTerraRedox_Volt(&ptRedox, "12345678-abcd-1234-ef00-1234567890ab");
/** End [pt_redox] */
#endif


#if defined BUILD_SENSOR_RAINI2C
// ==========================================================================
//  External I2C Rain Tipping Bucket Counter
// ==========================================================================
/** Start [i2c_rain] */
#include <sensors/RainCounterI2C.h>

const uint8_t RainCounterI2CAddress = 0x08;
// I2C Address for EnviroDIY external tip counter; 0x08 by default
const float depthPerTipEvent = 0.2;  // rain depth in mm per tip event

// Create a Rain Counter sensor object
#ifdef MS_RAIN_SOFTWAREWIRE
RainCounterI2C tbi2c(&softI2C, RainCounterI2CAddress, depthPerTipEvent);
// RainCounterI2C tbi2c(softwareSDA, softwareSCL, RainCounterI2CAddress,
//                      depthPerTipEvent);
#else
RainCounterI2C  tbi2c(RainCounterI2CAddress, depthPerTipEvent);
#endif

// Create number of tips and rain depth variable pointers for the tipping bucket
Variable* tbi2cTips =
    new RainCounterI2C_Tips(&tbi2c, "12345678-abcd-1234-ef00-1234567890ab");
Variable* tbi2cDepth =
    new RainCounterI2C_Depth(&tbi2c, "12345678-abcd-1234-ef00-1234567890ab");
/** End [i2c_rain] */
#endif


#if defined BUILD_SENSOR_TALLY
// ==========================================================================
//    Tally I2C Event Counter for rain or wind reed-switch sensors
// ==========================================================================
/** Start [i2c_wind_tally] */
#include <sensors/TallyCounterI2C.h>

const int8_t TallyPower = -1;  // Power pin (-1 if continuously powered)
// NorthernWidget Tally I2CPower is -1 by default because it is often deployed
// with power always on, but Tally also has a super capacitor that enables it
// to be self powered between readings/recharge as described at
// https://github.com/EnviroDIY/Project-Tally

const uint8_t TallyCounterI2CAddress = 0x33;
// NorthernWidget Tally I2C address is 0x33 by default

// Create a Tally Counter sensor object
TallyCounterI2C tallyi2c(TallyPower, TallyCounterI2CAddress);

// Create variable pointers for the Tally event counter
Variable* tallyEvents = new TallyCounterI2C_Events(
    &tallyi2c, "12345678-abcd-1234-ef00-1234567890ab");

// For  Wind Speed, create a Calculated Variable that converts, similar to:
// period = loggingInterval * 60.0;    // in seconds
// frequency = tallyEventCount/period; // average event frequency in Hz
// tallyWindSpeed = frequency * 2.5 * 1.60934;  // in km/h
// 2.5 mph/Hz & 1.60934 kmph/mph and 2.5 mph/Hz conversion factor from
// web: Inspeed-Version-II-Reed-Switch-Anemometer-Sensor-Only-WS2R
/** End [i2c_wind_tally] */
#endif


#if defined BUILD_SENSOR_INA219
// ==========================================================================
//  TI INA219 High Side Current/Voltage Sensor (Current mA, Voltage, Power)
// ==========================================================================
/** Start [ina219] */
#include <sensors/TIINA219.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t INA219Power    = sensorPowerPin;  // Power pin
uint8_t      INA219i2c_addr = 0x40;            // 1000000 (Board A0+A1=GND)
// The INA219 can have one of 16 addresses, depending on the connections of A0
// and A1
const uint8_t INA219ReadingsToAvg = 1;

// Create an INA219 sensor object
TIINA219 ina219(INA219Power, INA219i2c_addr, INA219ReadingsToAvg);

// Create current, voltage, and power variable pointers for the INA219
Variable* inaCurrent =
    new TIINA219_Current(&ina219, "12345678-abcd-1234-ef00-1234567890ab");
Variable* inaVolt  = new TIINA219_Volt(&ina219,
                                      "12345678-abcd-1234-ef00-1234567890ab");
Variable* inaPower = new TIINA219_Power(&ina219,
                                        "12345678-abcd-1234-ef00-1234567890ab");
/** End [ina219] */
#endif


#if defined BUILD_SENSOR_CYCLOPS
// ==========================================================================
//  Turner Cyclops-7F Submersible Fluorometer
// ==========================================================================
/** Start [cyclops] */
#include <sensors/TurnerCyclops.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const int8_t  cyclopsPower          = sensorPowerPin;  // Power pin
const uint8_t cyclopsNumberReadings = 10;
const uint8_t cyclopsADSi2c_addr = 0x48;  // The I2C address of the ADS1115 ADC
const int8_t  cyclopsADSChannel  = 0;     // ADS channel

// Cyclops calibration information
const float cyclopsStdConc = 1.000;  // Concentration of the standard used
                                     // for a 1-point sensor calibration.
const float cyclopsStdVolt =
    1.000;  // The voltage (in volts) measured for the conc_std.
const float cyclopsBlankVolt =
    0.000;  // The voltage (in volts) measured for a blank.

// Create a Turner Cyclops sensor object
TurnerCyclops cyclops(cyclopsPower, cyclopsADSChannel, cyclopsStdConc,
                      cyclopsStdVolt, cyclopsBlankVolt, cyclopsADSi2c_addr,
                      cyclopsNumberReadings);

// Create the voltage variable pointer - used for any type of Cyclops
Variable* cyclopsVoltage =
    new TurnerCyclops_Voltage(&cyclops, "12345678-abcd-1234-ef00-1234567890ab");

// Create the variable pointer for the primary output parameter.  Only use
// **ONE** of these!  Which is possible depends on your specific sensor!
Variable* cyclopsChloro = new TurnerCyclops_Chlorophyll(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsRWT = new TurnerCyclops_Rhodamine(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsFluoroscein = new TurnerCyclops_Fluorescein(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsPhycocyanin = new TurnerCyclops_Phycocyanin(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsPhycoerythrin = new TurnerCyclops_Phycoerythrin(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsCDOM =
    new TurnerCyclops_CDOM(&cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsCrudeOil = new TurnerCyclops_CrudeOil(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsBrighteners = new TurnerCyclops_Brighteners(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsTurbidity = new TurnerCyclops_Turbidity(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsPTSA =
    new TurnerCyclops_PTSA(&cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsBTEX =
    new TurnerCyclops_BTEX(&cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsTryptophan = new TurnerCyclops_Tryptophan(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
Variable* cyclopsRedChloro = new TurnerCyclops_RedChlorophyll(
    &cyclops, "12345678-abcd-1234-ef00-1234567890ab");
/** End [cyclops] */
#endif


#if defined BUILD_SENSOR_ANALOGEC
// ==========================================================================
//   Analog Electrical Conductivity using the Processor's Analog Pins
// ==========================================================================
/** Start [analog_cond] */
#include <sensors/AnalogElecConductivity.h>

const int8_t ECpwrPin   = A4;  // Power pin (-1 if continuously powered)
const int8_t ECdataPin1 = A0;  // Data pin (must be an analog pin, ie A#)

// Create an Analog Electrical Conductivity sensor object
AnalogElecConductivity analogEC_phy(ECpwrPin, ECdataPin1);

// Create a conductivity variable pointer for the analog sensor
Variable* analogEc_cond = new AnalogElecConductivity_EC(
    &analogEC_phy, "12345678-abcd-1234-ef00-1234567890ab");

// Create a calculated variable for the temperature compensated conductivity
// (that is, the specific conductance).  For this example, we will use the
// temperature measured by the Maxim DS18 saved as ds18Temp several sections
// above this.  You could use the temperature returned by any other water
// temperature sensor if desired.  **DO NOT** use your logger board temperature
// (ie, from the DS3231) to calculate specific conductance!
float calculateAnalogSpCond(void) {
    float spCond          = -9999;  // Always safest to start with a bad value
    float waterTemp       = ds18Temp->getValue();
    float rawCond         = analogEc_cond->getValue();
    float temperatureCoef = 0.019;
    // ^^ Linearized temperature correction coefficient per degrees Celsius.
    // The value of 0.019 comes from measurements reported here:
    // Hayashi M. Temperature-electrical conductivity relation of water for
    // environmental monitoring and geophysical data inversion. Environ Monit
    // Assess. 2004 Aug-Sep;96(1-3):119-28.
    // doi: 10.1023/b:emas.0000031719.83065.68. PMID: 15327152.
    if (waterTemp != -9999 && rawCond != -9999) {
        // make sure both inputs are good
        spCond = rawCond / (1 + temperatureCoef * (waterTemp - 25.0));
    }
    return spCond;
}

// Properties of the calculated variable
// The number of digits after the decimal place
const uint8_t analogSpCondResolution = 0;
// This must be a value from http://vocabulary.odm2.org/variablename/
const char* analogSpCondName = "specificConductance";
// This must be a value from http://vocabulary.odm2.org/units/
const char* analogSpCondUnit = "microsiemenPerCentimeter";
// A short code for the variable
const char* analogSpCondCode = "anlgSpCond";
// The (optional) universallly unique identifier
const char* analogSpCondUUID = "12345678-abcd-1234-ef00-1234567890ab";

// Finally, Create the specific conductance variable and return a pointer to it
Variable* analogEc_spcond = new Variable(
    calculateAnalogSpCond, analogSpCondResolution, analogSpCondName,
    analogSpCondUnit, analogSpCondCode, analogSpCondUUID);
/** End [analog_cond] */
#endif


#if defined BUILD_SENSOR_Y504
// ==========================================================================
//  Yosemitech Y504 Dissolved Oxygen Sensor
// ==========================================================================
/** Start [y504] */
#include <sensors/YosemitechY504.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

byte         y504ModbusAddress = 0x04;  // The modbus address of the Y504
const int8_t y504AdapterPower  = sensorPowerPin;  // RS485 adapter power pin
                                                  // (-1 if unconnected)
const int8_t  y504SensorPower = A3;               // Sensor power pin
const int8_t  y504EnablePin   = -1;  // Adapter RE/DE pin (-1 if not applicable)
const uint8_t y504NumberReadings = 5;
// The manufacturer recommends averaging 10 readings, but we take 5 to minimize
// power consumption

// Create a Yosemitech Y504 dissolved oxygen sensor object
YosemitechY504 y504(y504ModbusAddress, modbusSerial, y504AdapterPower,
                    y504SensorPower, y504EnablePin, y504NumberReadings);

// Create the dissolved oxygen percent, dissolved oxygen concentration, and
// temperature variable pointers for the Y504
Variable* y504DOpct =
    new YosemitechY504_DOpct(&y504, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y504DOmgL =
    new YosemitechY504_DOmgL(&y504, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y504Temp =
    new YosemitechY504_Temp(&y504, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y504] */
#endif


#if defined BUILD_SENSOR_Y510
// ==========================================================================
//  Yosemitech Y510 Turbidity Sensor
// ==========================================================================
/** Start [y510] */
#include <sensors/YosemitechY510.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

byte         y510ModbusAddress = 0x0B;  // The modbus address of the Y510
const int8_t y510AdapterPower  = sensorPowerPin;  // RS485 adapter power pin
                                                  // (-1 if unconnected)
const int8_t  y510SensorPower = A3;               // Sensor power pin
const int8_t  y510EnablePin   = -1;  // Adapter RE/DE pin (-1 if not applicable)
const uint8_t y510NumberReadings = 5;
// The manufacturer recommends averaging 10 readings, but we take 5 to minimize
// power consumption

// Create a Y510-B Turbidity sensor object
YosemitechY510 y510(y510ModbusAddress, modbusSerial, y510AdapterPower,
                    y510SensorPower, y510EnablePin, y510NumberReadings);

// Create turbidity and temperature variable pointers for the Y510
Variable* y510Turb =
    new YosemitechY510_Turbidity(&y510, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y510Temp =
    new YosemitechY510_Temp(&y510, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y510] */
#endif


#if defined BUILD_SENSOR_Y511
// ==========================================================================
//  Yosemitech Y511 Turbidity Sensor with Wiper
// ==========================================================================
/** Start [y511] */
#include <sensors/YosemitechY511.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

byte         y511ModbusAddress = 0x1A;  // The modbus address of the Y511
const int8_t y511AdapterPower  = sensorPowerPin;  // RS485 adapter power pin
                                                  // (-1 if unconnected)
const int8_t  y511SensorPower = A3;               // Sensor power pin
const int8_t  y511EnablePin   = -1;  // Adapter RE/DE pin (-1 if not applicable)
const uint8_t y511NumberReadings = 5;
// The manufacturer recommends averaging 10 readings, but we take 5 to minimize
// power consumption

// Create a Y511-A Turbidity sensor object
YosemitechY511 y511(y511ModbusAddress, modbusSerial, y511AdapterPower,
                    y511SensorPower, y511EnablePin, y511NumberReadings);

// Create turbidity and temperature variable pointers for the Y511
Variable* y511Turb =
    new YosemitechY511_Turbidity(&y511, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y511Temp =
    new YosemitechY511_Temp(&y511, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y511] */
#endif


#if defined BUILD_SENSOR_Y514
// ==========================================================================
//  Yosemitech Y514 Chlorophyll Sensor
// ==========================================================================
/** Start [y514] */
#include <sensors/YosemitechY514.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

// NOTE: Use -1 for any pins that don't apply or aren't being used.
byte          y514ModbusAddress  = 0x14;  // The modbus address of the Y514
const int8_t  y514AdapterPower   = sensorPowerPin;  // RS485 adapter power pin
const int8_t  y514SensorPower    = A3;              // Sensor power pin
const int8_t  y514EnablePin      = -1;              // Adapter RE/DE pin
const uint8_t y514NumberReadings = 5;
// The manufacturer recommends averaging 10 readings, but we take 5 to minimize
// power consumption

// Create a Y514 chlorophyll sensor object
YosemitechY514 y514(y514ModbusAddress, modbusSerial, y514AdapterPower,
                    y514SensorPower, y514EnablePin, y514NumberReadings);

// Create chlorophyll concentration and temperature variable pointers for the
// Y514
Variable* y514Chloro = new YosemitechY514_Chlorophyll(
    &y514, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y514Temp =
    new YosemitechY514_Temp(&y514, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y514] */
#endif


#if defined BUILD_SENSOR_Y520
// ==========================================================================
//  Yosemitech Y520 Conductivity Sensor
// ==========================================================================
/** Start [y520] */
#include <sensors/YosemitechY520.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

byte         y520ModbusAddress = 0x20;  // The modbus address of the Y520
const int8_t y520AdapterPower  = sensorPowerPin;  // RS485 adapter power pin
                                                  // (-1 if unconnected)
const int8_t  y520SensorPower = A3;               // Sensor power pin
const int8_t  y520EnablePin   = -1;  // Adapter RE/DE pin (-1 if not applicable)
const uint8_t y520NumberReadings = 5;
// The manufacturer recommends averaging 10 readings, but we take 5 to minimize
// power consumption

// Create a Y520 conductivity sensor object
YosemitechY520 y520(y520ModbusAddress, modbusSerial, y520AdapterPower,
                    y520SensorPower, y520EnablePin, y520NumberReadings);

// Create specific conductance and temperature variable pointers for the Y520
Variable* y520Cond =
    new YosemitechY520_Cond(&y520, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y520Temp =
    new YosemitechY520_Temp(&y520, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y520] */
#endif


#if defined BUILD_SENSOR_Y532
// ==========================================================================
//  Yosemitech Y532 pH
// ==========================================================================
/** Start [y532] */
#include <sensors/YosemitechY532.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

// NOTE: Use -1 for any pins that don't apply or aren't being used.
byte          y532ModbusAddress  = 0x32;  // The modbus address of the Y532
const int8_t  y532AdapterPower   = sensorPowerPin;  // RS485 adapter power pin
const int8_t  y532SensorPower    = A3;              // Sensor power pin
const int8_t  y532EnablePin      = 4;               // Adapter RE/DE pin
const uint8_t y532NumberReadings = 1;
// The manufacturer actually doesn't mention averaging for this one

// Create a Yosemitech Y532 pH sensor object
YosemitechY532 y532(y532ModbusAddress, modbusSerial, y532AdapterPower,
                    y532SensorPower, y532EnablePin, y532NumberReadings);

// Create pH, electrical potential, and temperature variable pointers for the
// Y532
Variable* y532Voltage =
    new YosemitechY532_Voltage(&y532, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y532pH =
    new YosemitechY532_pH(&y532, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y532Temp =
    new YosemitechY532_Temp(&y532, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y532] */
#endif


#if defined BUILD_SENSOR_Y533
// ==========================================================================
//  Yosemitech Y533 Oxidation Reduction Potential (ORP)
// ==========================================================================
/** Start [y533] */
#include <sensors/YosemitechY533.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

byte         y533ModbusAddress = 0x32;  // The modbus address of the Y533
const int8_t y533AdapterPower  = sensorPowerPin;  // RS485 adapter power pin
                                                  // (-1 if unconnected)
const int8_t  y533SensorPower = A3;               // Sensor power pin
const int8_t  y533EnablePin   = 4;  // Adapter RE/DE pin (-1 if not applicable)
const uint8_t y533NumberReadings = 1;
// The manufacturer actually doesn't mention averaging for this one

// Create a Yosemitech Y533 ORP sensor object
YosemitechY533 y533(y533ModbusAddress, modbusSerial, y533AdapterPower,
                    y533SensorPower, y533EnablePin, y533NumberReadings);

// Create ORP and temperature variable pointers for the Y533
Variable* y533ORP =
    new YosemitechY533_ORP(&y533, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y533Temp =
    new YosemitechY533_Temp(&y533, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y533] */
#endif


#if defined BUILD_SENSOR_Y550
// ==========================================================================
//  Yosemitech Y550 COD Sensor with Wiper
// ==========================================================================
/** Start [y550] */
#include <sensors/YosemitechY550.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

byte         y550ModbusAddress = 0x50;  // The modbus address of the Y550
const int8_t y550AdapterPower  = sensorPowerPin;  // RS485 adapter power pin
                                                  // (-1 if unconnected)
const int8_t  y550SensorPower = A3;               // Sensor power pin
const int8_t  y550EnablePin   = -1;  // Adapter RE/DE pin (-1 if not applicable)
const uint8_t y550NumberReadings = 5;
// The manufacturer recommends averaging 10 readings, but we take 5 to minimize
// power consumption

// Create a Y550 chemical oxygen demand sensor object
YosemitechY550 y550(y550ModbusAddress, modbusSerial, y550AdapterPower,
                    y550SensorPower, y550EnablePin, y550NumberReadings);

// Create COD, turbidity, and temperature variable pointers for the Y550
Variable* y550COD =
    new YosemitechY550_COD(&y550, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y550Turbid =
    new YosemitechY550_Turbidity(&y550, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y550Temp =
    new YosemitechY550_Temp(&y550, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y550] */
#endif


#if defined BUILD_SENSOR_Y4000
// ==========================================================================
//  Yosemitech Y4000 Multiparameter Sonde (DOmgL, Turbidity, Cond, pH, Temp,
//    ORP, Chlorophyll, BGA)
// ==========================================================================
/** Start [y4000] */
#include <sensors/YosemitechY4000.h>

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section

byte         y4000ModbusAddress = 0x05;  // The modbus address of the Y4000
const int8_t y4000AdapterPower  = sensorPowerPin;  // RS485 adapter power pin
                                                   // (-1 if unconnected)
const int8_t  y4000SensorPower = A3;               // Sensor power pin
const int8_t  y4000EnablePin = -1;  // Adapter RE/DE pin (-1 if not applicable)
const uint8_t y4000NumberReadings = 5;
// The manufacturer recommends averaging 10 readings, but we take 5 to minimize
// power consumption

// Create a Yosemitech Y4000 multi-parameter sensor object
YosemitechY4000 y4000(y4000ModbusAddress, modbusSerial, y4000AdapterPower,
                      y4000SensorPower, y4000EnablePin, y4000NumberReadings);

// Create all of the variable pointers for the Y4000
Variable* y4000DO =
    new YosemitechY4000_DOmgL(&y4000, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y4000Turb = new YosemitechY4000_Turbidity(
    &y4000, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y4000Cond =
    new YosemitechY4000_Cond(&y4000, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y4000pH =
    new YosemitechY4000_pH(&y4000, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y4000Temp =
    new YosemitechY4000_Temp(&y4000, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y4000ORP =
    new YosemitechY4000_ORP(&y4000, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y4000Chloro = new YosemitechY4000_Chlorophyll(
    &y4000, "12345678-abcd-1234-ef00-1234567890ab");
Variable* y4000BGA =
    new YosemitechY4000_BGA(&y4000, "12345678-abcd-1234-ef00-1234567890ab");
/** End [y4000] */
#endif


#if defined BUILD_SENSOR_DOPTO
// ==========================================================================
//  Zebra Tech D-Opto Dissolved Oxygen Sensor
// ==========================================================================
/** Start [dopto] */
#include <sensors/ZebraTechDOpto.h>

// NOTE: Use -1 for any pins that don't apply or aren't being used.
const char*  DOptoSDI12address = "5";  // The SDI-12 Address of the D-Opto
const int8_t ZTPower           = sensorPowerPin;  // Power pin
const int8_t ZTData            = 7;               // The SDI12 data pin

// Create a Zebra Tech DOpto dissolved oxygen sensor object
ZebraTechDOpto dopto(*DOptoSDI12address, ZTPower, ZTData);

// Create dissolved oxygen percent, dissolved oxygen concentration, and
// temperature variable pointers for the Zebra Tech
Variable* dOptoDOpct =
    new ZebraTechDOpto_DOpct(&dopto, "12345678-abcd-1234-ef00-1234567890ab");
Variable* dOptoDOmgL =
    new ZebraTechDOpto_DOmgL(&dopto, "12345678-abcd-1234-ef00-1234567890ab");
Variable* dOptoTemp =
    new ZebraTechDOpto_Temp(&dopto, "12345678-abcd-1234-ef00-1234567890ab");
/** End [dopto] */
#endif


// ==========================================================================
//  Calculated Variable[s]
// ==========================================================================
/** Start [calculated_variables] */
// Create the function to give your calculated result.
// The function should take no input (void) and return a float.
// You can use any named variable pointers to access values by way of
// variable->getValue()

float calculateVariableValue(void) {
    float calculatedResult = -9999;  // Always safest to start with a bad value
    // float inputVar1 = variable1->getValue();
    // float inputVar2 = variable2->getValue();
    // make sure both inputs are good
    // if (inputVar1 != -9999 && inputVar2 != -9999) {
    //     calculatedResult = inputVar1 + inputVar2;
    // }
    return calculatedResult;
}

// Properties of the calculated variable
// The number of digits after the decimal place
const uint8_t calculatedVarResolution = 3;
// This must be a value from http://vocabulary.odm2.org/variablename/
const char* calculatedVarName = "varName";
// This must be a value from http://vocabulary.odm2.org/units/
const char* calculatedVarUnit = "varUnit";
// A short code for the variable
const char* calculatedVarCode = "calcVar";
// The (optional) universallly unique identifier
const char* calculatedVarUUID = "12345678-abcd-1234-ef00-1234567890ab";

// Finally, Create a calculated variable and return a variable pointer to it
Variable* calculatedVar = new Variable(
    calculateVariableValue, calculatedVarResolution, calculatedVarName,
    calculatedVarUnit, calculatedVarCode, calculatedVarUUID);
/** End [calculated_variables] */


#if defined BUILD_TEST_CREATE_IN_ARRAY
// ==========================================================================
//  Creating the Variable Array[s] and Filling with Variable Objects
//  NOTE:  This shows three differnt ways of creating the same variable array
//         and filling it with variables
// ==========================================================================
/** Start [variables_create_in_array] */
// Version 1: Create pointers for all of the variables from the sensors,
// at the same time putting them into an array
Variable* variableList[] = {
    new ProcessorStats_SampleNumber(&mcuBoard,
                                    "12345678-abcd-1234-ef00-1234567890ab"),
    new ProcessorStats_FreeRam(&mcuBoard,
                               "12345678-abcd-1234-ef00-1234567890ab"),
    new ProcessorStats_Battery(&mcuBoard,
                               "12345678-abcd-1234-ef00-1234567890ab"),
    new MaximDS3231_Temp(&ds3231, "12345678-abcd-1234-ef00-1234567890ab"),
    //  ... Add more variables as needed!
    new Modem_RSSI(&modem, "12345678-abcd-1234-ef00-1234567890ab"),
    new Modem_SignalPercent(&modem, "12345678-abcd-1234-ef00-1234567890ab"),
    new Modem_Temp(&modem, "12345678-abcd-1234-ef00-1234567890ab"),
    new Variable(calculateVariableValue, calculatedVarResolution,
                 calculatedVarName, calculatedVarUnit, calculatedVarCode,
                 calculatedVarUUID),
};
// Count up the number of pointers in the array
int variableCount = sizeof(variableList) / sizeof(variableList[0]);
// Create the VariableArray object
VariableArray varArray(variableCount, variableList);
/** End [variables_create_in_array] */
#endif
// ==========================================================================


#if defined BUILD_TEST_SEPARATE_UUIDS
/** Start [variables_separate_uuids] */
// Version 2: Create two separate arrays, on for the variables and a separate
// one for the UUID's, then give both as input to the variable array
// constructor.  Be cautious when doing this though because order is CRUCIAL!
Variable* variableList[] = {
    new ProcessorStats_SampleNumber(&mcuBoard),
    new ProcessorStats_FreeRam(&mcuBoard),
    new ProcessorStats_Battery(&mcuBoard),
    new MaximDS3231_Temp(&ds3231),
    //  ... Add all of your variables!
    new Modem_RSSI(&modem),
    new Modem_SignalPercent(&modem),
    new Modem_Temp(&modem),
    new Variable(calculateVariableValue, calculatedVarResolution,
                 calculatedVarName, calculatedVarUnit, calculatedVarCode),
};
const char* UUIDs[] = {
    "12345678-abcd-1234-ef00-1234567890ab",
    //  ... The number of UUID's must match the number of variables!
    "12345678-abcd-1234-ef00-1234567890ab",
};
// Count up the number of pointers in the array
int variableCount = sizeof(variableList) / sizeof(variableList[0]);
// Create the VariableArray object and attach the UUID's
VariableArray varArray(variableCount, variableList, UUIDs);
/** End [variables_separate_uuids] */
#endif
// ==========================================================================


#if defined BUILD_TEST_PRE_NAMED_VARS
/** Start [variables_pre_named] */
// Version 3: Fill array with already created and named variable pointers
Variable* variableList[] = {
    mcuBoardSampNo,
    mcuBoardAvailableRAM,
    mcuBoardBatt,
    calculatedVar,
#if defined ARDUINO_ARCH_AVR || defined MS_SAMD_DS3231
    ds3231Temp,
#endif
#if defined BUILD_SENSOR_AM2315
    am2315Humid,
    am2315Temp,
#endif
#if defined BUILD_SENSOR_DHT
    dhtHumid,
    dhtTemp,
    dhtHI,
#endif
#if defined BUILD_SENSOR_SQ212
    sq212PAR,
    sq212voltage,
#endif
#if defined BUILD_SENSOR_ATLASCO2
    atlasCO2CO2,
    atlasCO2Temp,
#endif
#if defined BUILD_SENSOR_ATLASDO
    atlasDOconc,
    atlasDOpct,
#endif
#if defined BUILD_SENSOR_ATLASORP
    atlasORPot,
#endif
#if defined BUILD_SENSOR_ATLASPH
    atlaspHpH,
#endif
#if defined BUILD_SENSOR_ATLASRTD
    atlasTemp,
#endif
#if defined BUILD_SENSOR_ATLASEC
    atlasCond,
    atlasTDS,
    atlasSal,
    atlasGrav,
    atlasSpCond,
#endif
#if defined BUILD_SENSOR_BME280
    bme280Temp,
    bme280Humid,
    bme280Press,
    bme280Alt,
#endif
#if defined BUILD_SENSOR_OBS3
    obs3TurbLow,
    obs3VoltLow,
    obs3TurbHigh,
    obs3VoltHigh,
#endif
#if defined BUILD_SENSOR_CTD
    ctdCond,
    ctdTemp,
    ctdDepth,
#endif
#if defined BUILD_SENSOR_ES2
    es2Cond,
    es2Temp,
#endif
#if defined BUILD_SENSOR_VOLTAGE
    extvoltV,
#endif
#if defined BUILD_SENSOR_MPL115A2
    mplTemp,
    mplPress,
#endif
#if defined BUILD_SENSOR_INSITURDO
    rdoTemp,
    rdoDOpct,
    rdoDOmgL,
    rdoO2pp,
#endif
#if defined BUILD_SENSOR_ACCULEVEL
    acculevPress,
    acculevTemp,
    acculevHeight,
#endif
#if defined BUILD_SENSOR_NANOLEVEL
    nanolevPress,
    nanolevTemp,
    nanolevHeight,
#endif
#if defined BUILD_SENSOR_MAXBOTIX
    sonar1Range,
#endif
#if defined BUILD_SENSOR_DS18
    ds18Temp,
#endif
#if defined BUILD_SENSOR_MS5803
    ms5803Temp,
    ms5803Press,
#endif
#if defined BUILD_SENSOR_5TM
    fivetmEa,
    fivetmVWC,
    fivetmTemp,
#endif
#if defined BUILD_SENSOR_HYDROS21
    hydros21Cond,
    hydros21Temp,
    hydros21Depth,
#endif
#if defined BUILD_SENSOR_TEROS11
    teros11Ea,
    teros11Temp,
    teros11VWC,
#endif
#if defined BUILD_SENSOR_PALEOTERRA
    ptVolt,
#endif
#if defined BUILD_SENSOR_RAINI2C
    tbi2cTips,
    tbi2cDepth,
#endif
#if defined BUILD_SENSOR_TALLY
    tallyEvents,
#endif
#if defined BUILD_SENSOR_INA219
    inaVolt,
    inaCurrent,
    inaPower,
#endif
#if defined BUILD_SENSOR_CYCLOPS
    cyclopsVoltage,
    cyclopsChloro,
    cyclopsRWT,
    cyclopsFluoroscein,
    cyclopsPhycocyanin,
    cyclopsPhycoerythrin,
    cyclopsCDOM,
    cyclopsCrudeOil,
    cyclopsBrighteners,
    cyclopsTurbidity,
    cyclopsPTSA,
    cyclopsBTEX,
    cyclopsTryptophan,
    cyclopsRedChloro,
#endif
#if defined BUILD_SENSOR_ANALOGEC
    analogEc_cond,
    analogEc_spcond,
#endif
#if defined BUILD_SENSOR_Y504
    y504DOpct,
    y504DOmgL,
    y504Temp,
#endif
#if defined BUILD_SENSOR_Y510
    y510Turb,
    y510Temp,
#endif
#if defined BUILD_SENSOR_Y511
    y511Turb,
    y511Temp,
#endif
#if defined BUILD_SENSOR_Y514
    y514Chloro,
    y514Temp,
#endif
#if defined BUILD_SENSOR_Y520
    y520Cond,
    y520Temp,
#endif
#if defined BUILD_SENSOR_Y532
    y532Voltage,
    y532pH,
    y532Temp,
#endif
#if defined BUILD_SENSOR_Y533
    y533ORP,
    y533Temp,
#endif
#if defined BUILD_SENSOR_Y550
    y550COD,
    y550Turbid,
    y550Temp,
#endif
#if defined BUILD_SENSOR_Y4000
    y4000DO,
    y4000Turb,
    y4000Cond,
    y4000pH,
    y4000Temp,
    y4000ORP,
    y4000Chloro,
    y4000BGA,
#endif
#if defined BUILD_SENSOR_DOPTO
    dOptoDOpct,
    dOptoDOmgL,
    dOptoTemp,
#endif
    modemRSSI,
    modemSignalPct,
#ifdef TINY_GSM_MODEM_HAS_BATTERY
    modemBatteryState,
    modemBatteryPct,
    modemBatteryVoltage,
#endif
#ifdef TINY_GSM_MODEM_HAS_TEMPERATURE
    modemTemperature,
#endif
};
// Count up the number of pointers in the array
int variableCount = sizeof(variableList) / sizeof(variableList[0]);
// Create the VariableArray object
VariableArray varArray(variableCount, variableList);
/** End [variables_pre_named] */
#endif


// ==========================================================================
//  The Logger Object[s]
// ==========================================================================
/** Start [loggers] */
// Create a new logger instance
Logger dataLogger(LoggerID, loggingInterval, &varArray);
/** End [loggers] */


#if defined BUILD_PUB_MMW
// ==========================================================================
//  A Publisher to Monitor My Watershed / EnviroDIY Data Sharing Portal
// ==========================================================================
/** Start [monitormw] */
// Device registration and sampling feature information can be obtained after
// registration at https://monitormywatershed.org or https://data.envirodiy.org
const char* registrationToken =
    "12345678-abcd-1234-ef00-1234567890ab";  // Device registration token
const char* samplingFeature =
    "12345678-abcd-1234-ef00-1234567890ab";  // Sampling feature UUID

// Create a data publisher for the Monitor My Watershed/EnviroDIY POST endpoint
#include <publishers/EnviroDIYPublisher.h>
EnviroDIYPublisher EnviroDIYPOST(dataLogger, &modem.gsmClient,
                                 registrationToken, samplingFeature);
/** End [monitormw] */
#endif


#if defined BUILD_PUB_DREAMHOST
// ==========================================================================
//  A Publisher to DreamHost
// ==========================================================================
/** Start [dreamhost] */
// NOTE:  This is an outdated data collection tool used by the Stroud Center.
// It very, very unlikely that you will use this.

const char* DreamHostPortalRX = "xxxx";

// Create a data publisher to DreamHost
#include <publishers/DreamHostPublisher.h>
DreamHostPublisher DreamHostGET(dataLogger, &modem.gsmClient,
                                DreamHostPortalRX);
/** End [dreamhost] */
#endif


#if defined BUILD_PUB_THINGSPEAK
// ==========================================================================
//  ThingSpeak Data Publisher
// ==========================================================================
/** Start [thingspeak] */
// Create a channel with fields on ThingSpeak in advance.
// The fields will be sent in exactly the order they are in the variable array.
// Any custom name or identifier given to the field on ThingSpeak is irrelevant.
// No more than 8 fields of data can go to any one channel.  Any fields beyond
// the eighth in the array will be ignored.
const char* thingSpeakMQTTKey =
    "XXXXXXXXXXXXXXXX";  // Your MQTT API Key from Account > MyProfile.
const char* thingSpeakChannelID =
    "######";  // The numeric channel id for your channel
const char* thingSpeakChannelKey =
    "XXXXXXXXXXXXXXXX";  // The Write API Key for your channel

// Create a data publisher for ThingSpeak
#include <publishers/ThingSpeakPublisher.h>
ThingSpeakPublisher TsMqtt(dataLogger, &modem.gsmClient, thingSpeakMQTTKey,
                           thingSpeakChannelID, thingSpeakChannelKey);
/** End [thingspeak] */
#endif


// ==========================================================================
//  Working Functions
// ==========================================================================
/** Start [working_functions] */
// Flashes the LED's on the primary board
void greenredflash(uint8_t numFlash = 4, uint8_t rate = 75) {
    for (uint8_t i = 0; i < numFlash; i++) {
        digitalWrite(greenLED, HIGH);
        digitalWrite(redLED, LOW);
        delay(rate);
        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, HIGH);
        delay(rate);
    }
    digitalWrite(redLED, LOW);
}

// Uses the processor sensor object to read the battery voltage
// NOTE: This will actually return the battery level from the previous update!
float getBatteryVoltage() {
    if (mcuBoard.sensorValues[0] == -9999) mcuBoard.update();
    return mcuBoard.sensorValues[0];
}
/** End [working_functions] */


// ==========================================================================
//  Arduino Setup Function
// ==========================================================================
void setup() {
/** Start [setup_wait] */
// Wait for USB connection to be established by PC
// NOTE:  Only use this when debugging - if not connected to a PC, this
// could prevent the script from starting
#if defined SERIAL_PORT_USBVIRTUAL
    while (!SERIAL_PORT_USBVIRTUAL && (millis() < 10000L)) {}
#endif
    /** End [setup_wait] */

    /** Start [setup_prints] */
    // Start the primary serial connection
    Serial.begin(serialBaud);

    // Print a start-up note to the first serial port
    Serial.print(F("\n\nNow running "));
    Serial.print(sketchName);
    Serial.print(F(" on Logger "));
    Serial.println(LoggerID);
    Serial.println();

    Serial.print(F("Using ModularSensors Library version "));
    Serial.println(MODULAR_SENSORS_VERSION);
    Serial.print(F("TinyGSM Library version "));
    Serial.println(TINYGSM_VERSION);
    Serial.println();
/** End [setup_prints] */

/** Start [setup_softserial] */
// Allow interrupts for software serial
#if defined SoftwareSerial_ExtInts_h
    enableInterrupt(softSerialRx, SoftwareSerial_ExtInts::handle_interrupt,
                    CHANGE);
#endif
#if defined NeoSWSerial_h
    enableInterrupt(neoSSerial1Rx, neoSSerial1ISR, CHANGE);
#endif
    /** End [setup_softserial] */

    /** Start [setup_serial_begins] */
    // Start the serial connection with the modem
    modemSerial.begin(modemBaud);

    // Start the stream for the modbus sensors;
    // all currently supported modbus sensors use 9600 baud
    modbusSerial.begin(9600);

#if defined BUILD_SENSOR_MAXBOTIX
    // Start the SoftwareSerial stream for the sonar; it will always be at 9600
    // baud
    sonarSerial.begin(9600);
#endif
/** End [setup_serial_begins] */

// Assign pins SERCOM functionality for SAMD boards
// NOTE:  This must happen *after* the various serial.begin statements
/** Start [setup_samd_pins] */
#if defined ARDUINO_ARCH_SAMD
#ifndef ENABLE_SERIAL2
    pinPeripheral(10, PIO_SERCOM);  // Serial2 Tx/Dout = SERCOM1 Pad #2
    pinPeripheral(11, PIO_SERCOM);  // Serial2 Rx/Din = SERCOM1 Pad #0
#endif
#ifndef ENABLE_SERIAL3
    pinPeripheral(2, PIO_SERCOM);  // Serial3 Tx/Dout = SERCOM2 Pad #2
    pinPeripheral(5, PIO_SERCOM);  // Serial3 Rx/Din = SERCOM2 Pad #3
#endif
#endif
    /** End [setup_samd_pins] */

    /** Start [setup_flashing_led] */
    // Set up pins for the LED's
    pinMode(greenLED, OUTPUT);
    digitalWrite(greenLED, LOW);
    pinMode(redLED, OUTPUT);
    digitalWrite(redLED, LOW);
    // Blink the LEDs to show the board is on and starting up
    greenredflash();
    /** End [setup_flashing_led] */

    /** Start [setup_logger] */
    // Set the timezones for the logger/data and the RTC
    // Logging in the given time zone
    Logger::setLoggerTimeZone(timeZone);
    // It is STRONGLY RECOMMENDED that you set the RTC to be in UTC (UTC+0)
    Logger::setRTCTimeZone(0);

    // Attach the modem and information pins to the logger
    dataLogger.attachModem(modem);
    modem.setModemLED(modemLEDPin);
    dataLogger.setLoggerPins(wakePin, sdCardSSPin, sdCardPwrPin, buttonPin,
                             greenLED);

    // Begin the logger
    dataLogger.begin();
    /** End [setup_logger] */

    /** Start [setup_sesors] */
    // Note:  Please change these battery voltages to match your battery
    // Set up the sensors, except at lowest battery level
    if (getBatteryVoltage() > 3.4) {
        Serial.println(F("Setting up sensors..."));
        varArray.setupSensors();
    }
    /** End [setup_sesors] */

#if defined BUILD_MODEM_ESP8266 && F_CPU == 8000000L
    /** Start [setup_esp] */
    if (modemBaud > 57600) {
        modem.modemWake();  // NOTE:  This will also set up the modem
        modemSerial.begin(modemBaud);
        modem.gsmModem.sendAT(GF("+UART_DEF=9600,8,1,0,0"));
        modem.gsmModem.waitResponse();
        modemSerial.end();
        modemSerial.begin(9600);
    }
/** End [setup_esp] */
#endif

#if defined BUILD_TEST_SKYWIRE
    /** Start [setup_skywire] */
    modem.setModemStatusLevel(LOW);  // If using CTS, LOW
    modem.setModemWakeLevel(HIGH);   // Skywire dev board inverts the signal
    modem.setModemResetLevel(HIGH);  // Skywire dev board inverts the signal
    /** End [setup_skywire] */
#endif

#if defined BUILD_MODEM_SIM7080
    /** Start [setup_sim7080] */
    modem.setModemWakeLevel(HIGH);   // ModuleFun Bee inverts the signal
    modem.setModemResetLevel(HIGH);  // ModuleFun Bee inverts the signal
    Serial.println(F("Waking modem and setting Cellular Carrier Options..."));
    modem.modemWake();  // NOTE:  This will also set up the modem
    modem.gsmModem.setBaud(modemBaud);   // Make sure we're *NOT* auto-bauding!
    modem.gsmModem.setNetworkMode(38);   // set to LTE only
                                         // 2 Automatic
                                         // 13 GSM only
                                         // 38 LTE only
                                         // 51 GSM and LTE only
    modem.gsmModem.setPreferredMode(1);  // set to CAT-M
                                         // 1 CAT-M
                                         // 2 NB-IoT
                                         // 3 CAT-M and NB-IoT
    /** End [setup_sim7080] */
#endif

#if defined BUILD_MODEM_XBEE_CELLULAR
    /** Start [setup_xbeec_carrier] */
    // Extra modem set-up
    Serial.println(F("Waking modem and setting Cellular Carrier Options..."));
    modem.modemWake();  // NOTE:  This will also set up the modem
    // Go back to command mode to set carrier options
    modem.gsmModem.commandMode();
    // Carrier Profile - 0 = Automatic selection
    //                 - 1 = No profile/SIM ICCID selected
    //                 - 2 = AT&T
    //                 - 3 = Verizon
    // NOTE:  To select T-Mobile, you must enter bypass mode!
    modem.gsmModem.sendAT(GF("CP"), 2);
    modem.gsmModem.waitResponse();
    // Cellular network technology - 0 = LTE-M with NB-IoT fallback
    //                             - 1 = NB-IoT with LTE-M fallback
    //                             - 2 = LTE-M only
    //                             - 3 = NB-IoT only
    // NOTE:  As of 2020 in the USA, AT&T and Verizon only use LTE-M
    // T-Mobile uses NB-IOT
    modem.gsmModem.sendAT(GF("N#"), 2);
    modem.gsmModem.waitResponse();
    // Write changes to flash and apply them
    Serial.println(F("Wait while applying changes..."));
    // Write changes to flash
    modem.gsmModem.writeChanges();
    // Reset the cellular component to ensure network settings are changed
    modem.gsmModem.sendAT(GF("!R"));
    modem.gsmModem.waitResponse(30000L);
    // Force reset of the Digi component as well
    // This effectively exits command mode
    modem.gsmModem.sendAT(GF("FR"));
    modem.gsmModem.waitResponse(5000L);
/** End [setup_xbeec_carrier] */
#endif


#if defined BUILD_MODEM_XBEE_LTE_B
    /** Start [setup_r4_carrrier] */
    // Extra modem set-up
    Serial.println(F("Waking modem and setting Cellular Carrier Options..."));
    modem.modemWake();  // NOTE:  This will also set up the modem
    // Turn off the cellular radio while making network changes
    modem.gsmModem.sendAT(GF("+CFUN=0"));
    modem.gsmModem.waitResponse();
    // Mobile Network Operator Profile - 0 = SW default
    //                                 - 1 = SIM ICCID selected
    //                                 - 2: ATT
    //                                 - 6: China Telecom
    //                                 - 100: Standard Europe
    //                                 - 4: Telstra
    //                                 - 5: T-Mobile US
    //                                 - 19: Vodafone
    //                                 - 3: Verizon
    //                                 - 31: Deutsche Telekom
    modem.gsmModem.sendAT(GF("+UMNOPROF="), 2);
    modem.gsmModem.waitResponse();
    // Selected network technology - 7: LTE Cat.M1
    //                             - 8: LTE Cat.NB1
    // Fallback network technology - 7: LTE Cat.M1
    //                              - 8: LTE Cat.NB1
    // NOTE:  As of 2020 in the USA, AT&T and Verizon only use LTE-M
    // T-Mobile uses NB-IOT
    modem.gsmModem.sendAT(GF("+URAT="), 7, ',', 8);
    modem.gsmModem.waitResponse();
    // Restart the module to apply changes
    modem.gsmModem.sendAT(GF("+CFUN=1,1"));
    modem.gsmModem.waitResponse(10000L);
/** End [setup_r4_carrrier] */
#endif

    /** Start [setup_clock] */
    // Sync the clock if it isn't valid or we have battery to spare
    if (getBatteryVoltage() > 3.55 || !dataLogger.isRTCSane()) {
        // Synchronize the RTC with NIST
        // This will also set up the modem
        dataLogger.syncRTC();
    }
    /** End [setup_clock] */

    /** Start [setup_file] */
    // Create the log file, adding the default header to it
    // Do this last so we have the best chance of getting the time correct and
    // all sensor names correct
    // Writing to the SD card can be power intensive, so if we're skipping
    // the sensor setup we'll skip this too.
    if (getBatteryVoltage() > 3.4) {
        Serial.println(F("Setting up file on SD card"));
        dataLogger.turnOnSDcard(true);
        // true = wait for card to settle after power up
        dataLogger.createLogFile(true);  // true = write a new header
        dataLogger.turnOffSDcard(true);
        // true = wait for internal housekeeping after write
    }
    /** End [setup_file] */

    /** Start [setup_sleep] */
    // Call the processor sleep
    Serial.println(F("Putting processor to sleep\n"));
    dataLogger.systemSleep();
    /** End [setup_sleep] */
}


// ==========================================================================
//  Arduino Loop Function
// ==========================================================================
#ifndef BUILD_TEST_COMPLEX_LOOP
// Use this short loop for simple data logging and sending
/** Start [simple_loop] */
void loop() {
    // Note:  Please change these battery voltages to match your battery
    // At very low battery, just go back to sleep
    if (getBatteryVoltage() < 3.4) {
        dataLogger.systemSleep();
    } else if (getBatteryVoltage() < 3.55) {
        // At moderate voltage, log data but don't send it over the modem
        dataLogger.logData();
    } else {
        // If the battery is good, send the data to the world
        dataLogger.logDataAndPublish();
    }
}
/** End [simple_loop] */

#else
/** Start [complex_loop] */
// Use this long loop when you want to do something special
// Because of the way alarms work on the RTC, it will wake the processor and
// start the loop every minute exactly on the minute.
// The processor may also be woken up by another interrupt or level change on a
// pin - from a button or some other input.
// The "if" statements in the loop determine what will happen - whether the
// sensors update, testing mode starts, or it goes back to sleep.
void loop() {
    // Reset the watchdog
    dataLogger.watchDogTimer.resetWatchDog();

    // Assuming we were woken up by the clock, check if the current time is an
    // even interval of the logging interval
    // We're only doing anything at all if the battery is above 3.4V
    if (dataLogger.checkInterval() && getBatteryVoltage() > 3.4) {
        // Flag to notify that we're in already awake and logging a point
        Logger::isLoggingNow = true;
        dataLogger.watchDogTimer.resetWatchDog();

        // Print a line to show new reading
        Serial.println(F("------------------------------------------"));
        // Turn on the LED to show we're taking a reading
        dataLogger.alertOn();
        // Power up the SD Card, but skip any waits after power up
        dataLogger.turnOnSDcard(false);
        dataLogger.watchDogTimer.resetWatchDog();

        // Turn on the modem to let it start searching for the network
        // Only turn the modem on if the battery at the last interval was high
        // enough
        // NOTE:  if the modemPowerUp function is not run before the
        // completeUpdate
        // function is run, the modem will not be powered and will not
        // return a signal strength reading.
        if (getBatteryVoltage() > 3.6) modem.modemPowerUp();

        // Start the stream for the modbus sensors, if your RS485 adapter bleeds
        // current from data pins when powered off & you stop modbus serial
        // connection with digitalWrite(5, LOW), below.
        // https://github.com/EnviroDIY/ModularSensors/issues/140#issuecomment-389380833
        altSoftSerial.begin(9600);

        // Do a complete update on the variable array.
        // This this includes powering all of the sensors, getting updated
        // values, and turing them back off.
        // NOTE:  The wake function for each sensor should force sensor setup
        // to run if the sensor was not previously set up.
        varArray.completeUpdate();

        dataLogger.watchDogTimer.resetWatchDog();

        // Reset modbus serial pins to LOW, if your RS485 adapter bleeds power
        // on sleep, because Modbus Stop bit leaves these pins HIGH.
        // https://github.com/EnviroDIY/ModularSensors/issues/140#issuecomment-389380833
        digitalWrite(5, LOW);  // Reset AltSoftSerial Tx pin to LOW
        digitalWrite(6, LOW);  // Reset AltSoftSerial Rx pin to LOW

        // Create a csv data record and save it to the log file
        dataLogger.logToSD();
        dataLogger.watchDogTimer.resetWatchDog();

        // Connect to the network
        // Again, we're only doing this if the battery is doing well
        if (getBatteryVoltage() > 3.55) {
            dataLogger.watchDogTimer.resetWatchDog();
            if (modem.connectInternet()) {
                dataLogger.watchDogTimer.resetWatchDog();
                // Publish data to remotes
                Serial.println(F("Modem connected to internet."));
                dataLogger.publishDataToRemotes();

                // Sync the clock at midnight
                dataLogger.watchDogTimer.resetWatchDog();
                if (Logger::markedEpochTime != 0 &&
                    Logger::markedEpochTime % 86400 == 0) {
                    Serial.println(F("Running a daily clock sync..."));
                    dataLogger.setRTClock(modem.getNISTTime());
                    dataLogger.watchDogTimer.resetWatchDog();
                    modem.updateModemMetadata();
                    dataLogger.watchDogTimer.resetWatchDog();
                }

                // Disconnect from the network
                modem.disconnectInternet();
                dataLogger.watchDogTimer.resetWatchDog();
            }
            // Turn the modem off
            modem.modemSleepPowerDown();
            dataLogger.watchDogTimer.resetWatchDog();
        }

        // Cut power from the SD card - without additional housekeeping wait
        dataLogger.turnOffSDcard(false);
        dataLogger.watchDogTimer.resetWatchDog();
        // Turn off the LED
        dataLogger.alertOff();
        // Print a line to show reading ended
        Serial.println(F("------------------------------------------\n"));

        // Unset flag
        Logger::isLoggingNow = false;
    }

    // Check if it was instead the testing interrupt that woke us up
    if (Logger::startTesting) {
        // Start the stream for the modbus sensors, if your RS485 adapter bleeds
        // current from data pins when powered off & you stop modbus serial
        // connection with digitalWrite(5, LOW), below.
        // https://github.com/EnviroDIY/ModularSensors/issues/140#issuecomment-389380833
        altSoftSerial.begin(9600);

        dataLogger.testingMode();
    }

    // Reset modbus serial pins to LOW, if your RS485 adapter bleeds power
    // on sleep, because Modbus Stop bit leaves these pins HIGH.
    // https://github.com/EnviroDIY/ModularSensors/issues/140#issuecomment-389380833
    digitalWrite(5, LOW);  // Reset AltSoftSerial Tx pin to LOW
    digitalWrite(6, LOW);  // Reset AltSoftSerial Rx pin to LOW

    // Call the processor sleep
    dataLogger.systemSleep();
}
#endif
/** End [complex_loop] */
