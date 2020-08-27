#include "notationundostackcontroller.h"

#include "log.h"
#include "libmscore/score.h"

using namespace mu::notation;

NotationUndoStackController::NotationUndoStackController(IGetScore* getScore)
    : m_getScore(getScore)
{
}

void NotationUndoStackController::prepareChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->startCmd();
}

void NotationUndoStackController::rollbackChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->endCmd(false, true);
}

void NotationUndoStackController::commitChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->endCmd();
}
