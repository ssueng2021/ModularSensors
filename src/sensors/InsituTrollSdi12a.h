/*
 * @file InsituTrollSdi12a.h
 * @copyright 2020 Stroud Water Research Center
 * Part of the EnviroDIY modular sensors
 * @author Neil Hancock  https://github.com/neilh10/ModularSensors/
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Contains the InsituTrollSdi12a subclass of the SDI12Sensors class
 * along with the variable subclasses InsituTrollSdi12a_Pressure,
 * InsituTrollSdi12a_Temp, and InsituTrollSdi12a_Depth
 *
 * These are used for the Insitu Troll.
 * The order and units are the default settings for the ITROLL
 *
 * This depends on the EnviroDIY SDI-12 library and the SDI12Sensors super
 * class.
 *
 */
/* clang-format off */
/**
 * @defgroup sensor_instutroll Insitu LevelTroll 400 500 700
 * Classes for the Insitru LevelTroll feature sensors  pressure, temperature, and depth.
 * 
 * @ingroup sdi12_group
 *
 * @tableofcontents
 * @m_footernavigation
 *
 * @section sensor_instutroll_intro Introduction
 *
 * > A slim 1.8 cm diameter sensor, 
 * > depth measuremente temperature compensated to 0.1% (0.05%) across Full Scale depth range and across temperature range. 
 * >
 * > Has an internal logger for reliable data collection.
 * >
 * > Reports sensor serial number and model in uSD .csv file

 *
 * The Insitu Aqua/Level Troll require 8-36VDC
 * This can be achieved a Polo #boost device, instructions are at the end
 *
 * @warning Coming from the factory, Troll sensors are set at SDI-12 address '0'.
 *
 * The Insitu Aqua/Level Trolls are programmed through WinSitu.
 * 
 * The SDI address needs to be changed to what the class is set to - default is '1'.
 *
 * Parameters are very flexible and need to be aligned used WinSitu with this module.
 * 
 * The depth sensor third paramter may need to be created. The expected
 * paramters and order are Pressure (PSI),  Temperature (C),  Depth (ft). 
 *
 * Tested with Level Troll 500.
 * 
 * @section sensor_instutroll_datasheet Sensor Datasheet
 * Documentation for the SDI-12 Protocol commands and responses
 * The Insitu Level/Aqua Troll can be found at:
 * 
 * https://in-situ.com/en/pub/media/support/documents/SDI-12_Commands_Tech_Note.pdf
 * 
 * https://in-situ.com/us/support/documents/sdi-12-commands-and-level-troll-400500700-responses
 *
 * @section sensor_instutroll_flags Build flags
 * @see @ref sdi12_group_flags
 *
 * @menusnip{instutroll}
 */
/* clang-format on */

// Header Guards
#ifndef SRC_SENSORS_INSITUTROLLSDI12_H_
#define SRC_SENSORS_INSITUTROLLSDI12_H_

// Included Dependencies
#include "sensors/SDI12Sensors.h"

// Sensor Specific Defines
/** @ingroup sensor_insitutroll */
/**@{*/
/// @brief Sensor::_numReturnedValues; the Troll 500 can report 3 values.
#define ITROLLA_NUM_VARIABLES 3

/**
 * @anchor sensor_insitutroll_timing
 * @name Sensor Timing
 * The sensor timing for a Insitu Troll
 */
/**@{*/
/// @brief Sensor::_warmUpTime_ms; maximum warm-up time in SDI-12 mode: 500ms
#define ITROLLA_WARM_UP_TIME_MS 500


/// @brief Sensor::_stabilizationTime_ms; the Troll 500 is stable as soon as it
/// warms up (0ms stabilization).
#define ITROLLA_STABILIZATION_TIME_MS 0

/// @brief Sensor::_measurementTime_ms; maximum measurement duration: 500ms.
#define ITROLLA_MEASUREMENT_TIME_MS 500

/**
 * @anchor sensor_insitutroll_pressure
 * @name Pressure
 * The pressue variable from a Insitu Troll
 * - Range is 0 – x (depends on range eg 5psig)

 *
 * {{ @ref InsituTrollSdi12a_Pressure::InsituTrollSdi12a_Pressure }}
 */
/**@{*/
/**
 * @brief Decimals places in string representation; conductivity should have 1.
 *
 * 0 are reported, adding extra digit to resolution to allow the proper number
 * of significant figures for averaging - resolution is 0.001 mS/cm = 1 µS/cm
 */
#define ITROLLA_PRESSURE_RESOLUTION 5
/// @brief Sensor variable number; pressure is stored in sensorValues[0].
#define ITROLLA_PRESSURE_VAR_NUM 0
/// @brief Variable name in
/// [ODM2 controlled vocabulary](http://vocabulary.odm2.org/variablename/);
/// "specificConductance"
#define ITROLLA_PRESSURE_VAR_NAME "pressureGauge"
/// @brief Variable unit name in
/// [ODM2 controlled vocabulary](http://vocabulary.odm2.org/units/);
/// "pounds per square inch" (psi)
#define ITROLLA_PRESSURE_UNIT_NAME "psi"
/// @brief Default variable short code; "ITROLLpressure"
#define ITROLLA_PRESSURE_DEFAULT_CODE "ITROLLpressure"
/**@}*/

/**
 * @anchor sensor_insitutroll_temp
 * @name Temperature
 * The temperature variable from a Insitu Troll
 * - Range is -11°C to +49°C
 * - Accuracy is ±1°C
 *
 * {{ @ref InsituTrollSdi12a_Temp::InsituTrollSdi12a_Temp }}
 */
/**@{*/
/**
 * @brief Decimals places in string representation; temperature should have 2.
 *
 * 1 is reported, adding extra digit to resolution to allow the proper number
 * of significant figures for averaging  - resolution is 0.1°C
 */
#define ITROLLA_TEMP_RESOLUTION 2
/// @brief Sensor variable number; temperature is stored in sensorValues[1].
#define ITROLLA_TEMP_VAR_NUM 1
/// @brief Variable name in
/// [ODM2 controlled vocabulary](http://vocabulary.odm2.org/variablename/);
/// "temperature"
#define ITROLLA_TEMP_TEMP_VAR_NAME "temperature"
/// @brief Variable unit name in
/// [ODM2 controlled vocabulary](http://vocabulary.odm2.org/units/);
/// "degreeCelsius" (°C)
#define ITROLLA_TEMP_TEMP_UNIT_NAME "degreeCelsius"
/// @brief Default variable short code; "ITROLLtemp"
#define ITROL_TEMP_DEFAULT_CODE "ITROLLtemp"
/**@}*/

/**
 * @anchor sensor_insitutroll_depth
 * @name Water Depth
 * The water depth variable from a Insitu Troll
 * - Range is 0 to 3.5m to 350m depending on model
 * - Accuracy is ±0.05% of full scale
 *
 * {{ @ref InsituTrollSdi12a_Depth::InsituTrollSdi12a_Depth }}
 */
/**@{*/
/**
 * @brief Decimals places in string representation; depth should have 1.
 *
 * 0 are reported, adding extra digit to resolution to allow the proper number
 * of significant figures for averaging - resolution is 2 mm
 */
#define ITROLLA_DEPTH_RESOLUTION 5
/// @brief Sensor variable number; depth is stored in sensorValues[2].
#define ITROLLA_DEPTH_VAR_NUM 2
/// @brief Variable name in
/// [ODM2 controlled vocabulary](http://vocabulary.odm2.org/variablename/);
/// "waterDepth"
#define ITROLLA_DEPTH_VAR_NAME "waterDepth"
/// @brief Variable unit name in
/// [ODM2 controlled vocabulary](http://vocabulary.odm2.org/units/);
/// "millimeter"
#define ITROLLA_DEPTH_UNIT_NAME "feet"
/// @brief Default variable short code; "ITROLLdepth"
#define ITROLLA_DEPTH_DEFAULT_CODE "ITROLLdepth"
/**@}*/


/* clang-format off */
/**
 * @brief The Sensor sub-class for the
 * [Insitu Level/Aqua Troll pressure, temperature, and depth sensor](@ref sensor_insitutroll)
 *
 * @ingroup sensor_insitutroll
 */
/* clang-format on */

class InsituTrollSdi12a : public SDI12Sensors {
 public:
    // Constructors with overloads
    /**
     * @brief Construct a new ITROLL object.
     *
     * The SDI-12 address of the sensor, the Arduino pin controlling power
     * on/off, and the Arduino pin sending and receiving data are required for
     * the sensor constructor.  Optionally, you can include a number of distinct
     * readings to average.  The data pin must be a pin that supports pin-change
     * interrupts.
     *
     * @param SDI12address The SDI-12 address; can be a char,
     * char*, or int.
     * @warning The SDI-12 address **must** be changed from the factory
     * programmed value of "0" before the sensor can be used with
     * ModularSensors!
     * @param powerPin The pin on the mcu controlling power to the sensor.
     * Use -1 if it is continuously powered.
     * - The ITROLL requires a power supply, which can be turned off
     * between measurements
     * @param dataPin The pin on the mcu connected to the data line of the
     * SDI-12 circuit.
     * @param measurementsToAverage The number of measurements to take and
     * average before giving a "final" result from the sensor; optional with a
     * default value of 1.
     */
    InsituTrollSdi12a(char SDI12address, int8_t powerPin, int8_t dataPin,
                      uint8_t measurementsToAverage = 1)
        : SDI12Sensors(SDI12address, powerPin, dataPin, measurementsToAverage,
                       "InsituTrollSdi12a", ITROLLA_NUM_VARIABLES,
                       ITROLLA_WARM_UP_TIME_MS, ITROLLA_STABILIZATION_TIME_MS,
                       ITROLLA_MEASUREMENT_TIME_MS) {}
    InsituTrollSdi12a(char* SDI12address, int8_t powerPin, int8_t dataPin,
                      uint8_t measurementsToAverage = 1)
        : SDI12Sensors(SDI12address, powerPin, dataPin, measurementsToAverage,
                       "InsituTrollSdi12a", ITROLLA_NUM_VARIABLES,
                       ITROLLA_WARM_UP_TIME_MS, ITROLLA_STABILIZATION_TIME_MS,
                       ITROLLA_MEASUREMENT_TIME_MS) {}
    InsituTrollSdi12a(int SDI12address, int8_t powerPin, int8_t dataPin,
                      uint8_t measurementsToAverage = 1)
        : SDI12Sensors(SDI12address, powerPin, dataPin, measurementsToAverage,
                       "InsituTrollSdi12a", ITROLLA_NUM_VARIABLES,
                       ITROLLA_WARM_UP_TIME_MS, ITROLLA_STABILIZATION_TIME_MS,
                       ITROLLA_MEASUREMENT_TIME_MS) {}
    /**
     * @brief Destroy the ITROL object
     */
    ~InsituTrollSdi12a() {}
};


/* clang-format off */
/**
 * @brief The Variable sub-class used for the
 * [pressure output](@ref sensor_insitutroll_pressure) from a
 * [Insitu Troll 3-in-1 water level sensor.](@ref sensor_insitutroll)
 *
 * @ingroup sensor_insitutroll
 */
/* clang-format on */
class InsituTrollSdi12a_Pressure : public Variable {
 public:
    /**
     * @brief Construct a new InsituTrollSdi12a_Pressure object.
     *
     * @param parentSense The parent InsituTrollSdi12a providing values.
     * @param uuid A universally unique identifier (UUID or GUID) for the
     * variable; optional with the default value of an empty string.
     * @param varCode A short code to help identify the variable in files;
     * optional with a default value of "ITROLLPressure".
     */
    InsituTrollSdi12a_Pressure(
        Sensor* parentSense, const char* uuid = "",
        const char* varCode = ITROLLA_PRESSURE_DEFAULT_CODE)
        : Variable(parentSense, (const uint8_t)ITROLLA_PRESSURE_VAR_NUM,
                   (uint8_t)ITROLLA_PRESSURE_RESOLUTION,
                   ITROLLA_PRESSURE_VAR_NAME, ITROLLA_PRESSURE_UNIT_NAME,
                   varCode, uuid) {}
    /**
     * @brief Construct a new InsituTrollSdi12a_Pressure object.
     *
     * @note This must be tied with a parent InsituTrollSdi12a before it can be
     * used.
     */
    InsituTrollSdi12a_Pressure()
        : Variable((const uint8_t)ITROLLA_PRESSURE_VAR_NUM,
                   (uint8_t)ITROLLA_PRESSURE_RESOLUTION,
                   ITROLLA_PRESSURE_VAR_NAME, ITROLLA_PRESSURE_UNIT_NAME,
                   ITROLLA_PRESSURE_DEFAULT_CODE) {}
    /**
     * @brief Destroy the InsituTrollSdi12a_Pressure object - no action needed.
     */
    ~InsituTrollSdi12a_Pressure() {}
};


/* clang-format off */
/**
 * @brief The Variable sub-class used for the
 * [temperature Output](@ref sensor_insitutroll_temp) from a
 * [Insitu Troll 3-in-1 water level sensor.](@ref sensor_insitutroll)
 *
 * @ingroup sensor_insitutroll
 */
/* clang-format on */
class InsituTrollSdi12a_Temp : public Variable {
 public:
    /**
     * @brief Construct a new InsituTrollSdi12a_Temp object.
     *
     * @param parentSense The parent InsituTrollSdi12a providing the values.
     * @param uuid A universally unique identifier (UUID or GUID) for the
     * variable; optional with the default value of an empty string.
     * @param varCode A short code to help identify the variable in files;
     * optional with a default value of "ITROLLtemp".
     */
    InsituTrollSdi12a_Temp(Sensor* parentSense, const char* uuid = "",
                           const char* varCode = ITROL_TEMP_DEFAULT_CODE)
        : Variable(parentSense, (const uint8_t)ITROLLA_TEMP_VAR_NUM,
                   (uint8_t)ITROLLA_TEMP_RESOLUTION, ITROLLA_TEMP_TEMP_VAR_NAME,
                   ITROLLA_TEMP_TEMP_UNIT_NAME, varCode, uuid) {}

    /**
     * @brief Construct a new InsituTrollSdi12a_Temp object.
     *
     * @note This must be tied with a parent InsituTrollSdi12a before it can be
     * used.
     */
    InsituTrollSdi12a_Temp()
        : Variable((const uint8_t)ITROLLA_TEMP_VAR_NUM,
                   (uint8_t)ITROLLA_TEMP_RESOLUTION, ITROLLA_TEMP_TEMP_VAR_NAME,
                   ITROLLA_TEMP_TEMP_UNIT_NAME, ITROL_TEMP_DEFAULT_CODE) {}
    /**
     * @brief Destroy the InsituTrollSdi12a_Temp object - no action needed.
     */
    ~InsituTrollSdi12a_Temp() {}
};


/* clang-format off */
/**
 * @brief The Variable sub-class used for the
 * [depth output](@ref sensor_insitutroll_depth) from a
 * [Insitu Troll 3-in-1 water level sensor.](@ref sensor_insitutroll)
 *
 * @ingroup sensor_insitutroll
 */
/* clang-format on */
class InsituTrollSdi12a_Depth : public Variable {
 public:
    /**
     * @brief Construct a new InsituTrollSdi12a_Depth object.
     *
     * @param parentSense The parent InsituTrollSdi12a providing the values.
     * @param uuid A universally unique identifier (UUID or GUID) for the
     * variable; optional with the default value of an empty string.
     * @param varCode A short code to help identify the variable in files;
     * optional with a default value of "ITROLLdepth".
     */
    InsituTrollSdi12a_Depth(Sensor* parentSense, const char* uuid = "",
                            const char* varCode = ITROLLA_DEPTH_DEFAULT_CODE)
        : Variable(parentSense, (const uint8_t)ITROLLA_DEPTH_VAR_NUM,
                   (uint8_t)ITROLLA_DEPTH_RESOLUTION, ITROLLA_DEPTH_VAR_NAME,
                   ITROLLA_DEPTH_UNIT_NAME, varCode, uuid) {}
    /**
     * @brief Construct a new InsituTrollSdi12a_Depth object.
     *
     * @note This must be tied with a parent InsituTrollSdi12a before it can be
     * used.
     */
    InsituTrollSdi12a_Depth()
        : Variable((const uint8_t)ITROLLA_DEPTH_VAR_NUM,
                   (uint8_t)ITROLLA_DEPTH_RESOLUTION, ITROLLA_DEPTH_VAR_NAME,
                   ITROLLA_DEPTH_UNIT_NAME, ITROLLA_DEPTH_DEFAULT_CODE) {}
    /**
     * @brief Destroy the InsituTrollSdi12a_Depth object - no action needed.
     */
    ~InsituTrollSdi12a_Depth() {}
};
/**@}*/
#endif  // SRC_SENSORS_INSITUTROLLSDI12_H_
