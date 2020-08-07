#include "notationundostackcontroller.h"

#include "log.h"
#include "libmscore/score.h"

using namespace mu::domain::notation;

NotationUndoStackController::NotationUndoStackController(IGetScore* getScore)
    : m_getScore(getScore)
{
}

void NotationUndoStackController::prepareChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->masterScore()) {
        return;
    }

    m_getScore->masterScore()->startCmd();
}

void NotationUndoStackController::rollbackChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->masterScore()) {
        return;
    }

    m_getScore->masterScore()->endCmd(false, true);
}

void NotationUndoStackController::commitChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->masterScore()) {
        return;
    }

    m_getScore->masterScore()->endCmd();
}
