
#pragma once

#include "arithmetics.hpp"
#include <cstdlib>

namespace msdfgen {

/**
* A 2-dimensional euclidean vector with double precision.
* Implementation based on the Vector2 template from Artery Engine.
* @author Viktor Chlumsky
*/
struct Vector2 {

    static constexpr double Epsilon = 0.01;
    
    double x, y;

    Vector2(double val = 0) : x(val), y(val) { }
    Vector2(double x, double y) : x(x), y(y) { }
    /// Sets the vector to zero.
    void reset();
    /// Sets individual elements of the vector.
    void set(double x, double y);
	/// Returns the vector's square length.
	double squareLength() const {
		return x * x + y * y;
	}
    /// Returns the vector's length.
    double length() const {
		return approxSquareRoot(x*x + y*y);
	}
    /// Returns the angle of the vector in radians (atan2).
    double direction() const;
    /// Returns the normalized vector - one that has the same direction but unit length.
    Vector2 normalize(bool allowZero = false) const;
    /// Returns a vector with the same length that is orthogonal to this one.
    Vector2 getOrthogonal(bool polarity = true) const;
    /// Returns a vector with unit length that is orthogonal to this one.
    Vector2 getOrthonormal(bool polarity = true, bool allowZero = false) const;
    /// Returns a vector projected along this one.
    Vector2 project(const Vector2 &vector, bool positive = false) const;
    operator const void *() const;
    bool operator!() const;
    bool operator==(const Vector2 &other) const;
    bool operator!=(const Vector2 &other) const;
    Vector2 operator+() const;
    Vector2 operator-() const;
    Vector2 operator+(const Vector2 &other) const;
    Vector2 operator-(const Vector2 &other) const;
    Vector2 operator*(const Vector2 &other) const;
    Vector2 operator/(const Vector2 &other) const;
    Vector2 operator*(double value) const;
    Vector2 operator/(double value) const;
    Vector2 & operator+=(const Vector2 &other);
    Vector2 & operator-=(const Vector2 &other);
    Vector2 & operator*=(const Vector2 &other);
    Vector2 & operator/=(const Vector2 &other);
    Vector2 & operator*=(double value);
    Vector2 & operator/=(double value);
    bool same(const Vector2 &other) const;
    /// Dot product of two vectors.
    friend double dotProduct(const Vector2 &a, const Vector2 &b) {
		return a.x*b.x + a.y*b.y;
	}
    /// A special version of the cross product for 2D vectors (returns scalar value).
    friend double crossProduct(const Vector2 &a, const Vector2 &b) {
		return a.x*b.y - a.y*b.x;
	}
    friend Vector2 operator*(double value, const Vector2 &vector);
    friend Vector2 operator/(double value, const Vector2 &vector);

};

/// A vector may also represent a point, which shall be differentiated semantically using the alias Point2.
typedef Vector2 Point2;

}
