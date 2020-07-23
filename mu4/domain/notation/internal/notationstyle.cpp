#include "notationstyle.h"

#include "libmscore/score.h"
#include "log.h"

using namespace mu::domain::notation;

NotationStyle::NotationStyle(IGetScore* getScore) :
    m_getScore(getScore)
{
}

void NotationStyle::updateStyleValue(const StyleId& styleId, const QVariant& newValue) const
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->undoChangeStyleVal(styleId, newValue);
}

QVariant NotationStyle::styleValue(const StyleId& styleId)
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return QVariant();
    }

    return m_getScore->score()->styleV(styleId);
}
