#include "monitor.h"
#include "consumption.h"
#include "log.h"
#include "sensor.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    PROCESS_LIVE,
    PROCESS_REPLAY
} ProcessingMode;

typedef struct {
    uint32_t sensor_id;
    bool has_previous;
    MeterReading previous;
} SensorState;

struct Monitor {
    SensorState *sensors;
    size_t length;
    size_t capacity;

    Log *log;
};

Monitor *monitor_create(Log *log) {
    Monitor *monitor = malloc(sizeof *monitor);

    if (monitor == NULL)
        return NULL;

    *monitor = (Monitor){
        .sensors = NULL,
        .length = 0,
        .capacity = 0,
        .log  = log
    };

    return monitor;
}

void monitor_destroy(Monitor *monitor) {
    if (monitor == NULL)
        return;

    free(monitor->sensors);
    free(monitor);
}

static SensorState *find_sensor(struct Monitor *monitor, uint32_t id) {
    for (size_t i = 0; i < monitor->length; i++) {
        if (monitor->sensors[i].sensor_id == id)
            return &monitor->sensors[i];
    }

    return NULL;
}

/*
 * Ensures room for one additional sensor.
 */
static MonitorRegisterResult reserve_one_sensor(Monitor *monitor) {
    if (monitor->length < monitor->capacity)
        return MONITOR_REGISTER_OK;

    const size_t max_capacity = SIZE_MAX / sizeof *monitor->sensors;

    if (monitor->capacity >= max_capacity)
        return MONITOR_REGISTER_CAPACITY_LIMIT;

    size_t new_capacity;

    if (monitor->capacity == 0) {
        new_capacity = max_capacity < 8 ? max_capacity : 8;
    } else if (monitor->capacity > max_capacity / 2) {
        new_capacity = max_capacity;
    } else {
        new_capacity = monitor->capacity * 2;
    }

    SensorState *new_sensors = realloc(monitor->sensors, new_capacity * sizeof *monitor->sensors);

    if (new_sensors == NULL)
        return MONITOR_REGISTER_NO_MEMORY;

    monitor->sensors = new_sensors;
    monitor->capacity = new_capacity;

    return MONITOR_REGISTER_OK;
}

MonitorRegisterResult monitor_register_sensor(Monitor *monitor, uint32_t sensor_id) {
    if (find_sensor(monitor, sensor_id) != NULL)
        return MONITOR_REGISTER_DUPLICATE;

    MonitorRegisterResult result = reserve_one_sensor(monitor);

    if (result != MONITOR_REGISTER_OK)
        return result;

    monitor->sensors[monitor->length] = (SensorState){
        .sensor_id = sensor_id,
        .has_previous = false
    };

    monitor->length++;

    return MONITOR_REGISTER_OK;
}

static LogResult record_reading(Log *log, const MeterReading *reading) {
    LogResult result = log_append(log, reading);

    if (result != LOG_OK)
        return result;

    return log_flush(log);
}

MonitorIngestResult process_reading(Monitor *monitor, const MeterReading *reading, ConsumptionInterval *out, ProcessingMode mode) {
    SensorState *sensor = find_sensor(monitor, reading->sensor_id);

    if (sensor == NULL)
        return MONITOR_INGEST_UNKNOWN_SENSOR;

    if (!sensor->has_previous) {
        if (mode == PROCESS_LIVE) {
            if (record_reading(monitor->log, reading) != LOG_OK)
                return MONITOR_INGEST_LOG_ERROR;
        }

        sensor->previous = *reading;
        sensor->has_previous = true;

        return MONITOR_INGEST_BASELINE;
    }

    const MeterReading *previous = &sensor->previous;

    // checks if the reading is a duplicate of the previous one
    if (previous->sensor_id == reading->sensor_id &&
        previous->generation == reading->generation &&
        previous->occurred_at_ms == reading->occurred_at_ms &&
        previous->sequence == reading->sequence &&
        previous->total_liters == reading->total_liters
    ) {
        return MONITOR_INGEST_DUPLICATE;
    }

    ConsumptionInterval calculated;
    ConsumptionResult result = consumption_from_readings(previous, reading, &calculated);

    if (result != CONSUMPTION_OK)
        return MONITOR_INGEST_INVALID_READING;

    if (mode == PROCESS_LIVE) {
        if (record_reading(monitor->log, reading) != LOG_OK)
            return MONITOR_INGEST_LOG_ERROR;
    }

    *out = calculated;
    sensor->previous = *reading;

    return MONITOR_INGEST_INTERVAL_READY;
}

MonitorIngestResult monitor_ingest(Monitor *monitor, const MeterReading *reading, ConsumptionInterval *out) {
    return process_reading(monitor, reading, out, PROCESS_LIVE);
}

MonitorIngestResult monitor_replay_reading(Monitor *monitor, const MeterReading *reading, ConsumptionInterval *out) {
    return process_reading(monitor, reading, out, PROCESS_REPLAY);
}

MonitorRecoverResult monitor_recover(Log *log, const char *path, Monitor **out) {
    *out = NULL;

    if (log_flush(log) != LOG_OK)
        return MONITOR_RECOVER_IO_ERROR;

    FILE *input = fopen(path, "r");

    if (input == NULL)
        return MONITOR_RECOVER_IO_ERROR;

    Monitor *monitor = monitor_create(log);

    if (monitor == NULL) {
        fclose(input);
        return MONITOR_RECOVER_RESOURCE_LIMIT;
    }

    MonitorRecoverResult result = MONITOR_RECOVER_OK;

    for (;;) {
        MeterReading reading;
        LogReadResult read_result = log_read_next(input, &reading);

        if (read_result == LOG_READ_END)
            break;

        if (read_result != LOG_READ_OK) {
            result = read_result == LOG_READ_IO_ERROR
                ? MONITOR_RECOVER_IO_ERROR
                : MONITOR_RECOVER_INVALID_LOG;

            break;
        }

        MonitorRegisterResult registered = monitor_register_sensor(monitor, reading.sensor_id);

        if (registered != MONITOR_REGISTER_OK && registered != MONITOR_REGISTER_DUPLICATE) {
            result = MONITOR_RECOVER_RESOURCE_LIMIT;
            break;
        }

        ConsumptionInterval interval;
        MonitorIngestResult ingested = process_reading(
            monitor,
            &reading,
            &interval,
            PROCESS_REPLAY
        );

        if (ingested != MONITOR_INGEST_BASELINE &&
            ingested != MONITOR_INGEST_INTERVAL_READY &&
            ingested != MONITOR_INGEST_DUPLICATE) {
            result = MONITOR_RECOVER_INVALID_LOG;
            break;
        }
    }

    if (fclose(input) == EOF && result == MONITOR_RECOVER_OK)
        result = MONITOR_RECOVER_IO_ERROR;

    if (result != MONITOR_RECOVER_OK) {
        monitor_destroy(monitor);
        return result;
    }

    *out = monitor;
    return MONITOR_RECOVER_OK;
}

bool monitor_get_last_reading(const Monitor *monitor, uint32_t sensor_id, MeterReading *out) {
    for (size_t i = 0; i < monitor->length; i++) {
        const SensorState *sensor = &monitor->sensors[i];

        if (sensor->sensor_id == sensor_id) {
            if (!sensor->has_previous)
                return false;

            *out = sensor->previous;
            return true;
        }
    }

    return false;
}
