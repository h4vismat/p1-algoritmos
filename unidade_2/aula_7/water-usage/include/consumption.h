#ifndef WATER_CONSUMPTION_H
#define WATER_CONSUMPTION_H

#include "sensor.h"
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    CONSUMPTION_OK,
    CONSUMPTION_DIFFERENT_SENSOR,
    CONSUMPTION_DIFFERENT_GENERATION,
    CONSUMPTION_INVALID_SEQUENCE,
    CONSUMPTION_INVALID_TIME,
    CONSUMPTION_INVALID_VOLUME,
    CONSUMPTION_COUNTER_DECREASED
} ConsumptionResult;

typedef struct {
    uint32_t sensor_id;
    uint64_t generation;
    uint64_t start_timestamp_ms;
    uint64_t end_timestamp_ms;
    double liters;
} ConsumptionInterval;

/*
 * Borrowed contiguous sequence.
 * data may be NULL when length == 0.
 */
typedef struct {
    const ConsumptionInterval *data;
    size_t length;
} ConsumptionView;

/*
 * Computes an interval for two given meter readings.
 *
 * Success:
 *  Writes the interval between previous and current.
 *
 * Failure:
 *  Leaves *out unchanged.
 */
ConsumptionResult consumption_from_readings(
    const MeterReading *previous,
    const MeterReading *current,
    ConsumptionInterval *out
);

#endif
