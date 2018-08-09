#ifndef LIGHT_HELPERS_H
#define LIGHT_HELPERS_H

#include <stdbool.h>

/* Clamps x(value) between y(min) and z(max) in a nested ternary operation. 
 * if(x < y)
 * {
 *  y;
 * }else{
 *  if(x>z)
 *  {
 *    z;
 *  }else{
 *    x;
 *  }
 * }*/
#define LIGHT_CLAMP(x, y, z) ((x<y) ? (light_logInfClamp(y)) : ((x>z) ? (light_logSupClamp(z)) : x ))

#define LIGHT_LOG_FMT_BUF_SIZE 1024
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

light_loglevel_t light_verbosity;
char light_log_buffer[LIGHT_LOG_FMT_BUF_SIZE];

#define LIGHT_LOG(lvl,f,t,x)if(light_verbosity >= lvl){fprintf(f,t": \"%s\", in \"%s\" on line %u.\n", x, __FILE__, __LINE__);}

#define LIGHT_NOTE(x)LIGHT_LOG(LIGHT_NOTE_LEVEL,stdout,"notice",x)

#define LIGHT_WARN(x)LIGHT_LOG(LIGHT_WARN_LEVEL,stderr,"warning",x)

#define LIGHT_ERR(x)LIGHT_LOG(LIGHT_ERROR_LEVEL,stderr,"error",x)

#define LIGHT_LOG_FMT(x,s,f)if(snprintf(light_log_buffer, LIGHT_LOG_FMT_BUF_SIZE,x,s) > 0){f(light_log_buffer);}

#define LIGHT_NOTE_FMT(x,s)LIGHT_LOG_FMT(x,s,LIGHT_NOTE);

#define LIGHT_WARN_FMT(x,s)LIGHT_LOG_FMT(x,s,LIGHT_WARN);

#define LIGHT_ERR_FMT(x,s)LIGHT_LOG_FMT(x,s,LIGHT_ERR);

#define LIGHT_MEMERR() LIGHT_ERR("memory error");

#define LIGHT_PERMLOG(x,f)f##_FMT("could not open '%s' for "x,filename); f("check if this file exists or if you have the right permissions");

#define LIGHT_PERMERR(x) LIGHT_PERMLOG(x,LIGHT_ERR)

#define LIGHT_PERMWARN(x) LIGHT_PERMLOG(x,LIGHT_WARN)

/* Reads an unsigned integer from a file into `i` if able, otherwise returns false and leaves `i` untouched */
bool light_readUInt(char const *filename, unsigned int *v);

/* Writes an unsigned integer `i` into file `filename` if able, otherwise returns false */
bool light_writeUInt(char const *filename, unsigned int v);

bool light_writeULong(char const *filename, unsigned long v);
bool light_readULong(char const *filename, unsigned long *v);

/* Reads a file into null-terminated `buffer` if able, otherwise returns false
 * If `size` isn't NULL, it will be set to the read size.
 *
 * WARNING: `buffer` HAS to be freed by the user, also make sure it is NULL before passed */
bool light_readString(char const *filename, char *buffer, long *size);

/* Returns true if `path` is a valid directory, false otherwise */
bool light_isDir(char const *path);

/* Returns true if file is writable, false otherwise */
bool light_isWritable(char const *filename);

/* Returns true if file is readable, false otherwise */
bool light_isReadable(char const *filename);

/* Clamps the `percent` value between 0% and 100% */
double light_clampPercent(double percent);

/* Prints a notice about a value which was below `x` and was adjusted to it */
unsigned long light_logInfClamp(unsigned long x);

/* Prints a notice about a value which was above `x` and was adjusted to it */
unsigned long light_logSupClamp(unsigned long x);

#endif /* LIGHT_HELPERS_H */
