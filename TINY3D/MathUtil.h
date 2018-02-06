#ifndef MATH_UTIL_H
#define MATH_UTIL_H
#include <math.h>

const float kPi = 3.14159265f;
const float k2Pi = kPi * 2.0f;
const float kPiOver2 = kPi / 2.0f;
const float k1overPi = 1.0f / kPi;
const float k1over2Pi = 1.0f / k2Pi;

//let theta be [-pi,pi]
extern float wrapPi(float theta);
// safe arccos
extern float safeAcos(float x);

#endif // !MATH_UTIL_H
