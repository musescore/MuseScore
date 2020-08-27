#include "notationelements.h"

#include "libmscore/score.h"
#include "log.h"

using namespace mu::notation;

NotationElements::NotationElements(IGetScore* getScore)
    : m_getScore(getScore)
{
}

Ms::Measure* NotationElements::measureByIndex(const int measureIndex) const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }

    Ms::Score* score = m_getScore->score();

    return score->crMeasure(measureIndex);
}
