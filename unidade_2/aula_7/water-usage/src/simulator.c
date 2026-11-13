#include "simulator.h"
#include "sensor.h"

#include <math.h>
#include <stdint.h>

SensorSimulator sensor_simulator_create(
    uint32_t sensor_id,
    uint64_t generation,
    uint64_t start_timestamp_ms,
    double initial_liters,
    double flow_liters_per_hour
) {
    return (SensorSimulator){
        .current = {
            .sensor_id = sensor_id,
            .generation = generation,
            .sequence = 0,
            .occurred_at_ms = start_timestamp_ms,
            .total_liters = initial_liters
        },
        .flow_liters_per_hours = flow_liters_per_hour,
    };
}

SimulatorResult sensor_simulator_next(SensorSimulator *simulator, uint64_t elapsed_ms, MeterReading *out) {
    if (elapsed_ms == 0)
        return SIMULATOR_INVALID_STEP;

    const MeterReading *current = &simulator->current;
    double flow = simulator->flow_liters_per_hours;

    if (!isfinite(flow) || flow < 0.0 || !isfinite(current->total_liters) || current->total_liters < 0.0) {
        return SIMULATOR_INVALID_STATE;
    }

    if (current->sequence == UINT64_MAX || elapsed_ms > UINT64_MAX - current->occurred_at_ms) {
        return SIMULATOR_OVERFLOW;
    }

    double elapsed_hours = (double)elapsed_ms / 3600000.0;
    double added_liters = flow * elapsed_hours;
    double total_liters = current->total_liters + added_liters;

    if (!isfinite(added_liters) || !isfinite(total_liters))
        return SIMULATOR_OVERFLOW;

    MeterReading next = *current;
    next.sequence++;
    next.occurred_at_ms += elapsed_ms;
    next.total_liters = total_liters;

    simulator->current = next;
    *out = next;

    return SIMULATOR_OK;
}
