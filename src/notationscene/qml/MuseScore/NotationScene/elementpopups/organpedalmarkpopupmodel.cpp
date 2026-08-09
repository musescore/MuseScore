/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "organpedalmarkpopupmodel.h"

#include "engraving/dom/organpedalmark.h"
#include "engraving/dom/score.h"

#include "engraving/types/symnames.h"

using namespace mu::notation;
using namespace mu::engraving;

static const QMap<SymId, int> PEDAL_MARKS_OFFSET {
    { SymId::keyboardPedalToe2,      8  },
    { SymId::keyboardPedalHeel1,     9  },
    { SymId::keyboardPedalHeel3,     8  },
    { SymId::keyboardPedalToe1,      9  },
    { SymId::keyboardPedalHeel2,     8  },
    { SymId::keyboardPedalToeToHeel, 10 },
    { SymId::keyboardPedalHeelToToe, 10 },
    { SymId::keyboardPedalHeelToe,   12 }
};

OrganPedalMarkPopupModel::OrganPedalMarkPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_ORGAN_PEDAL_MARK, parent)
{
}

void OrganPedalMarkPopupModel::init()
{
    AbstractElementPopupModel::init();

    m_pages.clear();

    if (!m_item || !m_item->isOrganPedalMark()) {
        return;
    }

    IEngravingFontPtr engravingFont = m_item->score()->engravingFont();

    for (const SymIdList& pedalMarkSymIds : toOrganPedalMark(m_item)->getPedalMarksPopupSet()) {
        QVariantList variantPage;
        for (const SymId& pedalMarkSymId : pedalMarkSymIds) {
            QVariantMap variantMap {
                { "accessibleName", SymNames::translatedUserNameForSymId(pedalMarkSymId).toQString() },
                { "pedalMark", engravingFont->toString(pedalMarkSymId).toQString() },
                { "offset", PEDAL_MARKS_OFFSET[pedalMarkSymId] }
            };
            variantPage.append(variantMap);
        }
        m_pages.append(QVariant::fromValue(variantPage));
    }

    emit pagesChanged();
}

void OrganPedalMarkPopupModel::changePedalMark(int popupPageIndex, int pageElementIndex)
{
    if (!m_item || !m_item->isOrganPedalMark()) {
        return;
    }

    beginCommand(muse::TranslatableString("undoableAction", "Change organ pedal mark"));
    toOrganPedalMark(m_item)->changePedalMark(popupPageIndex, pageElementIndex);
    endCommand();

    updateNotation();
}

void OrganPedalMarkPopupModel::updateItemRect()
{
    bool placeAbove = true;
    if (m_item) {
        placeAbove = m_item->placeAbove() || !m_item->hasStaff();
    }

    AbstractElementPopupModel::updateItemRect();

    if (m_placeAbove != placeAbove) {
        m_placeAbove = placeAbove;
        emit placeAboveChanged();
    }
}

QVariantList OrganPedalMarkPopupModel::pages() const
{
    return m_pages;
}

bool OrganPedalMarkPopupModel::placeAbove() const
{
    return m_placeAbove;
}