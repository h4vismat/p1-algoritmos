#ifndef MONITOR_H
#define MONITOR_H

#include "consumption.h"
#include "log.h"
#include "sensor.h"

#include <stdbool.h>

typedef enum {
    MONITOR_REGISTER_OK,
    MONITOR_REGISTER_DUPLICATE,
    MONITOR_REGISTER_NO_MEMORY,
    MONITOR_REGISTER_CAPACITY_LIMIT
} MonitorRegisterResult;

typedef enum {
    MONITOR_INGEST_INTERVAL_READY,
    MONITOR_INGEST_BASELINE,
    MONITOR_INGEST_DUPLICATE,
    MONITOR_INGEST_UNKNOWN_SENSOR,
    MONITOR_INGEST_INVALID_READING,
    MONITOR_INGEST_LOG_ERROR
} MonitorIngestResult;

typedef enum {
    MONITOR_RECOVER_OK,
    MONITOR_RECOVER_RESOURCE_LIMIT,
    MONITOR_RECOVER_IO_ERROR,
    MONITOR_RECOVER_INVALID_LOG
} MonitorRecoverResult;

typedef struct Monitor Monitor;

/* Returns an empty monitor, or NULL if allocation fails. */
Monitor *monitor_create(Log *log);

/* Invalidates the monitor and all borrowed pointers. */
void monitor_destroy(Monitor *monitor);

MonitorRegisterResult monitor_register_sensor(Monitor *monitor, uint32_t sensor_id);

/*
 * Ingests a given reading.
 * Writes *out only when returning MONITOR_INGEST_INTERVAL_READY.
 * Rejected and duplicate readings leave monitor state unchanged.
 */
MonitorIngestResult monitor_ingest(
    Monitor *monitor,
    const MeterReading *reading,
    ConsumptionInterval *out
);

/*
 * Processes a reading without writing it to the log.
 * Intended for a startup replay into a fresh monitor.
 */
MonitorIngestResult monitor_replay_reading(
    Monitor *monitor,
    const MeterReading *reading,
    ConsumptionInterval *out
);

/*
 * Copies the sensor's last accepted reading into *out.
 * Returns false if the sensor is unknown or has no baseline.
 * Leaves *out unchanged on failure.
 */
bool monitor_get_last_reading(const Monitor *monitor, uint32_t sensor_id, MeterReading *out);

#endif
