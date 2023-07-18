#ifndef MU_ENGRAVING_SCORELAYOUT_H
#define MU_ENGRAVING_SCORELAYOUT_H

#include "layoutcontext.h"

namespace mu::engraving {
class Score;
}

namespace mu::engraving::layout::v0 {
class ScoreLayout
{
public:

    static void layoutRange(Score* score, const Fraction& st, const Fraction& et);

private:
    static void layoutLinear(LayoutContext& ctx);
    static void layoutLinear(LayoutContext& ctx, bool layoutAll);
    static void resetSystems(LayoutContext& ctx, bool layoutAll);
    static void collectLinearSystem(LayoutContext& ctx);

    static void doLayout(LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_SCORELAYOUT_H
