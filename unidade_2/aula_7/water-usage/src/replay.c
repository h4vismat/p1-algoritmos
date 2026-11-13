#include "replay.h"

#include "consumption.h"
#include "log.h"
#include "monitor.h"
#include "residence.h"
#include "sensor.h"

ReplayResult application_replay(FILE *input, Monitor *monitor, ResidenceStore *residences) {
    for (;;) {
        MeterReading reading;
        LogReadResult read_result = log_read_next(input, &reading);

        switch (read_result) {
            case LOG_READ_END:
                return REPLAY_OK;

            case LOG_READ_IO_ERROR:
                return REPLAY_IO_ERROR;

            case LOG_READ_INVALID_FORMAT:
                return REPLAY_INVALID_FORMAT;

            case LOG_READ_INCOMPLETE:
                return REPLAY_INCOMPLETE_LOG;

            case LOG_READ_OK:
                break;

            default:
                return REPLAY_IO_ERROR;
        }

        if (!residence_store_has_sensor(residences, reading.sensor_id))
            return REPLAY_UNASSIGNED_SENSOR;

        ConsumptionInterval interval;
        MonitorIngestResult result = monitor_replay_reading(monitor, &reading, &interval);

        switch (result) {
            case MONITOR_INGEST_BASELINE:
                break;

            case MONITOR_INGEST_DUPLICATE:
                break;

            case MONITOR_INGEST_INTERVAL_READY:
                if (residence_store_append(residences, &interval) != STORE_OK)
                    return REPLAY_STORE_ERROR;
                break;

            case MONITOR_INGEST_UNKNOWN_SENSOR:
                return REPLAY_UNKNOWN_SENSOR;

            case MONITOR_INGEST_INVALID_READING:
                return REPLAY_INVALID_READING;

            case MONITOR_INGEST_LOG_ERROR:
                return REPLAY_IO_ERROR;

            default:
                return REPLAY_INVALID_READING;
        }
    }
}
