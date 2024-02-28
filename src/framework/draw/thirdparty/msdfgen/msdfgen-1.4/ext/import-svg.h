
#pragma once

#include <cstdlib>
#include "../core/Shape.h"

namespace msdfgen {

/// Reads the first path found in the specified SVG file and stores it as a Shape in output.
bool loadSvgShape(Shape &output, const char *filename, int pathIndex = 0, Vector2 *dimensions = NULL);

}
