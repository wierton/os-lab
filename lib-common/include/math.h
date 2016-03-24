#ifndef __MATH_H__
#define __MATH_H__

int abs(int);

float sin(float);
float cos(float);
float fabs(float);
float sqrt(float);

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#endif
