#include "consumption.h"

#include <math.h>

ConsumptionResult consumption_from_readings(
    const MeterReading *previous,
    const MeterReading *current,
    ConsumptionInterval *out
) {
    if (previous->sensor_id != current->sensor_id)
        return CONSUMPTION_DIFFERENT_SENSOR;

    if (previous->generation != current->generation)
        return CONSUMPTION_DIFFERENT_GENERATION;

    if (current->sequence <= previous->sequence)
        return CONSUMPTION_INVALID_SEQUENCE;

    if (previous->sequence == current->sequence) {
        // Check if the reading is equal, if not, reject.
        if (previous->total_liters != current->total_liters && previous->occurred_at_ms != current->occurred_at_ms) {
            return CONSUMPTION_INVALID_SEQUENCE;
        }
    }

    if (!isfinite(previous->total_liters) ||
        !isfinite(current->total_liters) ||
        previous->total_liters < 0.0 ||
        current->total_liters < 0.0
    ) {
        return CONSUMPTION_INVALID_VOLUME;
    }

    if (current->total_liters < previous->total_liters)
        return CONSUMPTION_COUNTER_DECREASED;

    if (current->occurred_at_ms <= previous->occurred_at_ms)
        return CONSUMPTION_INVALID_TIME;

    out->sensor_id = current->sensor_id;
    out->generation = current->generation;
    out->start_timestamp_ms = previous->occurred_at_ms;
    out->end_timestamp_ms = current->occurred_at_ms;
    out->liters = current->total_liters - previous->total_liters;

    return CONSUMPTION_OK;
}
