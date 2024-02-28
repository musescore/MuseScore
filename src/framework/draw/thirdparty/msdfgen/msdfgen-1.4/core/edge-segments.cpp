#ifdef PLATFORM_WINDOWS
#pragma optimize("t", on)
#endif

#include "edge-segments.h"

#include "arithmetics.hpp"
#include "equation-solver.h"

namespace msdfgen {

//void EdgeSegment::distanceToPseudoDistance(SignedDistance &distance, Point2 origin, double param) const {
//    if (param < 0) {
//        Vector2 dir = direction(0).normalize();
//        Vector2 aq = origin-point(0);
//        double ts = dotProduct(aq, dir);
//        if (ts < 0) {
//            double pseudoDistance = crossProduct(aq, dir);
//            if (fabs(pseudoDistance) <= fabs(distance.distance)) {
//                distance.distance = pseudoDistance;
//                distance.dot = 0;
//            }
//        }
//    } else if (param > 1) {
//        Vector2 dir = direction(1).normalize();
//        Vector2 bq = origin-point(1);
//        double ts = dotProduct(bq, dir);
//        if (ts > 0) {
//            double pseudoDistance = crossProduct(bq, dir);
//            if (fabs(pseudoDistance) <= fabs(distance.distance)) {
//                distance.distance = pseudoDistance;
//                distance.dot = 0;
//            }
//        }
//    }
//}

LinearSegment::LinearSegment(Point2 p0, Point2 p1) {
    p[0] = p0;
    p[1] = p1;
}

QuadraticSegment::QuadraticSegment(Point2 p0, Point2 p1, Point2 p2) {
    if (p1 == p0 || p1 == p2)
        p1 = 0.5*(p0+p2);
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
}

CubicSegment::CubicSegment(Point2 p0, Point2 p1, Point2 p2, Point2 p3) {
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
    p[3] = p3;
}

Point2 LinearSegment::point(double param) const {
    return mix(p[0], p[1], param);
}

Point2 QuadraticSegment::point(double param) const {
    return mix(mix(p[0], p[1], param), mix(p[1], p[2], param), param);
}

Point2 CubicSegment::point(double param) const {
    Vector2 p12 = mix(p[1], p[2], param);
    return mix(mix(mix(p[0], p[1], param), p12, param), mix(p12, mix(p[2], p[3], param), param), param);
}

Vector2 LinearSegment::direction(double param) const {
    return p[1]-p[0];
}

Vector2 QuadraticSegment::direction(double param) const {
    return mix(p[1]-p[0], p[2]-p[1], param);
}

Vector2 CubicSegment::direction(double param) const {
    Vector2 tangent = mix(mix(p[1]-p[0], p[2]-p[1], param), mix(p[2]-p[1], p[3]-p[2], param), param);
    if (!tangent) {
        if (param == 0) return p[2]-p[0];
        if (param == 1) return p[3]-p[1];
    }
    return tangent;
}

double LinearSegment::signedDistance(Point2 origin) const {
	double param;
    Vector2 aq = origin-p[0];
    Vector2 ab = p[1]-p[0];
    param = dotProduct(aq, ab)/dotProduct(ab, ab);
    Vector2 eq = p[param > .5]-origin;
    double endpointDistance = eq.squareLength();
    if (param > 0 && param < 1) {
        double orthoDistance = dotProduct(ab.getOrthonormal(false), aq);
		orthoDistance *= orthoDistance;
        if (orthoDistance < endpointDistance)
            return orthoDistance;
    }
    return endpointDistance;
}

double QuadraticSegment::signedDistance(Point2 origin) const {
    Vector2 qa = p[0]-origin;
    Vector2 ab = p[1]-p[0];
    Vector2 br = p[0]+p[2]-p[1]-p[1];
    double a = dotProduct(br, br);
    double b = 3*dotProduct(ab, br);
    double c = 2*dotProduct(ab, ab)+dotProduct(qa, br);
    double d = dotProduct(qa, ab);
    double t[3];
    int solutions = solveCubic(t, a, b, c, d);

    double minDistance = qa.squareLength(); // distance from A
    {
        double distance = (p[2]-origin).squareLength(); // distance from B
        if (distance < minDistance) {
            minDistance = distance;
        }
    }
    for (int i = 0; i < solutions; ++i) {
        if (t[i] > 0 && t[i] < 1) {
            Point2 endpoint = p[0]+2*t[i]*ab+t[i]*t[i]*br;
            double distance = (endpoint-origin).squareLength();
            if (distance <= minDistance) {
                minDistance = distance;
            }
        }
    }

	return minDistance;
}

double CubicSegment::signedDistance(Point2 origin) const {
    Vector2 qa = p[0]-origin;
    Vector2 ab = p[1]-p[0];
    Vector2 br = p[2]-p[1]-ab;
    Vector2 as = (p[3]-p[2])-(p[2]-p[1])-br;

    Vector2 epDir = direction(0);
    double minDistance = qa.squareLength(); // distance from A
    {
        epDir = direction(1);
        double distance = (p[3]-origin).squareLength(); // distance from B
        if (distance < minDistance) {
            minDistance = distance;
        }
    }
    // Iterative minimum distance search
    for (int i = 0; i <= MSDFGEN_CUBIC_SEARCH_STARTS; ++i) {
        double t = (double) i*MSDFGEN_CUBIC_SEARCH_STARTS_REV;
        for (int step = 0;; ++step) {
            Vector2 qpt = point(t)-origin;
            double distance = qpt.squareLength();
            if (distance < minDistance) {
                minDistance = distance;
            }
            if (step == MSDFGEN_CUBIC_SEARCH_STEPS)
                break;
            // Improve t
            Vector2 d1 = 3*as*t*t+6*br*t+3*ab;
            Vector2 d2 = 6*as*t+6*br;
            t -= dotProduct(qpt, d1)/(dotProduct(d1, d1)+dotProduct(qpt, d2));
            if (t < 0 || t > 1)
                break;
        }
    }

	return minDistance;
}

static void pointBounds(Point2 p, double &l, double &b, double &r, double &t) {
    if (p.x < l) l = p.x;
    if (p.y < b) b = p.y;
    if (p.x > r) r = p.x;
    if (p.y > t) t = p.y;
}

void LinearSegment::bounds(double &l, double &b, double &r, double &t) const {
    pointBounds(p[0], l, b, r, t);
    pointBounds(p[1], l, b, r, t);
}

void QuadraticSegment::bounds(double &l, double &b, double &r, double &t) const {
    pointBounds(p[0], l, b, r, t);
    pointBounds(p[2], l, b, r, t);
    Vector2 bot = (p[1]-p[0])-(p[2]-p[1]);
    if (bot.x) {
        double param = (p[1].x-p[0].x)/bot.x;
        if (param > 0 && param < 1)
            pointBounds(point(param), l, b, r, t);
    }
    if (bot.y) {
        double param = (p[1].y-p[0].y)/bot.y;
        if (param > 0 && param < 1)
            pointBounds(point(param), l, b, r, t);
    }
}

void CubicSegment::bounds(double &l, double &b, double &r, double &t) const {
    pointBounds(p[0], l, b, r, t);
    pointBounds(p[3], l, b, r, t);
    Vector2 a0 = p[1]-p[0];
    Vector2 a1 = 2*(p[2]-p[1]-a0);
    Vector2 a2 = p[3]-3*p[2]+3*p[1]-p[0];
    double params[2];
    int solutions;
    solutions = solveQuadratic(params, a2.x, 1.0/a2.x, a1.x, a0.x);
    for (int i = 0; i < solutions; ++i)
        if (params[i] > 0 && params[i] < 1)
            pointBounds(point(params[i]), l, b, r, t);
    solutions = solveQuadratic(params, a2.y, 1.0/a2.y, a1.y, a0.y);
    for (int i = 0; i < solutions; ++i)
        if (params[i] > 0 && params[i] < 1)
            pointBounds(point(params[i]), l, b, r, t);
}

void LinearSegment::moveStartPoint(Point2 to) {
    p[0] = to;
}

void QuadraticSegment::moveStartPoint(Point2 to) {
    Vector2 origSDir = p[0]-p[1];
    Point2 origP1 = p[1];
    p[1] += crossProduct(p[0]-p[1], to-p[0])/crossProduct(p[0]-p[1], p[2]-p[1])*(p[2]-p[1]);
    p[0] = to;
    if (dotProduct(origSDir, p[0]-p[1]) < 0)
        p[1] = origP1;
}

void CubicSegment::moveStartPoint(Point2 to) {
    p[1] += to-p[0];
    p[0] = to;
}

void LinearSegment::moveEndPoint(Point2 to) {
    p[1] = to;
}

void QuadraticSegment::moveEndPoint(Point2 to) {
    Vector2 origEDir = p[2]-p[1];
    Point2 origP1 = p[1];
    p[1] += crossProduct(p[2]-p[1], to-p[2])/crossProduct(p[2]-p[1], p[0]-p[1])*(p[0]-p[1]);
    p[2] = to;
    if (dotProduct(origEDir, p[2]-p[1]) < 0)
        p[1] = origP1;
}

void CubicSegment::moveEndPoint(Point2 to) {
    p[2] += to-p[3];
    p[3] = to;
}

void LinearSegment::splitInThirds(EdgeSegment &part1, EdgeSegment &part2, EdgeSegment &part3) const {
    part1 = EdgeSegment(LinearSegment(p[0], point(1/3.)));
    part2 = EdgeSegment(LinearSegment(point(1/3.), point(2/3.)));
    part3 = EdgeSegment(LinearSegment(point(2/3.), p[1]));
}

void QuadraticSegment::splitInThirds(EdgeSegment &part1, EdgeSegment &part2, EdgeSegment &part3) const {
    part1 = EdgeSegment(QuadraticSegment(p[0], mix(p[0], p[1], 1/3.), point(1/3.)));
    part2 = EdgeSegment(QuadraticSegment(point(1/3.), mix(mix(p[0], p[1], 5/9.), mix(p[1], p[2], 4/9.), .5), point(2/3.)));
    part3 = EdgeSegment(QuadraticSegment(point(2/3.), mix(p[1], p[2], 2/3.), p[2]));
}

void CubicSegment::splitInThirds(EdgeSegment &part1, EdgeSegment &part2, EdgeSegment &part3) const {
    part1 = EdgeSegment(CubicSegment(p[0], p[0] == p[1] ? p[0] : mix(p[0], p[1], 1/3.), mix(mix(p[0], p[1], 1/3.), mix(p[1], p[2], 1/3.), 1/3.), point(1/3.)));
    part2 = EdgeSegment(CubicSegment(point(1/3.),
        mix(mix(mix(p[0], p[1], 1/3.), mix(p[1], p[2], 1/3.), 1/3.), mix(mix(p[1], p[2], 1/3.), mix(p[2], p[3], 1/3.), 1/3.), 2/3.),
        mix(mix(mix(p[0], p[1], 2/3.), mix(p[1], p[2], 2/3.), 2/3.), mix(mix(p[1], p[2], 2/3.), mix(p[2], p[3], 2/3.), 2/3.), 1/3.),
        point(2/3.)));
    part3 = EdgeSegment(CubicSegment(point(2/3.), mix(mix(p[1], p[2], 2/3.), mix(p[2], p[3], 2/3.), 2/3.), p[2] == p[3] ? p[3] : mix(p[2], p[3], 2/3.), p[3]));
}
    
bool LinearSegment::isDegenerate() const {
    return p[0].same(p[1]);
}
    
bool QuadraticSegment::isDegenerate() const {
    return p[0].same(p[2]);
}

bool CubicSegment::isDegenerate() const {
    return p[0].same(p[3]) && (p[0].same(p[1]) || p[2].same(p[1]));
}

/// Check how many times a ray from point R extending to the +X direction intersects
/// the given segment:
///  0 = no intersection or co-linear
/// +1 = intersection increasing in the Y axis
/// -1 = intersection decreasing in the Y axis
static int crossLine(const Point2& r, const Point2& p0, const Point2& p1, CrossingCallback* cb) {
    if (r.y < min(p0.y, p1.y))
        return 0;
    if (r.y >= max(p0.y, p1.y))
        return 0;
    if (r.x >= max(p0.x, p1.x))
        return 0;
    // Intersect the line at r.y and see if the intersection is before or after the origin.
    double xintercept = (p0.x + (r.y - p0.y) * (p1.x - p0.x) / (p1.y - p0.y));
    if (r.x < xintercept) {
        int w = (p0.y < p1.y) ? 1 : -1;
        if( cb != NULL ) {
            cb->intersection(Point2(xintercept, r.y), w);
        }
        return w;
    }
    return 0;
}

/// Check how many times a ray from point R extending to the +X direction intersects
/// the given segment:
///  0 = no intersection or co-linear
/// +1 = for each intersection increasing in the Y axis
/// -1 = for each intersection decreasing in the Y axis
static int crossQuad(const Point2& r, const Point2& p0, const Point2& c0, const Point2& p1, int depth, CrossingCallback* cb) {
    if (r.y < min(p0.y, min(c0.y, p1.y)))
        return 0;
    if (r.y > max(p0.y, max(c0.y, p1.y)))
        return 0;
    if (r.x >= max(p0.x, max(c0.x, p1.x)))
        return 0;

	if (p1.y == p0.y && c0.y == p0.y) {
		// in fact, this segment is a line and since r.y == p1.y == p0.y == c0.y,
		// the ray has an infinite number of the intersection points
		return 0;
	}

    // Recursively subdivide the curve to find the intersection point(s). If we haven't
    // converged on a solution by a given depth, just treat it as a linear segment
    // and call the approximation good enough.
    if( depth > 30 )
        return crossLine(r, p0, p1, cb);

    depth++;

    Point2 mc0 = (p0 + c0) * 0.5;
    Point2 mc1 = (c0 + p1) * 0.5;
    Point2 mid = (mc0 + mc1) * 0.5;

    return crossQuad(r, p0, mc0, mid, depth, cb) + crossQuad(r, mid, mc1, p1, depth, cb);
}

/// Check how many times a ray from point R extending to the +X direction intersects
/// the given segment:
///  0 = no intersection or co-linear
/// +1 = for each intersection increasing in the Y axis
/// -1 = for each intersection decreasing in the Y axis
static int crossCubic(const Point2& r, const Point2& p0, const Point2& c0, const Point2& c1, const Point2& p1, int depth, CrossingCallback* cb) {
    if (r.y < min(p0.y, min(c0.y, min(c1.y, p1.y))))
        return 0;
    if (r.y > max(p0.y, max(c0.y, max(c1.y, p1.y))))
        return 0;
    if (r.x >= max(p0.x, max(c0.x, max(c1.x, p1.x))))
        return 0;
    
    if (p0.y == c0.y && p0.y == c1.y && p0.y == p1.y)
        return 0;
    
    // Recursively subdivide the curve to find the intersection point(s). If we haven't
    // converged on a solution by a given depth, just treat it as a linear segment
    // and call the approximation good enough.
    if( depth > 30 )
        return crossLine(r, p0, p1, cb);
    
    depth++;
    
    Point2 mid = (c0 + c1) * 0.5;
    Point2 c00 = (p0 + c0) * 0.5;
    Point2 c11 = (c1 + p1) * 0.5;
    Point2 c01 = (c00 + mid) * 0.5;
    Point2 c10 = (c11 + mid) * 0.5;
    
    mid = (c01 + c10) * 0.5;
    
    return crossCubic(r, p0, c00, c01, mid, depth, cb) + crossCubic(r, mid, c10, c11, p1, depth, cb);
}
    
    
int LinearSegment::crossings(const Point2 &r, CrossingCallback* cb) const {
    return crossLine(r, p[0], p[1], cb);
}

    
int QuadraticSegment::crossings(const Point2 &r, CrossingCallback* cb) const {
    return crossQuad(r, p[0], p[1], p[2], 0, cb);
}

    
int CubicSegment::crossings(const Point2 &r, CrossingCallback* cb) const {
    return crossCubic(r, p[0], p[1], p[2], p[3], 0, cb);
}
    
}
