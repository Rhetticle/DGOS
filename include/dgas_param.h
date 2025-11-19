/*
 * dgas_param.h
 *
 *  Created on: 11 Jul. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_PARAM_H_
#define DGOS_INCLUDE_DGAS_PARAM_H_

#include <dgas_gauge.h>

extern const GaugeParam paramRPM, paramSpeed, paramEngineLoad, paramCoolant, paramBoost, paramAirTemp,
					paramMAF, paramFuelPressure;

typedef enum {
	GAUGE_PARAM_ID_RPM,
	GAUGE_PARAM_ID_SPEED,
	GAUGE_PARAM_ID_ENGINE_LOAD,
	GAUGE_PARAM_ID_COOLANT,
	GAUGE_PARAM_ID_BOOST,
	GAUGE_PARAM_ID_AIR_TEMP,
	GAUGE_PARAM_ID_MAF,
	GAUGE_PARAM_ID_FUEL_PRESSURE
}GaugeParamID;

// Parameter constants definitions

/********************* RPM ***************************/

#define GAUGE_PARAM_RPM_MIN					0
#define GAUGE_PARAM_RPM_MAX					4200
#define GAUGE_PARAM_RPM_UNITS				"RPM"
#define GAUGE_PARAM_RPM_NAME				"ENGINE SPEED"
#define GAUGE_PARAM_RPM_COLOUR				0xFFFF0000U

/**************** VEHICLE SPEED **********************/

#define GAUGE_PARAM_SPEED_MIN				0
#define GAUGE_PARAM_SPEED_MAX				150
#define GAUGE_PARAM_SPEED_UNITS				"KM/H"
#define GAUGE_PARAM_SPEED_NAME				"VEHICLE SPEED"
#define GAUGE_PARAM_SPEED_COLOUR			0xFF00FFFFU

/***************** ENGINE LOAD ***********************/

#define GAUGE_PARAM_ENGINE_LOAD_MIN			0
#define GAUGE_PARAM_ENGINE_LOAD_MAX			100
#define GAUGE_PARAM_ENGINE_LOAD_UNITS		"%"
#define GAUGE_PARAM_ENGINE_LOAD_NAME		"ENGINE LOAD"
#define GAUGE_PARAM_ENGINE_LOAD_COLOUR		0xFF04FF40U

/***************** COOLANT TEMP **********************/

#define GAUGE_PARAM_COOLANT_TEMP_MIN		0
#define GAUGE_PARAM_COOLANT_TEMP_MAX		120
#define GAUGE_PARAM_COOLANT_TEMP_UNITS		"C"
#define GAUGE_PARAM_COOLANT_TEMP_NAME		"COOLANT TEMP"
#define GAUGE_PARAM_COOLANT_TEMP_COLOUR		0xFF2196F3U

/********************* BOOST *************************/

#define GAUGE_PARAM_BOOST_MIN				0
#define GAUGE_PARAM_BOOST_MAX				60
#define GAUGE_PARAM_BOOST_UNITS				"PSI"
#define GAUGE_PARAM_BOOST_NAME				"BOOST"
#define GAUGE_PARAM_BOOST_COLOUR			0xFFF600B0U

/**************** INTAKE AIR TEMP ********************/

#define GAUGE_PARAM_INTAKE_TEMP_MIN			0
#define GAUGE_PARAM_INTAKE_TEMP_MAX			60
#define GAUGE_PARAM_INTAKE_TEMP_UNITS		"C"
#define GAUGE_PARAM_INTAKE_TEMP_NAME		"INTAKE TEMP"
#define GAUGE_PARAM_INTAKE_TEMP_COLOUR		0xFFF6F200U

/********************** MAF **************************/

#define GAUGE_PARAM_MAF_MIN					0
#define GAUGE_PARAM_MAF_MAX					120
#define GAUGE_PARAM_MAF_UNITS				"gram/s"
#define GAUGE_PARAM_MAF_NAME				"MAF"
#define GAUGE_PARAM_MAF_COLOUR				0xFF6021F3U

/****************** FUEL PRESSURE ********************/

#define GAUGE_PARAM_FUEL_PRESSURE_MIN		0
#define GAUGE_PARAM_FUEL_PRESSURE_MAX		60
#define GAUGE_PARAM_FUEL_PRESSURE_UNITS		"kPSI"
#define GAUGE_PARAM_FUEL_PRESSURE_NAME		"FUEL PRESSURE"
#define GAUGE_PARAM_FUEL_PRESSURE_COLOUR	0xFFF3A521U

#endif /* DGOS_INCLUDE_DGAS_PARAM_H_ */
