#ifndef LIGHT_HELPERS_H
#define LIGHT_HELPERS_H

#include <stdbool.h>

/* Clamps x(value) between y(min) and z(max) in a nested ternary operation */
#define LIGHT_CLAMP(val, min, max)					\
	(val < min							\
	 ? light_log_clamp_min(min)					\
	 : (val > max							\
	    ? light_log_clamp_max(max)					\
	    : val ))

/* Verbosity levels: 
 * 0 - No output
 * 1 - Errors
 * 2 - Errors, warnings 
 * 3 - Errors, warnings, notices
 */
typedef enum {
	LIGHT_ERROR_LEVEL = 1,
	LIGHT_WARN_LEVEL,
	LIGHT_NOTE_LEVEL
} light_loglevel_t;

light_loglevel_t light_loglevel;

#define LIGHT_LOG(lvl, fp, fmt, args...)				\
	if (light_loglevel >= lvl)					\
		fprintf(fp, "%s:%d:" fmt, __FILE__, __LINE__, ##args)

#define LIGHT_NOTE(fmt, args...) LIGHT_LOG(LIGHT_NOTE_LEVEL,  stdout, "NOTE:" fmt, ##args)
#define LIGHT_WARN(fmt, args...) LIGHT_LOG(LIGHT_WARN_LEVEL,  stderr, "WARN:" fmt, ##args)
#define LIGHT_ERR(fmt, args...)  LIGHT_LOG(LIGHT_ERROR_LEVEL, stderr, "!ERR:" fmt, ##args)
#define LIGHT_MEMERR()           LIGHT_ERR("memory error");
#define LIGHT_PERMLOG(act, log)						\
	do {								\
		log("could not open '%s' for " act, filename);		\
		log("Verify it exists with the right permissions");	\
	} while (0)
#define LIGHT_PERMERR(x)         LIGHT_PERMLOG(x, LIGHT_ERR)
#define LIGHT_PERMWARN(x)        LIGHT_PERMLOG(x, LIGHT_WARN)

bool light_file_write_val   (char const *filename, unsigned long val);
bool light_file_read_val    (char const *filename, unsigned long *val);

bool light_file_is_writable (char const *filename);
bool light_file_is_readable (char const *filename);

unsigned long light_log_clamp_min(unsigned long min);
unsigned long light_log_clamp_max(unsigned long max);

double light_percent_clamp(double percent);

#endif /* LIGHT_HELPERS_H */
