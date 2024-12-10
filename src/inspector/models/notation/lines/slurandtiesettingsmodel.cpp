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

#include "slurandtiesettingsmodel.h"

#include "inspector/types/linetypes.h"
#include "inspector/types/commontypes.h"

#include "engraving/types/types.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

using IconCode = muse::ui::IconCode::Code;

SlurAndTieSettingsModel::SlurAndTieSettingsModel(QObject* parent, IElementRepositoryService* repository, ElementType elementType)
    : AbstractInspectorModel(parent, repository)
{
    if (elementType == ElementType::Slur) {
        setModelType(InspectorModelType::TYPE_SLUR);
        setElementType(mu::engraving::ElementType::SLUR);
        setTitle(muse::qtrc("inspector", "Slur"));
        setIcon(IconCode::NOTE_SLUR);
    } else if (elementType == ElementType::Tie) {
        setModelType(InspectorModelType::TYPE_TIE);
        setElementType(mu::engraving::ElementType::TIE);
        setTitle(muse::qtrc("inspector", "Tie"));
        setIcon(IconCode::NOTE_TIE);
    } else if (elementType == ElementType::LaissezVib) {
        setModelType(InspectorModelType::TYPE_LAISSEZ_VIB);
        setElementType(mu::engraving::ElementType::LAISSEZ_VIB);
        setTitle(muse::qtrc("inspector", "Laissez vibrer"));
        setIcon(IconCode::NOTE_LV);
        m_isLaissezVib = true;
    } else if (elementType == ElementType::PartialTie) {
        setModelType(InspectorModelType::TYPE_PARTIAL_TIE);
        setElementType(mu::engraving::ElementType::PARTIAL_TIE);
        setTitle(muse::qtrc("inspector", "Tie (partial)"));
        setIcon(IconCode::NOTE_TIE);
    }

    createProperties();
}

PropertyItem* SlurAndTieSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* SlurAndTieSettingsModel::direction() const
{
    return m_direction;
}

PropertyItem* SlurAndTieSettingsModel::tiePlacement() const
{
    return m_tiePlacement;
}

PropertyItem* SlurAndTieSettingsModel::minLength() const
{
    return m_minLength;
}

bool SlurAndTieSettingsModel::isLaissezVib() const
{
    return m_isLaissezVib;
}

bool SlurAndTieSettingsModel::isTiePlacementAvailable() const
{
    return m_isTiePlacementAvailable;
}

bool SlurAndTieSettingsModel::isMinLengthAvailable() const
{
    return m_isMinLengthAvailable;
}

bool SlurAndTieSettingsModel::isLineStyleAvailable() const
{
    return m_isLineStyleAvailable;
}

QVariantList SlurAndTieSettingsModel::possibleLineStyles() const
{
    QVariantList result {
        object(SlurStyleType::Solid, muse::qtrc("inspector", "Normal"), IconCode::LINE_NORMAL),
        object(SlurStyleType::WideDashed, muse::qtrc("inspector", "Wide dashed"), IconCode::LINE_WIDE_DASHED),
        object(SlurStyleType::Dashed, muse::qtrc("inspector", "Dashed"), IconCode::LINE_DASHED),
        object(SlurStyleType::Dotted, muse::qtrc("inspector", "Dotted"), IconCode::LINE_DOTTED)
    };

    return result;
}

void SlurAndTieSettingsModel::createProperties()
{
    m_lineStyle = buildPropertyItem(mu::engraving::Pid::SLUR_STYLE_TYPE);
    m_direction = buildPropertyItem(mu::engraving::Pid::SLUR_DIRECTION);
    m_tiePlacement = buildPropertyItem(mu::engraving::Pid::TIE_PLACEMENT);
    m_minLength = buildPropertyItem(mu::engraving::Pid::MIN_LENGTH);
    updateIsTiePlacementAvailable();
    updateIsMinLengthAvailable();
    updateisLineStyleAvailable();
}

void SlurAndTieSettingsModel::loadProperties()
{
    loadPropertyItem(m_lineStyle);
    loadPropertyItem(m_direction);
    loadPropertyItem(m_tiePlacement);
    loadPropertyItem(m_minLength);
    updateIsTiePlacementAvailable();
    updateIsMinLengthAvailable();
    updateisLineStyleAvailable();
}

void SlurAndTieSettingsModel::resetProperties()
{
    m_lineStyle->resetToDefault();
    m_direction->resetToDefault();
    m_tiePlacement->resetToDefault();
    m_minLength->resetToDefault();
}

void SlurAndTieSettingsModel::updateIsTiePlacementAvailable()
{
    bool available = false;
    for (EngravingItem* item : m_elementList) {
        if (item->isTie()) {
            available = true;
            break;
        }
    }

    if (available != m_isTiePlacementAvailable) {
        m_isTiePlacementAvailable = available;
        emit isTiePlacementAvailableChanged(m_isTiePlacementAvailable);
    }
}

void SlurAndTieSettingsModel::updateIsMinLengthAvailable()
{
    bool available = false;
    for (EngravingItem* item : m_elementList) {
        if (item->isLaissezVib() && !item->style().styleB(Sid::laissezVibUseSmuflSym)) {
            available = true;
            break;
        }
    }

    if (available != m_isMinLengthAvailable) {
        m_isMinLengthAvailable = available;
        emit isMinLengthAvailableChanged(m_isMinLengthAvailable);
    }
}

void SlurAndTieSettingsModel::updateisLineStyleAvailable()
{
    bool available = true;
    for (EngravingItem* item : m_elementList) {
        if (item->isLaissezVib() && item->style().styleB(Sid::laissezVibUseSmuflSym)) {
            available = false;
            break;
        }
    }

    if (available != m_isLineStyleAvailable) {
        m_isLineStyleAvailable = available;
        emit isLineStyleAvailableChanged(m_isLineStyleAvailable);
    }
}
