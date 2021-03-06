/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#include <af/array.h>
#include <af/defines.h>
#include <af/arith.h>
#include <af/data.h>
#include <ArrayInfo.hpp>
#include <optypes.hpp>
#include <implicit.hpp>
#include <err_common.hpp>
#include <handle.hpp>
#include <backend.hpp>
#include <unary.hpp>
#include <implicit.hpp>
#include <complex.hpp>
#include <logic.hpp>
#include <cast.hpp>
#include <arith.hpp>

using namespace detail;

template<typename T, af_op_t op>
static inline af_array unaryOp(const af_array in)
{
    af_array res = getHandle(unaryOp<T, op>(castArray<T>(in)));
    return res;
}

template<af_op_t op>
static af_err af_unary(af_array *out, const af_array in)
{
    try {

        ArrayInfo in_info = getInfo(in);
        ARG_ASSERT(1, in_info.isReal());

        af_dtype in_type = in_info.getType();
        af_array res;

        // Convert all inputs to floats / doubles
        af_dtype type = implicit(in_type, f32);

        switch (type) {
        case f32 : res = unaryOp<float  , op>(in); break;
        case f64 : res = unaryOp<double , op>(in); break;
        default:
            TYPE_ERROR(1, in_type); break;
        }

        std::swap(*out, res);
    }
    CATCHALL;
    return AF_SUCCESS;
}

#define UNARY(fn)                                       \
    af_err af_##fn(af_array *out, const af_array in)    \
    {                                                   \
        return af_unary<af_##fn##_t>(out, in);          \
    }


UNARY(sin)
UNARY(cos)
UNARY(tan)

UNARY(asin)
UNARY(acos)
UNARY(atan)

UNARY(sinh)
UNARY(cosh)
UNARY(tanh)

UNARY(asinh)
UNARY(acosh)
UNARY(atanh)

UNARY(trunc)
UNARY(sign)
UNARY(round)
UNARY(floor)
UNARY(ceil)

UNARY(sigmoid)
UNARY(expm1)
UNARY(erf)
UNARY(erfc)

UNARY(log)
UNARY(log10)
UNARY(log1p)
UNARY(log2)

UNARY(sqrt)
UNARY(cbrt)

UNARY(tgamma)
UNARY(lgamma)

template<typename Tc, typename Tr>
af_array expCplx(const af_array a)
{
    Array<Tc> In = getArray<Tc>(a);
    Array<Tr> Real = real<Tr, Tc>(In);
    Array<Tr> Imag = imag<Tr, Tc>(In);

    Array<Tr> ExpReal = unaryOp<Tr, af_exp_t>(Real);
    Array<Tr> CosImag = unaryOp<Tr, af_cos_t>(Imag);
    Array<Tr> SinImag = unaryOp<Tr, af_sin_t>(Imag);

    Array<Tc> Unit  = cplx<Tc, Tr>(CosImag, SinImag, CosImag.dims());
    Array<Tc> Scale = cast<Tc, Tr>(ExpReal);

    Array<Tc> Result = arithOp<Tc, af_mul_t>(Scale, Unit, Scale.dims());

    return getHandle(Result);
}

af_err af_exp(af_array *out, const af_array in)
{
    try {

        ArrayInfo in_info = getInfo(in);
        af_dtype in_type = in_info.getType();
        af_array res;

        // Convert all inputs to floats / doubles
        af_dtype type = implicit(in_type, f32);

        switch (type) {
        case f32 : res = unaryOp<float  , af_exp_t>(in); break;
        case f64 : res = unaryOp<double , af_exp_t>(in); break;
        case c32 : res = expCplx<cfloat , float >(in); break;
        case c64 : res = expCplx<cdouble, double>(in); break;
        default:
            TYPE_ERROR(1, in_type); break;
        }

        std::swap(*out, res);
    }
    CATCHALL;
    return AF_SUCCESS;
}

af_err af_not(af_array *out, const af_array in)
{
    try {

        af_array tmp;
        ArrayInfo in_info = getInfo(in);

        AF_CHECK(af_constant(&tmp, 0,
                             in_info.ndims(),
                             in_info.dims().get(), in_info.getType()));

        AF_CHECK(af_eq(out, in, tmp, false));

        AF_CHECK(af_release_array(tmp));
    } CATCHALL;

    return AF_SUCCESS;
}

af_err af_arg(af_array *out, const af_array in)
{
    try {

        ArrayInfo in_info = getInfo(in);

        if (!in_info.isComplex()) {
            return af_constant(out, 0,
                               in_info.ndims(),
                               in_info.dims().get(), in_info.getType());
        }

        af_array real;
        af_array imag;

        AF_CHECK(af_real(&real, in));
        AF_CHECK(af_imag(&imag, in));

        AF_CHECK(af_atan2(out, imag, real, false));

        AF_CHECK(af_release_array(real));
        AF_CHECK(af_release_array(imag));
    } CATCHALL;

    return AF_SUCCESS;
}

af_err af_pow2(af_array *out, const af_array in)
{
    try {

        af_array two;
        ArrayInfo in_info = getInfo(in);

        AF_CHECK(af_constant(&two, 2,
                             in_info.ndims(),
                             in_info.dims().get(), in_info.getType()));

        AF_CHECK(af_pow(out, two, in, false));

        AF_CHECK(af_release_array(two));
    } CATCHALL;

    return AF_SUCCESS;
}

af_err af_factorial(af_array *out, const af_array in)
{
    try {

        af_array one;
        ArrayInfo in_info = getInfo(in);

        AF_CHECK(af_constant(&one, 1,
                             in_info.ndims(),
                             in_info.dims().get(), in_info.getType()));

        af_array inp1;
        AF_CHECK(af_add(&inp1, one, in, false));

        AF_CHECK(af_tgamma(out, inp1));

        AF_CHECK(af_release_array(one));
        AF_CHECK(af_release_array(inp1));
    } CATCHALL;

    return AF_SUCCESS;
}

template<typename T, af_op_t op>
static inline af_array checkOp(const af_array in)
{
    af_array res = getHandle(checkOp<T, op>(castArray<T>(in)));
    return res;
}

template<af_op_t op>
struct cplxLogicOp
{
    af_array operator()(Array<char> resR, Array<char> resI, dim4 dims)
    {
        return getHandle(logicOp<char, af_or_t>(resR, resI, dims));
    }
};

template <>
struct cplxLogicOp<af_iszero_t>
{
    af_array operator()(Array<char> resR, Array<char> resI, dim4 dims)
    {
        return getHandle(logicOp<char, af_and_t>(resR, resI, dims));
    }
};

template<typename T, typename BT, af_op_t op>
static inline af_array checkOpCplx(const af_array in)
{
    Array<BT> R = real<BT, T>(getArray<T>(in));
    Array<BT> I = imag<BT, T>(getArray<T>(in));

    Array<char> resR = checkOp<BT, op>(R);
    Array<char> resI = checkOp<BT, op>(I);

    ArrayInfo in_info = getInfo(in);
    dim4 dims = in_info.dims();
    cplxLogicOp<op> cplxLogic;
    af_array res = cplxLogic(resR, resI, dims);

    return res;
}

template<af_op_t op>
static af_err af_check(af_array *out, const af_array in)
{
    try {

        ArrayInfo in_info = getInfo(in);

        af_dtype in_type = in_info.getType();
        af_array res;

        // Convert all inputs to floats / doubles / complex
        af_dtype type = implicit(in_type, f32);

        switch (type) {
        case f32 : res = checkOp<float  , op>(in); break;
        case f64 : res = checkOp<double , op>(in); break;
        case c32 : res = checkOpCplx<cfloat , float , op>(in); break;
        case c64 : res = checkOpCplx<cdouble, double, op>(in); break;
        default:
            TYPE_ERROR(1, in_type); break;
        }

        std::swap(*out, res);
    }
    CATCHALL;
    return AF_SUCCESS;
}

#define CHECK(fn)                                       \
    af_err af_##fn(af_array *out, const af_array in)    \
    {                                                   \
        return af_check<af_##fn##_t>(out, in);          \
    }


CHECK(isinf)
CHECK(isnan)
CHECK(iszero)
