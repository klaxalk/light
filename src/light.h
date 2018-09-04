
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <limits.h> // NAME_MAX
#include <stddef.h> // NULL

#include "config.h"

#define LIGHT_YEAR   "2012 - 2018"
#define LIGHT_AUTHOR "Fredrik Haikarainen"

struct _light_device_target_t;
typedef struct _light_device_target_t light_device_target_t;

struct _light_device_t;
typedef struct _light_device_t light_device_t;

struct _light_device_enumerator_t;
typedef struct _light_device_enumerator_t light_device_enumerator_t;

/* Function pointers that implementations have to set for device targets */
typedef bool (*LFUNCVALSET)(light_device_target_t*, uint64_t);
typedef bool (*LFUNCVALGET)(light_device_target_t*, uint64_t*);
typedef bool (*LFUNCMAXVALGET)(light_device_target_t*, uint64_t*);
typedef bool (*LFUNCCUSTOMCMD)(light_device_target_t*, char const *);

/* Describes a target within a device (for example a led on a keyboard, or a controller for a backlight) */
struct _light_device_target_t
{
    char           name[256];
    LFUNCVALSET    set_value;
    LFUNCVALGET    get_value;
    LFUNCMAXVALGET get_max_value;
    LFUNCCUSTOMCMD custom_command;
    void           *device_target_data;
    light_device_t *device;
};

/* Describes a device (a backlight, a keyboard, a led-strip) */
struct _light_device_t
{
    char                  name[256];
    light_device_target_t **targets;
    uint64_t              num_targets;
    void                  *device_data;
    light_device_enumerator_t *enumerator;
};


typedef bool (*LFUNCENUMINIT)(light_device_enumerator_t*);
typedef bool (*LFUNCENUMFREE)(light_device_enumerator_t*);

/* An enumerator that is responsible for creating and freeing devices as well as their targets */
struct _light_device_enumerator_t
{
    char            name[256];
    LFUNCENUMINIT   init;
    LFUNCENUMFREE   free;

    light_device_t  **devices;
    uint64_t        num_devices;
};

typedef struct _light_context_t light_context_t;

// A command that can be run (set, get, add, subtract, print help, print version, list devices etc.)
typedef bool (*LFUNCCOMMAND)(light_context_t *);

struct _light_context_t
{
    struct 
    {
        LFUNCCOMMAND            command; // What command was issued 
        uint64_t                value; // The input value, in raw mode
        bool                    raw_mode; // Whether or not we use raw or percentage mode
        light_device_target_t   *device_target; // The device target to act on
    } run_params;

    struct
    {
        char                    conf_dir[NAME_MAX]; // The path to the application cache directory 
    } sys_params;
    
    light_device_enumerator_t   **enumerators;
    uint64_t                    num_enumerators;
};

// The different available commands
bool light_cmd_print_help(light_context_t *ctx); // H,h 
bool light_cmd_print_version(light_context_t *ctx); // V
bool light_cmd_list_devices(light_context_t *ctx); // L
bool light_cmd_set_brightness(light_context_t *ctx); // S
bool light_cmd_get_brightness(light_context_t *ctx); // G
bool light_cmd_get_max_brightness(light_context_t *ctx); // M
bool light_cmd_set_min_brightness(light_context_t *ctx); // N
bool light_cmd_get_min_brightness(light_context_t *ctx); // P
bool light_cmd_add_brightness(light_context_t *ctx); // A
bool light_cmd_sub_brightness(light_context_t *ctx); // U
bool light_cmd_save_brightness(light_context_t *ctx); // O
bool light_cmd_restore_brightness(light_context_t *ctx); // I

/* Initializes the application, given the command-line. Returns a context. */
light_context_t* light_initialize(int argc, char **argv);

/* Executes the given context. Returns true on success, false on failure. */
bool light_execute(light_context_t*);

/* Frees the given context */
void light_free(light_context_t*);

/* Create a device enumerator in the given context */
light_device_enumerator_t * light_create_enumerator(light_context_t *ctx, char const * name, LFUNCENUMINIT, LFUNCENUMFREE);

/* Initializes all the device enumerators (and its devices, targets) */
bool light_init_enumerators(light_context_t *ctx);

/* Frees all the device enumerators (and its devices, targets) */
bool light_free_enumerators(light_context_t *ctx);

void light_add_enumerator_device(light_device_enumerator_t *enumerator, light_device_t *new_device);

void light_add_device_target(light_device_t *device, light_device_target_t *new_target);

typedef struct _light_target_path_t light_target_path_t;
struct _light_target_path_t 
{
    char enumerator[NAME_MAX];
    char device[NAME_MAX];
    char target[NAME_MAX];
};

bool light_split_target_path(char const * in_path, light_target_path_t *out_path);

/* Returns the found device target, or null. Name should be enumerator/device/target */
light_device_target_t* light_find_device_target(light_context_t *ctx, char const * name);
