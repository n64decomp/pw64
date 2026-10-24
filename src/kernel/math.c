#include "common.h"
#include <math.h>

typedef struct {
    f64 d;
} Double;

typedef struct {
    f32 f;
} Float;

STATIC_DATA const double D_8024E3A0[] = {
    1.0, -0.166666595504277565, 0.00833306624608215474, -0.000198096029019379492, 2.60578063796803717e-06,
};

// to make the code use the matching operand order, these should be float literals
// however, the compiler doubles D_8024E3C8, D_8024E3D0, D_8024E3D8 in .rodata
// if using `static const`, compiler places D_8024E3C8, D_8024E3D0, D_8024E3D8 in .data
#define USE_CONST_DATA
#ifdef USE_FLOAT_LITERAL
#define D_8024E3C8 0.318309886183790691
#define D_8024E3D0 3.14159262180328369
#define D_8024E3D8 3.17865095470563921e-08
#elif defined(USE_CONST_DATA)
STATIC_DATA const Double D_8024E3C8 = { 0.318309886183790691 };
STATIC_DATA const Double D_8024E3D0 = { 3.14159262180328369 };
STATIC_DATA const Double D_8024E3D8 = { 3.17865095470563921e-08 };
STATIC_DATA const Float zero = { 0.0f };
#endif

f32 uvSqrtF(f32 value) {
    return sqrtf(value);
}

f32 uvSinF(f32 x) {
    f64 dx;
    f64 xsq;
    f64 poly;
    f64 temp_fa1;
    s32 number;
    s32 ix;

    ix = *(s32*)&x;

    if (((ix >> 22) & 0x1FF) < 0xFF) {
        dx = x;
        if (((ix >> 22) & 0x1FF) >= 0xE6) {
            xsq = SQ(dx);

            temp_fa1 = ((((D_8024E3A0[4] * xsq) + D_8024E3A0[3]) * xsq + D_8024E3A0[2]) * xsq + D_8024E3A0[1]);

            return (dx * xsq) * temp_fa1 + dx;
        }
        return x;
    }

    if (((ix >> 22) & 0x1FF) < 0x136) {
        dx = x;

        number = ROUNDF(dx * D_8024E3C8.d);

        dx -= ((poly = number) * D_8024E3D0.d);
        dx -= (poly * D_8024E3D8.d);
        xsq = dx * dx;

        temp_fa1 = ((((D_8024E3A0[4] * xsq) + D_8024E3A0[3]) * xsq + D_8024E3A0[2]) * xsq + D_8024E3A0[1]);

        if (!(number & 1)) {
            return (f32)((dx * xsq) * temp_fa1 + dx);
        } else {
            return -(f32)((dx * xsq) * temp_fa1 + dx);
        }
    }
    if (x != x) {
        return 2.1401436e9f;
    }
    return 0.0f;
}

f32 uvCosF(f32 x) {
    s32 ix = *(s32*)&x;
    f32 var_fv0;
    f64 fabsA0; // temp_fa0;
    f64 temp_fa1;
    f64 temp_ft4;
    f64 temp_fv0;
    f64 temp_fv1;
    s32 number;
    f32 temp;
    f64 poly;
    f64 dx;
    f64 xsq;

    // mask off exponent + mantissa MSbit of f32
    if (((ix >> 22) & 0x1FF) < 0x136) {
        dx = (f32)FABS(x);

        poly = dx * D_8024E3C8.d + 0.5;
        number = ROUNDF(poly);

        dx -= ((poly = (number - 0.5)) * D_8024E3D0.d);
        dx -= (poly * D_8024E3D8.d);
        xsq = dx * dx;

        temp_fa1 = ((((D_8024E3A0[4] * xsq) + D_8024E3A0[3]) * xsq + D_8024E3A0[2]) * xsq + D_8024E3A0[1]);

        if (!(number & 1)) {
            return (f32)((dx * xsq) * temp_fa1 + dx);
        } else {
            return -(f32)((dx * xsq) * temp_fa1 + dx);
        }
    }

    // NaN check
    if (x != x) {
        return 2.1401436e9f;
    }

    return 0.0f;
}

f32 uvLength2D(f32 x, f32 y) {
    return uvSqrtF(SQ(x) + SQ(y));
}

f32 uvLength3D(f32 x, f32 y, f32 z) {
    return uvSqrtF(SQ(x) + SQ(y) + SQ(z));
}

f32 uvAtan2F(f32 y, f32 x) {
    s32 flag;
    f32 ret;
    f32 absY;
    f32 absX;
    f32 tmp;
    f32 absdiff;

    if ((x == 0.0f) && (y == 0.0f)) {
        return 0.0f;
    }
    absX = FABS(x);
    absY = FABS(y);
    flag = 0;
    if (absX < absY) {
        tmp = absX;
        absX = absY;
        absY = tmp;
        flag = 1;
    }
    tmp = (absY / absX);
    absdiff = FABS(absY - absX);
    ret = (PW_PI_4 * (absY / absX)) + (0.309f * absdiff * tmp);

    tmp = 0; // fake store to prevent emitting additional mov instr.
    if (x >= 0.0f) {
        if (flag) {
            ret = PW_PI_2 - ret;
        }
    } else if (flag) {
        ret = PW_PI_2 + ret;
    } else {
        ret = PW_PI - ret;
    }

    if (y >= 0.0f) {
        return ret;
    } else {
        return -ret;
    }
}
