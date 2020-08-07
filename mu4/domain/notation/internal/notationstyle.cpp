#include "notationstyle.h"

#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/mscore.h"
#include "libmscore/undo.h"

#include "log.h"

using namespace mu::domain::notation;
using namespace mu::async;

NotationStyle::NotationStyle(IGetScore* getScore)
    : m_getScore(getScore)
{
}

QVariant NotationStyle::styleValue(const StyleId& styleId) const
{
    return m_getScore->masterScore()->styleV(styleId);
}

QVariant NotationStyle::defaultStyleValue(const StyleId& styleId) const
{
    return Ms::MScore::defaultStyle().value(styleId);
}

void NotationStyle::setStyleValue(const StyleId& styleId, const QVariant& newValue)
{
    if (styleId == StyleId::concertPitch) {
        m_getScore->masterScore()->cmdConcertPitchChanged(newValue.toBool());
    } else {
        m_getScore->masterScore()->undoChangeStyleVal(styleId, newValue);
    }

    m_styleChanged.notify();
}

bool NotationStyle::canApplyToAllParts() const
{
    return m_getScore->masterScore()->isMaster();
}

void NotationStyle::applyToAllParts()
{
    if (!canApplyToAllParts()) {
        return;
    }

    Ms::MStyle style = m_getScore->masterScore()->style();

    for (Ms::Excerpt* excerpt : m_getScore->masterScore()->excerpts()) {
        excerpt->partScore()->undo(new Ms::ChangeStyle(excerpt->partScore(), style));
        excerpt->partScore()->update();
    }
}

Notification NotationStyle::styleChanged() const
{
    return m_styleChanged;
}
