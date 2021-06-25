#pragma once
#include "LzyDefines.h"

#if _DEBUG
	#define LZY_LOG_TRACE_ENABLED
	#define LZY_LOG_INFO_ENABLED
	#define LZY_LOG_WARN_ENABLED
#endif

#define LZY_LOG_ERROR_ENABLED
#define LZY_LOG_FATAL_ENABLED



typedef enum LzyLogLevel
{
	LZY_LOG_LEVEL_TRACE,
	LZY_LOG_LEVEL_INFO,
	LZY_LOG_LEVEL_WARN,
	LZY_LOG_LEVEL_ERROR,
	LZY_LOG_LEVEL_FATAL
}LzyLogLevel;

LAPI void lzy_log_core(LzyLogLevel logLevel, const string  pFormat, const string pFile, i32 iLine, ...);
LAPI void lzy_log_user(LzyLogLevel logLevel, const string  pFormat, const string pFile, i32 iLine, ...);

#ifdef LZY_LOG_TRACE_ENABLED
#ifdef _MSC_VER
#define LCORETRACE(format, ...) lzy_log_core(LZY_LOG_LEVEL_TRACE, format, __FILE__, __LINE__, __VA_ARGS__)
#define LTRACE(format, ...) lzy_log_user(LZY_LOG_LEVEL_TRACE, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LCORETRACE(format, ...) lzy_log_core(LZY_LOG_LEVEL_TRACE, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LTRACE(format, ...) lzy_log_user(LZY_LOG_LEVEL_TRACE, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LCORETRACE(format, ...)
#define LTRACE(format, ...)
#endif

#ifdef LZY_LOG_INFO_ENABLED
#ifdef _MSC_VER
#define LCOREINFO(format, ...) lzy_log_core(LZY_LOG_LEVEL_INFO, format, __FILE__, __LINE__, __VA_ARGS__)
#define LINFO(format, ...) lzy_log_user(LZY_LOG_LEVEL_INFO, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LCOREINFO(format, ...) lzy_log_core(LZY_LOG_LEVEL_INFO, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LINFO(format, ...) lzy_log_core(LZY_LOG_LEVEL_INFO, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LCOREINFO(format, ...)
#define LINFO(format, ...)
#endif

#ifdef LZY_LOG_WARN_ENABLED
#ifdef _MSC_VER
#define LCOREWARN(format, ...) lzy_log_core(LZY_LOG_LEVEL_WARN, format, __FILE__, __LINE__, __VA_ARGS__)
#define LWARN(format, ...) lzy_log_user(LZY_LOG_LEVEL_WARN, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LCOREWARN(format, ...) lzy_log_core(LZY_LOG_LEVEL_WARN, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LWARN(format, ...) lzy_log_user(LZY_LOG_LEVEL_WARN, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LCOREWARN(format, ...)
#define LWARN(format, ...)
#endif

#ifdef LZY_LOG_ERROR_ENABLED
#ifdef _MSC_VER
#define LCOREERROR(format, ...) lzy_log_core(LZY_LOG_LEVEL_ERROR, format, __FILE__, __LINE__, __VA_ARGS__)
#define LERROR(format, ...) lzy_log_user(LZY_LOG_LEVEL_ERROR, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LCOREERROR(format, ...) lzy_log_core(LZY_LOG_LEVEL_ERROR, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LERROR(format, ...) lzy_log_user(LZY_LOG_LEVEL_ERROR, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LCOREERROR(format, ...)
#define LERROR(format, ...)
#endif

#ifdef LZY_LOG_FATAL_ENABLED
#ifdef _MSC_VER
#define LCOREFATAL(format, ...) lzy_log_core(LZY_LOG_LEVEL_FATAL, format, __FILE__, __LINE__, __VA_ARGS__)
#define LFATAL(format, ...) lzy_log_user(LZY_LOG_LEVEL_FATAL, format, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LCOREFATAL(format, ...) lzy_log_core(LZY_LOG_LEVEL_FATAL, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LFATAL(format, ...) lzy_log_user(LZY_LOG_LEVEL_FATAL, format, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif
#else
#define LCOREFATAL(format, ...)
#define LFATAL(format, ...)
#endif

#ifdef LZY_ASSERT_ENABLED
#define LCOREASSERT(cond, msg) if(cond) {} else LCOREFATAL("(Assertion failed) %s", msg)
#define LASSERT(cond, msg) if(cond) {} else LFATAL("(Assertion failed) %s", msg)
#else
#define LCOREASSERT(cond, msg)
#define LASSERT(cond, msg)
#endif
