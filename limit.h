#ifndef _LIMIT_
#define _LIMIT_

#include <math.h>

double limiter(double vin, double lim_v, double k)
{
    vin = vin > lim_v ? ((vin - lim_v) * k + lim_v) : vin;
    vin = vin < -lim_v ? (-(vin + lim_v) * k - lim_v) : vin;
    return vin;
}

#endif