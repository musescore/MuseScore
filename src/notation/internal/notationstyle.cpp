/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "notationstyle.h"

#include "engraving/style/defaultstyle.h"

#include "libmscore/masterscore.h"
#include "libmscore/excerpt.h"
#include "libmscore/mscore.h"
#include "libmscore/undo.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::async;

NotationStyle::NotationStyle(IGetScore* getScore, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_undoStack(undoStack)
{
}

QVariant NotationStyle::styleValue(const StyleId& styleId) const
{
    return m_getScore->score()->styleV(styleId);
}

QVariant NotationStyle::defaultStyleValue(const StyleId& styleId) const
{
    return engraving::DefaultStyle::defaultStyle().value(styleId);
}

void NotationStyle::setStyleValue(const StyleId& styleId, const QVariant& newValue)
{
    if (styleId == StyleId::concertPitch) {
        m_getScore->score()->cmdConcertPitchChanged(newValue.toBool());
    } else {
        m_getScore->score()->undoChangeStyleVal(styleId, newValue);
        m_getScore->score()->update();
    }

    m_styleChanged.notify();
}

void NotationStyle::resetStyleValue(const StyleId& styleId)
{
    m_getScore->score()->resetStyles({ styleId });
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

    for (Ms::Excerpt* excerpt : m_getScore->score()->masterScore()->excerpts()) {
        excerpt->partScore()->undo(new Ms::ChangeStyle(excerpt->partScore(), style));
        excerpt->partScore()->update();
    }
}

Notification NotationStyle::styleChanged() const
{
    return m_styleChanged;
}

bool NotationStyle::loadStyle(const mu::io::path& path, bool allowAnyVersion)
{
    m_undoStack->prepareChanges();
    bool result = m_getScore->score()->loadStyle(path.toQString(), allowAnyVersion);
    m_undoStack->commitChanges();
    if (result) {
        styleChanged().notify();
    }
    return result;
}

bool NotationStyle::saveStyle(const mu::io::path& path)
{
    return m_getScore->score()->saveStyle(path.toQString());
}
