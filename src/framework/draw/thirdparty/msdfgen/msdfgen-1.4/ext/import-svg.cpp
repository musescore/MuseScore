
#define _USE_MATH_DEFINES
#include "import-svg.h"

#include <cstdio>
#include <tinyxml2.h>
#include "../core/arithmetics.hpp"

#ifdef _WIN32
    #pragma warning(disable:4996)
    #define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
#endif

#define ARC_SEGMENTS_PER_PI 2
#define ENDPOINT_SNAP_RANGE_PROPORTION (1/16384.)

namespace msdfgen {

#if defined(_DEBUG) || !NDEBUG
#define REQUIRE(cond) { if (!(cond)) { fprintf(stderr, "SVG Parse Error (%s:%d): " #cond "\n", __FILE__, __LINE__); return false; } }
#else
#define REQUIRE(cond) { if (!(cond)) return false; }
#endif

static void skipExtraChars(const char *&pathDef) {
    while (*pathDef == ',' || *pathDef == ' ' || *pathDef == '\t' || *pathDef == '\r' || *pathDef == '\n')
        ++pathDef;
}

static bool readNodeType(char &output, const char *&pathDef) {
    skipExtraChars(pathDef);
    char nodeType = *pathDef;
    if (nodeType && nodeType != '+' && nodeType != '-' && nodeType != '.' && nodeType != ',' && (nodeType < '0' || nodeType > '9')) {
        ++pathDef;
        output = nodeType;
        return true;
    }
    return false;
}

static bool readCoord(Point2 &output, const char *&pathDef) {
    skipExtraChars(pathDef);
    int shift;
    double x, y;
    if (sscanf(pathDef, "%lf%lf%n", &x, &y, &shift) == 2 || sscanf(pathDef, "%lf , %lf%n", &x, &y, &shift) == 2) {
        output.x = x;
        output.y = y;
        pathDef += shift;
        return true;
    }
    return false;
}

static bool readDouble(double &output, const char *&pathDef) {
    skipExtraChars(pathDef);
    int shift;
    double v;
    if (sscanf(pathDef, "%lf%n", &v, &shift) == 1) {
        pathDef += shift;
        output = v;
        return true;
    }
    return false;
}

static bool readBool(bool &output, const char *&pathDef) {
    skipExtraChars(pathDef);
    int shift;
    int v;
    if (sscanf(pathDef, "%d%n", &v, &shift) == 1) {
        pathDef += shift;
        output = v != 0;
        return true;
    }
    return false;
}

static bool readFillRule(FillRule &fillRule, const char *&str) {

    const char *p = str;

    if (strncasecmp(p, "fill-rule:", 10) == 0) {
        p += 10;
        while (*p && isspace(*p))
            p++;
    }
    
    if (strncasecmp(p, "nonzero", 7) == 0) {
        fillRule = FillRule::NonZero;
        str = p + 7;
        return true;
    }
    if (strncasecmp(p, "evenodd", 7) == 0) {
        fillRule = FillRule::EvenOdd;
        str = p + 7;
        return true;
    }
    
    return false;
}

static double arcAngle(Vector2 u, Vector2 v) {
    return nonZeroSign(crossProduct(u, v))*acos(clamp(dotProduct(u, v)/(u.length()*v.length()), -1., +1.));
}

static Vector2 rotateVector(Vector2 v, Vector2 direction) {
    return Vector2(direction.x*v.x-direction.y*v.y, direction.y*v.x+direction.x*v.y);
}

static void addArcApproximate(Contour &contour, Point2 startPoint, Point2 endPoint, Vector2 radius, double rotation, bool largeArc, bool sweep) {
    if (endPoint == startPoint)
        return;
    if (radius.x == 0 || radius.y == 0)
        return contour.addEdge(LinearSegment(startPoint, endPoint));

    radius.x = fabs(radius.x);
    radius.y = fabs(radius.y);
    Vector2 axis(cos(rotation), sin(rotation));

    Vector2 rm = rotateVector(.5*(startPoint-endPoint), Vector2(axis.x, -axis.y));
    Vector2 rm2 = rm*rm;
    Vector2 radius2 = radius*radius;
    double radiusGap = rm2.x/radius2.x+rm2.y/radius2.y;
    if (radiusGap > 1) {
        radius *= sqrt(radiusGap);
        radius2 = radius*radius;
    }
    double dq = (radius2.x*rm2.y+radius2.y*rm2.x);
    double pq = radius2.x*radius2.y/dq-1;
    double q = (largeArc == sweep ? -1 : +1)*sqrt(max(pq, 0.));
    Vector2 rc(q*radius.x*rm.y/radius.y, -q*radius.y*rm.x/radius.x);
    Point2 center = .5*(startPoint+endPoint)+rotateVector(rc, axis);

    double angleStart = arcAngle(Vector2(1, 0), (rm-rc)/radius);
    double angleExtent = arcAngle((rm-rc)/radius, (-rm-rc)/radius);
    if (!sweep && angleExtent > 0)
        angleExtent -= 2*M_PI;
    else if (sweep && angleExtent < 0)
        angleExtent += 2*M_PI;

    int segments = (int) ceil(ARC_SEGMENTS_PER_PI/M_PI*fabs(angleExtent));
    double angleIncrement = angleExtent/segments;
    double cl = 4/3.*sin(.5*angleIncrement)/(1+cos(.5*angleIncrement));

    Point2 prevNode = startPoint;
    double angle = angleStart;
    for (int i = 0; i < segments; ++i) {
        Point2 controlPoint[2];
        Vector2 d(cos(angle), sin(angle));
        controlPoint[0] = center+rotateVector(Vector2(d.x-cl*d.y, d.y+cl*d.x)*radius, axis);
        angle += angleIncrement;
        d.set(cos(angle), sin(angle));
        controlPoint[1] = center+rotateVector(Vector2(d.x+cl*d.y, d.y-cl*d.x)*radius, axis);
        Point2 node = i == segments-1 ? endPoint : center+rotateVector(d*radius, axis);
        contour.addEdge(CubicSegment(prevNode, controlPoint[0], controlPoint[1], node));
        prevNode = node;
    }
}

static bool buildFromPath(Shape &shape, const char *pathDef, double size) {
    char nodeType = '\0';
    char prevNodeType = '\0';
    Point2 prevNode(0, 0);
    bool nodeTypePreread = false;
    Point2 startPoint;
    Point2 controlPoint[2];
    Point2 node;
    
    while (nodeTypePreread || readNodeType(nodeType, pathDef)) {
        nodeTypePreread = false;
        Contour &contour = shape.addContour();
        bool contourStart = true;

        while (*pathDef) {
            switch (nodeType) {
                case 'M': case 'm':
                    if (!contourStart) {
                        nodeTypePreread = true;
                        goto NEXT_CONTOUR;
                    }
                    REQUIRE(readCoord(node, pathDef));
                    if (nodeType == 'm')
                        node += prevNode;
                    startPoint = node;
                    --nodeType; // to 'L' or 'l'
                    break;
                case 'Z': case 'z':
                    REQUIRE(!contourStart);
                    goto NEXT_CONTOUR;
                case 'L': case 'l':
                    REQUIRE(readCoord(node, pathDef));
                    if (nodeType == 'l')
                        node += prevNode;
                    contour.addEdge(LinearSegment(prevNode, node));
                    break;
                case 'H': case 'h':
                    REQUIRE(readDouble(node.x, pathDef));
                    if (nodeType == 'h')
                        node.x += prevNode.x;
                    contour.addEdge(LinearSegment(prevNode, node));
                    break;
                case 'V': case 'v':
                    REQUIRE(readDouble(node.y, pathDef));
                    if (nodeType == 'v')
                        node.y += prevNode.y;
                    contour.addEdge(LinearSegment(prevNode, node));
                    break;
                case 'Q': case 'q':
                    REQUIRE(readCoord(controlPoint[0], pathDef));
                    REQUIRE(readCoord(node, pathDef));
                    if (nodeType == 'q') {
                        controlPoint[0] += prevNode;
                        node += prevNode;
                    }
                    contour.addEdge(QuadraticSegment(prevNode, controlPoint[0], node));
                    break;
                case 'T': case 't':
                    if (prevNodeType == 'Q' || prevNodeType == 'q' || prevNodeType == 'T' || prevNodeType == 't')
                        controlPoint[0] = node+node-controlPoint[0];
                    else
                        controlPoint[0] = node;
                    REQUIRE(readCoord(node, pathDef));
                    if (nodeType == 't')
                        node += prevNode;
                    contour.addEdge(QuadraticSegment(prevNode, controlPoint[0], node));
                    break;
                case 'C': case 'c':
                    REQUIRE(readCoord(controlPoint[0], pathDef));
                    REQUIRE(readCoord(controlPoint[1], pathDef));
                    REQUIRE(readCoord(node, pathDef));
                    if (nodeType == 'c') {
                        controlPoint[0] += prevNode;
                        controlPoint[1] += prevNode;
                        node += prevNode;
                    }
                    contour.addEdge(CubicSegment(prevNode, controlPoint[0], controlPoint[1], node));
                    break;
                case 'S': case 's':
                    if (prevNodeType == 'C' || prevNodeType == 'c' || prevNodeType == 'S' || prevNodeType == 's')
                        controlPoint[0] = node+node-controlPoint[1];
                    else
                        controlPoint[0] = node;
                    REQUIRE(readCoord(controlPoint[1], pathDef));
                    REQUIRE(readCoord(node, pathDef));
                    if (nodeType == 's') {
                        controlPoint[1] += prevNode;
                        node += prevNode;
                    }
                    contour.addEdge(CubicSegment(prevNode, controlPoint[0], controlPoint[1], node));
                    break;
                case 'A': case 'a':
                    {
                        Vector2 radius;
                        double angle;
                        bool largeArg;
                        bool sweep;
                        REQUIRE(readCoord(radius, pathDef));
                        REQUIRE(readDouble(angle, pathDef));
                        REQUIRE(readBool(largeArg, pathDef));
                        REQUIRE(readBool(sweep, pathDef));
                        REQUIRE(readCoord(node, pathDef));
                        if (nodeType == 'a')
                            node += prevNode;
                        angle *= M_PI/180.0;
                        addArcApproximate(contour, prevNode, node, radius, angle, largeArg, sweep);
                    }
                    break;
                default:
                    REQUIRE(!"Unknown node type");
            }
            contourStart &= nodeType == 'M' || nodeType == 'm';
            prevNode = node;
            prevNodeType = nodeType;
            readNodeType(nodeType, pathDef);
        }
    NEXT_CONTOUR:
        // Fix contour if it isn't properly closed
        if (!contour.edges.empty() && prevNode != startPoint) {
            if ((contour.edges[contour.edges.size()-1].point(1)-contour.edges[0].point(0)).length() < ENDPOINT_SNAP_RANGE_PROPORTION*size)
                contour.edges[contour.edges.size()-1].moveEndPoint(contour.edges[0].point(0));
            else
                contour.addEdge(LinearSegment(prevNode, startPoint));
        }
        prevNodeType = nodeType;
        prevNode = startPoint;
        prevNodeType = '\0';
    }
    return true;
}

bool loadSvgShape(Shape &output, const char *filename, int pathIndex, Vector2 *dimensions) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename))
        return false;
    tinyxml2::XMLElement *root = doc.FirstChildElement("svg");
    if (!root)
        return false;

    tinyxml2::XMLElement *path = NULL;
    if (pathIndex > 0) {
        path = root->FirstChildElement("path");
        if (!path) {
            tinyxml2::XMLElement *g = root->FirstChildElement("g");
            if (g)
                path = g->FirstChildElement("path");
        }
        while (path && --pathIndex > 0)
            path = path->NextSiblingElement("path");
    } else {
        path = root->LastChildElement("path");
        if (!path) {
            tinyxml2::XMLElement *g = root->LastChildElement("g");
            if (g)
                path = g->LastChildElement("path");
        }
        while (path && ++pathIndex < 0)
            path = path->PreviousSiblingElement("path");
     }
    if (!path)
        return false;
    const char *pd = path->Attribute("d");
    if (!pd)
        return false;

    output.contours.clear();
    output.inverseYAxis = true;
    Vector2 dims(root->DoubleAttribute("width"), root->DoubleAttribute("height"));
    if (!dims) {
        double left, top;
        const char *viewBox = root->Attribute("viewBox");
        if (viewBox)
            sscanf(viewBox, "%lf %lf %lf %lf", &left, &top, &dims.x, &dims.y);
    }
    if (dimensions)
        *dimensions = dims;
    
    // Try and determine fill-rule. It's not perfect, because full SVG implementations
    // can use CSS and whatnot to inject this property. But we'll make a best effort
    // attempt.
    tinyxml2::XMLNode *node = path;
    while (node) {
        tinyxml2::XMLElement *el = node->ToElement();
        if (!el)
            break;
        
        // Check explicit attribute.
        const char *p = el->Attribute("fill-rule");
        if (p && readFillRule(output.fillRule, p))
            break;
        
        // Or see if it's in the style attribute
        p = el->Attribute("style");
        if (p && (p = strstr(p, "fill-rule:")) != NULL && readFillRule(output.fillRule, p))
            break;
        
        // Otherwise, seek up the hierarchy until we do get one (or run out of parents).
        node = node->Parent();
    }
    
    return buildFromPath(output, pd, dims.length());
}

}
