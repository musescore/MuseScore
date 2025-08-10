/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/dom/masterscore.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/undo.h"

#include "io/file.h"

#include "log.h"
#include "types/translatablestring.h"

using namespace mu::notation;
using namespace muse::async;

NotationStyle::NotationStyle(IGetScore* getScore, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_undoStack(undoStack)
{
}

PropertyValue NotationStyle::styleValue(const StyleId& styleId) const
{
    return score()->style().styleV(styleId);
}

PropertyValue NotationStyle::defaultStyleValue(const StyleId& styleId) const
{
    return engraving::DefaultStyle::defaultStyle().value(styleId);
}

void NotationStyle::setStyleValue(const StyleId& styleId, const PropertyValue& newValue)
{
    if (styleValue(styleId) == newValue) {
        return;
    }

    if (styleId == StyleId::concertPitch) {
        score()->cmdConcertPitchChanged(newValue.toBool());
    } else {
        score()->undoChangeStyleVal(styleId, newValue);
        score()->update();
    }

    m_styleChanged.notify();
}

void NotationStyle::resetStyleValue(const StyleId& styleId)
{
    score()->resetStyleValue(styleId);
    score()->update();
    m_styleChanged.notify();
}

void NotationStyle::resetStyleValues(const std::vector<StyleId>& styleIds)
{
    for (StyleId id : styleIds) {
        score()->resetStyleValue(id);
    }
    score()->update();
    m_styleChanged.notify();
}

bool NotationStyle::canApplyToAllParts() const
{
    return !score()->isMaster(); // In parts only
}

void NotationStyle::applyToAllParts()
{
    if (!canApplyToAllParts()) {
        return;
    }

    mu::engraving::MStyle style = m_getScore->score()->style();

    for (mu::engraving::Excerpt* excerpt : score()->masterScore()->excerpts()) {
        excerpt->excerptScore()->undo(new mu::engraving::ChangeStyle(excerpt->excerptScore(), style));
        excerpt->excerptScore()->update();
    }
}

void NotationStyle::resetAllStyleValues(const StyleIdSet& exceptTheseOnes)
{
    score()->cmdResetAllStyles(exceptTheseOnes);
    score()->update();
    m_styleChanged.notify();
}

Notification NotationStyle::styleChanged() const
{
    return m_styleChanged;
}

bool NotationStyle::loadStyle(const muse::io::path_t& path, bool allowAnyVersion)
{
    m_undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Load style"));
    muse::io::File styleFile(path);
    bool result = score()->loadStyle(styleFile, allowAnyVersion);
    m_undoStack->commitChanges();

    if (result) {
        styleChanged().notify();
    }

    return result;
}

bool NotationStyle::saveStyle(const muse::io::path_t& path)
{
    return score()->saveStyle(path.toQString());
}

mu::engraving::Score* NotationStyle::score() const
{
    return m_getScore->score();
}
