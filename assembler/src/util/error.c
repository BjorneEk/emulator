/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * error message printing
 *
 *==========================================================*/
#include "error.h"
#include <stdio.h>
#include <stdlib.h>


#define ERROR_LABEL(_err_type) "[\033[31;1;4m" _err_type "\033[0m]"

#define WARNING_LABEL(_err_type) "[\033[33;1;4m" _err_type "\033[0m]"

void verror(const char * fmt, va_list args)
{
	fprintf(stderr, ERROR_LABEL("Error") ": ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

void verror_custom(const char *error, const char * fmt, va_list args)
{
	fprintf(stderr, "[\033[31;1;4m%s\033[0m]: ", error);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}
__attribute__((format(printf, 1, 2)))
void error(const char * fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, ERROR_LABEL("Error") ": ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}
__attribute__((format(printf, 2, 3)))
void error_custom(const char *error, const char * fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	verror_custom(error, fmt, args);
}

void vwarn(const char * fmt, va_list args)
{
	printf(WARNING_LABEL("Warning") ": ");
	vprintf(fmt, args);
	printf("\n");
}


void vwarn_custom(const char *warning, const char * fmt, va_list args)
{
	printf("[\033[33;1;4m%s\033[0m]: ", warning);
	vprintf(fmt, args);
	printf("\n");
}
__attribute__((format(printf, 1, 2)))
void warn(const char * fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	printf(WARNING_LABEL("Warning") ": ");
	vprintf(fmt, args);
	printf("\n");
}

__attribute__((format(printf, 2, 3)))
void warn_custom(const char *warning, const char * fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vwarn_custom(warning, fmt, args);
}

__attribute__((noreturn)) __attribute__((format(printf, 1, 2)))
void exit_error(const char * fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	verror(fmt, args);
	exit(-1);
}
__attribute__((noreturn)) __attribute__((format(printf, 2, 3)))
void exit_error_custom(const char *error, const char * fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	verror_custom(error, fmt, args);
	exit(-1);
}