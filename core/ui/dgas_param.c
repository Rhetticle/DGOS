/*
 * dgas_param.c
 *
 *  Created on: 11 Jul. 2025
 *      Author: rhett
 */

// Definitions of gauge parameters

#include <dgas_param.h>

// engine speed
const GaugeParam paramRPM = {.min = GAUGE_PARAM_RPM_MIN,
							 .max = GAUGE_PARAM_RPM_MAX,
							 .units = GAUGE_PARAM_RPM_UNITS,
							 .name = GAUGE_PARAM_RPM_NAME,
							 .colour = GAUGE_PARAM_RPM_COLOUR};

// vehicle speed
const GaugeParam paramSpeed = {.min = GAUGE_PARAM_SPEED_MIN,
							   .max = GAUGE_PARAM_SPEED_MAX,
							   .units = GAUGE_PARAM_SPEED_UNITS,
							   .name = GAUGE_PARAM_SPEED_NAME,
							   .colour = GAUGE_PARAM_SPEED_COLOUR};

// engine load
const GaugeParam paramEngineLoad = {.min = GAUGE_PARAM_ENGINE_LOAD_MIN,
									.max = GAUGE_PARAM_ENGINE_LOAD_MAX,
									.units = GAUGE_PARAM_ENGINE_LOAD_UNITS,
									.name = GAUGE_PARAM_ENGINE_LOAD_NAME,
									.colour = GAUGE_PARAM_ENGINE_LOAD_COLOUR};

// coolant temperature
const GaugeParam paramCoolant = {.min = GAUGE_PARAM_COOLANT_TEMP_MIN,
								 .max = GAUGE_PARAM_COOLANT_TEMP_MAX,
								 .units = GAUGE_PARAM_COOLANT_TEMP_UNITS,
								 .name = GAUGE_PARAM_COOLANT_TEMP_NAME,
								 .colour = GAUGE_PARAM_COOLANT_TEMP_COLOUR};

// Boost
const GaugeParam paramBoost = {.min = GAUGE_PARAM_BOOST_MIN,
							   .max = GAUGE_PARAM_BOOST_MAX,
							   .units = GAUGE_PARAM_BOOST_UNITS,
							   .name = GAUGE_PARAM_BOOST_NAME,
							   .colour = GAUGE_PARAM_BOOST_COLOUR};

// Intake air temperature
const GaugeParam paramAirTemp = {.min = GAUGE_PARAM_INTAKE_TEMP_MIN,
								 .max = GAUGE_PARAM_INTAKE_TEMP_MAX,
								 .units = GAUGE_PARAM_INTAKE_TEMP_UNITS,
								 .name = GAUGE_PARAM_INTAKE_TEMP_NAME,
								 .colour = GAUGE_PARAM_INTAKE_TEMP_COLOUR};

// Mass air flow (MAF)
const GaugeParam paramMAF = {.min = GAUGE_PARAM_MAF_MIN,
							 .max = GAUGE_PARAM_MAF_MAX,
							 .units = GAUGE_PARAM_MAF_UNITS,
							 .name = GAUGE_PARAM_MAF_NAME,
							 .colour = GAUGE_PARAM_MAF_COLOUR};

// Fuel pressure
const GaugeParam paramFuelPressure = {.min = GAUGE_PARAM_FUEL_PRESSURE_MIN,
									  .max = GAUGE_PARAM_FUEL_PRESSURE_MAX,
									  .units = GAUGE_PARAM_FUEL_PRESSURE_UNITS,
									  .name = GAUGE_PARAM_FUEL_PRESSURE_NAME,
									  .colour = GAUGE_PARAM_FUEL_PRESSURE_COLOUR};
