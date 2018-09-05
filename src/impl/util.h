
#pragma once 

#include "light.h"

// Implementation of the util enumerator
// Enumerates devices for utilities and testing

bool impl_util_init(light_device_enumerator_t *enumerator);
bool impl_util_free(light_device_enumerator_t *enumerator);

bool impl_util_dryrun_set(light_device_target_t *target, uint64_t in_value);
bool impl_util_dryrun_get(light_device_target_t *target, uint64_t *out_value);
bool impl_util_dryrun_getmax(light_device_target_t *target, uint64_t *out_value);
bool impl_util_dryrun_command(light_device_target_t *target, char const *command_string);
