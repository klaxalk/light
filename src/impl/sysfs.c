
#include "impl/sysfs.h"
#include "light.h"
#include "helpers.h"

#include <stdio.h> //snprintf
#include <stdlib.h> // malloc, free
#include <dirent.h> // opendir, readdir

static bool _impl_sysfs_init_leds(light_device_enumerator_t *enumerator)
{
    // Create a new backlight device
    light_device_t *leds_device = light_create_device(enumerator, "leds", NULL);

    // Iterate through the led controllers and create a device_target for each controller 
    DIR *leds_dir;
    struct dirent *curr_entry;
    
    if((leds_dir = opendir("/sys/class/leds")) == NULL)
    {
        LIGHT_ERR("failed to open leds controller directory for reading");
        return false;
    }
    
    while((curr_entry = readdir(leds_dir)) != NULL)
    {
        // Skip dot entries
        if(curr_entry->d_name[0] == '.')
        {
            continue;
        }
        
        // Setup the target data 
        impl_sysfs_data_t *dev_data = malloc(sizeof(impl_sysfs_data_t));
        snprintf(dev_data->brightness, sizeof(dev_data->brightness), "/sys/class/leds/%s/brightness", curr_entry->d_name);
        snprintf(dev_data->max_brightness, sizeof(dev_data->max_brightness), "/sys/class/leds/%s/max_brightness", curr_entry->d_name);
        
        // Create a new device target for the controller 
        light_create_device_target(leds_device, curr_entry->d_name, impl_sysfs_set, impl_sysfs_get, impl_sysfs_getmax, impl_sysfs_command, dev_data);
    }
    
    closedir(leds_dir);
    
    return true;
}

static bool _impl_sysfs_init_backlight(light_device_enumerator_t *enumerator)
{
    // Create a new backlight device
    light_device_t *backlight_device = light_create_device(enumerator, "backlight", NULL);

    // Iterate through the backlight controllers and create a device_target for each controller 
    DIR *backlight_dir;
    struct dirent *curr_entry;
    
    // Keep track of the best controller, and create an autodevice from that
    char best_controller[NAME_MAX];
    uint64_t best_value = 0;
    
    if((backlight_dir = opendir("/sys/class/backlight")) == NULL)
    {
        LIGHT_ERR("failed to open backlight controller directory for reading");
        return false;
    }
    
    while((curr_entry = readdir(backlight_dir)) != NULL)
    {
        // Skip dot entries
        if(curr_entry->d_name[0] == '.')
        {
            continue;
        }
        
        // Setup the target data 
        impl_sysfs_data_t *dev_data = malloc(sizeof(impl_sysfs_data_t));
        snprintf(dev_data->brightness, sizeof(dev_data->brightness), "/sys/class/backlight/%s/brightness", curr_entry->d_name);
        snprintf(dev_data->max_brightness, sizeof(dev_data->max_brightness), "/sys/class/backlight/%s/max_brightness", curr_entry->d_name);
        
        // Create a new device target for the controller 
        light_create_device_target(backlight_device, curr_entry->d_name, impl_sysfs_set, impl_sysfs_get, impl_sysfs_getmax, impl_sysfs_command, dev_data);
        
        // Read the max brightness to get the best one
        uint64_t curr_value = 0;
        if(light_file_read_uint64(dev_data->max_brightness, &curr_value))
        {
            if(curr_value > best_value)
            {
                best_value = curr_value;
                snprintf(best_controller, sizeof(best_controller), "%s", curr_entry->d_name);
            }
        }
    }
    
    closedir(backlight_dir);
    
    // If we found at least one usable controller, create an auto target mapped to that controller
    if(best_value > 0)
    {
        // Setup the target data 
        impl_sysfs_data_t *dev_data = malloc(sizeof(impl_sysfs_data_t));
        snprintf(dev_data->brightness, sizeof(dev_data->brightness), "/sys/class/backlight/%s/brightness", best_controller);
        snprintf(dev_data->max_brightness, sizeof(dev_data->max_brightness), "/sys/class/backlight/%s/max_brightness", best_controller);
        
        // Create a new device target for the controller 
        light_create_device_target(backlight_device, "auto", impl_sysfs_set, impl_sysfs_get, impl_sysfs_getmax, impl_sysfs_command, dev_data);
    }
    
    return true;
}

bool impl_sysfs_init(light_device_enumerator_t *enumerator)
{
    // Create a device for the backlight 
    _impl_sysfs_init_backlight(enumerator);
    
    // Create devices for the leds
    _impl_sysfs_init_leds(enumerator);
    
    return true;
}

bool impl_sysfs_free(light_device_enumerator_t *enumerator)
{
    return true;
}

bool impl_sysfs_set(light_device_target_t *target, uint64_t in_value)
{
    impl_sysfs_data_t *data = (impl_sysfs_data_t*)target->device_target_data;

    if(!light_file_write_uint64(data->brightness, in_value))
    {
        LIGHT_ERR("failed to write to sysfs device");
        return false;
    }
    
    return true;
}

bool impl_sysfs_get(light_device_target_t *target, uint64_t *out_value)
{
    impl_sysfs_data_t *data = (impl_sysfs_data_t*)target->device_target_data;

    if(!light_file_read_uint64(data->brightness, out_value))
    {
        LIGHT_ERR("failed to read from sysfs device");
        return false;
    }
    
    return true;
}

bool impl_sysfs_getmax(light_device_target_t *target, uint64_t *out_value)
{
    impl_sysfs_data_t *data = (impl_sysfs_data_t*)target->device_target_data;

    if(!light_file_read_uint64(data->max_brightness, out_value))
    {
        LIGHT_ERR("failed to read from sysfs device");
        return false;
    }
    
    return true;
}

bool impl_sysfs_command(light_device_target_t *target, char const *command_string)
{
    // No current need for custom commands in sysfs enumerator
    // To implement support, simply parse the command string to your liking, and return false on invalid input or results!
    return true;
}


