
#include "impl/util.h"
#include "light.h"
#include "helpers.h"

#include <stdio.h> //snprintf
#include <stdlib.h> // malloc, free
#include <dirent.h> // opendir, readdir
#include <inttypes.h> // PRIu64

bool impl_util_init(light_device_enumerator_t *enumerator)
{
    light_device_t *util_device = light_create_device(enumerator, "test", NULL);
    light_create_device_target(util_device, "dryrun", impl_util_dryrun_set, impl_util_dryrun_get, impl_util_dryrun_getmax, impl_util_dryrun_command, NULL);
    return true;
}

bool impl_util_free(light_device_enumerator_t *enumerator)
{
    return true;
}

bool impl_util_dryrun_set(light_device_target_t *target, uint64_t in_value)
{
    LIGHT_NOTE("impl_util_dryrun_set: writing brightness %" PRIu64 " to utility target %s", in_value, target->name);
    return true;
}

bool impl_util_dryrun_get(light_device_target_t *target, uint64_t *out_value)
{
    LIGHT_NOTE("impl_util_dryrun_get: reading brightness (0) from utility target %s", target->name);
    *out_value = 0;
    return true;
}

bool impl_util_dryrun_getmax(light_device_target_t *target, uint64_t *out_value)
{
    LIGHT_NOTE("impl_util_dryrun_getmax: reading max. brightness (255) from utility target %s", target->name);
    *out_value = 255;
    return true;
}

bool impl_util_dryrun_command(light_device_target_t *target, char const *command_string)
{
    LIGHT_NOTE("impl_util_dryrun_command: running custom command on utility target %s: \"%s\"", target->name, command_string);
    return true;
}


