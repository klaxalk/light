
#pragma once 

#include "light.h"

// Implementation of the sysfs enumerator
// Enumerates devices for backlights and leds

// Device target data 
struct _impl_sysfs_data_t
{
    char brightness[NAME_MAX];
    char max_brightness[NAME_MAX];
};

typedef struct _impl_sysfs_data_t impl_sysfs_data_t;

bool impl_sysfs_init(light_device_enumerator_t *enumerator);
bool impl_sysfs_free(light_device_enumerator_t *enumerator);

bool impl_sysfs_set(light_device_target_t *target, uint64_t in_value);
bool impl_sysfs_get(light_device_target_t *target, uint64_t *out_value);
bool impl_sysfs_getmax(light_device_target_t *target, uint64_t *out_value);
bool impl_sysfs_command(light_device_target_t *target, char const *command_string);
