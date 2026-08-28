#pragma once

#define SYR_MATH_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SYR_MATH_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define SYR_MATH_LERP(a, b, t) ((a) + (t) * ((b) - (a)))
