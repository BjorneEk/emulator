/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * error message printing
 *
 *==========================================================*/

#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdarg.h>

#define ASSERT_(_expr, ...) if(!(_expr)) { exit_error_custom("Assertion Error", __VA_ARGS__); }
#define ASSERT(_expr) if(!(_expr)) { exit_error_custom("Assert", "Assertion failed in function: %s, in file: %s, line: %i", __FUNCTION__, __FILE__, __LINE__);}
#define ERR() exit_error_custom("Error", "fatal error occured in function: %s, in file: %s, line: %i", __FUNCTION__, __FILE__, __LINE__)

void verror(const char * fmt, va_list args);

void verror_custom(const char *error, const char * fmt, va_list args);

void error(const char * fmt, ...);

void error_custom(const char *error, const char * fmt, ...);

__attribute__((noreturn))
void exit_error(const char * fmt, ...);

__attribute__((noreturn))
void exit_error_custom(const char *error, const char * fmt, ...);

#endif /* _ERROR_H_ */