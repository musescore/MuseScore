#include "notationelementsrepository.h"

#include "libmscore/score.h"
#include "log.h"

using namespace mu::domain::notation;

NotationElementsRepository::NotationElementsRepository(IGetScore* getScore) :
    m_getScore(getScore)
{
}

Ms::Measure* NotationElementsRepository::measureByIndex(const int measureIndex) const
{
    Ms::Score* score = m_getScore->score();

    IF_ASSERT_FAILED(m_getScore && score) {
        return nullptr;
    }

    return score->crMeasure(measureIndex);
}
