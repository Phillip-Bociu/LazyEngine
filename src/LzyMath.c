#include "LzyMath.h"
#include "LzyLog.h"
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>
#include <ammintrin.h>

u32 lzy_bit_scan_forward(i32 i)
{
	u32 retval;
#ifdef _MSC_VER
	_BitScanForward((DWORD*)&retval, i);
#else
	retval = __builtin_ffs(i);
#endif
	return retval;
}

u32 lzy_bit_scan_reverse(i32 i)
{
	u32 retval;
#ifdef _MSC_VER
	_BitScanReverse((DWORD*)&retval, i);
#else
	retval = __builtin_clz(i);
#endif
	return retval;
}


LzyVec2f lzy_vec2f_add(LzyVec2f a, LzyVec2f b)
{
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set_ps(0, 0, b.y, b.x);
	
	m1.xmm = _mm_add_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec2f lzy_vec2f_sub(LzyVec2f a, LzyVec2f b)
{
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set_ps(0, 0, b.y, b.x);
    
	m1.xmm = _mm_sub_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec2f lzy_vec2f_multiply(LzyVec2f a, LzyVec2f b)
{
    
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set_ps(0, 0, b.y, b.x);
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec2f lzy_vec2f_divide(LzyVec2f a, LzyVec2f b)
{
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1,m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set_ps(1, 1, b.y, b.x);
    
	m1.xmm = _mm_div_ps(m1.xmm, m2.xmm);
    
	return m1.retval;
}

f32 lzy_vec2f_dot_product(LzyVec2f a, LzyVec2f b)
{
    
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set_ps(0, 0, b.y, b.x);
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
    
	__m128 shuf = _mm_shuffle_ps(m1.xmm, m1.xmm, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 sums = _mm_add_ps(m1.xmm, shuf);
	shuf = _mm_movehl_ps(shuf, sums);
	sums = _mm_add_ss(sums, shuf);
    
	return  _mm_cvtss_f32(sums);
}

LzyVec2f lzy_vec2f_scalar_multiply(LzyVec2f a, f32 b)
{
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
	
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec2f lzy_vec2f_scalar_divide(LzyVec2f a, f32 b)
{
	LCOREASSERT(b != 0.0f, "Division by Zero");
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_div_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec2f lzy_vec2f_scalar_add(LzyVec2f a, f32 b)
{
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_add_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec2f lzy_vec2f_scalar_sub(LzyVec2f a, f32 b)
{
	union
	{
		LzyVec2f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, 0, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_sub_ps(m1.xmm, m2.xmm);
	return m1.retval;
}


LzyVec3f lzy_vec3f_add(LzyVec3f a, LzyVec3f b)
{
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(0, b.z, b.y, b.x);
    
	m1.xmm = _mm_add_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec3f lzy_vec3f_sub(LzyVec3f a, LzyVec3f b)
{
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(0, b.z, b.y, b.x);
    
	m1.xmm = _mm_sub_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec3f lzy_vec3f_multiply(LzyVec3f a, LzyVec3f b)
{
    
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(0, b.z, b.y, b.x);
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec3f lzy_vec3f_divide(LzyVec3f a, LzyVec3f b)
{
	LCOREASSERT(b.x != 0 && b.y != 0 && b.z != 0, "Division by zero");
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(1, b.z, b.y, b.x);
    
	m1.xmm = _mm_div_ps(m1.xmm, m2.xmm);
    
	return m1.retval;
}

f32		 lzy_vec3f_dot_product(LzyVec3f a, LzyVec3f b)
{
    
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(0, b.z, b.y, b.x);
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
    
	__m128 shuf = _mm_shuffle_ps(m1.xmm, m1.xmm, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 sums = _mm_add_ps(m1.xmm, shuf);
	shuf = _mm_movehl_ps(shuf, sums);
	sums = _mm_add_ss(sums, shuf);
    
	return  _mm_cvtss_f32(sums);
}

LzyVec3f lzy_vec3f_scalar_multiply(LzyVec3f a, f32 b)
{
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec3f lzy_vec3f_scalar_divide(LzyVec3f a, f32 b)
{
	LCOREASSERT(b != 0.0f, "Division by zero");
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_div_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec3f lzy_vec3f_scalar_add(LzyVec3f a, f32 b)
{
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_add_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec3f lzy_vec3f_scalar_sub(LzyVec3f a, f32 b)
{
	union
	{
		LzyVec3f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(0, a.z, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_sub_ps(m1.xmm, m2.xmm);
	return m1.retval;
}


LzyVec4f lzy_vec4f_add(LzyVec4f a, LzyVec4f b)
{
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(a.w, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(b.w, b.z, b.y, b.x);
    
	m1.xmm = _mm_add_ps(m1.xmm, m2.xmm);
    
	return m1.retval;
}

LzyVec4f lzy_vec4f_sub(LzyVec4f a, LzyVec4f b)
{
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(a.w, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(b.w, b.z, b.y, b.x);
    
	m1.xmm = _mm_sub_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec4f lzy_vec4f_multiply(LzyVec4f a, LzyVec4f b)
{
    
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(a.w, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(b.w, b.z, b.y, b.x);
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
    
	return m1.retval;
}

LzyVec4f lzy_vec4f_divide(LzyVec4f a, LzyVec4f b)
{
	LCOREASSERT(b.x != 0 && b.y != 0 && b.z != 0 && b.w != 0, "Division by zero");
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(a.w, a.z, a.y, a.x);
	m2.xmm = _mm_set_ps(b.w, b.z, b.y, b.x);
    
	m1.xmm = _mm_div_ps(m1.xmm, m2.xmm);
    
	return m1.retval;
}

f32		 lzy_vec4f_dot_product(LzyVec4f a, LzyVec4f b)
{
    
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.retval = a;
	m2.retval = b;
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
    
	__m128 shuf = _mm_shuffle_ps(m1.xmm, m1.xmm, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 sums = _mm_add_ps(m1.xmm, shuf);
	shuf = _mm_movehl_ps(shuf, sums);
	sums = _mm_add_ss(sums, shuf);
	return  _mm_cvtss_f32(sums);
}

LzyVec4f lzy_vec4f_scalar_multiply(LzyVec4f a, f32 b)
{
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(a.w, a.z, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_mul_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec4f lzy_vec4f_scalar_divide(LzyVec4f a, f32 b)
{
	LCOREASSERT(b != 0.0f, "Division by zero");
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(a.w, a.z, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_div_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec4f lzy_vec4f_scalar_add(LzyVec4f a, f32 b)
{
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.xmm = _mm_set_ps(a.w, a.z, a.y, a.x);
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_add_ps(m1.xmm, m2.xmm);
	return m1.retval;
}

LzyVec4f lzy_vec4f_scalar_sub(LzyVec4f a, f32 b)
{
	union
	{
		LzyVec4f retval;
		__m128 xmm;
	}m1, m2;
    
	m1.retval = a;
	m2.xmm = _mm_set1_ps(b);
    
	m1.xmm = _mm_sub_ps(m1.xmm, m2.xmm);
	return m1.retval;
}


LzyMat4f lzy_mat4f_multiply(LzyMat4f a, LzyMat4f b)
{
    
	LzyMat4f retval;
    
	for (int i = 0; i < 4; i++) 
    {
		__m128 brod1 = _mm_set1_ps(a.el[i][0]);
		__m128 brod2 = _mm_set1_ps(a.el[i][1]);
		__m128 brod3 = _mm_set1_ps(a.el[i][2]);
		__m128 brod4 = _mm_set1_ps(a.el[i][3]);
		__m128 row   = _mm_fmadd_ps(brod1, b.xmms[0], _mm_fmadd_ps(brod2, b.xmms[1], _mm_fmadd_ps(brod3, b.xmms[2], _mm_mul_ps(brod4, b.xmms[3]))));
		_mm_store_ps(&retval.el[i][0], row);
	}
    
	return retval;
}

LzyMat4f lzy_mat4f_add(LzyMat4f a, LzyMat4f b)
{
	a.xmms[0] = _mm_add_ps(a.xmms[0], b.xmms[0]);
	a.xmms[1] = _mm_add_ps(a.xmms[1], b.xmms[1]);
	a.xmms[2] = _mm_add_ps(a.xmms[2], b.xmms[2]);
	a.xmms[3] = _mm_add_ps(a.xmms[3], b.xmms[3]);
    
	return a;
}

LzyMat4f lzy_mat4f_sub(LzyMat4f a, LzyMat4f b)
{
	a.xmms[0] = _mm_sub_ps(a.xmms[0], b.xmms[0]);
	a.xmms[1] = _mm_sub_ps(a.xmms[1], b.xmms[1]);
	a.xmms[2] = _mm_sub_ps(a.xmms[2], b.xmms[2]);
	a.xmms[3] = _mm_sub_ps(a.xmms[3], b.xmms[3]);
    
	return a;
}

LzyMat4f lzy_mat4f_scalar_multiply(LzyMat4f a, f32 b)
{
	__m128 c = _mm_set1_ps(b);
	a.xmms[0] = _mm_mul_ps(a.xmms[0], c);
	a.xmms[1] = _mm_mul_ps(a.xmms[0], c);
	a.xmms[2] = _mm_mul_ps(a.xmms[0], c);
	a.xmms[3] = _mm_mul_ps(a.xmms[0], c);
	return a;
}

LzyMat4f lzy_mat4f_scalar_add(LzyMat4f a, f32 b)
{
	__m128 c = _mm_set1_ps(b);
	a.xmms[0] = _mm_add_ps(a.xmms[0], c);
	a.xmms[1] = _mm_add_ps(a.xmms[0], c);
	a.xmms[2] = _mm_add_ps(a.xmms[0], c);
	a.xmms[3] = _mm_add_ps(a.xmms[0], c);
	return a;
}

LzyMat4f lzy_mat4f_scalar_sub(LzyMat4f a, f32 b)
{
	__m128 c = _mm_set1_ps(b);
	a.xmms[0] = _mm_sub_ps(a.xmms[0], c);
	a.xmms[1] = _mm_sub_ps(a.xmms[0], c);
	a.xmms[2] = _mm_sub_ps(a.xmms[0], c);
	a.xmms[3] = _mm_sub_ps(a.xmms[0], c);
	return a;
}

LzyMat4f lzy_mat4f_scalar_divide(LzyMat4f a, f32 b)
{
	LCOREASSERT(b != 0, "Division by zero");
    
	__m128 c = _mm_set1_ps(b);
	a.xmms[0] = _mm_div_ps(a.xmms[0], c);
	a.xmms[1] = _mm_div_ps(a.xmms[0], c);
	a.xmms[2] = _mm_div_ps(a.xmms[0], c);
	a.xmms[3] = _mm_div_ps(a.xmms[0], c);
    
	return a;
}


