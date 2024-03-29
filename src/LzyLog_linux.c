#ifdef __linux__
#include "Lzy.h"
#include <stdio.h>
#include <stdarg.h>

void lzy_log_core(LzyLogLevel logLevel, const string pFormat, const string pFile, i32 iLine, ...)
{

	c8 buffer[32000];
	const string prefixes[5] = {
		"[TRACE]", 
		"[INFO]",
		"[WARN]",
		"[ERROR]",
		"[FATAL]",
	};
	
	const string prefixColors[5] = {
		"\033[0;37m",
		"\033[032m",
		"\033[0;33m",
		"\033[0;31m",	
		"\033[0;35m"	
	};

	const string resetColor = "\033[0m";

	va_list args;

	va_start(args, iLine);
	vsprintf(buffer, pFormat, args);
	va_end(args);

	printf(prefixColors[logLevel]);
    printf("[LZY_CORE]%s[%s, line %d]: %s\n", prefixes[logLevel], pFile, iLine, buffer);
	printf(resetColor);
}

void lzy_log_user(LzyLogLevel logLevel, const string  pFormat, const string pFile, i32 iLine, ...)
{

	c8 buffer[32000];
	const string prefixes[5] = {
		"[TRACE]", 
		"[INFO]",
		"[WARN]",
		"[ERROR]",
		"[FATAL]",
	};
	
	const string prefixColors[5] = {
		"\033[0;37m",
		"\033[032m",
		"\033[0;33m",
		"\033[0;31m",	
		"\033[0;35m"	
	};

	const string resetColor = "\033[0m";

	va_list args;

	va_start(args, iLine);
	vsprintf(buffer, pFormat, args);
	va_end(args);

	printf(prefixColors[logLevel]);
	printf("%s[%s, line %d]: %s\n", prefixes[logLevel], pFile, iLine, buffer);
	printf(resetColor);
	
}

#endif