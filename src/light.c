#include "light.h"

#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>

/* Sets default values for the configuration */
static void light_defaults(void)
{
	ctx.ctrl = LIGHT_AUTO;
	memset(&ctx.ctrl_name, '\0', NAME_MAX + 1);
	ctx.cmd = LIGHT_GET;
	ctx.val_mode = LIGHT_PERCENT;
	ctx.val_raw = 0;
	ctx.val_percent = 0.0;
	ctx.target = LIGHT_BACKLIGHT;
	ctx.field = LIGHT_BRIGHTNESS;
	ctx.has_cached_brightness_max = false;
	ctx.cached_brightness_max = 0;
	light_loglevel = 0;
}

static bool light_check_ops(void)
{
	bool valid = true;
	light_cmd_t op = ctx.cmd;

	/* Nothing to check if we just print info */
	if (op == LIGHT_PRINT_HELP || op == LIGHT_PRINT_VERSION || op == LIGHT_LIST_CTRL)
		return true;

	switch (ctx.field) {
	case LIGHT_BRIGHTNESS:
		if (op != LIGHT_GET && op != LIGHT_SET &&
		    op != LIGHT_ADD && op != LIGHT_SUB && op != LIGHT_SAVE && op != LIGHT_RESTORE) {
			valid = false;
			fprintf(stderr,
				"Wrong operation specified for brightness. You can use only -G -S -A or -U\n\n");
		}
		break;

	case LIGHT_MAX_BRIGHTNESS:
		if (op != LIGHT_GET) {
			valid = false;
			fprintf(stderr, "Wrong operation specified for max brightness. You can only use -G\n\n");
		}
		break;

	case LIGHT_MIN_CAP:
		if (op != LIGHT_GET && op != LIGHT_SET) {
			valid = false;
			fprintf(stderr, "Wrong operation specified for min cap. You can only use -G or -S\n\n");
		}

	default:
		break;
	}

	return valid;
}

static bool light_check_ctrl(char const *controller)
{
	if (!controller) {
		LIGHT_WARN("Invalid or missing controller name");
		return false;
	}

	if (strlen(controller) > NAME_MAX) {
		LIGHT_WARN("Too long controller name, %s", controller);
		return false;
	}

	return true;
}

/* Prints help regardless of verbosity level */
static void light_usage(void)
{
	printf("Usage:\n"
	       "  light [OPTIONS] <COMMAND> [VALUE]\n"
	       "\n"
	       "Commands:\n"
	       "  -A VAL  Add value\n"
	       "  -G      Get (read) value, default command\n"
	       "  -H, -h  Show this help and exit\n"
	       "  -I      Restore brightness\n"
	       "  -L      List available controllers\n"
	       "  -O      Save brightness\n"
	       "  -S VAL  Set (write) value\n"
	       "  -U VAL  Subtract value\n"
	       "  -V      Show program version and exit\n"
	       "\n"
	       "Options:\n"
	       "  -a      Automatic controller selection, default\n"
	       "  -b      Brightness, default\n"
	       "  -c      Act on minimum cap, only possible to read and set\n"
	       "  -k      Act on keyboard backlight\n"
	       "  -l      Act on screen backlight, default\n"
	       "  -m      Maximum brightness, only possible to read\n"
	       "  -p      Interpret input, and output, values in percent, default\n"
	       "  -r      Interpret input, and outpot, values in raw mode\n"
	       "  -s ARG  Specify controller to use, use -L and -Lk to list available\n"
	       "  -v ARG  Verbosity level:\n"
	       "            0: Values only, default\n"
	       "            1: Values, Errors.\n"
	       "            2: Values, Errors, Warnings.\n"
	       "            3: Values, Errors, Warnings, Notices.\n"
	       "\n");

	printf("Copyright (C) %s  %s\n", LIGHT_YEAR, LIGHT_AUTHOR);
	printf("This is free software, see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\n"
	       "\n");
}

static bool light_parse_args(int argc, char **argv)
{
	bool cmd_set    = false;
	bool target_set = false;
	bool field_set  = false;
	bool ctrl_set   = false;
	bool val_set    = false;
	int loglevel;
	int val;

	while ((val = getopt(argc, argv, "HhVGSAULIObmclkas:prv:")) != -1) {
		switch (val) {
			/* -- Operations -- */
		case 'H':
		case 'h':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_PRINT_HELP;
			break;

		case 'V':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_PRINT_VERSION;
			break;

		case 'G':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_GET;
			break;

		case 'S':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_SET;
			break;

		case 'A':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_ADD;
			break;

		case 'U':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_SUB;
			break;

		case 'L':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_LIST_CTRL;
			break;

		case 'I':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_RESTORE;
			break;

		case 'O':
			ASSERT_CMDSET();
			ctx.cmd = LIGHT_SAVE;
			break;

			/* -- Targets -- */
		case 'l':
			ASSERT_TARGETSET();
			ctx.target = LIGHT_BACKLIGHT;
			break;

		case 'k':
			ASSERT_TARGETSET();
			ctx.target = LIGHT_KEYBOARD;
			break;

			/* -- Fields -- */
		case 'b':
			ASSERT_FIELDSET();
			ctx.field = LIGHT_BRIGHTNESS;
			break;

		case 'm':
			ASSERT_FIELDSET();
			ctx.field = LIGHT_MAX_BRIGHTNESS;
			break;

		case 'c':
			ASSERT_FIELDSET();
			ctx.field = LIGHT_MIN_CAP;
			break;

			/* -- Controller selection -- */
		case 'a':
			ASSERT_CTRLSET();
			ctx.ctrl = LIGHT_AUTO;
			break;;

		case 's':
			ASSERT_CTRLSET();
			ctx.ctrl = LIGHT_SPECIFY;
			if (!light_check_ctrl(optarg))
				return false;
			strncpy(ctx.ctrl_name, optarg, NAME_MAX);
			ctx.ctrl_name[NAME_MAX] = '\0';
			break;
			/* -- Value modes -- */

		case 'p':
			ASSERT_VALSET();
			ctx.val_mode = LIGHT_PERCENT;
			break;

		case 'r':
			ASSERT_VALSET();
			ctx.val_mode = LIGHT_RAW;
			break;

			/* -- Other -- */
		case 'v':
			if (sscanf(optarg, "%i", &loglevel) != 1) {
				fprintf(stderr, "-v ARG is not recognizable.\n\n");
				light_usage();
				return false;
			}
			if (loglevel < 0 || loglevel > 3) {
				fprintf(stderr, "-v ARG must be between 0 and 3.\n\n");
				light_usage();
				return false;
			}
			light_loglevel = (light_loglevel_t)loglevel;
			break;
		}
	}

	if (!light_check_ops()) {
		light_usage();
		return false;
	}

	/* If we need a <value> (for writing), make sure we have it! */
	if (ctx.cmd == LIGHT_SET ||
	    ctx.cmd == LIGHT_ADD || ctx.cmd == LIGHT_SUB) {
		if (argc - optind != 1) {
			fprintf(stderr, "Light needs an argument for <value>.\n\n");
			light_usage();
			return false;
		}

		if (ctx.val_mode == LIGHT_PERCENT) {
			if (sscanf(argv[optind], "%lf", &ctx.val_percent) != 1) {
				fprintf(stderr, "<value> is not specified in a recognizable format.\n\n");
				light_usage();
				return false;
			}
			ctx.val_percent = light_percent_clamp(ctx.val_percent);
		} else {
			if (sscanf(argv[optind], "%lu", &ctx.val_raw) != 1) {
				fprintf(stderr, "<value> is not specified in a recognizable format.\n\n");
				light_usage();
				return false;
			}
		}

	}

	return true;
}

static int mkpath(char *dir, mode_t mode)
{
	struct stat sb;

	if (!dir) {
		errno = EINVAL;
		return -1;
	}

	if (!stat(dir, &sb))
		return 0;

	mkpath(dirname(strdupa(dir)), mode);

	return mkdir(dir, mode);
}

bool light_initialize(int argc, char **argv)
{
	light_cmd_t mode;
	int rc;

	/* Classic SUID root mode or new user based cache files */
	if (geteuid() == 0)
		snprintf(ctx.prefix, sizeof(ctx.prefix), "%s", "/etc/light");
	else
		snprintf(ctx.prefix, sizeof(ctx.prefix), "%s/.cache/light", getenv("HOME"));

	light_defaults();
	if (!light_parse_args(argc, argv)) {
		LIGHT_ERR("could not parse arguments");
		return false;
	}

	/* Just return true for operation modes that do not need initialization */
	mode = ctx.cmd;
	if (mode == LIGHT_PRINT_HELP || mode == LIGHT_PRINT_VERSION || mode == LIGHT_LIST_CTRL)
		return true;

	/* Make sure we have a valid cache directory for all files */
	if (mode == LIGHT_SAVE || (mode == LIGHT_SET && ctx.field == LIGHT_MIN_CAP)) {
		const char *dirs[5] = {
			"/mincap/kbd", "/save/kbd", NULL
		};
		char path[strlen(ctx.prefix) + 20];
		int i;

		for (i = 0; dirs[i]; i++) {
			snprintf(path, sizeof(path), "%s%s", ctx.prefix, dirs[i]);

			rc = mkpath(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (rc && errno != EEXIST) {
				LIGHT_ERR("'%s' does not exist and could not be created,"
					  " make sure this application is run as root.", path);
				return false;
			}
		}
	}

	/* Make sure we have a valid controller before we proceed */
	if (ctx.ctrl == LIGHT_AUTO) {
		LIGHT_NOTE("Automatic mode -- finding best controller");
		if (!light_ctrl_probe(ctx.ctrl_name)) {
			LIGHT_ERR("could not find suitable controller");
			return false;
		}
	} else if (!light_ctrl_exist(ctx.ctrl_name)) {
		LIGHT_ERR("selected controller '%s' is not valid", ctx.ctrl_name);
		return false;
	}

	return true;
}

/* Print help and version info */
static bool light_info(void)
{
	if (ctx.cmd == LIGHT_PRINT_HELP) {
		light_usage();
		return true;
	}

	if (ctx.cmd == LIGHT_PRINT_VERSION) {
		printf("v%s\n", VERSION);
		return true;
	}

	if (ctx.cmd == LIGHT_LIST_CTRL) {
		/* listControllers() can return false, but only if it does not find any controllers. That is not enough for an unsuccessfull run. */
		light_ctrl_list();
		return true;
	}

	return false;
}

static bool validate(unsigned long *cur_raw, unsigned long *max_raw, bool *has_cap, unsigned long *min_cap)
{
	if (ctx.has_cached_brightness_max) {
		*max_raw = ctx.cached_brightness_max;
	} else if (!light_ctrl_get_brightness_max(ctx.ctrl_name, max_raw)) {
		LIGHT_ERR("could not get max brightness");
		return false;
	}

	/* No need to go further if targetting mincap */
	if (ctx.field == LIGHT_MIN_CAP || ctx.field == LIGHT_MAX_BRIGHTNESS) {
		/* Init other values to 0 */
		*cur_raw = *min_cap = 0;
		*has_cap = false;
		return true;
	}

	if (!light_ctrl_get_brightness(ctx.ctrl_name, cur_raw)) {
		LIGHT_ERR("could not get brightness");
		return false;
	}

	if (!light_ctrl_get_cap_min(ctx.ctrl_name, has_cap, min_cap)) {
		LIGHT_ERR("could not get min brightness");
		return false;
	}

	if (*has_cap && *min_cap > *max_raw) {
		LIGHT_WARN("invalid minimum cap (raw) value of '%lu' for controller, ignoring and using 0",
			       *min_cap);
		LIGHT_WARN("minimum cap must be inferior to '%lu'", *max_raw);
		min_cap = 0;
	}
	return true;
}

bool light_execute(void)
{
	light_val_mode_t val_mode;

	unsigned long cur_raw;	/* The current brightness, in raw units */
	double cur_percent;	/* The current brightness, in percent  */
	unsigned long max_raw;	/* The max brightness, in percent      */

	unsigned long min_cap;	/* The minimum cap, in raw units */
	double min_cap_percent;	/* The minimum cap, in percent */
	bool has_cap;		/* If we have a minimum cap     */

	if (light_info())
		return true;

	if (!validate(&cur_raw, &max_raw, &has_cap, &min_cap))
		return false;

	val_mode = ctx.val_mode;
	cur_percent = light_percent_clamp(((double)cur_raw) / ((double)max_raw) * 100);
	min_cap_percent = light_percent_clamp(((double)min_cap) / ((double)max_raw) * 100);

	LIGHT_NOTE("executing light on '%s' controller", ctx.ctrl_name);

	/* Handle get operations */
	if (ctx.cmd == LIGHT_GET) {
		switch (ctx.field) {
		case LIGHT_BRIGHTNESS:
			(val_mode == LIGHT_RAW) ? printf("%lu\n", cur_raw) : printf("%.2f\n", cur_percent);
			break;
		case LIGHT_MAX_BRIGHTNESS:
			(val_mode == LIGHT_RAW) ? printf("%lu\n", max_raw) : printf("100.00\n");	/* <- I know how stupid it is but it might just make someones life easier */
			break;
		case LIGHT_MIN_CAP:
			(val_mode == LIGHT_RAW) ? printf("%lu\n", min_cap) : printf("%.2f\n", min_cap_percent);
			break;
		case LIGHT_SAVERESTORE:
			break;
		}
		return true;
	}

	/* Handle saves and restores */
	if (ctx.cmd == LIGHT_SAVE) {
		if (!light_ctrl_save_brightness(ctx.ctrl_name, cur_raw)) {
			LIGHT_ERR("could not save brightness");
			return false;
		}

		return true;
	}

	if (ctx.cmd == LIGHT_RESTORE) {
		if (!light_ctrl_restore_brightness(ctx.ctrl_name)) {
			LIGHT_ERR("could not restore brightness");
			return false;
		}

		return true;
	}

	/* Handle set/add/sub operations */
	if (ctx.cmd == LIGHT_SET ||
	    ctx.cmd == LIGHT_ADD || ctx.cmd == LIGHT_SUB) {
		unsigned long raw;

		if (val_mode == LIGHT_RAW)
			raw = ctx.val_raw;
		else
			raw = (unsigned long)((ctx.val_percent * ((double)max_raw)) / 100.0);

		if (ctx.field == LIGHT_MIN_CAP) {
			/* Handle minimum cap files */
			if (!light_ctrl_set_cap_min(ctx.ctrl_name, LIGHT_CLAMP(raw, 0, max_raw))) {
				LIGHT_ERR("could not set minimum cap");
				return false;
			}

			/* All good? Return true. */
			return true;

		} else if (ctx.field == LIGHT_BRIGHTNESS) {
			/* Handle brightness writing */
			unsigned long writeVal;

			switch (ctx.cmd) {
			case LIGHT_SET:
				writeVal = LIGHT_CLAMP(raw, min_cap, max_raw);
				break;
			case LIGHT_ADD:
				writeVal = LIGHT_CLAMP(cur_raw + raw, min_cap, max_raw);
				break;
			case LIGHT_SUB:
				/* check if we're going below 0, which wouldn't work with unsigned values */
				if (cur_raw < raw) {
					light_log_clamp_min(min_cap);
					writeVal = min_cap;
					break;
				}
				writeVal = LIGHT_CLAMP(cur_raw - raw, min_cap, max_raw);
				break;
				/* we have already taken other possibilities, so we shouldn't get here */
			default:
				return false;
			}

			/* Attempt to write */
			if (!light_ctrl_set_brightness(ctx.ctrl_name, writeVal)) {
				LIGHT_ERR("could not set brightness");
				return false;
			}

			/* All good? return true. */
			return true;
		}
	}

	/* Handle saves and restores */
	if (ctx.cmd == LIGHT_SAVE) {
		if (!light_ctrl_save_brightness(ctx.ctrl_name, cur_raw)) {
			LIGHT_ERR("could not save brightness");
			return false;
		}

		return true;
	}

	if (ctx.cmd == LIGHT_RESTORE) {
		if (!light_ctrl_restore_brightness(ctx.ctrl_name)) {
			LIGHT_ERR("could not restore brightness");
			return false;
		}

		return true;
	}

	fprintf(stderr,
		"Controller : %s\n"
		"Value      : %lu\n"
		"Value %%    : %.2f %%\n"
		"Command    : %u\n"
		"Mode       : %u\n"
		"Field      : %u\n"
		"\n",
		ctx.ctrl_name, ctx.val_raw, ctx.val_percent, ctx.cmd, val_mode, ctx.field);

	fprintf(stderr, "You did not specify a valid combination of command line arguments.\n");
	light_usage();

	return false;
}

void light_free(void)
{

}

/*
 * WARNING: `buffer` HAS to be freed by the user if not null once
 * returned!  Size is always NAME_MAX + 1
 */
bool light_gen_path(char const *controller, light_target_t target, light_field_t type, char **buffer)
{
	char *path;
	int val = -1;

	if (!light_check_ctrl(controller)) {
		LIGHT_ERR("invalid controller, couldn't generate path");
		return false;
	}

	if (!buffer) {
		LIGHT_ERR("a valid buffer is required");
		return false;
	}
	*buffer = NULL;

	/* PATH_MAX define includes the '\0' character, so no + 1 here */
	path = malloc(PATH_MAX);
	if (!path) {
		LIGHT_MEMERR();
		return false;
	}

	if (target == LIGHT_BACKLIGHT) {
		switch (type) {
		case LIGHT_BRIGHTNESS:
			val = snprintf(path, PATH_MAX, "/sys/class/backlight/%s/brightness", controller);
			break;

		case LIGHT_MAX_BRIGHTNESS:
			val = snprintf(path, PATH_MAX, "/sys/class/backlight/%s/max_brightness", controller);
			break;

		case LIGHT_MIN_CAP:
			val = snprintf(path, PATH_MAX, "%s/mincap/%s", ctx.prefix, controller);
			break;

		case LIGHT_SAVERESTORE:
			val = snprintf(path, PATH_MAX, "%s/save/%s", ctx.prefix, controller);
			break;
		}
	} else {
		switch (type) {
		case LIGHT_BRIGHTNESS:
			val = snprintf(path, PATH_MAX, "/sys/class/leds/%s/brightness", controller);
			break;

		case LIGHT_MAX_BRIGHTNESS:
			val = snprintf(path, PATH_MAX, "/sys/class/leds/%s/max_brightness", controller);
			break;

		case LIGHT_MIN_CAP:
			val = snprintf(path, PATH_MAX, "%s/mincap/kbd/%s", ctx.prefix, controller);
			break;

		case LIGHT_SAVERESTORE:
			val = snprintf(path, PATH_MAX, "%s/save/kbd/%s", ctx.prefix, controller);
			break;
		}
	}

	if (val < 0) {
		LIGHT_ERR("snprintf failed");
		free(path);
		return false;
	}

	/* PATH_MAX define includes the '\0' character, so - 1 here */
	if (val > PATH_MAX - 1) {
		LIGHT_ERR("generated path is too long to be handled");
		return false;
	}

	*buffer = path;
	return true;
}

bool light_ctrl_get_brightnessPath(char const *controller, char **path)
{
	if (!light_gen_path(controller, ctx.target, LIGHT_BRIGHTNESS, path)) {
		LIGHT_ERR("could not generate path to brightness file");
		return false;
	}
	return true;
}

bool light_ctrl_get_brightness(char const *controller, unsigned long *v)
{
	char *path = NULL;
	bool rc = false;

	if (!light_ctrl_get_brightnessPath(controller, &path))
		return false;

	rc = light_file_read_val(path, v);
	free(path);

	if (!rc) {
		LIGHT_ERR("could not read value from brightness file");
		return false;
	}
	return true;
}

bool light_ctrl_get_brightness_maxPath(char const *controller, char **path)
{
	if (!light_gen_path(controller, ctx.target, LIGHT_MAX_BRIGHTNESS, path)) {
		LIGHT_ERR("could not generate path to maximum brightness file");
		return false;
	}
	return true;
}

bool light_ctrl_get_brightness_max(char const *controller, unsigned long *v)
{
	char *path = NULL;
	bool rc = false;

	if (!light_ctrl_get_brightness_maxPath(controller, &path))
		return false;

	rc = light_file_read_val(path, v);
	free(path);

	if (!rc) {
		LIGHT_ERR("could not read value from max brightness file");
		return false;
	}

	if (*v == 0) {
		LIGHT_ERR("max brightness is 0, so controller is not valid");
		return false;
	}

	return true;
}

bool light_ctrl_set_brightness(char const *controller, unsigned long v)
{
	char *path = NULL;
	bool rc;

	if (!light_gen_path(controller, ctx.target, ctx.field, &path)) {
		LIGHT_ERR("could not generate path to brightness file");
		return false;
	}

	LIGHT_NOTE("setting brightness %lu (raw) to controller", v);
	rc = light_file_write_val(path, v);

	if (!rc) {
		LIGHT_ERR("could not write value to brightness file");
	}

	free(path);
	return rc;
}

bool light_ctrl_exist(char const *controller)
{
	char *path = NULL;

	/* On auto mode, we need to check if we can read the max brightness value
	   of the controller for later computation */
	if (ctx.ctrl == LIGHT_AUTO || ctx.field == LIGHT_MAX_BRIGHTNESS) {
		if (!light_ctrl_get_brightness_maxPath(controller, &path))
			return false;
		if (!light_file_is_readable(path)) {
			LIGHT_WARN("could not open controller max brightness "
				   "file for reading, so controller is not accessible");
			free(path);
			return false;
		}
		free(path);
	}

	if (!light_ctrl_get_brightnessPath(controller, &path))
		return false;

	if (ctx.cmd != LIGHT_GET && ctx.cmd != LIGHT_SAVE &&
	    ctx.field != LIGHT_MIN_CAP && !light_file_is_writable(path)) {
		LIGHT_WARN("could not open controller brightness file for writing, so controller is not accessible");
		free(path);
		return false;
	} else if (!light_file_is_readable(path)) {
		LIGHT_WARN("could not open controller brightness file for reading, so controller is not accessible");
		free(path);
		return false;
	}

	free(path);
	return true;
}

static bool light_ctrl_init(DIR **dir)
{
	if (!dir) {
		errno = EINVAL;
		return false;
	}

	if (ctx.target == LIGHT_KEYBOARD)
		*dir = opendir("/sys/class/leds");
	else
		*dir = opendir("/sys/class/backlight");

	if (!*dir)
		return false;

	return true;
}

static bool light_ctrl_iterate(DIR *dir, char *current)
{
	struct dirent *d;
	bool found = false;

	if (!dir || !current)
		return false;

	while (!found) {
		d = readdir(dir);
		if (!d)
			return false;

		if (d->d_name[0] != '.') {
			if (!light_check_ctrl(d->d_name)) {
				LIGHT_WARN("invalid controller '%s' found, continuing...", d->d_name);
				continue;
			}
			found = true;
		}
	}

	strncpy(current, d->d_name, NAME_MAX);
	current[NAME_MAX] = '\0';

	return true;
}

/* WARNING: `controller` HAS to be at most NAME_MAX, otherwise fails */
bool light_ctrl_probe(char *controller)
{
	DIR *dir;
	unsigned long best = 0;
	bool found = false;
	char best_name[NAME_MAX + 1];
	char current[NAME_MAX + 1];

	if (!controller) {
		LIGHT_ERR("Missing controller name");
		return false;
	}

	if (!light_ctrl_init(&dir)) {
		LIGHT_ERR("Failed listing controllers: %s", strerror(errno));
		return false;
	}

	while (light_ctrl_iterate(dir, current)) {
		unsigned long val = 0;

		LIGHT_NOTE("found '%s' controller", current);
		if (light_ctrl_exist(current)) {

			if (light_ctrl_get_brightness_max(current, &val)) {
				if (val > best) {
					found = true;
					best = val;
					strncpy(best_name, current, NAME_MAX);
					best_name[NAME_MAX] = '\0';
					ctx.has_cached_brightness_max = true;
					ctx.cached_brightness_max = val;
				} else {
					LIGHT_NOTE("ignoring controller as better one already found");
				}
			} else {
				LIGHT_WARN("could not read max brightness from file");
			}
		} else {
			LIGHT_WARN("controller not accessible");
		}
	}

	closedir(dir);

	if (!found) {
		LIGHT_ERR("could not find an accessible controller");
		return false;
	}

	if (best == 0) {
		LIGHT_ERR("found accessible controller but it's useless/corrupt");
		return false;
	}

	strncpy(controller, best_name, NAME_MAX);
	controller[NAME_MAX] = '\0';
	return true;
}

bool light_ctrl_get_cap_min(char const *controller, bool *has_cap, unsigned long *min_cap)
{
	char *path = NULL;

	if (!light_gen_path(controller, ctx.target, LIGHT_MIN_CAP, &path)) {
		LIGHT_ERR("could not generate path to minimum cap file");
		return false;
	}

	if (!light_file_is_readable(path)) {
		*has_cap = false;
		*min_cap = 0;
		free(path);
		LIGHT_NOTE("cap file doesn't exist or can't read from it, so assuming a minimum brightness of 0");
		return true;
	}

	if (!light_file_read_val(path, min_cap)) {
		LIGHT_ERR("could not read minimum cap from file");
		free(path);
		return false;
	}

	*has_cap = true;

	free(path);
	return true;
}

bool light_ctrl_set_cap_min(char const *controller, unsigned long val)
{
	char *path = NULL;

	if (!light_gen_path(controller, ctx.target, LIGHT_MIN_CAP, &path)) {
		LIGHT_ERR("could not generate path to minimum cap file");
		return false;
	}

	LIGHT_NOTE("setting minimum cap to %lu (raw)", val);
	if (!light_file_write_val(path, val)) {
		LIGHT_ERR("could not write to minimum cap file");
		free(path);
		return false;
	}

	free(path);
	return true;
}

bool light_ctrl_list(void)
{
	char controller[NAME_MAX + 1];
	bool found = false;
	DIR *dir;

	if (!light_ctrl_init(&dir)) {
		LIGHT_ERR("Failed listing controllers: %s", strerror(errno));
		return false;
	}

	while (light_ctrl_iterate(dir, controller)) {
		printf("%s\n", controller);
		found = true;
	}

	if (!found) {
		LIGHT_WARN("no controllers found, either check your system or your permissions");
		return false;
	}

	return true;
}

bool light_ctrl_save_brightness(char const *controller, unsigned long val)
{
	char *path = NULL;

	if (!light_gen_path(controller, ctx.target, LIGHT_SAVERESTORE, &path)) {
		LIGHT_ERR("could not generate path to save/restore file");
		return false;
	}

	LIGHT_NOTE("saving brightness %lu (raw) to save file %s\n", val, path);
	if (!light_file_write_val(path, val)) {
		LIGHT_ERR("could not write to save/restore file");
		free(path);
		return false;
	}

	free(path);
	return true;
}

bool light_ctrl_restore_brightness(char const *controller)
{
	char *path = NULL;
	unsigned long val = 0;

	if (!light_gen_path(controller, ctx.target, LIGHT_SAVERESTORE, &path)) {
		LIGHT_ERR("could not generate path to save/restore file");
		return false;
	}

	LIGHT_NOTE("restoring brightness from saved file %s", path);
	if (!light_file_read_val(path, &val)) {
		LIGHT_ERR("could not read saved value");
		free(path);
		return false;
	}

	if (!light_ctrl_set_brightness(controller, val)) {
		LIGHT_ERR("could not set restored brightness");
		free(path);
		return false;
	}

	free(path);
	return true;
}
