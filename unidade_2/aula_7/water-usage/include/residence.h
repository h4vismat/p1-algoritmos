#ifndef RESIDENCE_H
#define RESIDENCE_H

#include "consumption.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    STORE_OK,
    STORE_NOT_FOUND,
    STORE_ALREADY_EXISTS,
    STORE_SENSOR_ALREADY_ASSIGNED,
    STORE_SENSOR_UNASSIGNED,
    STORE_RESOURCE_LIMIT
} StoreResult;

typedef struct ResidenceStore ResidenceStore;

ResidenceStore *resident_store_create(void);
void residence_store_destroy(ResidenceStore *store);

StoreResult residence_store_add(
    ResidenceStore *store,
    uint32_t residence_id
);

StoreResult residence_store_assign_sensor(
    ResidenceStore *store,
    uint32_t residence_id,
    uint32_t sensor_id
);

/*
 * Copies an interval into its residence's history.
 * On failure, contents remain unchanged.
 */
StoreResult residence_store_append(
    ResidenceStore *store,
    const ConsumptionInterval *interval
);

/*
 * Clears *out when the residence does not exist.
 */
StoreResult residence_store_get_history(
    const ResidenceStore *store,
    uint32_t id,
    ConsumptionView *out
);

bool residence_store_has_sensor(
    const ResidenceStore *store,
    uint32_t sensor_id
);

#endif
