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

#include "engraving/dom/masterscore.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/undo.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::async;

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

void NotationStyle::resetAllStyleValues(const std::set<StyleId>& exceptTheseOnes)
{
    static const std::set<StyleId> stylesNotToReset {
        StyleId::pageWidth,
        StyleId::pageHeight,
        StyleId::pagePrintableWidth,
        StyleId::pageEvenTopMargin,
        StyleId::pageEvenBottomMargin,
        StyleId::pageEvenLeftMargin,
        StyleId::pageOddTopMargin,
        StyleId::pageOddBottomMargin,
        StyleId::pageOddLeftMargin,
        StyleId::pageTwosided,
        StyleId::spatium,
        StyleId::concertPitch,
        StyleId::createMultiMeasureRests
    };

    int beginIdx = int(StyleId::NOSTYLE) + 1;
    int endIdx = int(StyleId::STYLES);
    for (int idx = beginIdx; idx < endIdx; idx++) {
        StyleId styleId = StyleId(idx);
        if (stylesNotToReset.find(styleId) == stylesNotToReset.cend() && exceptTheseOnes.find(styleId) == exceptTheseOnes.cend()) {
            score()->resetStyleValue(styleId);
        }
    }

    score()->update();
    m_styleChanged.notify();
}

Notification NotationStyle::styleChanged() const
{
    return m_styleChanged;
}

bool NotationStyle::loadStyle(const mu::io::path_t& path, bool allowAnyVersion)
{
    m_undoStack->prepareChanges();
    bool result = score()->loadStyle(path.toQString(), allowAnyVersion);
    m_undoStack->commitChanges();

    if (result) {
        styleChanged().notify();
    }

    return result;
}

bool NotationStyle::saveStyle(const mu::io::path_t& path)
{
    return score()->saveStyle(path.toQString());
}

mu::engraving::Score* NotationStyle::score() const
{
    return m_getScore->score();
}
