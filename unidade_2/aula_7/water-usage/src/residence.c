#include "residence.h"
#include "consumption.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint32_t id;

    ConsumptionInterval *history;
    size_t length;
    size_t capacity;
} Residence;

typedef struct {
    uint32_t sensor_id;
    size_t residence_idx;
} SensorAssignment;

struct ResidenceStore {
    Residence *residences;
    size_t residence_count;
    size_t residence_capacity;

    SensorAssignment *assignments;
    size_t assignment_count;
    size_t assignment_capacity;
};

static void *grow_array(
    void *data,
    size_t *capacity,
    size_t element_size
) {
    size_t maximum = SIZE_MAX / element_size;

    if (*capacity >= maximum)
        return NULL;

    size_t next;

    if (*capacity == 0)
        next = maximum < 8 ? maximum : 8;
    else if (*capacity > maximum / 2)
        next = maximum;
    else
        next = *capacity * 2;

    void *grown = realloc(data, next * element_size);

    if (grown == NULL)
        return NULL;

    *capacity = next;
    return grown;
}

/* Returns residence_count when no residence matches. */
static size_t find_residence(const ResidenceStore *store, uint32_t residence_id) {
    for (size_t i = 0; i < store->residence_count; i++) {
        if (store->residences[i].id == residence_id) {
            return i;
        }
    }

    return store->residence_count;
}

/* Returns assignment_count when no sensor matches. */
static size_t find_assignment(const ResidenceStore *store, uint32_t sensor_id) {
    for (size_t i = 0; i < store->assignment_count; i++) {
        if (store->assignments[i].sensor_id == sensor_id) {
            return i;
        }
    }

    return store->assignment_count;
}

ResidenceStore *residence_store_create(void) {
    ResidenceStore *store = malloc(sizeof *store);

    if (store != NULL)
        *store = (ResidenceStore){0};

    return store;
}

void residence_store_destroy(ResidenceStore *store) {
    if (store == NULL)
        return;

    for (size_t i = 0; i < store->residence_count; i++)
        free(store->residences[i].history);

    free(store->residences);
    free(store->assignments);
    free(store);
}

StoreResult residence_store_add(ResidenceStore *store, uint32_t residence_id) {
    if (find_residence(store, residence_id) != store->residence_count)
        return STORE_ALREADY_EXISTS;

    if (store->residence_count == store->residence_capacity) {
        Residence *grown = grow_array(store->residences, &store->residence_capacity, sizeof *store->residences);

        if (grown == NULL)
            return STORE_RESOURCE_LIMIT;

        store->residences = grown;
    }

    store->residences[store->residence_count++] = (Residence){
        .id = residence_id
    };

    return STORE_OK;
}

StoreResult residence_store_assign_sensor(ResidenceStore *store, uint32_t residence_id, uint32_t sensor_id) {
    size_t residence_idx = find_residence(store, residence_id);

    if (residence_idx == store->residence_count)
        return STORE_NOT_FOUND;

    if (find_assignment(store, sensor_id) != store->assignment_count)
        return STORE_SENSOR_ALREADY_ASSIGNED;

    if (store->assignment_count == store->assignment_capacity) {
        SensorAssignment *grown = grow_array(store->assignments, &store->assignment_capacity, sizeof *store->assignments);

        if (grown == NULL)
            return STORE_RESOURCE_LIMIT;

        store->assignments = grown;
    }

    store->assignments[store->assignment_count++] = (SensorAssignment){
        .sensor_id = sensor_id,
        .residence_idx = residence_idx
    };

    return STORE_OK;
}

StoreResult residence_store_append(ResidenceStore *store, const ConsumptionInterval *interval) {
    // copy before realloc so borrowed interval does not become
    // a dangling input during growth.
    ConsumptionInterval copy = *interval;

    size_t assignment_idx = find_assignment(store, copy.sensor_id);

    if (assignment_idx == store->assignment_count)
        return STORE_SENSOR_UNASSIGNED;

    size_t residence_idx = store->assignments[assignment_idx].residence_idx;

    Residence *residence = &store->residences[residence_idx];

    if (residence->length == residence->capacity) {
        ConsumptionInterval *grown = grow_array(
            residence->history,
            &residence->capacity,
            sizeof *residence->history
        );

        if (grown == NULL)
            return STORE_RESOURCE_LIMIT;

        residence->history = grown;
    }

    residence->history[residence->length++] = copy;

    return STORE_OK;
}

StoreResult residence_store_get_history(const ResidenceStore *store, uint32_t id, ConsumptionView *out) {
    *out = (ConsumptionView){0};

    size_t idx = find_residence(store, id);

    if (idx == store->residence_count)
        return STORE_NOT_FOUND;

    const Residence *residence = &store->residences[idx];

    *out = (ConsumptionView){
        .data = residence->history,
        .length = residence->length
    };

    return STORE_OK;
}

bool residence_store_has_sensor(const ResidenceStore *store, uint32_t sensor_id) {
    return find_assignment(store, sensor_id) != store->assignment_count;
}
