
#pragma once

#include "Vector2.h"
#include "SignedDistance.h"
//#include "EdgeColor.h"
#include <cassert>

namespace msdfgen {

// Parameters for iterative search of closest point on a cubic Bezier curve. Increase for higher precision.
#define MSDFGEN_CUBIC_SEARCH_STARTS 4
#define MSDFGEN_CUBIC_SEARCH_STARTS_REV 0.25
#define MSDFGEN_CUBIC_SEARCH_STEPS 4

class EdgeSegment;

class CrossingCallback {
public:
	virtual ~CrossingCallback() {}
	/// Callback for receiving intersection points. Winding is either:
	/// +1 if the segment is increasing in the Y axis
	/// -1 if the segment is decreasing in the Y axis
	virtual void intersection(const Point2& p, int winding) = 0;
};

/// A line segment.
class LinearSegment {

public:
	Point2 p[2];

	LinearSegment() = delete;
	LinearSegment(Point2 p0, Point2 p1);
	Point2 point(double param) const;
	Vector2 direction(double param) const;
	double signedDistance(Point2 origin) const;
	void bounds(double &l, double &b, double &r, double &t) const;

	void moveStartPoint(Point2 to);
	void moveEndPoint(Point2 to);
	void splitInThirds(EdgeSegment &part1, EdgeSegment &part2, EdgeSegment &part3) const;

	bool isDegenerate() const;

	int crossings(const Point2 &r, CrossingCallback *cb = NULL) const;
};

/// A quadratic Bezier curve.
class QuadraticSegment {

public:
	Point2 p[3];

	QuadraticSegment() = delete;
	QuadraticSegment(Point2 p0, Point2 p1, Point2 p2);
	Point2 point(double param) const;
	Vector2 direction(double param) const;
    double signedDistance(Point2 origin) const;
	void bounds(double &l, double &b, double &r, double &t) const;

	void moveStartPoint(Point2 to);
	void moveEndPoint(Point2 to);
	void splitInThirds(EdgeSegment &part1, EdgeSegment &part2, EdgeSegment &part3) const;

	bool isDegenerate() const;

	int crossings(const Point2 &r, CrossingCallback *cb = NULL) const;
};

/// A cubic Bezier curve.
class CubicSegment {

public:
	Point2 p[4];

	CubicSegment() = delete;
	CubicSegment(Point2 p0, Point2 p1, Point2 p2, Point2 p3);
	Point2 point(double param) const;
	Vector2 direction(double param) const;
    double signedDistance(Point2 origin) const;
	void bounds(double &l, double &b, double &r, double &t) const;

	void moveStartPoint(Point2 to);
	void moveEndPoint(Point2 to);
	void splitInThirds(EdgeSegment &part1, EdgeSegment &part2, EdgeSegment &part3) const;

	bool isDegenerate() const;

	int crossings(const Point2 &r, CrossingCallback *cb = NULL) const;
};

/// An abstract edge segment.
class EdgeSegment {
public:
	union Segments
	{
		LinearSegment linear;
		QuadraticSegment quadratic;
		CubicSegment cubic;

		Segments() {}
		Segments(const LinearSegment &ls) : linear(ls) {}
		Segments(const QuadraticSegment &qs) : quadratic(qs) {}
		Segments(const CubicSegment &cs) : cubic(cs) {}
	} segments;

	enum class ActualType { Undefined, Linear, Quadratic, Cubic } actualType = ActualType::Undefined;

	//EdgeColor color;

	EdgeSegment() { }
	EdgeSegment(LinearSegment &&ls) : segments(ls), actualType(ActualType::Linear) {}
	EdgeSegment(QuadraticSegment &&qs) : segments(qs), actualType(ActualType::Quadratic) {}
	EdgeSegment(CubicSegment &&cs) : segments(cs), actualType(ActualType::Cubic) {}

	bool isUndefined() const
	{
		return actualType == ActualType::Undefined;
	}

#define RETBYTYPE(func, ...) \
	if (actualType == ActualType::Linear) return segments.linear.func(__VA_ARGS__); \
	else if (actualType == ActualType::Quadratic) return segments.quadratic.func(__VA_ARGS__); \
	else { assert(actualType == ActualType::Cubic); return segments.cubic.func(__VA_ARGS__); }

	/// Returns the point on the edge specified by the parameter (between 0 and 1).
	Point2 point(double param) const
	{
		RETBYTYPE(point, param);
	}
	/// Returns the direction the edge has at the point specified by the parameter.
	Vector2 direction(double param) const
	{
		RETBYTYPE(direction, param);
	}
	/// Returns the minimum signed distance between origin and the edge.
    double signedDistance(Point2 origin) const
	{
		RETBYTYPE(signedDistance, origin);
	}
	/// Converts a previously retrieved signed distance from origin to pseudo-distance.
	//void distanceToPseudoDistance(SignedDistance &distance, Point2 origin, double param) const;
	/// Adjusts the bounding box to fit the edge segment.
	void bounds(double &l, double &b, double &r, double &t) const
	{
		RETBYTYPE(bounds, l, b, r, t);
	}

	/// Moves the start point of the edge segment.
	void moveStartPoint(Point2 to)
	{
		RETBYTYPE(moveStartPoint, to);
	}
	/// Moves the end point of the edge segment.
	void moveEndPoint(Point2 to)
	{
		RETBYTYPE(moveEndPoint, to);
	}
	/// Splits the edge segments into thirds which together represent the original edge.
	void splitInThirds(EdgeSegment &part1, EdgeSegment &part2, EdgeSegment &part3) const
	{
		RETBYTYPE(splitInThirds, part1, part2, part3);
	}

	bool isDegenerate() const
	{
		RETBYTYPE(isDegenerate);
	}
    
    /// Calculate how many times the segment intersects the infinite ray that extends
    /// to the right (+X) from the given point. Returns:
    ///  0 for no intersection (or co-linear)
    /// +1 for each intersection where Y is increasing
    /// -1 for each intersection where Y is decreasing.
	int crossings(const Point2 &r, CrossingCallback *cb = NULL) const
	{
		RETBYTYPE(crossings, r, cb);
	}
    
#undef RETBYTYPE
};

}
