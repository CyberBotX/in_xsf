/*
	Copyright (C) 2006-2007 shash
	Copyright (C) 2007-2012 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include <cstring>

#include "types.h"
#include "mem.h"

/*#ifdef __SSE__
#include <xmmintrin.h>
#endif*/

#ifdef __SSE2__
#include <emmintrin.h>
#endif

/*struct MatrixStack
{
	MatrixStack(int size, int type);
	int32_t *matrix;
	int32_t position;
	int32_t size;
	uint8_t type;
};*/

//void MatrixInit(float *matrix);
//void MatrixInit(int32_t *matrix);

//In order to conditionally use these asm optimized functions in visual studio
//without having to make new build types to exclude the assembly files.
//a bit sloppy, but there aint much to it

//float	MatrixGetMultipliedIndex	(int index, float *matrix, float *rightMatrix);
//int32_t	MatrixGetMultipliedIndex	(int index, int32_t *matrix, int32_t *rightMatrix);
//void	MatrixSet				(float *matrix, int x, int y, float value);
//void	MatrixCopy				(float * matrixDST, const float * matrixSRC);
//void	MatrixCopy				(int32_t * matrixDST, const int32_t * matrixSRC);
//int		MatrixCompare				(const float * matrixDST, const float * matrixSRC);
//void	MatrixIdentity			(float *matrix);
//void	MatrixIdentity			(int32_t *matrix);

//void	MatrixTranspose				(float *matrix);
//void	MatrixStackInit				(MatrixStack *stack);
//void	MatrixStackSetMaxSize		(MatrixStack *stack, int size);
//void	MatrixStackPushMatrix		(MatrixStack *stack, const int32_t *ptr);
//void	MatrixStackPopMatrix		(int32_t *mtxCurr, MatrixStack *stack, int size);
//int32_t*	MatrixStackGetPos			(MatrixStack *stack, int pos);
//int32_t*	MatrixStackGet				(MatrixStack *stack);
//void	MatrixStackLoadMatrix		(MatrixStack *stack, int pos, const int32_t *ptr);

//void Vector2Copy(float *dst, const float *src);
//void Vector2Add(float *dst, const float *src);
//void Vector2Subtract(float *dst, const float *src);
//float Vector2Dot(const float *a, const float *b);
//float Vector2Cross(const float *a, const float *b);

//float Vector3Dot(const float *a, const float *b);
//void Vector3Cross(float* dst, const float *a, const float *b);
//float Vector3Length(const float *a);
//void Vector3Add(float *dst, const float *src);
//void Vector3Subtract(float *dst, const float *src);
//void Vector3Scale(float *dst, const float scale);
//void Vector3Copy(float *dst, const float *src);
//void Vector3Normalize(float *dst);

//void Vector4Copy(float *dst, const float *src);

//these functions are an unreliable, inaccurate floor.
//it should only be used for positive numbers
//this isnt as fast as it could be if we used a visual c++ intrinsic, but those appear not to be universally available
inline uint32_t u32floor(float f)
{
#ifdef __SSE2__
	return (uint32_t)_mm_cvtt_ss2si(_mm_set_ss(f));
#else
	return (uint32_t)f;
#endif
}
inline uint32_t u32floor(double d)
{
#ifdef __SSE2__
	return (uint32_t)_mm_cvttsd_si32(_mm_set_sd(d));
#else
	return (uint32_t)d;
#endif
}

//same as above but works for negative values too.
//be sure that the results are the same thing as floorf!
inline int32_t s32floor(float f)
{
#ifdef __SSE2__
	return _mm_cvtss_si32( _mm_add_ss(_mm_set_ss(-0.5f),_mm_add_ss(_mm_set_ss(f), _mm_set_ss(f))) ) >> 1;
#else
	return (int32_t)floorf(f);
#endif
}
inline int32_t s32floor(double d)
{
	return s32floor((float)d);
}

//switched SSE2 functions
//-------------
#ifdef __SSE2__

/*template<int NUM>
inline void memset_u16_le(void* dst, uint16_t val)
{
	uint32_t u32val;
	//just for the endian safety
	T1WriteWord((uint8_t*)&u32val,0,val);
	T1WriteWord((uint8_t*)&u32val,2,val);
	////const __m128i temp = _mm_set_epi32(u32val,u32val,u32val,u32val);

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	const __m128i temp = _mm_set_epi32(u32val,u32val,u32val,u32val);
	MACRODO_N(NUM/8,_mm_store_si128((__m128i*)((uint8_t*)dst+(X)*16), temp));
#else
	__m128 temp; temp.m128_i32[0] = u32val;
	//MACRODO_N(NUM/8,_mm_store_si128((__m128i*)((uint8_t*)dst+(X)*16), temp));
	MACRODO_N(NUM/8,_mm_store_ps1((float*)((uint8_t*)dst+(X)*16), temp));
#endif
}*/

#else //no sse2

/*template<int NUM>
static inline void memset_u16_le(void* dst, uint16_t val)
{
	for(int i=0;i<NUM;i++)
		T1WriteWord((uint8_t*)dst,i<<1,val);
}*/

#endif

// NOSSE version always used in gfx3d.cpp
//void _NOSSE_MatrixMultVec4x4 (const float *matrix, float *vecPtr);
//void MatrixMultVec3x3_fixed(const int32_t *matrix, int32_t *vecPtr);

//---------------------------
//switched SSE functions
#ifdef __SSE__

/*struct SSE_MATRIX
{
	SSE_MATRIX(const float *matrix)
		: row0(_mm_load_ps(matrix))
		, row1(_mm_load_ps(matrix+4))
		, row2(_mm_load_ps(matrix+8))
		, row3(_mm_load_ps(matrix+12))
	{}

	union {
		__m128 rows[4];
		struct { __m128 row0; __m128 row1; __m128 row2; __m128 row3; };
	};

};*/

/*inline __m128 _util_MatrixMultVec4x4_(const SSE_MATRIX &mat, __m128 vec)
{
	__m128 xmm5 = _mm_shuffle_ps(vec, vec, B8(01010101));
	__m128 xmm6 = _mm_shuffle_ps(vec, vec, B8(10101010));
	__m128 xmm7 = _mm_shuffle_ps(vec, vec, B8(11111111));
	__m128 xmm4 = _mm_shuffle_ps(vec, vec, B8(00000000));

	xmm4 = _mm_mul_ps(xmm4,mat.row0);
	xmm5 = _mm_mul_ps(xmm5,mat.row1);
	xmm6 = _mm_mul_ps(xmm6,mat.row2);
	xmm7 = _mm_mul_ps(xmm7,mat.row3);
	xmm4 = _mm_add_ps(xmm4,xmm5);
	xmm4 = _mm_add_ps(xmm4,xmm6);
	xmm4 = _mm_add_ps(xmm4,xmm7);
	return xmm4;
}*/

/*inline void MatrixMultiply(float * matrix, const float * rightMatrix)
{
	//this seems to generate larger code, including many movaps, but maybe it is less harsh on the registers than the
	//more hand-tailored approach
	__m128 row0 = _util_MatrixMultVec4x4_((SSE_MATRIX)matrix,_mm_load_ps(rightMatrix));
	__m128 row1 = _util_MatrixMultVec4x4_((SSE_MATRIX)matrix,_mm_load_ps(rightMatrix+4));
	__m128 row2 = _util_MatrixMultVec4x4_((SSE_MATRIX)matrix,_mm_load_ps(rightMatrix+8));
	__m128 row3 = _util_MatrixMultVec4x4_((SSE_MATRIX)matrix,_mm_load_ps(rightMatrix+12));
	_mm_store_ps(matrix,row0);
	_mm_store_ps(matrix+4,row1);
	_mm_store_ps(matrix+8,row2);
	_mm_store_ps(matrix+12,row3);
}*/



/*inline void MatrixMultVec4x4(const float *matrix, float *vecPtr)
{
	_mm_store_ps(vecPtr,_util_MatrixMultVec4x4_((SSE_MATRIX)matrix,_mm_load_ps(vecPtr)));
}*/

/*inline void MatrixMultVec4x4_M2(const float *matrix, float *vecPtr)
{
	//there are hardly any gains from merging these manually
	MatrixMultVec4x4(matrix+16,vecPtr);
	MatrixMultVec4x4(matrix,vecPtr);
}*/

/*inline void MatrixMultVec3x3(const float * matrix, float * vecPtr)
{
	const __m128 vec = _mm_load_ps(vecPtr);

	__m128 xmm5 = _mm_shuffle_ps(vec, vec, B8(01010101));
	__m128 xmm6 = _mm_shuffle_ps(vec, vec, B8(10101010));
	__m128 xmm4 = _mm_shuffle_ps(vec, vec, B8(00000000));

	const SSE_MATRIX mat(matrix);

	xmm4 = _mm_mul_ps(xmm4,mat.row0);
	xmm5 = _mm_mul_ps(xmm5,mat.row1);
	xmm6 = _mm_mul_ps(xmm6,mat.row2);
	xmm4 = _mm_add_ps(xmm4,xmm5);
	xmm4 = _mm_add_ps(xmm4,xmm6);

	_mm_store_ps(vecPtr,xmm4);
}*/

/*inline void MatrixTranslate(float *matrix, const float *ptr)
{
	__m128 xmm4 = _mm_load_ps(ptr);
	__m128 xmm5 = _mm_shuffle_ps(xmm4, xmm4, B8(01010101));
	__m128 xmm6 = _mm_shuffle_ps(xmm4, xmm4, B8(10101010));
	xmm4 = _mm_shuffle_ps(xmm4, xmm4, B8(00000000));

	xmm4 = _mm_mul_ps(xmm4,_mm_load_ps(matrix));
	xmm5 = _mm_mul_ps(xmm5,_mm_load_ps(matrix+4));
	xmm6 = _mm_mul_ps(xmm6,_mm_load_ps(matrix+8));
	xmm4 = _mm_add_ps(xmm4,xmm5);
	xmm4 = _mm_add_ps(xmm4,xmm6);
	xmm4 = _mm_add_ps(xmm4,_mm_load_ps(matrix+12));
	_mm_store_ps(matrix+12,xmm4);
}*/

/*inline void MatrixScale(float *matrix, const float *ptr)
{
	__m128 xmm4 = _mm_load_ps(ptr);
	__m128 xmm5 = _mm_shuffle_ps(xmm4, xmm4, B8(01010101));
	__m128 xmm6 = _mm_shuffle_ps(xmm4, xmm4, B8(10101010));
	xmm4 = _mm_shuffle_ps(xmm4, xmm4, B8(00000000));

	xmm4 = _mm_mul_ps(xmm4,_mm_load_ps(matrix));
	xmm5 = _mm_mul_ps(xmm5,_mm_load_ps(matrix+4));
	xmm6 = _mm_mul_ps(xmm6,_mm_load_ps(matrix+8));
	_mm_store_ps(matrix,xmm4);
	_mm_store_ps(matrix+4,xmm5);
	_mm_store_ps(matrix+8,xmm6);
}*/

/*template<int NUM_ROWS>
inline void vector_fix2float(float* matrix, const float divisor)
{
	CTASSERT(NUM_ROWS==3 || NUM_ROWS==4);

	const __m128 val = _mm_set_ps1(divisor);

	_mm_store_ps(matrix,_mm_div_ps(_mm_load_ps(matrix),val));
	_mm_store_ps(matrix+4,_mm_div_ps(_mm_load_ps(matrix+4),val));
	_mm_store_ps(matrix+8,_mm_div_ps(_mm_load_ps(matrix+8),val));
	if(NUM_ROWS==4)
		_mm_store_ps(matrix+12,_mm_div_ps(_mm_load_ps(matrix+12),val));
}*/

//WARNING: I do not think this is as fast as a memset, for some reason.
//at least in vc2005 with sse enabled. better figure out why before using it
/*template<int NUM>
static inline void memset_u8(void* _dst, uint8_t val)
{
	memset(_dst,val,NUM);
	//const uint8_t* dst = (uint8_t*)_dst;
	//uint32_t u32val = (val<<24)|(val<<16)|(val<<8)|val;
	//const __m128i temp = _mm_set_epi32(u32val,u32val,u32val,u32val);
	//MACRODO_N(NUM/16,_mm_store_si128((__m128i*)(dst+(X)*16), temp));
}*/

#else //no sse

//void MatrixMultVec4x4 (const float *matrix, float *vecPtr);
//void MatrixMultVec3x3(const float * matrix, float * vecPtr);
//void MatrixMultiply(float * matrix, const float * rightMatrix);
//void MatrixTranslate(float *matrix, const float *ptr);
//void MatrixScale(float * matrix, const float * ptr);

/*inline void MatrixMultVec4x4_M2(const float *matrix, float *vecPtr)
{
	//there are hardly any gains from merging these manually
	MatrixMultVec4x4(matrix+16,vecPtr);
	MatrixMultVec4x4(matrix,vecPtr);
}*/

/*template<int NUM_ROWS>
inline void vector_fix2float(float* matrix, const float divisor)
{
	for(int i=0;i<NUM_ROWS*4;i++)
		matrix[i] /= divisor;
}*/

/*template<int NUM>
static inline void memset_u8(void* dst, uint8_t val)
{
	memset(dst,val,NUM);
}*/

#endif //switched SSE functions

//void MatrixMultVec4x4 (const int32_t *matrix, int32_t *vecPtr);

//void MatrixMultVec4x4_M2(const int32_t *matrix, int32_t *vecPtr);

//void MatrixMultiply(int32_t* matrix, const int32_t* rightMatrix);
//void MatrixScale(int32_t *matrix, const int32_t *ptr);
//void MatrixTranslate(int32_t *matrix, const int32_t *ptr);

#endif
