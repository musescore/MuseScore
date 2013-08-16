#ifndef IMPORTMIDI_INNER_H
#define IMPORTMIDI_INNER_H

#include "libmscore/fraction.h"


// ---------------------------------------------------------------------------------------
// These inner classes definitions are used in cpp files only
// Include this header to link tests
// ---------------------------------------------------------------------------------------


namespace Ms {
namespace Meter {

            // max level for tuplets: duration cannot go over the tuplet boundary
            // this level should be greater than any other level
const int TUPLET_BOUNDARY_LEVEL = 10;

struct MaxLevel
      {
      int level = 0;         // 0 - the biggest, whole bar level; other: -1, -2, ...
      int levelCount = 0;    // number of ticks with 'level' value
      ReducedFraction lastPos = ReducedFraction(-1, 1);   // position of last tick with value 'level'; -1 - undefined pos
      };

struct DivLengthInfo
      {
      ReducedFraction len;
      int level;
      };

struct DivisionInfo
      {
      ReducedFraction onTime;        // division start tick (tick is counted from the beginning of bar)
      ReducedFraction len;           // length of this whole division
      bool isTuplet = false;
      std::vector<DivLengthInfo> divLengths;    // lengths of 'len' subdivisions
      };

} // namespace Meter
} // namespace Ms


#endif // IMPORTMIDI_INNER_H
