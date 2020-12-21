//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
