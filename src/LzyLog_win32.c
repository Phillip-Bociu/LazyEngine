#ifdef _WIN32
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
	
	va_list args;
    
	va_start(args, iLine);
	vsprintf(buffer, pFormat, args);
	va_end(args);
    
    printf("[LZY_CORE]%s[%s, line %d]: %s\n", prefixes[logLevel], pFile, iLine, buffer);
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
	
	va_list args;
    
	va_start(args, iLine);
	vsprintf(buffer, pFormat, args);
	va_end(args);
    
    printf("%s[%s, line %d]: %s\n", prefixes[logLevel], pFile, iLine, buffer);
	
}
#endif