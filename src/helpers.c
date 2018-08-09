#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

bool light_readUInt(char const *filename, unsigned int *i)
{
	FILE *fileHandle;
	unsigned int iCopy;

	fileHandle = fopen(filename, "r");

	if (!fileHandle) {
		LIGHT_PERMERR("reading");
		return false;
	}

	if (fscanf(fileHandle, "%u", &iCopy) != 1) {
		LIGHT_ERR_FMT("Couldn't parse a positive integer number from '%s'", filename);
		fclose(fileHandle);
		return false;
	}

	*i = iCopy;

	fclose(fileHandle);
	return true;
}

bool light_writeUInt(char const *filename, unsigned int i)
{
	FILE *fileHandle;

	fileHandle = fopen(filename, "w");

	if (!fileHandle) {
		LIGHT_PERMERR("writing");
		return false;
	}

	if (fprintf(fileHandle, "%u", i) < 0) {
		LIGHT_ERR("fprintf failed");
		fclose(fileHandle);
		return false;
	}

	fclose(fileHandle);
	return true;
}

bool light_readULong(char const *filename, unsigned long *i)
{
	FILE *fileHandle;
	unsigned long iCopy;;

	fileHandle = fopen(filename, "r");

	if (!fileHandle) {
		LIGHT_PERMERR("reading");
		return false;
	}

	if (fscanf(fileHandle, "%lu", &iCopy) != 1) {
		LIGHT_ERR_FMT("Couldn't parse a positive integer number from '%s'", filename);
		fclose(fileHandle);
		return false;
	}

	*i = iCopy;

	fclose(fileHandle);
	return true;
}

bool light_writeULong(char const *filename, unsigned long i)
{
	FILE *fileHandle;

	fileHandle = fopen(filename, "w");

	if (!fileHandle) {
		LIGHT_PERMERR("writing");
		return false;
	}

	if (fprintf(fileHandle, "%lu", i) < 0) {
		LIGHT_ERR("fprintf failed");
		fclose(fileHandle);
		return false;
	}

	fclose(fileHandle);
	return true;
}

bool light_readString(char const *filename, char *buffer, long *size)
{
	FILE *fileHandle;
	long fileSize;
	long readSize;

	/* Make sure buffer pointer is null */
	if (buffer != NULL) {
		LIGHT_ERR("buffer passed to function isn't NULL");
		return false;
	}

	/* Open file */
	fileHandle = fopen(filename, "r");

	if (!fileHandle) {
		LIGHT_PERMERR("reading");
		return false;
	}

	/* Get the file size */
	fseek(fileHandle, 0L, SEEK_END);
	fileSize = ftell(fileHandle);
	rewind(fileHandle);

	/* Allocate the string and null-terminate it */
	buffer = malloc(sizeof(char) * (fileSize + 1));
	memset(buffer, '\0', fileSize);

	if (buffer == NULL) {
		LIGHT_MEMERR();
		fclose(fileHandle);
		return false;
	}

	/* Read the file */
	readSize = fread(buffer, sizeof(char), fileSize, fileHandle);

	if (readSize != fileSize) {
		LIGHT_ERR("read error");
		free(buffer);
		fclose(fileHandle);
		return false;
	}

	/* Set the size */
	if (size != NULL) {
		*size = readSize;
	}

	/* All well, close handle and return true */
	fclose(fileHandle);
	return true;
}

bool light_isDir(char const *path)
{
	DIR *dirHandle = opendir(path);

	if (!dirHandle) {
		return false;
	}

	closedir(dirHandle);
	return true;
}

bool light_isWritable(char const *filename)
{
	FILE *fileHandle = fopen(filename, "r+");

	if (!fileHandle) {
		LIGHT_PERMWARN("writing");
		return false;
	}

	fclose(fileHandle);
	return true;
}

bool light_isReadable(char const *filename)
{
	FILE *fileHandle = fopen(filename, "r");

	if (!fileHandle) {
		LIGHT_PERMWARN("reading");
		return false;
	}

	fclose(fileHandle);
	return true;
}

unsigned long light_logInfClamp(unsigned long x)
{
	LIGHT_NOTE_FMT("specified value is inferior to %lu (raw), so adjusting it to this mininum value", x);
	return x;
}

unsigned long light_logSupClamp(unsigned long x)
{
	LIGHT_NOTE_FMT("specified value is superior to %lu (raw), so adjusting it to this maximum value", x);
	return x;
}

double light_clampPercent(double p)
{
	if (p < 0.0) {
		LIGHT_WARN_FMT("specified value %g%% is not valid, adjusting it to 0%%", p);
		return 0.0;
	} else if (p > 100.0) {
		LIGHT_WARN_FMT("specified value %g%% is not valid, adjusting it to 100%%", p);
		return 100.0;
	}
	return p;
}
