
#include "impl/razer.h"
#include "light.h"
#include "helpers.h"

#include <stdio.h> //snprintf
#include <stdlib.h> // malloc, free
#include <dirent.h> // opendir, readdir

static void _impl_razer_add_target(light_device_t *device, char const *name, char const *filename, uint64_t max_brightness)
{
    impl_razer_data_t *target_data = malloc(sizeof(impl_razer_data_t));
    snprintf(target_data->brightness, sizeof(target_data->brightness), "/sys/bus/hid/drivers/razerkbd/%s/%s", device->name, filename);
    target_data->max_brightness = max_brightness;
    
    // Only add targets that actually exist, as we aren't fully sure exactly what targets exist for a given device
    if(light_file_exists(target_data->brightness))
    {
        light_create_device_target(device, name, impl_razer_set, impl_razer_get, impl_razer_getmax, impl_razer_command, target_data);
    }
    else 
    {
        LIGHT_WARN("razer: couldn't add target %s to device %s, the file %s doesn't exist", name, device->name, filename);
        // target_data will not be freed automatically if we dont add a device target with it as userdata, so free it here
        free(target_data);
    }
}

static void _impl_razer_add_device(light_device_enumerator_t *enumerator, char const *device_id)
{
    // Create a new razer device
    light_device_t *new_device = light_create_device(enumerator, device_id, NULL);

    // Setup a target to backlight
    _impl_razer_add_target(new_device, "backlight", "matrix_brightness", 255);
    
    // Setup targets to different possible leds
    _impl_razer_add_target(new_device, "game_led", "game_led_state", 1);
    _impl_razer_add_target(new_device, "macro_led", "macro_led_state", 1);
    _impl_razer_add_target(new_device, "logo_led", "logo_led_state", 1);
    _impl_razer_add_target(new_device, "profile_led_r", "profile_led_red", 1);
    _impl_razer_add_target(new_device, "profile_led_g", "profile_led_green", 1);
    _impl_razer_add_target(new_device, "profile_led_b", "profile_led_blue", 1);
}

bool impl_razer_init(light_device_enumerator_t *enumerator)
{
    // Iterate through the led controllers and create a device_target for each controller 
    DIR *razer_dir;
    struct dirent *curr_entry;
    
    if((razer_dir = opendir("/sys/bus/hid/drivers/razerkbd/")) == NULL)
    {
        // Razer driver isnt properly installed, so we cant add devices in this enumerator 
        return true;
    }
    
    while((curr_entry = readdir(razer_dir)) != NULL)
    {
        // Skip dot entries
        if(curr_entry->d_name[0] == '.')
        {
            continue;
        }
 
        _impl_razer_add_device(enumerator, curr_entry->d_name);
    }
    
    closedir(razer_dir);
    
    return true;
}

bool impl_razer_free(light_device_enumerator_t *enumerator)
{
    return true;
}

bool impl_razer_set(light_device_target_t *target, uint64_t in_value)
{
    impl_razer_data_t *data = (impl_razer_data_t*)target->device_target_data;

    if(!light_file_write_uint64(data->brightness, in_value))
    {
        LIGHT_ERR("failed to write to razer device");
        return false;
    }
    
    return true;
}

bool impl_razer_get(light_device_target_t *target, uint64_t *out_value)
{
    impl_razer_data_t *data = (impl_razer_data_t*)target->device_target_data;

    if(!light_file_read_uint64(data->brightness, out_value))
    {
        LIGHT_ERR("failed to read from razer device");
        return false;
    }
    
    return true;
}

bool impl_razer_getmax(light_device_target_t *target, uint64_t *out_value)
{
    impl_razer_data_t *data = (impl_razer_data_t*)target->device_target_data;

    *out_value = data->max_brightness;
    
    return true;
}

bool impl_razer_command(light_device_target_t *target, char const *command_string)
{
    // No current need for custom commands in sysfs enumerator
    // To implement support, simply parse the command string to your liking, and return false on invalid input or results!
    return true;
}


