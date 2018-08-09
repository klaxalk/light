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

#define ASSERT_SET(t,v)							\
	if (v) {							\
		fprintf(stderr, t" arguments cannot be used in conjunction.\n"); \
		return false;						\
	}								\
	v = true;

#define ASSERT_OPSET() ASSERT_SET("Operation", opSet)
#define ASSERT_TARGETSET() ASSERT_SET("Target", targetSet)
#define ASSERT_FIELDSET() ASSERT_SET("Field", fieldSet)
#define ASSERT_CTRLSET() ASSERT_SET("Controller", ctrlSet)
#define ASSERT_VALSET() ASSERT_SET("Value", valSet)

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

typedef struct light_runtimeArguments_s {
	/* Which controller to use */
	light_ctrl_mode_t controllerMode;
	char specifiedController[NAME_MAX + 1];

	/* What to do with the controller */
	light_cmd_t operationMode;
	light_val_mode_t valueMode;
	unsigned long specifiedValueRaw;	/* The specified value in raw mode */
	double specifiedValuePercent;	/* The specified value in percent */

	light_target_t target;
	light_field_t field;

	/* Cache data */
	bool hasCachedMaxBrightness;
	unsigned long cachedMaxBrightness;

} light_runtimeArguments, *light_runtimeArguments_p;

/* -- Global variable holding the settings for the current run -- */
light_runtimeArguments light_Configuration;

/* Sets default values for the configuration */
void light_defaultConfig();

/* Parses the program arguments and sets the configuration accordingly (unsanitized) */
bool light_parseArguments(int argc, char **argv);

/* Prints a header if verbosity level > 0 */
void light_printVersion(void);

/* Prints help regardless of verbosity level */
void light_printHelp(void);

/* -- SECTION: Main code -- */

/* Initializes the application */
bool light_initialize(int argc, char **argv);

/* Does the work */
bool light_execute(void);

/* Frees up resources */
void light_free();

/* SECTION: Controller functionality */

/* WARNING: `buffer` HAS to be freed by the user if not null once returned!
 * Size is always NAME_MAX + 1 */
bool light_genPath(char const *controller, light_target_t target, light_field_t type, char **buffer);

bool light_validControllerName(char const *controller);

bool light_getBrightness(char const *controller, unsigned long *v);

bool light_getMaxBrightness(char const *controller, unsigned long *v);

bool light_setBrightness(char const *controller, unsigned long v);

bool light_controllerAccessible(char const *controller);

/* WARNING: `controller` HAS to be at most NAME_MAX, otherwise fails */
bool light_getBestController(char *controller);

bool light_getMinCap(char const *controller, bool * hasMinCap, unsigned long *minCap);

bool light_setMinCap(char const *controller, unsigned long v);

bool light_listControllers();

bool light_saveBrightness(char const *controller, unsigned long v);

bool light_restoreBrightness(char const *controller);

#endif /* LIGHT_H_ */
