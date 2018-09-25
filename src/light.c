
#include "light.h"
#include "helpers.h"

// The different device implementations
#include "impl/sysfs.h"
#include "impl/util.h"
#include "impl/razer.h"

#include <stdlib.h> // malloc, free
#include <string.h> // strstr
#include <stdio.h>  // snprintf
#include <unistd.h>	// geteuid
#include <sys/types.h> // geteuid
#include <errno.h>
#include <inttypes.h> // PRIu64

/* Static helper functions for this file only, prefix with _ */


static void _light_add_enumerator_device(light_device_enumerator_t *enumerator, light_device_t *new_device)
{
    // Create a new device array 
    uint64_t new_num_devices = enumerator->num_devices + 1;
    light_device_t **new_devices = malloc(new_num_devices * sizeof(light_device_t*));
    
    // Copy old device array to new one
    for(uint64_t i = 0; i < enumerator->num_devices; i++)
    {
        new_devices[i] = enumerator->devices[i];
    }
    
    // Set the new device
    new_devices[enumerator->num_devices] = new_device;
    
    // Free the old devices array, if needed
    if(enumerator->devices != NULL)
    {
        free(enumerator->devices);
    }
    
    // Replace the devices array with the new one
    enumerator->devices = new_devices;
    enumerator->num_devices = new_num_devices;
}

static void _light_add_device_target(light_device_t *device, light_device_target_t *new_target)
{
    // Create a new targets array 
    uint64_t new_num_targets = device->num_targets + 1;
    light_device_target_t **new_targets = malloc(new_num_targets * sizeof(light_device_target_t*));
    
    // Copy old targets array to new one
    for(uint64_t i = 0; i < device->num_targets; i++)
    {
        new_targets[i] = device->targets[i];
    }
    
    // Set the new target
    new_targets[device->num_targets] = new_target;
    
    // Free the old targets array, if needed
    if(device->targets != NULL)
    {
        free(device->targets);
    }
    
    // Replace the targets array with the new one
    device->targets= new_targets;
    device->num_targets = new_num_targets;
}

static void _light_get_target_path(light_context_t* ctx, char* output_path, size_t output_size)
{
    snprintf(output_path, output_size,
                "%s/targets/%s/%s/%s",
                ctx->sys_params.conf_dir,
                ctx->run_params.device_target->device->enumerator->name,
                ctx->run_params.device_target->device->name,
                ctx->run_params.device_target->name
            );
}

static void _light_get_target_file(light_context_t* ctx, char* output_path, size_t output_size, char const * file)
{
    snprintf(output_path, output_size,
                "%s/targets/%s/%s/%s/%s",
                ctx->sys_params.conf_dir,
                ctx->run_params.device_target->device->enumerator->name,
                ctx->run_params.device_target->device->name,
                ctx->run_params.device_target->name,
                file
            );
}

static uint64_t _light_get_min_cap(light_context_t *ctx)
{
    char target_path[NAME_MAX];
    _light_get_target_file(ctx, target_path, sizeof(target_path), "minimum");

    uint64_t minimum_value = 0;
    if(!light_file_read_uint64(target_path, &minimum_value))
    {
        return 0;
    }
    
    return minimum_value;
}

static light_device_enumerator_t* _light_find_enumerator(light_context_t *ctx, char const *comp)
{
    for(uint64_t e = 0; e < ctx->num_enumerators; e++)
    {
        if(strncmp(comp, ctx->enumerators[e]->name, NAME_MAX) == 0)
        {
            return ctx->enumerators[e];
        }
    }
    
    return NULL;
}

static light_device_t* _light_find_device(light_device_enumerator_t *en, char const *comp)
{
    for(uint64_t d = 0; d < en->num_devices; d++)
    {
        if(strncmp(comp, en->devices[d]->name, NAME_MAX) == 0)
        {
            return en->devices[d];
        }
    }
    
    return NULL;
}

static light_device_target_t* _light_find_target(light_device_t * dev, char const *comp)
{
    for(uint64_t t = 0; t < dev->num_targets; t++)
    {
        if(strncmp(comp, dev->targets[t]->name, NAME_MAX) == 0)
        {
            return dev->targets[t];
        }
    }
    
    return NULL;
}

static bool _light_raw_to_percent(light_device_target_t *target, uint64_t inraw, double *outpercent)
{
        double inraw_d = (double)inraw;
        uint64_t max_value = 0;
        if(!target->get_max_value(target, &max_value))
        {
            LIGHT_ERR("couldn't read from target");
            return false;
        }
        double max_value_d = (double)max_value;
        double percent = light_percent_clamp((inraw_d / max_value_d) * 100.0);
        *outpercent = percent;
        
        return true;
}

static bool _light_percent_to_raw(light_device_target_t *target, double inpercent, uint64_t *outraw)
{
    uint64_t max_value = 0;
    if(!target->get_max_value(target, &max_value))
    {
        LIGHT_ERR("couldn't read from target");
        return false;
    }

    double max_value_d = (double)max_value;
    double target_value_d = max_value_d * (light_percent_clamp(inpercent) / 100.0);
    uint64_t target_value = LIGHT_CLAMP((uint64_t)target_value_d, 0, max_value);
    *outraw = target_value;
    
    return true;
}

static void _light_print_usage()
{
    printf("Usage:\n"
        "  light [OPTIONS] [VALUE]\n"
        "\n"
        "Commands:\n"
        "  -H, -h      Show this help and exit\n"
        "  -V          Show program version and exit\n"
        "  -L          List available devices\n"
        
        "  -A          Increase brightness by value\n"
        "  -U          Decrease brightness by value\n" 
        "  -S          Set brightness to value\n"
        "  -G          Get brightness\n"
        "  -N          Set minimum brightness to value\n"
        "  -P          Get minimum brightness\n"
        "  -I          Save the current brightness\n"
        "  -O          Restore the previously saved brightness\n"


        "\n"
        "Options:\n"
        "  -r          Interpret input and output values in raw mode\n"
        "  -s          Specify device target path to use, use -L to list available\n"
        "  -v          Specify the verbosity level (default 0)\n"
        "                 0: Values only\n"
        "                 1: Values, Errors.\n"
        "                 2: Values, Errors, Warnings.\n"
        "                 3: Values, Errors, Warnings, Notices.\n"
        "\n");

    printf("Copyright (C) %s  %s\n", LIGHT_YEAR, LIGHT_AUTHOR);
    printf("This is free software, see the source for copying conditions.  There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\n"
        "\n");
}

static bool _light_set_context_command(light_context_t *ctx, LFUNCCOMMAND new_cmd)
{
    if(ctx->run_params.command != NULL)
    {
        LIGHT_WARN("a command was already set. ignoring.");
        return false;
    }
    
    ctx->run_params.command = new_cmd;
    return true;
}

static bool _light_parse_arguments(light_context_t *ctx, int argc, char** argv)
{
    int32_t curr_arg = -1;
    int32_t log_level = 0;
    
    char ctrl_name[NAME_MAX];
    bool need_value = false;
    bool need_target = true; // default cmd is get brightness
    bool specified_target = false;
    snprintf(ctrl_name, sizeof(ctrl_name), "%s", "sysfs/backlight/auto");
    
    while((curr_arg = getopt(argc, argv, "HhVGSLMNPAUOIv:s:r")) != -1)
    {
        switch(curr_arg)
        {
            // Options
            
            case 'v':
                if (sscanf(optarg, "%i", &log_level) != 1)
                {
                    fprintf(stderr, "-v argument is not an integer.\n\n");
                    _light_print_usage();
                    return false;
                }
                
                if (log_level < 0 || log_level > 3)
                {
                    fprintf(stderr, "-v argument must be between 0 and 3.\n\n");
                    _light_print_usage();
                    return false;
                }
                
                light_loglevel = (light_loglevel_t)log_level;
                break;
            case 's':
                snprintf(ctrl_name, sizeof(ctrl_name), "%s", optarg);
                specified_target = true;
                break;
            case 'r':
                ctx->run_params.raw_mode = true;
                break;
            
            // Commands
            case 'H':
            case 'h':
                _light_set_context_command(ctx, light_cmd_print_help);
                break;
            case 'V':
                _light_set_context_command(ctx, light_cmd_print_version);
                break;
            case 'G':
                _light_set_context_command(ctx, light_cmd_get_brightness);
                need_target = true;
                break;
            case 'S':
                _light_set_context_command(ctx, light_cmd_set_brightness);
                need_value = true;
                need_target = true;
                break;
            case 'L':
                _light_set_context_command(ctx, light_cmd_list_devices);
                break;
            case 'M':
                _light_set_context_command(ctx, light_cmd_get_max_brightness);
                need_target = true;
                break;
            case 'N':
                _light_set_context_command(ctx, light_cmd_set_min_brightness);
                need_target = true;
                need_value = true;
                break;
            case 'P':
                _light_set_context_command(ctx, light_cmd_get_min_brightness);
                need_target = true;
                break;
            case 'A':
                _light_set_context_command(ctx, light_cmd_add_brightness);
                need_target = true;
                need_value = true;
                break;
            case 'U':
                _light_set_context_command(ctx, light_cmd_sub_brightness);
                need_target = true;
                need_value = true;
                break;
            case 'O':
                _light_set_context_command(ctx, light_cmd_save_brightness);
                need_target = true;
                break;
            case 'I':
                _light_set_context_command(ctx, light_cmd_restore_brightness);
                need_target = true;
                break;
        }
    }

    if(ctx->run_params.command == NULL)
    {
        _light_set_context_command(ctx, light_cmd_get_brightness);
    }
    
    if(need_target)
    {
        light_device_target_t *curr_target = light_find_device_target(ctx, ctrl_name);
        if(curr_target == NULL)
        {
            if(specified_target)
            {
                fprintf(stderr, "We couldn't find the specified device target at the path \"%s\". Use -L to find one.\n\n", ctrl_name);
                return false;
            }
            else 
            {
                fprintf(stderr, "No backlight controller was found, so we could not decide an automatic target. The current command will have no effect. Please use -L to find a target and then specify it with -s.\n\n");
                curr_target = light_find_device_target(ctx, "util/test/dryrun");
            }
        }
        
        ctx->run_params.device_target = curr_target;
    }
    
    if(need_value)
    {
        if ( (argc - optind) != 1)
        {
            fprintf(stderr, "please specify a <value> for this command.\n\n");
            _light_print_usage();
            return false;
        }

        if (ctx->run_params.raw_mode)
        {
            if (sscanf(argv[optind], "%lu", &ctx->run_params.value) != 1)
            {
                fprintf(stderr, "<value> is not an integer.\n\n");
                _light_print_usage();
                return false;
            }
            
        }
        else
        {
            double percent_value = 0.0;
            if (sscanf(argv[optind], "%lf", &percent_value) != 1)
            {
                fprintf(stderr, "<value> is not a decimal.\n\n");
                _light_print_usage();
                return false;
            }
            
            percent_value = light_percent_clamp(percent_value);
            
            uint64_t raw_value = 0;
            if(!_light_percent_to_raw(ctx->run_params.device_target, percent_value, &raw_value))
            {
                LIGHT_ERR("failed to convert from percent to raw for device target");
                return false;
            }
            
            ctx->run_params.value = raw_value;
        }
    }
    
    return true;
    
}




/* API function definitions */

light_context_t* light_initialize(int argc, char **argv)
{
    light_context_t *new_ctx = malloc(sizeof(light_context_t));

    // Setup default values and runtime params
    new_ctx->enumerators = NULL;
    new_ctx->num_enumerators = 0;
    new_ctx->run_params.command = NULL;
    new_ctx->run_params.device_target = NULL;
    new_ctx->run_params.value = 0;
    new_ctx->run_params.raw_mode = false;

    // Setup the configuration folder
    // If we are root, use the system-wide configuration folder, otherwise try to find a user-specific folder, or fall back to ~/.config
    if (geteuid() == 0)
    {
        snprintf(new_ctx->sys_params.conf_dir, sizeof(new_ctx->sys_params.conf_dir), "%s", "/etc/light");
    }
    else
    {
        char *xdg_conf = getenv("XDG_CONFIG_HOME");
        
        if (xdg_conf != NULL)
        {
            snprintf(new_ctx->sys_params.conf_dir, sizeof(new_ctx->sys_params.conf_dir), "%s/light", xdg_conf);
        }
        else
        {
            snprintf(new_ctx->sys_params.conf_dir, sizeof(new_ctx->sys_params.conf_dir), "%s/.config/light", getenv("HOME"));
        }
    }
    
    // Make sure the configuration folder exists, otherwise attempt to create it
    int32_t rc = light_mkpath(new_ctx->sys_params.conf_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (rc && errno != EEXIST)
    {
        LIGHT_WARN("couldn't create configuration directory");
        return false;
    }
    
    // Create the built-in enumerators
    light_create_enumerator(new_ctx, "sysfs", &impl_sysfs_init, &impl_sysfs_free);
    light_create_enumerator(new_ctx, "util", &impl_util_init, &impl_util_free);
    light_create_enumerator(new_ctx, "razer", &impl_razer_init, &impl_razer_free);

    // This is where we would create enumerators from plugins as well
    // 1. Run the plugins get_name() function to get its name
    // 2. Point to the plugins init() and free() functions when creating the enumerator

    // initialize all enumerators, this will create all the devices and their targets
    if(!light_init_enumerators(new_ctx))
    {
        LIGHT_WARN("failed to initialize all enumerators");
    }

    // Parse arguments
    if(!_light_parse_arguments(new_ctx, argc, argv))
    {
        LIGHT_ERR("failed to parse arguments");
        return NULL;
    }
    
    return new_ctx;
}

bool light_execute(light_context_t *ctx)
{
    if(ctx->run_params.command == NULL)
    {
        LIGHT_ERR("run parameters command was null, can't execute");
        return false;
    }
    
    return ctx->run_params.command(ctx);
}

void light_free(light_context_t *ctx)
{
    if(!light_free_enumerators(ctx))
    {
        LIGHT_WARN("failed to free all enumerators");
    }
    
    free(ctx);
}

light_device_enumerator_t * light_create_enumerator(light_context_t *ctx, char const * name, LFUNCENUMINIT init_func, LFUNCENUMFREE free_func)
{
    // Create a new enumerator array 
    uint64_t new_num_enumerators = ctx->num_enumerators + 1;
    light_device_enumerator_t **new_enumerators = malloc(new_num_enumerators * sizeof(light_device_enumerator_t*));
    
    // Copy old enumerator array to new one
    for(uint64_t i = 0; i < ctx->num_enumerators; i++)
    {
        new_enumerators[i] = ctx->enumerators[i];
    }
    
    // Allocate the new enumerator
    new_enumerators[ctx->num_enumerators] = malloc(sizeof(light_device_enumerator_t));
    light_device_enumerator_t *returner = new_enumerators[ctx->num_enumerators];
    
    returner->devices = NULL;
    returner->num_devices = 0;
    returner->init = init_func;
    returner->free = free_func;
    snprintf(returner->name, sizeof(returner->name), "%s", name);
    
    // Free the old enumerator array, if needed
    if(ctx->enumerators != NULL)
    {
        free(ctx->enumerators);
    }
    
    // Replace the enumerator array with the new one
    ctx->enumerators = new_enumerators;
    ctx->num_enumerators = new_num_enumerators;
    
    // Return newly created device
    return returner;
}

bool light_init_enumerators(light_context_t *ctx)
{
    bool success = true;
    for(uint64_t i = 0; i < ctx->num_enumerators; i++)
    {
        light_device_enumerator_t * curr_enumerator = ctx->enumerators[i];
        if(!curr_enumerator->init(curr_enumerator))
        {
            success = false;
        }
    }
    
    return success;
}

bool light_free_enumerators(light_context_t *ctx)
{
    bool success = true;
    for(uint64_t i = 0; i < ctx->num_enumerators; i++)
    {
        light_device_enumerator_t * curr_enumerator = ctx->enumerators[i];
        
        if(!curr_enumerator->free(curr_enumerator))
        {
            success = false;
        }
        
        if(curr_enumerator->devices != NULL)
        {
            for(uint64_t d = 0; d < curr_enumerator->num_devices; d++)
            {
                light_delete_device(curr_enumerator->devices[d]);
            }
            
            free(curr_enumerator->devices);
            curr_enumerator->devices = NULL;
        }
        
        free(curr_enumerator);
    }
    
    free(ctx->enumerators);
    ctx->enumerators = NULL;
    ctx->num_enumerators = 0;
    
    return success;
}

bool light_split_target_path(char const *in_path, light_target_path_t *out_path)
{
    char const * begin = in_path;
    char const * end = strstr(begin, "/");
    if(end == NULL)
    {
        LIGHT_WARN("invalid path passed to split_target_path");
        return false;
    }
    
    size_t size = end - begin;
    strncpy(out_path->enumerator, begin, size);
    out_path->enumerator[size] = '\0';
    
    begin = end + 1;
    end = strstr(begin, "/");
    if(end == NULL)
    {
        LIGHT_WARN("invalid path passed to split_target_path");
        return false;
    }
    
    size = end - begin;
    strncpy(out_path->device, begin, size);
    out_path->device[size] = '\0';
    
    strcpy(out_path->target, end + 1);
    
    return true;
}

light_device_target_t* light_find_device_target(light_context_t *ctx, char const * name)
{
    light_target_path_t new_path;
    if(!light_split_target_path(name, &new_path))
    {
        LIGHT_WARN("light_find_device_target needs a path in the format of \"enumerator/device/target\", the following format is not recognized:  \"%s\"", name);
        return NULL;
    }
    
    /*
    Uncomment to debug the split function
    printf("enumerator: %s %u\ndevice: %s %u\ntarget: %s %u\n",
            new_path.enumerator, strlen(new_path.enumerator),
            new_path.device, strlen(new_path.device),
            new_path.target, strlen(new_path.target));
    */
    
    // find a matching enumerator 
    
    light_device_enumerator_t *enumerator = _light_find_enumerator(ctx, new_path.enumerator);
    if(enumerator == NULL)
    {
        LIGHT_WARN("no such enumerator, \"%s\"", new_path.enumerator);
        return NULL;
    }
    
    light_device_t *device = _light_find_device(enumerator, new_path.device);
    if(device == NULL)
    {
        LIGHT_WARN("no such device, \"%s\"", new_path.device);
        return NULL;
    }
    
    light_device_target_t *target = _light_find_target(device, new_path.target);
    if(target == NULL)
    {
        LIGHT_WARN("no such target, \"%s\"", new_path.target);
        return NULL;
    }
    
    return target;
}

bool light_cmd_print_help(light_context_t *ctx)
{
    _light_print_usage();
    return true;
}

bool light_cmd_print_version(light_context_t *ctx)
{
    printf("v%s\n", VERSION);
    return true;
}

bool light_cmd_list_devices(light_context_t *ctx)
{
    printf("Listing device targets:\n");
    for(uint64_t enumerator = 0; enumerator < ctx->num_enumerators; enumerator++)
    {
        light_device_enumerator_t *curr_enumerator = ctx->enumerators[enumerator];
        for(uint64_t device = 0; device < curr_enumerator->num_devices; device++)
        {
            light_device_t *curr_device = curr_enumerator->devices[device];
            for(uint64_t target = 0; target < curr_device->num_targets; target++)
            {
                light_device_target_t *curr_target = curr_device->targets[target];
                
                printf("\t%s/%s/%s\n", curr_enumerator->name, curr_device->name, curr_target->name);
            }
        }
    }
    
    return true;
}

bool light_cmd_set_brightness(light_context_t *ctx)
{
    light_device_target_t *target = ctx->run_params.device_target;
    if(target == NULL)
    {
        LIGHT_ERR("didn't have a valid target, programmer mistake");
        return false;
    }
    
    
    uint64_t mincap = _light_get_min_cap(ctx);
    uint64_t value = ctx->run_params.value;
    if(mincap > value)
    {
        value = mincap;
    }
    
    if(!target->set_value(target, value))
    {
        LIGHT_ERR("failed to write to target");
        return false;
    }
    
    return true;
}

bool light_cmd_get_brightness(light_context_t *ctx)
{
    light_device_target_t *target = ctx->run_params.device_target;
    if(target == NULL)
    {
        LIGHT_ERR("didn't have a valid target, programmer mistake");
        return false;
    }
    
    uint64_t value = 0;
    if(!target->get_value(target, &value))
    {
        LIGHT_ERR("failed to read from target");
        return false;
    }
    
    if(ctx->run_params.raw_mode)
    {
        printf("%" PRIu64 "\n", value);
    }
    else 
    {
        double percent = 0.0;
        if(!_light_raw_to_percent(target, value, &percent))
        {
            LIGHT_ERR("failed to convert from raw to percent from device target");
            return false;
        }
        printf("%.2f\n", percent);
    }
    
    return true;
}

bool light_cmd_get_max_brightness(light_context_t *ctx)
{
    light_device_target_t *target = ctx->run_params.device_target;
    if(target == NULL)
    {
        LIGHT_ERR("didn't have a valid target, programmer mistake");
        return false;
    }
    
    if(!ctx->run_params.raw_mode)
    {
        printf("100.0\n");
        return true;
    }
    
    uint64_t max_value = 0;
    if(!target->get_max_value(target, &max_value))
    {
        LIGHT_ERR("failed to read from device target");
        return false;
    }
    
    printf("%" PRIu64 "\n", max_value);
    return true;
}

bool light_cmd_set_min_brightness(light_context_t *ctx)
{
    char target_path[NAME_MAX];
    _light_get_target_path(ctx, target_path, sizeof(target_path));
    
    // Make sure the target folder exists, otherwise attempt to create it
    int32_t rc = light_mkpath(target_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (rc && errno != EEXIST)
    {
        LIGHT_ERR("couldn't create target directory for minimum brightness");
        return false;
    }
    
    char target_filepath[NAME_MAX];
    _light_get_target_file(ctx, target_filepath, sizeof(target_filepath), "minimum");
    
    if(!light_file_write_uint64(target_filepath, ctx->run_params.value))
    {
        LIGHT_ERR("couldn't write value to minimum file");
        return false;
    }
    
    return true;
}

bool light_cmd_get_min_brightness(light_context_t *ctx)
{
    char target_path[NAME_MAX];
    _light_get_target_file(ctx, target_path, sizeof(target_path), "minimum");

    uint64_t minimum_value = 0;
    if(!light_file_read_uint64(target_path, &minimum_value))
    {
        if(ctx->run_params.raw_mode)
        {
            printf("0\n");
        }
        else 
        {
            printf("0.00\n");
        }

        return true;
    }
    
    if(ctx->run_params.raw_mode)
    {
        printf("%" PRIu64 "\n", minimum_value);
    }
    else 
    {
        double minimum_d = 0.0;
        if(!_light_raw_to_percent(ctx->run_params.device_target, minimum_value, &minimum_d))
        {
            LIGHT_ERR("failed to convert value from raw to percent for device target");
            return false;
        }
        
        printf("%.2f\n", minimum_d);
    }
    
    return true;
}

bool light_cmd_add_brightness(light_context_t *ctx)
{
    light_device_target_t *target = ctx->run_params.device_target;
    if(target == NULL)
    {
        LIGHT_ERR("didn't have a valid target, programmer mistake");
        return false;
    }
    
    uint64_t value = 0;
    if(!target->get_value(target, &value))
    {
        LIGHT_ERR("failed to read from target");
        return false;
    }
    
    uint64_t max_value = 0;
    if(!target->get_max_value(target, &max_value))
    {
        LIGHT_ERR("failed to read from target");
        return false;
    }
    
    value += ctx->run_params.value;
    
    uint64_t mincap = _light_get_min_cap(ctx);
    if(mincap > value)
    {
        value = mincap;
    }
    
    
    if(value > max_value)
    {
        value = max_value;
    }
    
    if(!target->set_value(target, value))
    {
        LIGHT_ERR("failed to write to target");
        return false;
    }
    
    return true;
}

bool light_cmd_sub_brightness(light_context_t *ctx)
{
    light_device_target_t *target = ctx->run_params.device_target;
    if(target == NULL)
    {
        LIGHT_ERR("didn't have a valid target, programmer mistake");
        return false;
    }
    
    uint64_t value = 0;
    if(!target->get_value(target, &value))
    {
        LIGHT_ERR("failed to read from target");
        return false;
    }
    
    if(value > ctx->run_params.value)
    {
        value -= ctx->run_params.value;
    }
    else 
    {
        value = 0;
    }
    
    uint64_t mincap = _light_get_min_cap(ctx);
    if(mincap > value)
    {
        value = mincap;
    }

    if(!target->set_value(target, value))
    {
        LIGHT_ERR("failed to write to target");
        return false;
    }
    
    return true;
}

bool light_cmd_save_brightness(light_context_t *ctx)
{
    char target_path[NAME_MAX];
    _light_get_target_path(ctx, target_path, sizeof(target_path));
    
    // Make sure the target folder exists, otherwise attempt to create it
    int32_t rc = light_mkpath(target_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (rc && errno != EEXIST)
    {
        LIGHT_ERR("couldn't create target directory for save brightness");
        return false;
    }
    
    char target_filepath[NAME_MAX];
    _light_get_target_file(ctx, target_filepath, sizeof(target_filepath), "save");

    uint64_t curr_value = 0;
    if(!ctx->run_params.device_target->get_value(ctx->run_params.device_target, &curr_value))
    {
        LIGHT_ERR("couldn't read from device target");
        return false;
    }
    
    if(!light_file_write_uint64(target_filepath, curr_value))
    {
        LIGHT_ERR("couldn't write value to savefile");
        return false;
    }
    
    return true;
}

bool light_cmd_restore_brightness(light_context_t *ctx)
{
    char target_path[NAME_MAX];
    _light_get_target_file(ctx, target_path, sizeof(target_path), "save");

    uint64_t saved_value = 0;
    if(!light_file_read_uint64(target_path, &saved_value))
    {
        LIGHT_ERR("couldn't read value from savefile");
        return false;
    }
    
    uint64_t mincap = _light_get_min_cap(ctx);
    if(mincap > saved_value)
    {
        saved_value = mincap;
    }
    
    if(!ctx->run_params.device_target->set_value(ctx->run_params.device_target, saved_value))
    {
        LIGHT_ERR("couldn't write saved value to device target");
        return false;
    }
    
    return true;
}

light_device_t *light_create_device(light_device_enumerator_t *enumerator, char const *name, void *device_data)
{
    light_device_t *new_device = malloc(sizeof(light_device_t));
    new_device->enumerator = enumerator;
    new_device->targets = NULL;
    new_device->num_targets = 0;
    new_device->device_data = device_data;
    
    snprintf(new_device->name, sizeof(new_device->name), "%s", name);
    
    _light_add_enumerator_device(enumerator, new_device);
    
    return new_device;
}

void light_delete_device(light_device_t *device)
{
    for(uint64_t i = 0; i < device->num_targets; i++)
    {
        light_delete_device_target(device->targets[i]);
    }
    
    if(device->targets != NULL)
    {
        free(device->targets);
    }
    
    if(device->device_data != NULL)
    {
        free(device->device_data);
    }
    
    free(device);
}

light_device_target_t *light_create_device_target(light_device_t *device, char const *name, LFUNCVALSET setfunc, LFUNCVALGET getfunc, LFUNCMAXVALGET getmaxfunc, LFUNCCUSTOMCMD cmdfunc, void *target_data)
{
    light_device_target_t *new_target = malloc(sizeof(light_device_target_t));
    new_target->device = device;
    new_target->set_value = setfunc;
    new_target->get_value = getfunc;
    new_target->get_max_value = getmaxfunc;
    new_target->custom_command = cmdfunc;
    new_target->device_target_data = target_data;
    
    snprintf(new_target->name, sizeof(new_target->name), "%s", name);
    
    _light_add_device_target(device, new_target);
    
    return new_target;
}

void light_delete_device_target(light_device_target_t *device_target)
{
    if(device_target->device_target_data != NULL)
    {
        free(device_target->device_target_data);
    }
    
    free(device_target);
}
