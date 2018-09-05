# Developer Instructions

This file is aimed at developers of light, or developers who want to implement "drivers" (enumerators) for their own hardware.

## Coding Standards

Light is a small project, which helps keep it clean. However we still would like to see a consistent styling throughout the project, as well as in third-party enumerator implementations. The actual source code may not be fully "up to code" yet, but it's getting there.

We use 4 spaces for indentation. We have an empty line at the top and bottom of each file.

The following two sources should be clear enough examples of our coding style:

### Header files 

```c
    
    #pragma once
    
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdfoo.h> /* foo_type_t */
    
    
    typedef struct _some_struct_t some_struct_t;
    struct _some_struct_t 
    {
        uint64_t    id;
        foo_type_t  my_foo_thing;
        foo_type_t  *my_foo_ptr;
    }
    
    /* Describe what the purpose of this function is, what it does with/to foo_struct, and what it returns. */
    bool do_some_stuff(some_struct_t *foo_struct);
    
```

### Source files

The second line of each source file should be the include to the corresponding header file, followed by an empty line.

Internal/static functions are always prefixed with an underscore (_).

```c

#include "some_struct.h"

static void _increment_one(uint64_t *out_value)
{
    *out_value += 1;
}

bool do_some_stuff(some_struct_t *foo_struct)
{
    _increment_one(foo_struct->id);
    
    if(foo_struct->id > 33)
    {
        return false;
    }
    
    if(foo_struct->my_foo_ptr != NULL)
    {
        free(foo_struct->my_foo_ptr);
    }
    
    foo_struct->my_foo_ptr = malloc(sizeof(foo_type_t));
    
    return true;
}

```

## Implementing an enumerator 

Implementing your own devices through an enumerator is pretty easy. The required steps are as follows:

### Step 1

Create a headerfile and a corresponding sourcefile under the `impl` folder, Call them `foo.h` and `foo.c`. Open up the `sysfs.c` and `sysfs.h` files for reference implementations.

### Step 2

In the header, you need to first do a `#pragma once` (obviously), then `#include "light.h"` to get access to some struct declarations, then at the bare minimum declare 6 functions. If you need to store your own data for each device or device-target, you will need to declare structs for these as well.

```c

#pragma once 

#include "light.h"

// Implementation of the foo enumerator
// Enumerates devices for quacking ducks

// Device target data 
struct _impl_foo_data_t
{
    int32_t internal_quack_id;
};

typedef struct _impl_foo_data_t impl_foo_data_t;

bool impl_foo_init(light_device_enumerator_t *enumerator);
bool impl_foo_free(light_device_enumerator_t *enumerator);

bool impl_foo_set(light_device_target_t *target, uint64_t in_value);
bool impl_foo_get(light_device_target_t *target, uint64_t *out_value);
bool impl_foo_getmax(light_device_target_t *target, uint64_t *out_value);
bool impl_foo_command(light_device_target_t *target, char const *command_string);

```

### Step 3

In the sourcefile, you need to implement the 6 methods. Make sure to return `true` on success and `false` on failure. If you do not actually implement a function (for example `impl_foo_command`), just return `true`.

The job of the enumerator is to identify/enumerate a bunch of different devices (or just one, or even zero if it doesnt find any). You are also responsible to create the device targets for them (i.e, the things that you actually write to on the device). You do this by setting the devices and targets up in `impl_foo_init` and to free these in `impl_foo_free`. The enumerator (this code) owns the memory of the devices and targets, and thus is in charge of allocating and freeing it. 

```c

#include "impl/foo.h"

#include "light.h"
#include "helpers.h"

bool impl_foo_init(light_device_enumerator_t *enumerator)
{
    /* Lets create a single device, with a single target, for simplicity */
    
    /* Allocate a new device */
    light_device_t *new_device = malloc(sizeof(light_device_t));
    
    /* Set the name of the new device to "new_device_name" */
    snprintf(leds_device->name, sizeof(leds_device->name), "%s", "new_device_name");
    
    /* Add the newly created device to the enumerator */
    light_add_enumerator_device(enumerator, new_device);

    
    
    /* Allocate a new device target for the newly created device */
    light_device_target_t *new_target = malloc(sizeof(light_device_target_t));
    
    /* Set the name of the new target */
    snprintf(new_target->name, sizeof(new_target->name), "%s", "new_target_name");
    
    /* Setup the function bindings for this target, i.e. what functions will run internally when you run different commands on this device target */
    new_target->set_value = impl_foo_set;
    new_target->get_value = impl_foo_get;
    new_target->get_max_value = impl_foo_getmax;
    new_target->custom_command = impl_foo_command;
    
    /* Finally add the target to the device */
    light_add_device_target(new_device, new_target);
    
    
    
    /* Optional: Setup data specific to either a device or a target, or both. */ 
    /* Useful to for example reference an ID in a third-party API or likewise */
    impl_foo_data_t *custom_data = malloc(sizeof(impl_foo_data_t));
    custom_data->internal_quack_id = 333;
    
    /* You can set it to the device itself, or to a target */
    new_device->device_data = custom_data;
    new_target->device_target_data = custom_data;

    /* Return true because we didnt get any errors! */
    return true;
}

bool impl_foo_free(light_device_enumerator_t *enumerator)
{
    /* We are responsible to free the memory we allocated in init, so lets do that */
    
    /* Iterate through the devices in the enumerator */
    for(uint64_t d = 0; d < enumerator->num_devices; d++)
    {
        light_device_t *curr_device = enumerator->devices[d];

        /* Iterate throug hthe targets in the device */
        for(uint64_t t = 0; t < curr_device->num_targets; t++)
        {
            light_device_target_t *curr_target = curr_device->targets[t];
            
            /* If we allocated any target data, free it here */
            if(curr_target->device_target_data != NULL)
            {
                free(curr_target->device_target_data);
            }
            
            free(curr_target);
        }

        /* If we allocated any device data, free it here */
        if(curr_device->device_data != NULL)
        {
            free(curr_device->device_data);
        }
        
        /* We need to 'dispose' of the device when we are done with it, to free some internal memory. Always do this when you are about to free a device that you allocated. */
        light_dispose_device(curr_device);
        
        /* Free the device */
        free(curr_device);
    }
    
    return true;
}

/* Implement the other functions to do their thing. Get, Set and GetMax should be self-explanatory. Command is reserved for future use, but basically will allow the user to run custom commands on a target. */

```

### Step 4

Now that you have implemented your enumerator, it is time to inject it to the application itself. You will be able to compile your enumerator into a plugin in the future, but for now, locate the `light_initialize` function inside `light.c`. You will see some calls (perhaps just one call) to `light_create_enumerator` inside of this function. Add one more call to this function to register your enumerator in the application:

The first argument is the application context, the second is the name that your enumerator will get, and the last two are the init and free functions that we implemented.

```c
light_create_enumerator(new_ctx, "foo", &impl_foo_init, &impl_foo_free);
```

Once you do this, you should be able to find your device target when running `light -L`, and it should be called something like `foo/new_device_name/new_target_name` if you followed this guide.

The only thing left now is to create a pull request so that the rest of the world can share the functionality that you just implemented!


## Troubleshooting

If you run into any issues, feel free to create a new Github issue, even if you are just asking for "support" or likewise.
