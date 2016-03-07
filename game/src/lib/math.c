#include "common.h"

int abs(int x)
{
	return x > 0 ? x : (-x);
}

float fabs(float x)
{
	return x > 0 ? x : (-x);
}

float sqrt(float number) {
    long i;
    float x, y;
    const float f = 1.5F;
    x = number * 0.5F;
    y = number;
    i = * ( long * ) &y;
    i = 0x5f3759df - ( i >> 1 ); //注意这一行 
    y = * ( float * ) &i;
    y = y * ( f - ( x * y * y ) );
    y = y * (f - ( x * y * y ));               
    return number * y;
}

float sin(float x)
{
	int i;
	float t = 0;
	float squarex = x * x;
	float xp = x;
	float n = 1;
	for(i = 1; i <= 13; i++)
	{
		if(i & 1)
		{
			t = t + xp / n;
		}
		else
		{
			t = t - xp / n;
		}
		xp = xp * squarex;
		n = n * (2 * i) * (2 * i + 1);
	}
	return t;
}

float cos(float x)
{
	int i;
	float t = 1;
	float squarex = x * x;
	float xp = squarex;
	float n = 2;
	for(i = 1; i <= 13; i++)
	{
		if(i & 1)
		{
			t = t - xp / n;
		}
		else
		{
			t = t + xp / n;
		}
		xp = xp * squarex;
		n = n * (2 * i + 1) * (2 * i + 2);
	}
	return t;
}
