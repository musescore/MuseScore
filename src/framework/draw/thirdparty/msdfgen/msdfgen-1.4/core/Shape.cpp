
#include "Shape.h"

namespace msdfgen {

Shape::Shape() : inverseYAxis(false), fillRule(FillRule::NonZero) { }

void Shape::addContour(const Contour &contour) {
    contours.push_back(contour);
}

#ifdef MSDFGEN_USE_CPP11
void Shape::addContour(Contour &&contour) {
    contours.push_back((Contour &&) contour);
}
#endif

Contour & Shape::addContour() {
    contours.resize(contours.size()+1);
    return contours[contours.size()-1];
}

bool Shape::validate() const {
    for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour) {
        if (!contour->edges.empty()) {
            Point2 corner = contour->edges.back().point(1);
            for (std::vector<EdgeSegment>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
                if (edge->isUndefined())
                    return false;
                if (!edge->point(0).same(corner) )
                    return false;
                corner = edge->point(1);
            }
        }
    }
    return true;
}

void Shape::normalize() {
    for (std::vector<Contour>::iterator contour = contours.begin(); contour != contours.end(); ++contour) {
        // First, erase all degenerate edges.
        std::vector<EdgeSegment>::iterator edge_it = contour->edges.begin();
        while( edge_it != contour->edges.end() ) {
            if( edge_it->isDegenerate() ) {
                edge_it = contour->edges.erase(edge_it);
            }
            else {
                edge_it++;
            }
        }
        
        if (contour->edges.size() == 1) {
			EdgeSegment currentSegment = contour->edges[0];
			contour->edges.resize(3);
			currentSegment.splitInThirds(contour->edges[0], contour->edges[1], contour->edges[2]);
        }
        else {
            
            // Make sure that start points match end points exactly or we'll get artifacts.
            size_t n = contour->edges.size();
            for(size_t i = 0; i < n; i++ )
            {
                EdgeSegment *s1 = &contour->edges[i];
                EdgeSegment *s2 = &contour->edges[(i + 1) % n];
                if( s1->point(1) != s2->point(0) )
                {
                    s1->moveEndPoint(s2->point(0));
                }
            }
        }
    }
}

void Shape::bounds(double &l, double &b, double &r, double &t) const {
    for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour)
        contour->bounds(l, b, r, t);
}

void Shape::mergeContours()
{
	if (contours.size() < 2)
		return;
	for (size_t index = 1, size = contours.size(); index < size; ++index)
		contours.front().edges.insert(contours.front().edges.begin(), contours[index].edges.begin(), contours[index].edges.end());
	contours.resize(1);
}

}
