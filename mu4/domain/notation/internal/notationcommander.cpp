#include "notationcommander.h"

#include "log.h"
#include "libmscore/score.h"

using namespace mu::domain::notation;

NotationCommander::NotationCommander(IGetScore* getScore) :
    m_getScore(getScore)
{
}

void NotationCommander::beginCommand()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->startCmd();
}

void NotationCommander::endCommand()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->endCmd();
}
