#include <math.h>

extern "C"
{
    __attribute__((__visibility__("default"), __cdecl__)) double __pow_finite(double a, double b)
    {
        return pow(a, b);
    }
}