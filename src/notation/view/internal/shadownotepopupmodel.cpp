/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "shadownotepopupmodel.h"
#include "engraving/dom/shadownote.h"

using namespace mu::notation;

ShadowNotePopupModel::ShadowNotePopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_HARP_DIAGRAM, parent)
{
}

bool ShadowNotePopupModel::canOpen(const EngravingItem* element)
{
    if (!element || !element->isShadowNote() || !element->visible()) {
        return false;
    }

    if (element->staff() && element->staff()->isDrumStaff(element->tick())) {
        // Can open with PERCUSSON_CONTENT type
        return true;
    }

    return false;
}

void ShadowNotePopupModel::init()
{
    AbstractElementPopupModel::init();

    m_item = interaction()->shadowNote();

    interaction()->shadowNoteChanged().onNotify(this, [this]() {
        updateItemRect();
    });

    emit currentPopupTypeChanged();
}

ShadowNotePopupContent::ContentType ShadowNotePopupModel::currentPopupType() const
{
    const mu::engraving::ShadowNote* shadowNote = interaction()->shadowNote();
    if (!shadowNote || !shadowNote->visible()) {
        return ShadowNotePopupContent::ContentType::NONE;
    }

    if (shadowNote->staff() && shadowNote->staff()->isDrumStaff(shadowNote->tick())) {
        return ShadowNotePopupContent::ContentType::PERCUSSION_CONTENT;
    }

    return ShadowNotePopupContent::ContentType::NONE;
}

void ShadowNotePopupModel::updateItemRect()
{
    const mu::engraving::ShadowNote* shadowNote = interaction()->shadowNote();
    if (!shadowNote || !shadowNote->visible()) {
        m_itemRect = QRect();
        emit itemRectChanged(m_itemRect);
    }

    muse::RectF noteHeadRect = shadowNote->symBbox(shadowNote->noteheadSymbol());
    noteHeadRect.translate(shadowNote->canvasPos().x(), shadowNote->canvasPos().y());
    m_itemRect = fromLogical(noteHeadRect).toQRect();
    emit itemRectChanged(m_itemRect);
}
