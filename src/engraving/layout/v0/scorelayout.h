#ifndef MU_ENGRAVING_SCORELAYOUT_H
#define MU_ENGRAVING_SCORELAYOUT_H

#include "../layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Score;
}

namespace mu::engraving::layout::v0 {
class ScoreLayout
{
public:

    static void layoutRange(Score* score, const LayoutOptions& options, const Fraction& st, const Fraction& et);

private:
    static void doLayoutRange(Score* score, const LayoutOptions& options, const Fraction&, const Fraction&);
    static void layoutLinear(const LayoutOptions& options, LayoutContext& ctx);
    static void layoutLinear(bool layoutAll, const LayoutOptions& options, LayoutContext& ctx);
    static void resetSystems(LayoutContext& ctx, bool layoutAll);
    static void collectLinearSystem(const LayoutOptions& options, LayoutContext& ctx);

    static void doLayout(const LayoutOptions& options, LayoutContext& lc);
};
}

#endif // MU_ENGRAVING_SCORELAYOUT_H
