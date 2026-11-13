#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

typedef struct {
    uint32_t sensor_id;
    uint64_t generation; // identifies a lifetime of a meter's sequence numbers.
    uint64_t sequence;
    uint64_t occurred_at_ms;
    double total_liters;
} MeterReading;

typedef struct {
    uint32_t id;
    uint64_t total_liters;
    uint64_t sequence;
} Sensor;

typedef struct SensorRegistry SensorRegistry;

#endif
