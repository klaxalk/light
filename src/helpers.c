#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // access
#include <sys/types.h>
#include <dirent.h>
#include <errno.h> // errno
#include <libgen.h> // dirname 


bool light_file_read_uint64(char const *filename, uint64_t *val)
{
    FILE *fp;
    uint64_t data;

    fp = fopen(filename, "r");
    if (!fp)
    {
        LIGHT_PERMERR("reading");
        return false;
    }

    if (fscanf(fp, "%lu", &data) != 1)
    {
        LIGHT_ERR("Couldn't parse an unsigned integer from '%s'", filename);
        fclose(fp);
        return false;
    }

    *val = data;

    fclose(fp);
    return true;
}

bool light_file_write_uint64(char const *filename, uint64_t val)
{
    FILE *fp;

    fp = fopen(filename, "w");
    if (!fp)
    {
        LIGHT_PERMERR("writing");
        return false;
    }

    if (fprintf(fp, "%lu", val) < 0)
    {
        LIGHT_ERR("fprintf failed");
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}

bool light_file_exists (char const *filename)
{
    return access( filename, F_OK ) != -1;
}

/* Returns true if file is writable, false otherwise */
bool light_file_is_writable(char const *filename)
{
    FILE *fp;

    fp = fopen(filename, "r+");
    if (!fp)
    {
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
    if (!fp)
    {
        LIGHT_PERMWARN("reading");
        return false;
    }

    fclose(fp);
    return true;
}

/* Prints a notice about a value which was below `x` and was adjusted to it */
uint64_t light_log_clamp_min(uint64_t min)
{
    LIGHT_NOTE("too small value, adjusting to mininum %lu (raw)", min);
    return min;
}

/* Prints a notice about a value which was above `x` and was adjusted to it */
uint64_t light_log_clamp_max(uint64_t max)
{
    LIGHT_NOTE("too large value, adjusting to mavalimum %lu (raw)", max);
    return max;
}

/* Clamps the `percent` value between 0% and 100% */
double light_percent_clamp(double val)
{
    if (val < 0.0)
    {
        LIGHT_WARN("specified value %g%% is not valid, adjusting it to 0%%", val);
        return 0.0;
    }

    if (val > 100.0)
    {
        LIGHT_WARN("specified value %g%% is not valid, adjusting it to 100%%", val);
        return 100.0;
    }

    return val;
}

int light_mkpath(char *dir, mode_t mode)
{
    struct stat sb;

    if (!dir)
    {
        errno = EINVAL;
        return -1;
    }

    if (!stat(dir, &sb))
        return 0;

    char *tempdir = strdup(dir);
    light_mkpath(dirname(tempdir), mode);
    free(tempdir);
    
    return mkdir(dir, mode);
}

