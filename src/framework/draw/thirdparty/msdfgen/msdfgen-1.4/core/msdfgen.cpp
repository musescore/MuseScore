#ifdef PLATFORM_WINDOWS
#pragma optimize("t", on)
#endif

#include "../msdfgen.h"

#include "arithmetics.hpp"
#include <algorithm> // for std::sort

using namespace std; // to access signbit, not all libraries put signbit in std

namespace msdfgen {


const char* getVersion() {
    return MSDFGEN_VERSION;
}


struct MultiDistance {
    double r, g, b;
    double med;
};
    
/// A utility structure for holding winding spans for a single horizontal scanline.
/// First initialize a row by calling collect(), then use advance() to walk the row
/// and determine "inside"-ness as you go.
struct WindingSpanner: public CrossingCallback {
    
    std::vector<std::pair<double, int>> crossings;
    
    FillRule fillRule;
    
    WindingSpanner(): curW(0) {
        curSpan = crossings.cend();
    }

	void clear() {
		curW = 0;
		crossings.clear();
		curSpan = crossings.cend();
	}
    
    void collect(const Shape& shape, const Point2& p) {
		assert(shape.contours.size() == 1);
        fillRule = shape.fillRule;
        crossings.clear();
        for (const EdgeSegment &e : shape.contours[0].edges) {
            e.crossings(p, this);
        }
        
        // Make sure we've collected them all in increasing x order.
        std::sort(crossings.begin(), crossings.end(), compareX);
        
        // And set up a traversal.
        if( fillRule == FillRule::EvenOdd )
            curW = 1;
        else
            curW = 0;
        curSpan = crossings.cbegin();
    }
    
    /// Scan to the provided X coordinate and use the winding rule to return the current sign as either:
    /// -1 = pixel is "outside" the shape (i.e. not filled)
    /// +1 = pixel is "inside" the shape (i.e. filled)
    /// (Note: This is actually the inverse of the final distance field sign.)
    int advanceTo(double x) {
        while( curSpan != crossings.cend() && x > curSpan->first ) {
            curW += curSpan->second;
            ++curSpan;
        }

        switch( fillRule ) {
            case FillRule::NonZero:
                return curW != 0 ? 1 : -1;
            case FillRule::EvenOdd:
                return curW % 2 == 0 ? 1 : -1;
            case FillRule::None:
                return curSpan != crossings.cend() ? sign(curSpan->second) : 0;
        }

        return 0;
    }
    
private:

    int curW;
    
    std::vector<std::pair<double, int>>::const_iterator curSpan;
    
    void intersection(const Point2& p, int winding) {
        crossings.push_back(std::pair<double, int>(p.x, winding));
    }

    static bool compareX(const std::pair<double,int>& a, std::pair<double,int>& b) {
        return a.first < b.first;
    }
};

namespace
{
	WindingSpanner Spanner;
}

void generateSDF(Bitmap<unsigned char> &output, const Shape &shape, double bound_l, double range, const Vector2 &scale, const Vector2 &translate) {
	assert(shape.contours.size() == 1);
	int w = output.width(), h = output.height();
	Vector2 scaleRev = 1.0 / scale;
	double rangeRev = 1.0 / range;
	double dy = 0.5;
	bound_l -= 0.5;
    
	Spanner.clear();
    
	for (int y = 0; y < h; ++y, dy += 1.0) {
		double dx = 0.5;
		int row = shape.inverseYAxis ? h-y-1 : y;

		// Start slightly off the -X edge so we ensure we find all spans.
		Spanner.collect(shape, Vector2(bound_l, dy*scaleRev.y - translate.y));

		for (int x = 0; x < w; ++x, dx += 1.0) {
			Point2 p = Vector2(dx, dy)*scaleRev - translate;
			double minDistance = INFINITY;

			for (const EdgeSegment &edge : shape.contours[0].edges) {
				double distance = edge.signedDistance(p);
				if (distance < minDistance)
					minDistance = distance;
			}

			minDistance = approxSquareRoot(minDistance);
			minDistance *= Spanner.advanceTo(p.x);
			output(x, row) = (unsigned char)std::clamp<double>((minDistance * rangeRev + 0.5) * 0x100, 0, 255);
		}
	}
}

//static inline bool pixelClash(const FloatRGB &a, const FloatRGB &b, double threshold) {
//    // Only consider pair where both are on the inside or both are on the outside
//    bool aIn = (a.r > .5f)+(a.g > .5f)+(a.b > .5f) >= 2;
//    bool bIn = (b.r > .5f)+(b.g > .5f)+(b.b > .5f) >= 2;
//    if (aIn != bIn) return false;
//    // If the change is 0 <-> 1 or 2 <-> 3 channels and not 1 <-> 1 or 2 <-> 2, it is not a clash
//    if ((a.r > .5f && a.g > .5f && a.b > .5f) || (a.r < .5f && a.g < .5f && a.b < .5f)
//        || (b.r > .5f && b.g > .5f && b.b > .5f) || (b.r < .5f && b.g < .5f && b.b < .5f))
//        return false;
//    // Find which color is which: _a, _b = the changing channels, _c = the remaining one
//    float aa, ab, ba, bb, ac, bc;
//    if ((a.r > .5f) != (b.r > .5f) && (a.r < .5f) != (b.r < .5f)) {
//        aa = a.r, ba = b.r;
//        if ((a.g > .5f) != (b.g > .5f) && (a.g < .5f) != (b.g < .5f)) {
//            ab = a.g, bb = b.g;
//            ac = a.b, bc = b.b;
//        } else if ((a.b > .5f) != (b.b > .5f) && (a.b < .5f) != (b.b < .5f)) {
//            ab = a.b, bb = b.b;
//            ac = a.g, bc = b.g;
//        } else
//            return false; // this should never happen
//    } else if ((a.g > .5f) != (b.g > .5f) && (a.g < .5f) != (b.g < .5f)
//        && (a.b > .5f) != (b.b > .5f) && (a.b < .5f) != (b.b < .5f)) {
//        aa = a.g, ba = b.g;
//        ab = a.b, bb = b.b;
//        ac = a.r, bc = b.r;
//    } else
//        return false;
//    // Find if the channels are in fact discontinuous
//    return (fabsf(aa-ba) >= threshold)
//        && (fabsf(ab-bb) >= threshold)
//        && fabsf(ac-.5f) >= fabsf(bc-.5f); // Out of the pair, only flag the pixel farther from a shape edge
//}

//void msdfErrorCorrection(Bitmap<FloatRGB> &output, const Vector2 &threshold) {
//    std::vector<std::pair<int, int> > clashes;
//    int w = output.width(), h = output.height();
//    for (int y = 0; y < h; ++y)
//        for (int x = 0; x < w; ++x) {
//            if ((x > 0 && pixelClash(output(x, y), output(x-1, y), threshold.x))
//                || (x < w-1 && pixelClash(output(x, y), output(x+1, y), threshold.x))
//                || (y > 0 && pixelClash(output(x, y), output(x, y-1), threshold.y))
//                || (y < h-1 && pixelClash(output(x, y), output(x, y+1), threshold.y)))
//                clashes.push_back(std::make_pair(x, y));
//        }
//    for (std::vector<std::pair<int, int> >::const_iterator clash = clashes.begin(); clash != clashes.end(); ++clash) {
//        FloatRGB &pixel = output(clash->first, clash->second);
//        float med = median(pixel.r, pixel.g, pixel.b);
//        pixel.r = med, pixel.g = med, pixel.b = med;
//    }
//}

//void generatePseudoSDF(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate) {
//    int contourCount = shape.contours.size();
//    int w = output.width(), h = output.height();
//    
//    WindingSpanner spanner;
//    double bound_l, bound_t, bound_b, bound_r;
//    shape.bounds(bound_l, bound_b, bound_r, bound_t);
//    
//    {
//        for (int y = 0; y < h; ++y) {
//            int row = shape.inverseYAxis ? h-y-1 : y;
//            
//            // Start slightly off the -X edge so we ensure we find all spans.
//            spanner.collect(shape, Vector2(bound_l - 0.5, (y + 0.5)/scale.y - translate.y));
//
//            for (int x = 0; x < w; ++x) {
//                Point2 p = Vector2(x+.5, y+.5)/scale-translate;
//                
//                SignedDistance sd = SignedDistance::INFINITE;
//                const EdgeHolder *nearEdge = NULL;
//                double nearParam = 0;
//                
//                std::vector<Contour>::const_iterator contour = shape.contours.begin();
//                for (int i = 0; i < contourCount; ++i, ++contour) {
//                    for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
//                        double param;
//                        SignedDistance distance = (*edge)->signedDistance(p, param);
//                        if (distance < sd) {
//                            sd = distance;
//                            nearEdge = &*edge;
//                            nearParam = param;
//                        }
//                    }
//                }
//                
//                if (nearEdge)
//                    (*nearEdge)->distanceToPseudoDistance(sd, p, nearParam);
//                
//                double d = fabs(sd.distance) * spanner.advanceTo(p.x);
//                output(x, row) = float(d / range + 0.5);
//            }
//        }
//    }
//}
//
//void generateMSDF(Bitmap<FloatRGB> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, double edgeThreshold) {
//    int contourCount = shape.contours.size();
//    int w = output.width(), h = output.height();
//    
//    WindingSpanner spanner;
//    double bound_l, bound_t, bound_b, bound_r;
//    shape.bounds(bound_l, bound_b, bound_r, bound_t);
//    
//    {
//        std::vector<MultiDistance> contourSD;
//        contourSD.resize(contourCount);
//        for (int y = 0; y < h; ++y) {
//            int row = shape.inverseYAxis ? h-y-1 : y;
//            
//            // Start slightly off the -X edge so we ensure we find all spans.
//            spanner.collect(shape, Vector2(bound_l - 0.5, (y + 0.5)/scale.y - translate.y));
//            
//            for (int x = 0; x < w; ++x) {
//                Point2 p = Vector2(x+.5, y+.5)/scale-translate;
//                
//                struct EdgePoint {
//                    SignedDistance minDistance;
//                    const EdgeHolder *nearEdge;
//                    double nearParam;
//                } sr, sg, sb;
//                sr.nearEdge = sg.nearEdge = sb.nearEdge = NULL;
//                sr.nearParam = sg.nearParam = sb.nearParam = 0;
//                int realSign = spanner.advanceTo(p.x);
//                
//                std::vector<Contour>::const_iterator contour = shape.contours.begin();
//                for (int i = 0; i < contourCount; ++i, ++contour) {
//                    EdgePoint r, g, b;
//                    r.nearEdge = g.nearEdge = b.nearEdge = NULL;
//                    r.nearParam = g.nearParam = b.nearParam = 0;
//                    
//                    for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
//                        
//                        double param;
//                        SignedDistance distance = (*edge)->signedDistance(p, param);
//                        if ((*edge)->color&RED && distance < r.minDistance) {
//                            r.minDistance = distance;
//                            r.nearEdge = &*edge;
//                            r.nearParam = param;
//                        }
//                        if ((*edge)->color&GREEN && distance < g.minDistance) {
//                            g.minDistance = distance;
//                            g.nearEdge = &*edge;
//                            g.nearParam = param;
//                        }
//                        if ((*edge)->color&BLUE && distance < b.minDistance) {
//                            b.minDistance = distance;
//                            b.nearEdge = &*edge;
//                            b.nearParam = param;
//                        }
//                    }
//                    
//                    if (r.minDistance < sr.minDistance) {
//                        sr = r;
//                    }
//                    if (g.minDistance < sg.minDistance) {
//                        sg = g;
//                    }
//                    if (b.minDistance < sb.minDistance) {
//                        sb = b;
//                    }
//                }
//                if (sr.nearEdge)
//                    (*sr.nearEdge)->distanceToPseudoDistance(sr.minDistance, p, sr.nearParam);
//                if (sg.nearEdge)
//                    (*sg.nearEdge)->distanceToPseudoDistance(sg.minDistance, p, sg.nearParam);
//                if (sb.nearEdge)
//                    (*sb.nearEdge)->distanceToPseudoDistance(sb.minDistance, p, sb.nearParam);
//
//                double dr = sr.minDistance.distance;
//                double dg = sg.minDistance.distance;
//                double db = sb.minDistance.distance;
//                
//                double med = median(dr, dg, db);
//                // Note: Use signbit() not sign() here because we need to know -0 case.
//                int medSign = signbit(med) ? -1 : 1;
//
//                if( medSign != realSign ) {
//                    dr = -dr;
//                    dg = -dg;
//                    db = -db;
//                }
//                
//                output(x, row).r = float(dr/range+.5);
//                output(x, row).g = float(dg/range+.5);
//                output(x, row).b = float(db/range+.5);
//            }
//        }
//    }
//    
//    if (edgeThreshold > 0)
//        msdfErrorCorrection(output, edgeThreshold/(scale*range));
//}

}
