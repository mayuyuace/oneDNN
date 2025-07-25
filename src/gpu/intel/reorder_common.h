/*******************************************************************************
* Copyright 2019-2025 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#define DT_UNDEF 1
#include "gpu/intel/ocl_math_utils.h"
#include "gpu/intel/ocl_types.h"

#if SRC_DT_F16 || DST_DT_F16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif

#if SRC_DT_F64 || DST_DT_F64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#undef SRC_OFF
#undef DST_OFF

#define SRC_OFF(x0, x1, x2, x3, x4, x5) \
    OFF_MD(SRC, (x0), (x1), (x2), (x3), (x4), (x5))
#define DST_OFF(x0, x1, x2, x3, x4, x5) \
    OFF_MD(DST, (x0), (x1), (x2), (x3), (x4), (x5))

#define SRC_OFF_G(gr, x0, x1, x2, x3, x4) \
    OFF_MD(SRC, gr, (x0), (x1), (x2), (x3), (x4))
#define DST_OFF_G(gr, x0, x1, x2, x3, x4) \
    OFF_MD(DST, gr, (x0), (x1), (x2), (x3), (x4))

#if SRC_DT_S8
#define SRC_BLOCK_READ(src) \
    as_char(intel_sub_group_block_read_uc((const __global uchar *)(src)))
#define SRC_BLOCK_READ8(src) \
    as_char8(intel_sub_group_block_read_uc8((const __global uchar *)(src)))
#define SRC_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_uc((__global uchar *)(dst), as_uchar(val))
#define SRC_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_uc8((__global uchar *)(dst), as_uchar8(val))
#endif // SRC_DT_S8

#if SRC_DT_U8 || SRC_DT_HF8 || SRC_DT_BF8
#define SRC_BLOCK_READ(src) \
    as_uchar(intel_sub_group_block_read_uc((const __global uchar *)(src)))
#define SRC_BLOCK_READ8(src) \
    as_uchar8(intel_sub_group_block_read_uc8((const __global uchar *)(src)))
#define SRC_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_uc((__global uchar *)(dst), as_uchar(val))
#define SRC_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_uc8((__global uchar *)(dst), as_uchar8(val))
#endif // SRC_DT_U8

#if SRC_DT_F16
#define SRC_BLOCK_READ(src) \
    as_half(intel_sub_group_block_read_us((const __global ushort *)(src)))
#define SRC_BLOCK_READ8(src) \
    as_half8(intel_sub_group_block_read_us8((const __global ushort *)(src)))
#define SRC_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_us((__global ushort *)(dst), as_ushort(val))
#define SRC_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_us8((__global ushort *)(dst), as_ushort8(val))
#endif // SRC_DT_F16

#if SRC_DT_S32
#define SRC_BLOCK_READ(src) \
    as_int(intel_sub_group_block_read((const __global uint *)(src)))
#define SRC_BLOCK_READ8(src) \
    as_int8(intel_sub_group_block_read8((const __global uint *)(src)))
#define SRC_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write((__global uint *)(dst), as_uint(val))
#define SRC_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write8((__global uint *)(dst), as_uint8(val))
#endif // SRC_DT_S32

#if SRC_DT_F32
#define SRC_BLOCK_READ(src) \
    as_float(intel_sub_group_block_read((const __global uint *)(src)))
#define SRC_BLOCK_READ8(src) \
    as_float8(intel_sub_group_block_read8((const __global uint *)(src)))
#define SRC_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write((__global uint *)(dst), as_uint(val))
#define SRC_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write8((__global uint *)(dst), as_uint8(val))
#endif // SRC_DT_F32

#if SRC_DT_F64
#define SRC_BLOCK_READ(src) \
    as_double(intel_sub_group_block_read2((const __global uint *)(src)))
#define SRC_BLOCK_READ8(src) \
    (double8)((as_double4(intel_sub_group_block_read8( \
                      (const __global uint *)(src)))), \
            (as_double4(intel_sub_group_block_read8( \
                    (const __global uint *)(src + 8)))))
#define SRC_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write2((__global uint *)(dst), as_uint2(val))
#define SRC_BLOCK_WRITE8(dst, val) \
    do { \
        intel_sub_group_block_write8( \
                (__global uint *)(dst), as_uint8(val.lo)); \
        intel_sub_group_block_write8( \
                (__global uint *)(dst + 8), as_uint8(val.hi)); \
    } while (0)
#endif // SRC_DT_F64

#if SRC_DT_BF16
#define SRC_BLOCK_READ(src) \
    as_ushort(intel_sub_group_block_read_us((const __global ushort *)(src)))
#define SRC_BLOCK_READ8(src) \
    as_ushort8(intel_sub_group_block_read_us8((const __global ushort *)(src)))
#define SRC_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_us((__global ushort *)(dst), as_ushort(val))
#define SRC_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_us8((__global ushort *)(dst), as_ushort8(val))
#endif // SRC_DT_F16

#if DST_DT_S8
#define DST_BLOCK_READ(src) \
    as_char(intel_sub_group_block_read_uc((const __global uchar *)(src)))
#define DST_BLOCK_READ8(src) \
    as_char8(intel_sub_group_block_read_uc8((const __global uchar *)(src)))
#define DST_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_uc((__global uchar *)(dst), as_uchar(val))
#define DST_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_uc8((__global uchar *)(dst), as_uchar8(val))
#endif // DST_DT_S8

#if DST_DT_U8 || DST_DT_BF8 || DST_DT_HF8
#define DST_BLOCK_READ(src) \
    as_uchar(intel_sub_group_block_read_uc((const __global uchar *)(src)))
#define DST_BLOCK_READ8(src) \
    as_uchar8(intel_sub_group_block_read_uc8((const __global uchar *)(src)))
#define DST_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_uc((__global uchar *)(dst), as_uchar(val))
#define DST_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_uc8((__global uchar *)(dst), as_uchar8(val))
#endif // SRC_DT_U8

#if DST_DT_F16
#define DST_BLOCK_READ(src) \
    as_half(intel_sub_group_block_read_us((const __global ushort *)(src)))
#define DST_BLOCK_READ8(src) \
    as_half8(intel_sub_group_block_read_us8((const __global ushort *)(src)))
#define DST_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_us((__global ushort *)(dst), as_ushort(val))
#define DST_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_us8((__global ushort *)(dst), as_ushort8(val))
#endif // DST_DT_F16

#if DST_DT_S32
#define DST_BLOCK_READ(src) \
    as_int(intel_sub_group_block_read((const __global uint *)(src)))
#define DST_BLOCK_READ8(src) \
    as_int8(intel_sub_group_block_read8((const __global uint *)(src)))
#define DST_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write((__global uint *)(dst), as_uint(val))
#define DST_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write8((__global uint *)(dst), as_uint8(val))
#endif // DST_DT_S32

#if DST_DT_F32
#define DST_BLOCK_READ(src) \
    as_float(intel_sub_group_block_read((const __global uint *)(src)))
#define DST_BLOCK_READ8(src) \
    as_float8(intel_sub_group_block_read8((const __global uint *)(src)))
#define DST_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write((__global uint *)(dst), as_uint(val))
#define DST_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write8((__global uint *)(dst), as_uint8(val))
#endif // DST_DT_F32

#if DST_DT_F64
#define DST_BLOCK_READ(src) \
    as_double(intel_sub_group_block_read2((const __global uint *)(src)))
#define DST_BLOCK_READ8(src) \
    (double8)((as_double4(intel_sub_group_block_read8( \
                      (const __global uint *)(src)))), \
            (as_double4(intel_sub_group_block_read8( \
                    (const __global uint *)(src)))))
#define DST_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write2((__global uint *)(dst), as_uint2(val))
#define DST_BLOCK_WRITE8(dst, val) \
    do { \
        intel_sub_group_block_write8( \
                (__global uint *)(dst), as_uint8(val.lo)); \
        intel_sub_group_block_write8( \
                (__global uint *)(dst + 8), as_uint8(val.hi)); \
    } while (0)
#endif // DST_DT_F64

#if DST_DT_BF16
#define DST_BLOCK_READ(src) \
    as_ushort(intel_sub_group_block_read_us((const __global ushort *)(src)))
#define DST_BLOCK_READ8(src) \
    as_ushort8(intel_sub_group_block_read_us8((const __global ushort *)(src)))
#define DST_BLOCK_WRITE(dst, val) \
    intel_sub_group_block_write_us((__global ushort *)(dst), as_ushort(val))
#define DST_BLOCK_WRITE8(dst, val) \
    intel_sub_group_block_write_us8((__global ushort *)(dst), as_ushort8(val))
#endif // SRC_DT_F16

#if (SRC_DT_S8 && DST_DT_S8) || (SRC_DT_U8 && DST_DT_U8) \
        || (SRC_DT_BF16 && DST_DT_BF16) || (SRC_DT_F16 && DST_DT_F16) \
        || (SRC_DT_F32 && DST_DT_F32) || (SRC_DT_S32 && DST_DT_S32) \
        || (SRC_DT_F64 && DST_DT_F64) || (SRC_DT_BF8 && DST_DT_BF8) \
        || (SRC_DT_HF8 && DST_DT_HF8)
#define SRC_TO_DST(x) (x)
#define SRC_TO_DST8(x) (x)
#else
#define SRC_TO_DST(x) TO_DST(SRC_TO_REF(x))
#define SRC_TO_DST8(x) TO_DST8(SRC_TO_REF8(x))
#endif

#define SCALE_MUL(x, s) (x) * (s)
#define SCALE_DIV(x, s) (x) / (s)
#define SCALE_NONE(x, s) (x)

#if WITH_SRC_SCALE
#define SRC_SCALE SCALE_MUL
#else
#define SRC_SCALE SCALE_NONE
#endif

#if WITH_DST_SCALE
#define DST_SCALE SCALE_DIV
#else
#define DST_SCALE SCALE_NONE
#endif

#if WITH_SUM_SCALE
#define SUM_SCALE SCALE_MUL
#else
#define SUM_SCALE SCALE_NONE
#endif

#define ZP_SHIFT(x, x0) (x) - (float)(x0)
#define ZP_UNSHIFT(x, x0) (x) + (float)(x0)
#define ZP_NO_SHIFT(x, x0) (x)
#define ZP_READ_VAL(x) x
#define ZP_READ_PTR(x) x[0]
#define ZP_ZERO(x) 0

#if WITH_SRC_ZPOINT
#define SRC_SHIFT ZP_SHIFT
#define GET_SRC_ZP ZP_READ_PTR
#else
#define SRC_SHIFT ZP_NO_SHIFT
#define GET_SRC_ZP ZP_ZERO
#endif

#if WITH_DST_ZPOINT
#define DST_SHIFT ZP_UNSHIFT
#define GET_DST_ZP ZP_READ_PTR
#else
#define DST_SHIFT ZP_NO_SHIFT
#define GET_DST_ZP ZP_ZERO
#endif

#if WITH_SUM_ZPOINT
#define SUM_SHIFT ZP_SHIFT
#define GET_SUM_ZP ZP_READ_VAL
#else
#define SUM_SHIFT ZP_NO_SHIFT
#define GET_SUM_ZP ZP_ZERO
#endif

#define SRC_AXPY(a, x, x0) SRC_SCALE(SRC_SHIFT(x, x0), a)
#define DST_AXPY(a, x, x0) DST_SHIFT(DST_SCALE(x, a), x0)
#define SUM_AXPY(a, x, x0) SUM_SCALE(SUM_SHIFT(x, x0), a)
#define AXPY(src, dst, a, b, c, x0, y0, z0) \
    DST_AXPY(b, (SRC_AXPY(a, src, x0) + SUM_AXPY(c, dst, y0 + z0)), y0)

#if WITH_SRC_SCALE || WITH_SRC_ZPOINT
#define WITH_SRC_MOD 1
#endif

#if WITH_DST_SCALE || WITH_DST_ZPOINT
#define WITH_DST_MOD 1
#endif

#if WITH_SUM_SCALE || WITH_SUM_ZPOINT
#define WITH_SUM_MOD 1
#endif

#if WITH_SUM_MOD
#define REORDER(round, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        const float _x = SRC_TO_REF(_src); \
        const float _y = DST_TO_REF(_dst); \
        const float _s = AXPY(_x, _y, _a, _b, _c, _x0, _y0, _z0); \
        _dst = TO_DST(round(_s)); \
    } while (0)
#define REORDER8(round8, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        const float8 _x = convert_float8(SRC_TO_REF8(_src)); \
        const float8 _y = convert_float8(DST_TO_REF8(_dst)); \
        const float8 _s = AXPY(_x, _y, _a, _b, _c, _x0, _y0, _z0); \
        _dst = TO_DST8(round8(_s)); \
    } while (0)
#elif WITH_SRC_MOD || WITH_DST_MOD
#define REORDER(round, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        const float _x = SRC_TO_REF(_src); \
        const float _s = AXPY(_x, 0.f, _a, _b, _c, _x0, _y0, _z0); \
        _dst = TO_DST(round(_s)); \
    } while (0)
#define REORDER8(round8, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        const float8 _x = convert_float8(SRC_TO_REF8(_src)); \
        const float8 _s = AXPY(_x, 0.f, _a, _b, _c, _x0, _y0, _z0); \
        _dst = TO_DST8(round8(_s)); \
    } while (0)
#elif WITH_SROUND
#define REORDER(round, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        _dst = TO_DST(round(SRC_TO_REF(_src))); \
    } while (0)
#define REORDER8(round8, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        _dst = TO_DST8(round8(SRC_TO_REF8(_src)); \
    } while (0)
#else
#define REORDER(round, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        _dst = SRC_TO_DST(_src); \
    } while (0)
#define REORDER8(round8, _dst, _src, _a, _b, _c, _x0, _y0, _z0) \
    do { \
        _dst = SRC_TO_DST8(_src); \
    } while (0)
#endif // WITH_SUM_MOD

#define DEFAULT_ROUND(f) f

#if WITH_SRC_SCALE || WITH_DST_SCALE || WITH_SRC_ZPOINT || WITH_DST_ZPOINT
#define MASK_DIM(prefix, mask, dim) ((CONCAT2(prefix, mask) >> dim) & 1)
#define QUANT_DIM(prefix, mask, dim) \
    (MASK_DIM(prefix, mask, dim) ? CONCAT3(prefix, _D, dim) : 1)
#define GROUP_DIM(prefix, quant_group_dim, quant_group, dim) \
    ((CONCAT2(prefix, quant_group_dim) == dim) ? CONCAT2(prefix, quant_group) \
                                               : 1)
#define QUANT_S5(prefix, mask, quant_group_dim, quant_group) (1)
#define QUANT_S4(prefix, mask, quant_group_dim, quant_group) \
    ((QUANT_DIM(prefix, mask, 5) \
             / GROUP_DIM(prefix, quant_group_dim, quant_group, 5)) \
            * QUANT_S5(prefix, mask, quant_group_dim, quant_group))
#define QUANT_S3(prefix, mask, quant_group_dim, quant_group) \
    ((QUANT_DIM(prefix, mask, 4) \
             / GROUP_DIM(prefix, quant_group_dim, quant_group, 4)) \
            * QUANT_S4(prefix, mask, quant_group_dim, quant_group))
#define QUANT_S2(prefix, mask, quant_group_dim, quant_group) \
    ((QUANT_DIM(prefix, mask, 3) \
             / GROUP_DIM(prefix, quant_group_dim, quant_group, 3)) \
            * QUANT_S3(prefix, mask, quant_group_dim, quant_group))
#define QUANT_S1(prefix, mask, quant_group_dim, quant_group) \
    ((QUANT_DIM(prefix, mask, 2) \
             / GROUP_DIM(prefix, quant_group_dim, quant_group, 2)) \
            * QUANT_S2(prefix, mask, quant_group_dim, quant_group))
#define QUANT_S0(prefix, mask, quant_group_dim, quant_group) \
    ((QUANT_DIM(prefix, mask, 1) \
             / GROUP_DIM(prefix, quant_group_dim, quant_group, 1)) \
            * QUANT_S1(prefix, mask, quant_group_dim, quant_group))
#define QUANT_STRIDE(prefix, mask, quant_group_dim, quant_group, dim) \
    (CONCAT2(QUANT_S, dim)(prefix, mask, quant_group_dim, quant_group) \
            * MASK_DIM(prefix, mask, dim))

#if WITH_SRC_SCALE || WITH_DST_SCALE
#define SCALE_OFF(prefix, x0, x1, x2, x3, x4, x5) \
    (((x0) / GROUP_DIM(prefix, _SCALE_GROUP_DIM, _SCALE_GROUP, 0)) \
                    * QUANT_STRIDE(prefix, _SCALE_MASK, _SCALE_GROUP_DIM, \
                            _SCALE_GROUP, 0) \
            + ((x1) / GROUP_DIM(prefix, _SCALE_GROUP_DIM, _SCALE_GROUP, 1)) \
                    * QUANT_STRIDE(prefix, _SCALE_MASK, _SCALE_GROUP_DIM, \
                            _SCALE_GROUP, 1) \
            + ((x2) / GROUP_DIM(prefix, _SCALE_GROUP_DIM, _SCALE_GROUP, 2)) \
                    * QUANT_STRIDE(prefix, _SCALE_MASK, _SCALE_GROUP_DIM, \
                            _SCALE_GROUP, 2) \
            + ((x3) / GROUP_DIM(prefix, _SCALE_GROUP_DIM, _SCALE_GROUP, 3)) \
                    * QUANT_STRIDE(prefix, _SCALE_MASK, _SCALE_GROUP_DIM, \
                            _SCALE_GROUP, 3) \
            + ((x4) / GROUP_DIM(prefix, _SCALE_GROUP_DIM, _SCALE_GROUP, 4)) \
                    * QUANT_STRIDE(prefix, _SCALE_MASK, _SCALE_GROUP_DIM, \
                            _SCALE_GROUP, 4) \
            + ((x5) / GROUP_DIM(prefix, _SCALE_GROUP_DIM, _SCALE_GROUP, 5)) \
                    * QUANT_STRIDE(prefix, _SCALE_MASK, _SCALE_GROUP_DIM, \
                            _SCALE_GROUP, 5))
#endif // WITH_SRC_SCALE || WITH_DST_SCALE

#if WITH_SRC_ZPOINT || WITH_DST_ZPOINT
#define ZPOINT_OFF(prefix, x0, x1, x2, x3, x4, x5) \
    (((x0) / GROUP_DIM(prefix, _ZPOINT_GROUP_DIM, _ZPOINT_GROUP, 0)) \
                    * QUANT_STRIDE(prefix, _ZPOINT_MASK, _ZPOINT_GROUP_DIM, \
                            _ZPOINT_GROUP, 0) \
            + ((x1) / GROUP_DIM(prefix, _ZPOINT_GROUP_DIM, _ZPOINT_GROUP, 1)) \
                    * QUANT_STRIDE(prefix, _ZPOINT_MASK, _ZPOINT_GROUP_DIM, \
                            _ZPOINT_GROUP, 1) \
            + ((x2) / GROUP_DIM(prefix, _ZPOINT_GROUP_DIM, _ZPOINT_GROUP, 2)) \
                    * QUANT_STRIDE(prefix, _ZPOINT_MASK, _ZPOINT_GROUP_DIM, \
                            _ZPOINT_GROUP, 2) \
            + ((x3) / GROUP_DIM(prefix, _ZPOINT_GROUP_DIM, _ZPOINT_GROUP, 3)) \
                    * QUANT_STRIDE(prefix, _ZPOINT_MASK, _ZPOINT_GROUP_DIM, \
                            _ZPOINT_GROUP, 3) \
            + ((x4) / GROUP_DIM(prefix, _ZPOINT_GROUP_DIM, _ZPOINT_GROUP, 4)) \
                    * QUANT_STRIDE(prefix, _ZPOINT_MASK, _ZPOINT_GROUP_DIM, \
                            _ZPOINT_GROUP, 4) \
            + ((x5) / GROUP_DIM(prefix, _ZPOINT_GROUP_DIM, _ZPOINT_GROUP, 5)) \
                    * QUANT_STRIDE(prefix, _ZPOINT_MASK, _ZPOINT_GROUP_DIM, \
                            _ZPOINT_GROUP, 5))
#endif // WITH_SRC_ZP || WITH_DST_ZP

#endif // WITH_SRC_SCALE || WITH_DST_SCALE || WITH_SRC_ZP || WITH_DST_ZP
