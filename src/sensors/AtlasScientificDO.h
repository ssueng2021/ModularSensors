/**
 * @file AtlasScientificDO.h
 * @brief Contains the AtlasScientificCO2 subclass of the AtlasParent sensor
 * class along with the variable subclasses AtlasScientificDO_DOmgL and
 * AtlasScientificDO_DOpct.
 *
 * These are used for any sensor attached to an Atlas EZO DO circuit.
 *
 * This depends on the Arduino core Wire library.
 *
 * The Atlas Scientifc DO sensor outputs DO in both mg/L and percent saturation
 *     Accuracy is ± 0.05 mg/L
 *     Range is 0.01 − 100+ mg/L (0.1 − 400+ % saturation)
 *     Resolution is 0.01 mg/L or 0.1 % saturation
 *
 * Part of the EnviroDIY ModularSensors library for Arduino
 * @copyright 2020 Stroud Water Research Center
 * @author Initial developement for Atlas Sensors was done by Adam Gold
 * Files were edited by Sara Damiano <sdamiano@stroudcenter.org>
 */

// Header Guards
#ifndef SRC_SENSORS_ATLASSCIENTIFICDO_H_
#define SRC_SENSORS_ATLASSCIENTIFICDO_H_

// Debugging Statement
// #define MS_ATLASSCIENTIFICDO_DEBUG

#ifdef MS_ATLASSCIENTIFICDO_DEBUG
#define MS_DEBUGGING_STD "AtlasScientificDO"
#endif

// Included Dependencies
#include "ModSensorDebugger.h"
#undef MS_DEBUGGING_STD
#include "VariableBase.h"
#include "sensors/AtlasParent.h"

// I2C address
#define ATLAS_DO_I2C_ADDR 0x61  // 97

// Sensor Specific Defines
#define ATLAS_DO_NUM_VARIABLES 2

#define ATLAS_DO_WARM_UP_TIME_MS 745  // 737-739 in tests
#define ATLAS_DO_STABILIZATION_TIME_MS 0
// 555 measurement time in tests, but keep the 600 recommended by manual
#define ATLAS_DO_MEASUREMENT_TIME_MS 600

#define ATLAS_DOMGL_RESOLUTION 2
#define ATLAS_DOMGL_VAR_NUM 0

#define ATLAS_DOPCT_RESOLUTION 1
#define ATLAS_DOPCT_VAR_NUM 1

// The main class for the Atlas Scientific DO sensor
class AtlasScientificDO : public AtlasParent {
 public:
    explicit AtlasScientificDO(int8_t  powerPin,
                               uint8_t i2cAddressHex = ATLAS_DO_I2C_ADDR,
                               uint8_t measurementsToAverage = 1);
    ~AtlasScientificDO();

    bool setup(void) override;
};

// The class for the DO Concentration Variable
class AtlasScientificDO_DOmgL : public Variable {
 public:
    explicit AtlasScientificDO_DOmgL(AtlasScientificDO* parentSense,
                                     const char*        uuid    = "",
                                     const char*        varCode = "AtlasDOmgL")
        : Variable(parentSense, (const uint8_t)ATLAS_DOMGL_VAR_NUM,
                   (uint8_t)ATLAS_DOMGL_RESOLUTION, "oxygenDissolved",
                   "milligramPerLiter", varCode, uuid) {}
    AtlasScientificDO_DOmgL()
        : Variable((const uint8_t)ATLAS_DOMGL_VAR_NUM,
                   (uint8_t)ATLAS_DOMGL_RESOLUTION, "oxygenDissolved",
                   "milligramPerLiter", "AtlasDOmgL") {}
    ~AtlasScientificDO_DOmgL() {}
};

// The class for the DO Percent of Saturation Variable
class AtlasScientificDO_DOpct : public Variable {
 public:
    explicit AtlasScientificDO_DOpct(AtlasScientificDO* parentSense,
                                     const char*        uuid    = "",
                                     const char*        varCode = "AtlasDOpct")
        : Variable(parentSense, (const uint8_t)ATLAS_DOPCT_VAR_NUM,
                   (uint8_t)ATLAS_DOPCT_RESOLUTION,
                   "oxygenDissolvedPercentOfSaturation", "percent", varCode,
                   uuid) {}
    AtlasScientificDO_DOpct()
        : Variable((const uint8_t)ATLAS_DOPCT_VAR_NUM,
                   (uint8_t)ATLAS_DOPCT_RESOLUTION,
                   "oxygenDissolvedPercentOfSaturation", "percent",
                   "AtlasDOpct") {}
    ~AtlasScientificDO_DOpct() {}
};

#endif  // SRC_SENSORS_ATLASSCIENTIFICDO_H_
