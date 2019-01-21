/*****************************************************************************
logging_to_EnviroDIY.ino
Written By:  Sara Damiano (sdamiano@stroudcenter.org)
Development Environment: PlatformIO
Hardware Platform: EnviroDIY Mayfly Arduino Datalogger
Software License: BSD-3.
  Copyright (c) 2017, Stroud Water Research Center (SWRC)
  and the EnviroDIY Development Team

This example sketch is written for ModularSensors library version 0.19.3

This sketch is an example of logging data to an SD card and sending the data to
the EnviroDIY data portal.

DISCLAIMER:
THIS CODE IS PROVIDED "AS IS" - NO WARRANTY IS GIVEN.
*****************************************************************************/

// ==========================================================================
//    Include the base required libraries
// ==========================================================================
#include <Arduino.h>  // The base Arduino library
#include <EnableInterrupt.h>  // for external and pin change interrupts


// ==========================================================================
//    Data Logger Settings
// ==========================================================================
// The library version this example was written for
const char *libraryVersion = "0.19.3";
// The name of this file
const char *sketchName = "logging_to_EnviroDIY.ino";
// Logger ID, also becomes the prefix for the name of the data file on SD card
const char *LoggerID = "XXXXX";
// How frequently (in minutes) to log data
const uint8_t loggingInterval = 5;
// Your logger's timezone.
const int8_t timeZone = -5;  // Eastern Standard Time
// NOTE:  Daylight savings time will not be applied!  Please use standard time!


// ==========================================================================
//    Primary Arduino-Based Board and Processor
// ==========================================================================
#include <sensors/ProcessorStats.h>

const long serialBaud = 115200;   // Baud rate for the primary serial port for debugging
const int8_t greenLED = 8;        // MCU pin for the green LED (-1 if not applicable)
const int8_t redLED = 9;          // MCU pin for the red LED (-1 if not applicable)
const int8_t buttonPin = 21;      // MCU pin for a button to use to enter debugging mode  (-1 if not applicable)
const int8_t wakePin = A7;        // MCU interrupt/alarm pin to wake from sleep
// Set the wake pin to -1 if you do not want the main processor to sleep.
// In a SAMD system where you are using the built-in rtc, set wakePin to 1
const int8_t sdCardPin = 12;      // MCU SD card chip select/slave select pin (must be given!)
const int8_t sensorPowerPin = 22; // MCU pin controlling main sensor power (-1 if not applicable)

// Create and return the processor "sensor"
const char *MFVersion = "v0.5b";
ProcessorStats mayfly(MFVersion);


// ==========================================================================
//    Modem MCU Type and TinyGSM Client
// ==========================================================================

// Select your modem chip - this determines the exact commands sent to it
#define TINY_GSM_MODEM_XBEE  // Select for Digi brand WiFi or Cellular XBee's

#if defined(TINY_GSM_MODEM_XBEE)
  #define TINY_GSM_YIELD() { delay(1); }  // Use to counter slow (9600) baud rate
#endif

// Include TinyGSM for the modem
// This include must be included below the define of the modem name!
#include <TinyGsmClient.h>

// Create a reference to the serial port for the modem
HardwareSerial &modemSerial = Serial1;  // Use hardware serial if possible

// Create a new TinyGSM modem to run on that serial port and return a pointer to it
TinyGsm *tinyModem = new TinyGsm(modemSerial);

// Use this to create a modem if you want to spy on modem communication through
// a secondary Arduino stream.  Make sure you install the StreamDebugger library!
// https://github.com/vshymanskyy/StreamDebugger
// #include <StreamDebugger.h>
// StreamDebugger modemDebugger(modemSerial, Serial);
// TinyGsm *tinyModem = new TinyGsm(modemDebugger);

// Create a new TCP client on that modem and return a pointer to it
TinyGsmClient *tinyClient = new TinyGsmClient(*tinyModem);


// ==========================================================================
//    Specific Modem Pins and On-Off Methods
// ==========================================================================

// This should apply to all Digi brand XBee modules.
#define USE_XBEE_WIFI  // If you're using a S6B wifi XBee
// Describe the physical pin connection of your modem to your board
const long ModemBaud = 9600;        // Communication speed of the modem
const int8_t modemVccPin = -2;      // MCU pin controlling modem power (-1 if not applicable)
const int8_t modemSleepRqPin = 23;  // MCU pin used for modem sleep/wake request (-1 if not applicable)
const int8_t modemStatusPin = 19;   // MCU pin used to read modem status (-1 if not applicable)
const bool modemStatusLevel = LOW;  // The level of the status pin when the module is active (HIGH or LOW)

// Create the wake and sleep methods for the modem
// These can be functions of any type and must return a boolean
// After enabling pin sleep, the sleep request pin is held LOW to keep the XBee on
// Enable pin sleep in the setup function or using XCTU prior to connecting the XBee
bool sleepFxn(void)
{
    if (modemSleepRqPin >= 0)  // Don't go to sleep if there's not a wake pin!
    {
        digitalWrite(modemSleepRqPin, HIGH);
        digitalWrite(redLED, LOW);
        return true;
    }
    else return true;
}
bool wakeFxn(void)
{
    if (modemVccPin >= 0)  // Turns on when power is applied
        return true;
    else if (modemSleepRqPin >= 0)
    {
        digitalWrite(modemSleepRqPin, LOW);
        digitalWrite(redLED, HIGH);  // Because the XBee doesn't have any lights
        return true;
    }
    else return true;
}


// ==========================================================================
//    Network Information and LoggerModem Object
// ==========================================================================

// Network connection information
const char *apn = "xxxxx";  // The APN for the gprs connection, unnecessary for WiFi
const char *wifiId = "xxxxx";  // The WiFi access point, unnecessary for gprs
const char *wifiPwd = "xxxxx";  // The password for connecting to WiFi, unnecessary for gprs

// Create the loggerModem instance
#include <LoggerModem.h>
// A "loggerModem" is a combination of a TinyGSM Modem, a Client, and functions for wake and sleep
#if defined(USE_XBEE_WIFI)
loggerModem modem(modemVccPin, modemStatusPin, modemStatusLevel, wakeFxn, sleepFxn, tinyModem, tinyClient, wifiId, wifiPwd);
// ^^ Use this for WiFi
#else
loggerModem modem(modemVccPin, modemStatusPin, modemStatusLevel, wakeFxn, sleepFxn, tinyModem, tinyClient, apn);
// ^^ Use this for cellular
#endif


// ==========================================================================
//    Maxim DS3231 RTC (Real Time Clock)
// ==========================================================================
#include <sensors/MaximDS3231.h>

// Create and return the DS3231 sensor object
MaximDS3231 ds3231(1);

// ==========================================================================
//    The array that contains all variables to be logged
// ==========================================================================
#include <VariableArray.h>

// Create pointers for all of the variables from the sensors
// at the same time putting them into an array
Variable *variableList[] = {
    new ProcessorStats_SampleNumber(&mayfly, "12345678-abcd-1234-efgh-1234567890ab"),
    new ProcessorStats_FreeRam(&mayfly, "12345678-abcd-1234-efgh-1234567890ab"),
    new ProcessorStats_Batt(&mayfly, "12345678-abcd-1234-efgh-1234567890ab"),
    new MaximDS3231_Temp(&ds3231, "12345678-abcd-1234-efgh-1234567890ab"),
    new Modem_RSSI(&modem, "12345678-abcd-1234-efgh-1234567890ab"),
    new Modem_SignalPercent(&modem, "12345678-abcd-1234-efgh-1234567890ab")
};
// Count up the number of pointers in the array
int variableCount = sizeof(variableList) / sizeof(variableList[0]);

// Create the VariableArray object
VariableArray varArray(variableCount, variableList);

// Create a new logger instance
#include <LoggerBase.h>
Logger dataLogger(LoggerID, loggingInterval, sdCardPin, wakePin, &varArray);


// ==========================================================================
// Device registration and sampling feature information
//   This should be obtained after registration at http://data.envirodiy.org
// ==========================================================================
const char *registrationToken = "12345678-abcd-1234-efgh-1234567890ab";   // Device registration token
const char *samplingFeature = "12345678-abcd-1234-efgh-1234567890ab";     // Sampling feature UUID

// Create a data publisher for the EnviroDIY/WikiWatershed POST endpoint
#include <publishers/EnviroDIYPublisher.h>
EnviroDIYPublisher EnviroDIYPOST(dataLogger, registrationToken, samplingFeature);


// ==========================================================================
//    Working Functions
// ==========================================================================

// Flashes the LED's on the primary board
void greenredflash(uint8_t numFlash = 4, uint8_t rate = 75)
{
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


// Read's the battery voltage
float getBatteryVoltage(const char *version = MFVersion)
{
    float batteryVoltage;
    if (strcmp(version, "v0.3") == 0 or strcmp(version, "v0.4") == 0)
    {
        // Get the battery voltage
        float rawBattery = analogRead(A6);
        batteryVoltage = (3.3 / 1023.) * 1.47 * rawBattery;
    }
    if (strcmp(version, "v0.5") == 0 or strcmp(version, "v0.5b") == 0)
    {
        // Get the battery voltage
        float rawBattery = analogRead(A6);
        batteryVoltage = (3.3 / 1023.) * 4.7 * rawBattery;
    }
    return batteryVoltage;
}


// ==========================================================================
// Main setup function
// ==========================================================================
void setup()
{
    // Start the primary serial connection
    Serial.begin(serialBaud);

    // Print a start-up note to the first serial port
    Serial.print(F("Now running "));
    Serial.print(sketchName);
    Serial.print(F(" on Logger "));
    Serial.println(LoggerID);
    Serial.println();

    Serial.print(F("Using ModularSensors Library version "));
    Serial.println(MODULAR_SENSORS_VERSION);

    if (String(MODULAR_SENSORS_VERSION) !=  String(libraryVersion))
        Serial.println(F(
            "WARNING: THIS EXAMPLE WAS WRITTEN FOR A DIFFERENT VERSION OF MODULAR SENSORS!!"));

    // Start the serial connection with the modem
    modemSerial.begin(ModemBaud);

    // Set up pins for the LED's
    pinMode(greenLED, OUTPUT);
    digitalWrite(greenLED, LOW);
    pinMode(redLED, OUTPUT);
    digitalWrite(redLED, LOW);
    // Blink the LEDs to show the board is on and starting up
    greenredflash();

    // Set up some of the power pins so the board boots up with them off
    // NOTE:  This isn't necessary at all.  The logger begin() function
    // should leave all power pins off when it finishes.
    if (modemVccPin >= 0)
    {
        pinMode(modemVccPin, OUTPUT);
        digitalWrite(modemVccPin, LOW);
    }
    if (sensorPowerPin >= 0)
    {
        pinMode(sensorPowerPin, OUTPUT);
        digitalWrite(sensorPowerPin, LOW);
    }

    // Set up the sleep/wake pin for the modem and put its inital value as "off"
    Serial.println(F("Setting up sleep mode on the XBee."));
    pinMode(modemSleepRqPin, OUTPUT);
    digitalWrite(modemSleepRqPin, LOW);  // Turn it on to talk, just in case
    tinyModem->init();  // initialize
    if (tinyModem->commandMode())
    {
        tinyModem->sendAT(F("SM"),1);  // Pin sleep
        tinyModem->waitResponse();
        tinyModem->sendAT(F("DO"),0);  // Disable remote manager
        tinyModem->waitResponse();
        tinyModem->sendAT(F("SO"),0);  // For Cellular - disconnected sleep
        tinyModem->waitResponse();
        tinyModem->sendAT(F("SO"),200);  // For WiFi - Disassociate from AP for Deep Sleep
        tinyModem->waitResponse();
        tinyModem->writeChanges();
        tinyModem->exitCommand();
    }
    digitalWrite(modemSleepRqPin, HIGH);  // back to sleep

    // Set the timezone and offsets
    // Logging in the given time zone
    Logger::setTimeZone(timeZone);
    // Offset is the same as the time zone because the RTC is in UTC
    Logger::setTZOffset(timeZone);

    // Attach the modem and information pins to the logger
    dataLogger.attachModem(modem);
    dataLogger.setAlertPin(greenLED);
    dataLogger.setTestingModePin(buttonPin);

    // Begin the logger
    // At lowest battery level, skip sensor set-up
    // Note:  Please change these battery voltages to match your battery
    if (getBatteryVoltage() < 3.4) dataLogger.begin(true);
    else dataLogger.begin();  // set up sensors

    // At very good battery voltage, or with suspicious time stamp, sync the clock
    // Note:  Please change these battery voltages to match your battery
    if (getBatteryVoltage() > 3.9 ||
        dataLogger.getNowEpoch() < 1545091200 ||  /*Before 12/18/2018*/
        dataLogger.getNowEpoch() > 1735689600)  /*Before 1/1/2025*/
        dataLogger.syncRTC();
}


// ==========================================================================
// Main loop function
// ==========================================================================
void loop()
{
    // Log the data
    // Note:  Please change these battery voltages to match your battery
    if (getBatteryVoltage() < 3.4) dataLogger.systemSleep();  // just go back to sleep
    else if (getBatteryVoltage() < 3.7) dataLogger.logData();  // log data, but don't send
    else dataLogger.logDataAndSend();  // send data
}
