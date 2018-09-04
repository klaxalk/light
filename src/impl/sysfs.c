
#include "impl/sysfs.h"
#include "light.h"
#include "helpers.h"

#include <stdio.h> //snprintf
#include <stdlib.h> // malloc, free
#include <dirent.h> // opendir, readdir

static bool _impl_sysfs_init_leds(light_device_enumerator_t *enumerator)
{
    // Create a new backlight device
    light_device_t *leds_device = malloc(sizeof(light_device_t));
    snprintf(leds_device->name, sizeof(leds_device->name), "%s", "leds");
    
    // Add it to the enumerator 
    light_add_enumerator_device(enumerator, leds_device);
    
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
        
        // Create a new device target for the controller 
        light_device_target_t *led_controller = malloc(sizeof(light_device_target_t));
        snprintf(led_controller->name, sizeof(led_controller->name), "%s", curr_entry->d_name);
        
        // Setup the function bindings
        led_controller->set_value = impl_sysfs_set;
        led_controller->get_value = impl_sysfs_get;
        led_controller->get_max_value = impl_sysfs_getmax;
        led_controller->custom_command = impl_sysfs_command;
        
        // Setup the target data 
        impl_sysfs_data_t *dev_data = malloc(sizeof(impl_sysfs_data_t));
        led_controller->device_target_data = dev_data;
        snprintf(dev_data->brightness, sizeof(dev_data->brightness), "/sys/class/leds/%s/brightness", curr_entry->d_name);
        snprintf(dev_data->max_brightness, sizeof(dev_data->max_brightness), "/sys/class/leds/%s/max_brightness", curr_entry->d_name);
        
        // Add it to the device
        light_add_device_target(leds_device, led_controller);
    }
    
    closedir(leds_dir);
    
    return true;
}

static bool _impl_sysfs_init_backlight(light_device_enumerator_t *enumerator)
{
    // Create a new backlight device
    light_device_t *backlight_device = malloc(sizeof(light_device_t));
    snprintf(backlight_device->name, sizeof(backlight_device->name), "%s", "backlight");
    
    // Add it to the enumerator 
    light_add_enumerator_device(enumerator, backlight_device);
    
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
        
        // Create a new device target for the controller 
        light_device_target_t *backlight_controller = malloc(sizeof(light_device_target_t));
        snprintf(backlight_controller->name, sizeof(backlight_controller->name), "%s", curr_entry->d_name);
        
        // Setup the function bindings
        backlight_controller->set_value = impl_sysfs_set;
        backlight_controller->get_value = impl_sysfs_get;
        backlight_controller->get_max_value = impl_sysfs_getmax;
        backlight_controller->custom_command = impl_sysfs_command;
        
        // Setup the target data 
        impl_sysfs_data_t *dev_data = malloc(sizeof(impl_sysfs_data_t));
        backlight_controller->device_target_data = dev_data;
        snprintf(dev_data->brightness, sizeof(dev_data->brightness), "/sys/class/backlight/%s/brightness", curr_entry->d_name);
        snprintf(dev_data->max_brightness, sizeof(dev_data->max_brightness), "/sys/class/backlight/%s/max_brightness", curr_entry->d_name);
        
        // Read the max brightness to get the best one
        uint64_t curr_value = 0;
        if(light_file_read_uint64(dev_data->max_brightness, &curr_value))
        {
            if(curr_value > best_value)
            {
                best_value = curr_value;
                snprintf(best_controller, sizeof(best_controller), "%s", backlight_controller->name);
            }
        }
        
        // Add it to the device
        light_add_device_target(backlight_device, backlight_controller);
    }
    
    closedir(backlight_dir);
    
    // Create an auto controller 
    light_device_target_t *auto_controller = malloc(sizeof(light_device_target_t));
    snprintf(auto_controller->name, sizeof(auto_controller->name), "%s", "auto");
    
    // Setup the function bindings
    auto_controller->set_value = impl_sysfs_set;
    auto_controller->get_value = impl_sysfs_get;
    auto_controller->get_max_value = impl_sysfs_getmax;
    auto_controller->custom_command = impl_sysfs_command;
    
    // Setup the target data 
    impl_sysfs_data_t *dev_data = malloc(sizeof(impl_sysfs_data_t));
    auto_controller->device_target_data = dev_data;
    snprintf(dev_data->brightness, sizeof(dev_data->brightness), "/sys/class/backlight/%s/brightness", best_controller);
    snprintf(dev_data->max_brightness, sizeof(dev_data->max_brightness), "/sys/class/backlight/%s/max_brightness", best_controller);
    
    // Add it to the device
    light_add_device_target(backlight_device, auto_controller);
    
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
    // Iterate through the devices in the enumerator
    for(uint64_t d = 0; d < enumerator->num_devices; d++)
    {
        light_device_t *curr_device = enumerator->devices[d];
        
        // If the given device points to NULL, we can safely skip it
        if(curr_device == NULL)
        {
            continue;
        }
        
        for(uint64_t t = 0; t < curr_device->num_targets; t++)
        {
            light_device_target_t *curr_target = curr_device->targets[t];
            
            if(curr_target == NULL)
            {
                continue;
            }
            
            if(curr_target->device_target_data != NULL)
            {
                free(curr_target->device_target_data);
            }
            
            free(curr_target);
        }
        
        // If the given device has any device_data, free it
        if(curr_device->device_data != NULL)
        {
            free(curr_device->device_data);
        }
                
        light_dispose_device(curr_device);    

        // Free the device
        free(curr_device);
    }
    
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


