/*****************************************************************************
ms_cfg.h_LT5_wifi  - ModularSensors Configuration - tgt _LT5 /WiFi
Written By:  Neil Hancock www.envirodiy.org/members/neilh20/
Development Environment: PlatformIO
Hardware Platform(s): EnviroDIY Mayfly Arduino Datalogger+RS485 Wingboard

Software License: BSD-3.
  Copyright (c) 2018, Neil Hancock - all rights assigned to Stroud Water
Research Center (SWRC) and they may change this title to Stroud Water Research
Center as required and the EnviroDIY Development Team


DISCLAIMER:
THIS CODE IS PROVIDED "AS IS" - NO WARRANTY IS GIVEN.
*****************************************************************************/
#ifndef ms_cfg_h
#define ms_cfg_h
#include <Arduino.h>  // The base Arduino library
// Local default defitions here

//**************************************************************************
// This configuration is for a standard Mayfly0.5b
// Sensors Used - two std to begin then
//#define AnalogProcEC_ACT 1
// Power Availability monitoring decisions use LiIon Voltge,
// Battery Voltage measurements can be derived from a number of sources
// MAYFLY_BAT_A6  - standard measures Solar Charging or LiIon battry V which ever is greated
// MAYFLY_BAT_AA0  - ExternalVolt/ADS1115 requires external R - ECO4
// MAYFLY_BAT_STC3100  sensor IC on RS485 WINGBOARD_KNH002
// MAYFLY_BAT_DIGI Digi Modem LTE with onboard battery measurement
// Choices applied to define MAYFLY_BAT_xx 1) Stc3100 2) ExternVolage_ACT 3) Digi Mode 4) MAYFLY_BAT_A6

#define MAYFLY_BAT_A6 4
//#define MAYFLY_BAT_AA0 2
//FUT #define MAYFLY_BAT_DIGI 3


//#define ENVIRODIY_MAYFLY_TEMPERATURE 1
//#define Decagon_CTD_UUID 1
#define Insitu_TrollSdi12_UUID 1

#define WINGBOARD_KNH002 1
#if defined WINGBOARD_KNH002
//This supports RS485 1.9W and STC3100
//#define USE_STC3100_DD 1
#define MAYFLY_BAT_STC3100 1
// Only one of NOT both KellerAcculevel and KellerNanolevel as share same ADDR
//#define KellerAcculevel_ACT 1
// KellerAcculevel units can be 1 (meter) 2 (feet)
//#define KellerAcculevel_DepthUnits 2

//#define KellerNanolevel_ACT 1
#endif //WINGBOARD_KNH002

//Select one of following MAYFLY_BAT_xx as the source for BatterManagement Analysis
//#define MAYFLY_BAT_CHOICE MAYFLY_BAT_A6
//#define MAYFLY_BAT_CHOICE MAYFLY_BAT_AA0
#define MAYFLY_BAT_CHOICE MAYFLY_BAT_STC3100
// FUT #define MAYFLY_BAT_CHOICE  MAYFLY_BAT_DIGI

//#define ASONG_AM23XX_UUID 1

//Two heavy sensors with power useage
#define BM_PWR_SENSOR_CONFIG_BUILD_SPECIFIC BM_PWR_LOW_REQ

// Mayfly configuration
// Carrier board for Digi XBEE LTE CAT-M1 and jumper from battery
// Digi WiFi S6 plugged in directly
// For debug: C4 removed, strap for AA2/Vbat AA3/SolarV,
#define MFVersion_DEF "v0.5b"
#define MFName_DEF "Mayfly"
#define HwVersion_DEF MFVersion_DEF
#define HwName_DEF MFName_DEF
#define CONFIGURATION_DESCRIPTION_STR "tu_LT5_wifi LT500"

#define USE_MS_SD_INI 1
#define USE_PS_EEPROM 1
#define USE_PS_HW_BOOT 1

//#define USE_PS_modularSensorsCommon 1

#define greenLEDPin 8  // MCU pin for the green LED (-1 if not applicable)
#define redLEDPin 9    // MCU pin for the red LED (-1 if not applicable)

#define sensorPowerPin_DEF 22
#define modemSleepRqPin_DEF 23
#define modemStatusPin_DEF \
    19  // MCU pin used to read modem status (-1 if not applicable)
#define modemResetPin_DEF \
    20  // MCU pin connected to modem reset pin (-1 if unconnected)

#define LOGGERID_DEF_STR "msLog01"
#define NEW_LOGGERID_MAX_SIZE 40
#define configIniID_DEF_STR "ms_cfg.ini"
#define CONFIG_TIME_ZONE_DEF -8

// ** How frequently (in minutes) to log data **
// For two Loggers defined logger2Mult with the faster loggers timeout and the
// multiplier to the slower loggger
// #define  loggingInterval_Fast_MIN (1)
// #define logger2Mult 5 ~Not working for mayfly

// How frequently (in minutes) to log data
#if defined logger2Mult
#define loggingInterval_CDEF_MIN (loggingInterval_Fast_MIN * logger2Mult)
#else
#define loggingInterval_CDEF_MIN 15
#endif  // logger2Mult
// Maximum logging setting allowed
#define loggingInterval_MAX_CDEF_MIN 6 * 60

// Maximum logging setting allowed
#define loggingInterval_MAX_CDEF_MIN 6 * 60


// Instructions: define only one  _Module
#define DigiXBeeWifi_Module 1
//#warning infoMayflyWithDigiXBeeWiFi
//#define DigiXBeeCellularTransparent_Module 1
//#warning infoMayflyWithDigiXBeeCellTransparent
// #define DigiXBeeLTE_Module 1 - unstable
// #define TINY_GSM_MODEM_SIM800  // Select for a SIM800, SIM900, or variant
// thereof #define TINY_GSM_MODEM_UBLOX  // Select for most u-blox cellular
// modems #define TINY_GSM_MODEM_ESP8266  // Select for an ESP8266 using the
// DEFAULT AT COMMAND FIRMWARE End TinyGsmClient.h options
#if defined(DigiXBeeWifi_Module) || defined(DigiXBeeCellularTransparent_Module)
// The Modem is used to push data and also sync Time
// In standalong logger, no internet, Modem can be required at factor to do a
// sync Time Normally enable both of the following. In standalone, disable
// UseModem_PushData.
#define UseModem_Module 1
#define UseModem_PushData 1
//Select buildtime Publishers  supported. 
// The persisten resources (EEPROM) are allocated as a baselevel no matter what options 
#define USE_PUB_MMW      1
//#define USE_PUB_TSMQTT   1
//#define  USE_PUB_UBIDOTS 1

// Required for TinyGsmClient.h
#define TINY_GSM_MODEM_XBEE

// The APN for the gprs connection, unnecessary for WiFi
#define APN_CDEF "VZWINTERNET"

// The WiFi access point  never set to real, as should be set by config.
#define WIFIID_CDEF "WiFiIdDef"
// NULL for none, or  password for connecting to WiFi,
#define WIFIPWD_CDEF "WiFiPwdDef"
#define MMW_TIMER_POST_TIMEOUT_MS_DEF 5000L
//POST PACING ms 0-15000
#define MMW_TIMER_POST_PACING_MS_DEF 100L
//Post MAX Num - is num of MAX num at one go. 0 no limit
//#define MMWGI_POST_MAX_RECS_MUM_DEF 100 //ms_common.h
//Manage Internet - common for all providers
#define MNGI_COLLECT_READINGS_DEF 1
#define MNGI_SEND_OFFSET_MIN_DEF 0
#endif  // Modules

// end of _Module

// This might need revisiting
#define ARD_ANLAOG_MULTIPLEX_PIN A6

//#define SENSOR_CONFIG_GENERAL 1
//#define KellerAcculevel_ACT 1
// Defaults for data.envirodiy.org
#define registrationToken_UUID "registrationToken_UUID"
#define samplingFeature_UUID "samplingFeature_UUID"


#ifdef Decagon_CTD_UUID
// Mayfly definitions
#define CTD10_DEPTH_UUID "CTD10_DEPTH_UUID"
#define CTD10_TEMP_UUID "CTD10_TEMP_UUID"
#define CTD10_COND_UUID "CTD10_COND_UUID"
#endif  // Decagon_CTD_UUID


#ifdef Insitu_TrollSdi12_UUID
// Mayfly definitions
#define ITROLL_DEPTH_UUID "ITROLL_DEPTH_UUID"
#define ITROLL_TEMP_UUID "ITROLL_TEMP_UUID"
//#define ITROLL_PRESSURE_UUID  "ITROLL_PRESSURE_UUID"
#endif  // Insitu_Troll_UUID


#ifdef KellerAcculevel_ACT
#define KellerXxlevel_Height_UUID "KellerXxlevel_Height_UUID"
#define KellerXxlevel_Temp_UUID "KellerXxlevel_Temp_UUID"
#define CONFIG_SENSOR_RS485_PHY 1
#define KellerAcculevelModbusAddress_DEF 0x01
#endif  // KellerAcculevel_ACT

#ifdef KellerNanolevel_ACT
#define KellerXxlevel_Height_UUID "KellerXxlevel_Height_UUID"
#define KellerXxlevel_Temp_UUID "KellerXxlevel_Temp_UUID"
#define CONFIG_SENSOR_RS485_PHY 1
#define KellerNanolevelModbusAddress_DEF 0x01
#endif  // KellerNanolevel_ACT

//#define InsituLTrs485_ACT 1 -not working
#ifdef InsituLTrs485_ACT
#define CONFIG_SENSOR_RS485_PHY 1
#define InsituLTrs485_Height_UUID "KellerNanolevel_Height_UUID"
#define InsituLTrs485_Temp_UUID "KellerNanolevel_Temp_UUID"
#define InsituLTrs485ModbusAddress_DEF 0x01
// Default is 19200 lets hope serial works with it.
#define MODBUS_BAUD_RATE 19200
#endif  // InsituLTrs485_ACT

#ifdef CONFIG_SENSOR_RS485_PHY
// Mayfly definitions
#define CONFIG_HW_RS485PHY_TX_PIN 5  // Mayfly OCRA1 map AltSoftSerial Tx pin
#define CONFIG_HW_RS485PHY_RX_PIN 6  // Mayfly ICES1 map AltSoftSerial Rx pin
#define CONFIG_HW_RS485PHY_DIR_PIN -1
#define max485EnablePin_DEF -1
#define rs485AdapterPower_DEF \
    22  // Pin to switch RS485 adapter power on and off (-1 if unconnected)
#define modbusSensorPower_DEF \
    22;  // Pin to switch power on and off (-1 if unconnected)
#ifndef MODBUS_BAUD_RATE
#define MODBUS_BAUD_RATE 9600
#endif  // MODBUS_BAUD_RATE
#endif  // CONFIG_SENSOR_RS485_PHY


#ifdef AnalogProcEC_ACT
#define EC1_UUID "EC1_UUID"
#define ECpwrPin_DEF A4
#define ECdataPin1_DEF A0
#endif  // AnalogProcEC_ACT

//#define INA219M_PHY_ACT 1
#ifdef INA219M_PHY_ACT
#define INA219M_MA_UUID "INA219_MA_UUID"
#define INA219M_VOLT_UUID "INA219_VOLT_UUID"
#endif  // INA219_PHY_ACT

#if defined ASONG_AM23XX_UUID
#define ASONG_AM23_Air_Temperature_UUID "Air_Temperature_UUID"
#define ASONG_AM23_Air_TemperatureF_UUID "Air_TemperatureF_UUID"
#define ASONG_AM23_Air_Humidity_UUID "Air_Humidity_UUID"
#endif  // ASONG_AM23XX_UUID


#ifdef ENVIRODIY_MAYFLY_TEMPERATURE
#define MaximDS3231_TEMP_UUID "MaximDS3231_TEMP_UUID"
//#define MaximDS3231_TEMPF_UUID "MaximDS3231_TEMPF_UUID"
#endif  // ENVIRODIY_MAYFLY_TEMPERATURE

#if defined UseModem_Module
// This seems to be de-stabilizing Digi S6B
#define DIGI_RSSI_UUID "DIGI_RSSI_UUID"
//#define Modem_SignalPercent_UUID    "SignalPercent_UUID"
#endif  // UseModem_Module

#define ProcessorStats_ACT 1
#if defined ProcessorStats_ACT
#define ProcessorStats_SampleNumber_UUID "SampleNumber_UUID"
#endif  // ProcessorStats_ACT
#if defined MAYFLY_BAT_A6
#define ProcessorStats_Batt_UUID "Batt_UUID"
#endif  // MAYFLY_BAT_A6

#if defined MAYFLY_BAT_STC3100
#define STC3100_Volt_UUID "STC3100Volt_UUID"
#define STC3100_USED1_mAhr_UUID "STC3100used1_mAhr_UUID"
#define STC3100_AVLBL_mAhr_UUID "STC3100avlbl_mAhr_UUID"
#endif // MAYFLY_BAT_STC3100

#ifdef MAYFLY_BAT_AA0
// AA0(AIN0) is 1/10 of Vbat using R+R divider. Requires Mayfly ECO 04
//#define ExternalVoltage_Volt0_UUID "Batt_UUID"
#define ExternalVoltage_Volt0_UUID "Volt0_UUID"
//#define ExternalVoltage_Volt1_UUID "Volt1_UUID"
//#else  // MAYFLY_BAT_AA0
#endif  // MAYFLY_BAT_AA0
#if 0// defined MAYFLY_BAT_A6
#define ProcVolt_ACT 1
#if defined ProcVolt_ACT
#define ProcVolt0_UUID "Batt_UUID"
//#define ProcVolt0_UUID "Volt0_UUID"
//#define ProcVolt1_UUID "Volt1_UUID"
#endif  // ProcVolt_ACT
#endif  // MAYFLY_BAT_A6

#endif  // ms_cfg_h
