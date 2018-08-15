#ifndef LIGHT_H_
#define LIGHT_H_

#include "config.h"
#include "helpers.h"

#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>

#define LIGHT_YEAR   "2012-2018"
#define LIGHT_AUTHOR "Fredrik Haikarainen"

#define ASSERT_SET(t, v)						\
	if (v) {							\
		fprintf(stderr, t " cannot be used more than once.\n"); \
		return false;						\
	}								\
	v = true;

#define ASSERT_CMDSET()    ASSERT_SET("Commands", cmd_set)
#define ASSERT_TARGETSET() ASSERT_SET("Targets", target_set)
#define ASSERT_FIELDSET()  ASSERT_SET("Fields", field_set)
#define ASSERT_CTRLSET()   ASSERT_SET("Controllers", ctrl_set)
#define ASSERT_VALSET()    ASSERT_SET("Values", val_set)

typedef enum {
	LIGHT_BRIGHTNESS = 0,
	LIGHT_MAX_BRIGHTNESS,
	LIGHT_MIN_CAP,
	LIGHT_SAVERESTORE
} light_field_t;

typedef enum {
	LIGHT_BACKLIGHT = 0,
	LIGHT_KEYBOARD
} light_target_t;

typedef enum {
	LIGHT_AUTO = 0,
	LIGHT_SPECIFY
} light_ctrl_mode_t;

typedef enum {
	LIGHT_GET = 0,
	LIGHT_SET,
	LIGHT_ADD,
	LIGHT_SUB,
	LIGHT_PRINT_HELP,	/* Prints help and exits  */
	LIGHT_PRINT_VERSION,	/* Prints version info and exits */
	LIGHT_LIST_CTRL,
	LIGHT_RESTORE,
	LIGHT_SAVE
} light_cmd_t;

typedef enum {
	LIGHT_RAW = 0,
	LIGHT_PERCENT
} light_val_mode_t;

typedef struct {
	/* Cache file prefix */
	char               prefix[NAME_MAX + 1];

	/* Which controller to use */
	light_ctrl_mode_t  ctrl;
	char               ctrl_name[NAME_MAX + 1];

	/* What to do with the controller */
	light_cmd_t        cmd;
	light_val_mode_t   val_mode;
	unsigned long      val_raw;	/* The specified value in raw mode */
	double             val_percent;	/* The specified value in percent */

	light_target_t     target;
	light_field_t      field;

	/* Cache data */
	bool               has_cached_brightness_max;
	unsigned long      cached_brightness_max;
} light_ctx_t;

/* -- Global variable holding the settings for the current run -- */
light_ctx_t ctx;

bool light_initialize              (int argc, char **argv);
bool light_execute                 (void);
void light_free                    (void);

bool light_ctrl_list               (void);
bool light_ctrl_probe              (char *controller, size_t len);
bool light_ctrl_exist              (char const *controller);

bool light_ctrl_get_brightness     (char const *controller, unsigned long *v);
bool light_ctrl_set_brightness     (char const *controller, unsigned long v);
bool light_ctrl_get_brightness_max (char const *controller, unsigned long *v);

bool light_ctrl_get_cap_min        (char const *controller, bool *hasMinCap, unsigned long *minCap);
bool light_ctrl_set_cap_min        (char const *controller, unsigned long val);

bool light_ctrl_save_brightness    (char const *controller, unsigned long val);
bool light_ctrl_restore_brightness (char const *controller);

#endif /* LIGHT_H_ */
