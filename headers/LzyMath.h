#pragma once
#include "LzyDefines.h"
#include <xmmintrin.h>


typedef struct LzyVec2f
{
	union
	{
		struct
		{
			f32 x, y;
		};

		struct
		{
			f32 r, g;
		};
	};
}LzyVec2f, LzyVec2;

typedef struct LzyVec3f
{
	union
	{
		struct
		{
			f32 x, y, z;
		};

		struct
		{
			f32 r, g, b;
		};
	};
}LzyVec3f, LzyVec3;

typedef struct LzyVec4f
{

	union
	{
		struct
		{
			f32 x, y, z, w;
		};

		struct
		{
			f32 r, g, b, a;
		};
	};
}LzyVec4f, LzyVec4;


typedef struct LzyMat4f
{
	union
	{
		f32 el[4][4];
		__m128 xmms[4];
	};
}LzyMat4f, LzyMat4;



LAPI u32 lzy_bit_scan_forward(i32 i);


LAPI LzyVec2f lzy_vec2f_add(LzyVec2f a, LzyVec2f b);
LAPI LzyVec2f lzy_vec2f_sub(LzyVec2f a, LzyVec2f b);
LAPI LzyVec2f lzy_vec2f_mul(LzyVec2f a, LzyVec2f b);
LAPI LzyVec2f lzy_vec2f_divide(LzyVec2f a, LzyVec2f b);
LAPI f32 lzy_vec2f_dot_product(LzyVec2f a, LzyVec2f b);
LAPI LzyVec2f lzy_vec2f_scalar_mul(LzyVec2f a, f32 b);
LAPI LzyVec2f lzy_vec2f_scalar_divide(LzyVec2f a, f32 b);
LAPI LzyVec2f lzy_vec2f_scalar_add(LzyVec2f a, f32 b);
LAPI LzyVec2f lzy_vec2f_scalar_sub(LzyVec2f a, f32 b);

LAPI LzyVec3f lzy_vec3f_add(LzyVec3f a, LzyVec3f b);
LAPI LzyVec3f lzy_vec3f_sub(LzyVec3f a, LzyVec3f b);
LAPI LzyVec3f lzy_vec3f_mul(LzyVec3f a, LzyVec3f b);
LAPI LzyVec3f lzy_vec3f_divide(LzyVec3f a, LzyVec3f b);
LAPI f32 lzy_vec3f_dot_product(LzyVec3f a, LzyVec3f b);
LAPI LzyVec3f lzy_vec3f_scalar_mul(LzyVec3f a, f32 b);
LAPI LzyVec3f lzy_vec3f_scalar_divide(LzyVec3f a, f32 b);
LAPI LzyVec3f lzy_vec3f_scalar_add(LzyVec3f a, f32 b);
LAPI LzyVec3f lzy_vec3f_scalar_sub(LzyVec3f a, f32 b);


LAPI LzyVec4f lzy_vec4f_add(LzyVec4f a, LzyVec4f b);
LAPI LzyVec4f lzy_vec4f_sub(LzyVec4f a, LzyVec4f b);
LAPI LzyVec4f lzy_vec4f_mul(LzyVec4f a, LzyVec4f b);
LAPI LzyVec4f lzy_vec4f_divide(LzyVec4f a, LzyVec4f b);
LAPI f32 lzy_vec4f_dot_product(LzyVec4f a, LzyVec4f b);
LAPI LzyVec4f lzy_vec4f_scalar_mul(LzyVec4f a, f32 b);
LAPI LzyVec4f lzy_vec4f_scalar_divide(LzyVec4f a, f32 b);
LAPI LzyVec4f lzy_vec4f_scalar_add(LzyVec4f a, f32 b);
LAPI LzyVec4f lzy_vec4f_scalar_sub(LzyVec4f a, f32 b);

LAPI LzyMat4f lzy_mat4f_identity();
LAPI LzyMat4f lzy_mat4f_mul(LzyMat4f a, LzyMat4f b);
LAPI LzyMat4f lzy_mat4f_add(LzyMat4f a, LzyMat4f b);
LAPI LzyMat4f lzy_mat4f_sub(LzyMat4f a, LzyMat4f b);
LAPI LzyMat4f lzy_mat4f_scalar_mul(LzyMat4f a, f32 b);
LAPI LzyMat4f lzy_mat4f_scalar_add(LzyMat4f a, f32 b);
LAPI LzyMat4f lzy_mat4f_scalar_sub(LzyMat4f a, f32 b);
LAPI LzyMat4f lzy_mat4f_scalar_divide(LzyMat4f a, f32 b);

LAPI LzyMat4f lzy_mat4f_rotate(LzyVec3f eulerAngles);
LAPI LzyMat4f lzy_mat4f_translate(LzyVec3f position);
LAPI LzyMat4f lzy_mat4f_scale(LzyVec3f scale);
LAPI LzyMat4f lzy_mat4f_perspective(f32 fFov, f32 fAspectRatio, f32 fNear, f32 fFar);

