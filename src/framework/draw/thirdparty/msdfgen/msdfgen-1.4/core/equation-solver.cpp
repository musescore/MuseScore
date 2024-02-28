
#include "equation-solver.h"
#include "arithmetics.hpp"

#include <cassert>

namespace msdfgen {

int solveQuadratic(double x[2], double a, double aRev, double b, double c) {
    if (fabs(a) < 1e-14) {
        if (fabs(b) < 1e-14) {
            if (c == 0)
                return -1;
            return 0;
        }
        x[0] = -c/b;
        return 1;
    }
    double dscr = b*b-4*a*c;
    if (dscr > 0) {
        dscr = approxSquareRoot(dscr);
        x[0] = (-b+dscr)*(2*aRev);
        x[1] = (-b-dscr)*(2*aRev);
        return 2;
    } else if (dscr == 0) {
        x[0] = -b*(2*aRev);
        return 1;
    } else
        return 0;
}

//static float acbrt(float x0) {
//	union { int ix; float x; };
//
//	x = x0;                      // x can be viewed as int.
//	ix = ix / 4 + ix / 16;           // Approximate divide by 3.
//	ix = ix + ix / 16;
//	ix = ix + ix / 256;
//	ix = 0x2a5137a0 + ix;        // Initial guess.
//	x = 0.33333333f*(2.0f*x + x0 / (x*x));  // Newton step.
//	return x;
//}

// relative error at +-0.0316
static float acbrt2(float x0) {
	union { int ix; float x; };

	x = x0;                      // x can be viewed as int.
	ix = 0x2a51067f + ix / 3;      // Initial guess.
	return x;
}

static double approxCos(double x) {
	assert(x >= -M_PI * 2 && x <= M_PI * 2);
	x += 1.57079632;
	if (x > 3.14159265)
		x -= 6.28318531;
	double cos = 1.27323954 * x;
	double addition = 0.405284735 * x * x;
	if (x >= 0) addition = -addition;
	cos += addition;
	double mul = cos * cos;
	if (cos < 0) mul = -mul;
	cos = .225 * (mul - cos) + cos;
	return cos;
}

//static double approxCos2(double x) {
//	assert(x >= -M_PI * 2 && x <= M_PI * 2);
//	static constexpr double tp = 1. / (2.*M_PI);
//	x *= tp;
//	x -= .25 + std::floor(x + .25);
//	x *= 16. * (std::abs(x) - .5);
//	x += .225 * x * (std::abs(x) - 1.);
//	return x;
//}

float rsqrt1(float x0) {
	union { int ix; float x; };

	x = x0;                      // x can be viewed as an int.
	float xhalf = 0.5f*x;
	ix = 0x5f37599e - (ix >> 1); // Initial guess.
	x = x * (1.5f - xhalf * x*x);    // Newton step.
	//x = x * (1.5f - xhalf * x*x);    // Newton step again.
	return x;
}

int solveCubicNormed(double *x, double a, double b, double c) {
	static constexpr double oneThird = 1.0/3.0;
	static constexpr double oneNinth = 1.0/9.0;
	static constexpr double oneFiftyFourth = 1.0/54.0;
    double a2 = a*a;
    double q  = (a2 - 3*b)*oneNinth;
    double r  = (a*(2*a2-9*b) + 27*c)*oneFiftyFourth;
    double r2 = r*r;
    double q3 = q*q*q;
    double A, B;
    if (r2 < q3) {
        double t = r*rsqrt1(q3);
        if (t < -1) t = -1;
        if (t > 1) t = 1;
        t = acos(t);
        a *= oneThird; q = -2*approxSquareRoot(q);
        x[0] = q*approxCos(t*oneThird)-a;
        x[1] = q*approxCos((t+2*M_PI)*oneThird)-a;
        x[2] = q*approxCos((t-2*M_PI)*oneThird)-a;
        return 3;
    } else {
        A = acbrt2(fabs(r)+approxSquareRoot(r2-q3));
        if (r >= 0) A = -A;
        B = A == 0 ? 0 : q/A;
        a *= oneThird;
        x[0] = (A+B)-a;
        x[1] = -0.5*(A+B)-a;
        x[2] = 0.86602540378443864676372317075295*(A-B);
        if (fabs(x[2]) < 1e-14)
            return 2;
        return 1;
    }
}

int solveCubic(double x[3], double a, double b, double c, double d) {
    if (fabs(a) < 1e-14)
        return solveQuadratic(x, b, 1.0/b, c, d);
	double aRev = 1.0/a;
    return solveCubicNormed(x, b*aRev, c*aRev, d*aRev);
}

}
