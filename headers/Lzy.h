#pragma once
#include <stdlib.h>
#include <stdio.h>

#define MAKE_HANDLE(type) typedef struct type##_t* type

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
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

static_assert(sizeof(i8) == 1, "Size of types is different than expected");
static_assert(sizeof(i16) == 2, "Size of types is different than expected");
static_assert(sizeof(i32) == 4, "Size of types is different than expected");
static_assert(sizeof(i64) == 8, "Size of types is different than expected");

static_assert(sizeof(u8) == 1, "Size of types is different than expected");
static_assert(sizeof(u16) == 2, "Size of types is different than expected");
static_assert(sizeof(u32) == 4, "Size of types is different than expected");
static_assert(sizeof(u64) == 8, "Size of types is different than expected");

static_assert(sizeof(f32) == 4, "Size of types is different than expected");
static_assert(sizeof(f64) == 8, "Size of types is different than expected");

static_assert(sizeof(b8) == 1, "Size of types is different than expected");
static_assert(sizeof(b32) == 4, "Size of types is different than expected");

#define global static
#define internal_func static
#define local_persistent static


MAKE_HANDLE(LzyWindow);

b8 lzy_create_window(LzyWindow* pWindow, u32 uResX, u32 uResY);

