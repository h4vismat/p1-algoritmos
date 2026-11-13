#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "sensor.h"

#include <stdint.h>

typedef struct {
    MeterReading current;
    double flow_liters_per_hours;
} SensorSimulator;

typedef enum {
    SIMULATOR_OK,
    SIMULATOR_INVALID_STATE,
    SIMULATOR_INVALID_STEP,
    SIMULATOR_OVERFLOW
} SimulatorResult;

/*
 * Creates a baseline with sequence 0.
 */
SensorSimulator sensor_simulator_create(
    uint32_t sensor_id,
    uint64_t generation,
    uint64_t start_timestamp_ms,
    double initial_liters,
    double flow_liters_per_hour
);

/*
 * Advances time and emits a reading.
 * On failures, leaves simulator and *out unchanged.
 */
SimulatorResult sensor_simulator_next(SensorSimulator *simulator, uint64_t elapsed_ms, MeterReading *out);

#endif
