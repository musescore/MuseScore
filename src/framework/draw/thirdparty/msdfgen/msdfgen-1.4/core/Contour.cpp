
#include "Contour.h"

#include "arithmetics.hpp"

namespace msdfgen {

static double shoelace(const Point2 &a, const Point2 &b) {
    return (b.x-a.x)*(a.y+b.y);
}

void Contour::addEdge(const EdgeSegment &edge) {
    edges.push_back(edge);
}

#ifdef MSDFGEN_USE_CPP11
void Contour::addEdge(EdgeSegment &&edge) {
    edges.push_back((EdgeSegment &&) edge);
}
#endif

EdgeSegment & Contour::addEdge() {
    edges.resize(edges.size()+1);
    return edges[edges.size()-1];
}

void Contour::bounds(double &l, double &b, double &r, double &t) const {
    for (std::vector<EdgeSegment>::const_iterator edge = edges.begin(); edge != edges.end(); ++edge)
        edge->bounds(l, b, r, t);
}

int Contour::winding() const {
    if (edges.empty())
        return 0;
    double total = 0;
    if (edges.size() == 1) {
        Point2 a = edges[0].point(0), b = edges[0].point(1/3.), c = edges[0].point(2/3.);
        total += shoelace(a, b);
        total += shoelace(b, c);
        total += shoelace(c, a);
    } else if (edges.size() == 2) {
        Point2 a = edges[0].point(0), b = edges[0].point(.5), c = edges[1].point(0), d = edges[1].point(.5);
        total += shoelace(a, b);
        total += shoelace(b, c);
        total += shoelace(c, d);
        total += shoelace(d, a);
    } else {
        Point2 prev = edges.back().point(0);
        for (std::vector<EdgeSegment>::const_iterator edge = edges.begin(); edge != edges.end(); ++edge) {
            Point2 cur = edge->point(0);
            total += shoelace(prev, cur);
            prev = cur;
        }
    }
    return sign(total);
}

}
