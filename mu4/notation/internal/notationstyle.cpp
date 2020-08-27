#include "notationstyle.h"

#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/mscore.h"
#include "libmscore/undo.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::async;

NotationStyle::NotationStyle(IGetScore* getScore)
    : m_getScore(getScore)
{
}

QVariant NotationStyle::styleValue(const StyleId& styleId) const
{
    return m_getScore->score()->styleV(styleId);
}

QVariant NotationStyle::defaultStyleValue(const StyleId& styleId) const
{
    return Ms::MScore::defaultStyle().value(styleId);
}

void NotationStyle::setStyleValue(const StyleId& styleId, const QVariant& newValue)
{
    if (styleId == StyleId::concertPitch) {
        m_getScore->score()->cmdConcertPitchChanged(newValue.toBool());
    } else {
        m_getScore->score()->undoChangeStyleVal(styleId, newValue);
    }

    m_styleChanged.notify();
}

bool NotationStyle::canApplyToAllParts() const
{
    return m_getScore->score()->isMaster();
}

void NotationStyle::applyToAllParts()
{
    if (!canApplyToAllParts()) {
        return;
    }

    Ms::MStyle style = m_getScore->score()->style();

    for (Ms::Excerpt* excerpt : m_getScore->score()->excerpts()) {
        excerpt->partScore()->undo(new Ms::ChangeStyle(excerpt->partScore(), style));
        excerpt->partScore()->update();
    }
}

Notification NotationStyle::styleChanged() const
{
    return m_styleChanged;
}
