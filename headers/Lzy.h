#pragma once
#include <stdlib.h>
#include <stdio.h>

#ifndef DEBUG
#ifdef LEXPORT
	#ifdef _MSC_VER
	#define LAPI __declspec(dllexport)
#else
	#define LAPI __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define LAPI __declspec(dllimport)
#else
#define LAPI
#endif
#endif
#else
#define LAPI
#endif

#define MAKE_HANDLE(type) typedef struct type##_t* type

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifdef __GNUC__
#define STATIC_ASSERT(exp, msg) _Static_assert(exp, msg) 
#else
#define STATIC_ASSERT(exp, msg) static_assert(exp, msg)
#endif


#define true 1
#define false 0

typedef char b8;
typedef int b32;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef char* string;
typedef unsigned char byte;

typedef char c8;

STATIC_ASSERT(sizeof(i8) == 1, "Size of types is different than expected");
STATIC_ASSERT(sizeof(i16) == 2, "Size of types is different than expected");
STATIC_ASSERT(sizeof(i32) == 4, "Size of types is different than expected");
STATIC_ASSERT(sizeof(i64) == 8, "Size of types is different than expected");

STATIC_ASSERT(sizeof(u8) == 1, "Size of types is different than expected");
STATIC_ASSERT(sizeof(u16) == 2, "Size of types is different than expected");
STATIC_ASSERT(sizeof(u32) == 4, "Size of types is different than expected");
STATIC_ASSERT(sizeof(u64) == 8, "Size of types is different than expected");

STATIC_ASSERT(sizeof(f32) == 4, "Size of types is different than expected");
STATIC_ASSERT(sizeof(f64) == 8, "Size of types is different than expected");

STATIC_ASSERT(sizeof(b8) == 1, "Size of types is different than expected");
STATIC_ASSERT(sizeof(b32) == 4, "Size of types is different than expected");

#define global static
#define internal_func static
#define local_persistent static

void lzy_sleep(u64 uMs);
f64 lzy_get_time();

MAKE_HANDLE(LzyWindow);

b8 lzy_window_create(LzyWindow* pWindow, const char* pWindowTitle,u16 uResX, u16 uResY);
b8 lzy_window_should_close(LzyWindow window);
b8 lzy_window_poll_events(LzyWindow window);


typedef struct LzyApplicationConfig
{
	char* pApplicationName;
	u16 uResX;
	u16 uResY;
}LzyApplicationConfig;

typedef struct LzyApplication
{
	f64 fLastTime;
	LzyWindow pWindow;
	u16 uResX;
	u16 uResY;
	b8 bIsRunning;
	b8 bIsSuspended;
}LzyApplication;

LAPI b8 lzy_application_create(LzyApplicationConfig* pAppConfig);
LAPI b8 lzy_application_run();


typedef enum LzyLogLevel
{
	LZY_LOG_LEVEL_TRACE,
	LZY_LOG_LEVEL_INFO,
	LZY_LOG_LEVEL_WARN,
	LZY_LOG_LEVEL_ERROR,
	LZY_LOG_LEVEL_FATAL
}LzyLogLevel;

void lzy_log(LzyLogLevel logLevel, const string  pFormat, const string pFile, i32 iLine, ...);

#ifdef LZY_LOG_TRACE_ENABLED
#define LTRACE(msg, ...) lzy_log(LZY_LOG_LEVEL_TRACE, msg, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LTRACE(msg, ...)
#endif

#ifdef LZY_LOG_INFO_ENABLED
#define LINFO(msg, ...) lzy_log(LZY_LOG_LEVEL_INFO, msg, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LINFO(msg, ...)
#endif

#ifdef LZY_LOG_WARN_ENABLED
#define LWARN(msg, ...) lzy_log(LZY_LOG_LEVEL_WARN, msg, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LWARN(msg, ...)
#endif

#ifdef LZY_LOG_ERROR_ENABLED
#define LERROR(msg, ...) lzy_log(LZY_LOG_LEVEL_ERROR, msg, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LERROR(msg, ...)
#endif

#ifdef LZY_LOG_FATAL_ENABLED
#define LFATAL(msg, ...) lzy_log(LZY_LOG_LEVEL_FATAL, msg, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LFATAL(msg, ...)
#endif

#ifdef LZY_ASSERT_ENABLED
#define LASSERT(cond, msg) if(cond) {} else LFATAL("(Assertion failed) %s", msg)
#else
#define LASSERT(cond, msg)
#endif
