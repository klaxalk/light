#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

bool light_file_read_val(char const *filename, unsigned long *val)
{
	FILE *fp;
	unsigned long data;

	fp = fopen(filename, "r");
	if (!fp) {
		LIGHT_PERMERR("reading");
		return false;
	}

	if (fscanf(fp, "%lu", &data) != 1) {
		LIGHT_ERR("Couldn't parse a positive integer number from '%s'", filename);
		fclose(fp);
		return false;
	}

	*val = data;

	fclose(fp);
	return true;
}

bool light_file_write_val(char const *filename, unsigned long val)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if (!fp) {
		LIGHT_PERMERR("writing");
		return false;
	}

	if (fprintf(fp, "%lu", val) < 0) {
		LIGHT_ERR("fprintf failed");
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

/* Returns true if file is writable, false otherwise */
bool light_file_is_writable(char const *filename)
{
	FILE *fp;

	fp = fopen(filename, "r+");
	if (!fp) {
		LIGHT_PERMWARN("writing");
		return false;
	}

	fclose(fp);
	return true;
}

/* Returns true if file is readable, false otherwise */
bool light_file_is_readable(char const *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp) {
		LIGHT_PERMWARN("reading");
		return false;
	}

	fclose(fp);
	return true;
}

/* Prints a notice about a value which was below `x` and was adjusted to it */
unsigned long light_log_clamp_min(unsigned long min)
{
	LIGHT_NOTE("too small value, adjusting to mininum %lu (raw)", min);
	return min;
}

/* Prints a notice about a value which was above `x` and was adjusted to it */
unsigned long light_log_clamp_max(unsigned long max)
{
	LIGHT_NOTE("too large value, adjusting to mavalimum %lu (raw)", max);
	return max;
}

/* Clamps the `percent` value between 0% and 100% */
double light_percent_clamp(double val)
{
	if (val < 0.0) {
		LIGHT_WARN("specified value %g%% is not valid, adjusting it to 0%%", val);
		return 0.0;
	}

	if (val > 100.0) {
		LIGHT_WARN("specified value %g%% is not valid, adjusting it to 100%%", val);
		return 100.0;
	}

	return val;
}
