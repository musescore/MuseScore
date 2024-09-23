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
#include "scorecallbacks.h"

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/system.h"

#include "inotationinteraction.h"
#include "igetscore.h"

#include "log.h"

using namespace mu::notation;

void ScoreCallbacks::dataChanged(const muse::RectF&)
{
    NOT_IMPLEMENTED;
}

void ScoreCallbacks::updateAll()
{
    NOT_IMPLEMENTED;
}

void ScoreCallbacks::drawBackground(muse::draw::Painter*, const muse::RectF&) const
{
    NOT_IMPLEMENTED;
}

const muse::Rect ScoreCallbacks::geometry() const
{
    NOT_IMPLEMENTED;
    return muse::Rect();
}

qreal ScoreCallbacks::selectionProximity() const
{
    return m_selectionProximity;
}

void ScoreCallbacks::setSelectionProximity(qreal proximity)
{
    m_selectionProximity = proximity;
}

void ScoreCallbacks::setDropTarget(mu::engraving::EngravingItem* dropTarget)
{
    IF_ASSERT_FAILED(m_interaction) {
        return;
    }

    m_interaction->setDropTarget(dropTarget, false);
}

void ScoreCallbacks::setDropRectangle(const muse::RectF& rect)
{
    IF_ASSERT_FAILED(m_interaction) {
        return;
    }

    m_interaction->setDropRect(rect);
}

void ScoreCallbacks::changeEditElement(mu::engraving::EngravingItem* newElement)
{
    IF_ASSERT_FAILED(m_interaction) {
        return;
    }

    m_interaction->changeEditElement(newElement);
}

void ScoreCallbacks::adjustCanvasPosition(const mu::engraving::EngravingItem* el, int staffIndex)
{
    IF_ASSERT_FAILED(m_interaction) {
        return;
    }

    m_interaction->showItem(el, staffIndex);
}

void ScoreCallbacks::setNotationInteraction(INotationInteraction* interaction)
{
    m_interaction = interaction;
}
