
#pragma once 

#include "light.h"

// Implementation of the razer enumerator
// Enumerates devices for the openrazer driver https://github.com/openrazer/openrazer

// Device target data 
struct _impl_razer_data_t
{
    char brightness[NAME_MAX];
    uint64_t max_brightness;
};

typedef struct _impl_razer_data_t impl_razer_data_t;

bool impl_razer_init(light_device_enumerator_t *enumerator);
bool impl_razer_free(light_device_enumerator_t *enumerator);

bool impl_razer_set(light_device_target_t *target, uint64_t in_value);
bool impl_razer_get(light_device_target_t *target, uint64_t *out_value);
bool impl_razer_getmax(light_device_target_t *target, uint64_t *out_value);
bool impl_razer_command(light_device_target_t *target, char const *command_string);
