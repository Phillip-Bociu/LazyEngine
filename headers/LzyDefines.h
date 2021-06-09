#pragma once
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