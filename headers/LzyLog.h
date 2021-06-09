#pragma once
#include "LzyDefines.h"

#ifdef _DEBUG
	#define LZY_LOG_TRACE_ENABLED
	#define LZY_LOG_INFO_ENABLED
	#define LZY_LOG_WARN_ENABLED
#endif

#define LZY_LOG_ERROR_ENABLED
#define LZY_LOG_FATAL_ENABLED

#ifdef _WIN32
#define OPTIONAL_VARIADIC ,
#elif __linux__
#define OPTIONAL_VARIADIC __VA_OPT__(,)
#endif

typedef enum LzyLogLevel
{
	LZY_LOG_LEVEL_TRACE,
	LZY_LOG_LEVEL_INFO,
	LZY_LOG_LEVEL_WARN,
	LZY_LOG_LEVEL_ERROR,
	LZY_LOG_LEVEL_FATAL
}LzyLogLevel;

LAPI void lzy_log(LzyLogLevel logLevel, const string  pFormat, const string pFile, i32 iLine, ...);

#ifdef LZY_LOG_TRACE_ENABLED
#ifdef _MSC_VER
#define LTRACE(format, ...) lzy_log(LZY_LOG_LEVEL_TRACE, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LTRACE(format, ...) lzy_log(LZY_LOG_LEVEL_TRACE, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LTRACE(format, ...)
#endif

#ifdef LZY_LOG_INFO_ENABLED
#ifdef _MSC_VER
#define LINFO(format, ...) lzy_log(LZY_LOG_LEVEL_INFO, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LINFO(format, ...) lzy_log(LZY_LOG_LEVEL_INFO, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LINFO(format, ...)
#endif

#ifdef LZY_LOG_WARN_ENABLED
#ifdef _MSC_VER
#define LWARN(format, ...) lzy_log(LZY_LOG_LEVEL_WARN, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LWARN(format, ...) lzy_log(LZY_LOG_LEVEL_WARN, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LWARN(format, ...)
#endif

#ifdef LZY_LOG_ERROR_ENABLED
#ifdef _MSC_VER
#define LERROR(format, ...) lzy_log(LZY_LOG_LEVEL_ERROR, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LERROR(format, ...) lzy_log(LZY_LOG_LEVEL_ERROR, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LERROR(format, ...)
#endif

#ifdef LZY_LOG_FATAL_ENABLED
#ifdef _MSC_VER
#define LFATAL(format, ...) lzy_log(LZY_LOG_LEVEL_FATAL, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LFATAL(format, ...) lzy_log(LZY_LOG_LEVEL_FATAL, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LFATAL(format, ...)
#endif

#ifdef LZY_ASSERT_ENABLED
#define LASSERT(cond, msg) if(cond) {} else LFATAL("(Assertion failed) %s", msg)
#else
#define LASSERT(cond, msg)
#endif