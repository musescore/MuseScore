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

#include "slurandtiesettingsmodel.h"

#include "inspector/types/linetypes.h"
#include "inspector/types/commontypes.h"

#include "engraving/types/types.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

using IconCode = mu::ui::IconCode::Code;

SlurAndTieSettingsModel::SlurAndTieSettingsModel(QObject* parent, IElementRepositoryService* repository, ElementType elementType)
    : AbstractInspectorModel(parent, repository)
{
    if (elementType == ElementType::Slur) {
        setModelType(InspectorModelType::TYPE_SLUR);
        setElementType(mu::engraving::ElementType::SLUR);
        setTitle(qtrc("inspector", "Slur"));
        setIcon(IconCode::NOTE_SLUR);
    } else {
        setModelType(InspectorModelType::TYPE_TIE);
        setElementType(mu::engraving::ElementType::TIE);
        setTitle(qtrc("inspector", "Tie"));
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

QVariantList SlurAndTieSettingsModel::possibleLineStyles() const
{
    QVariantList result {
        object(SlurStyleType::Solid, qtrc("inspector", "Normal"), IconCode::LINE_NORMAL),
        object(SlurStyleType::WideDashed, qtrc("inspector", "Wide dashed"), IconCode::LINE_WIDE_DASHED),
        object(SlurStyleType::Dashed, qtrc("inspector", "Dashed"), IconCode::LINE_DASHED),
        object(SlurStyleType::Dotted, qtrc("inspector", "Dotted"), IconCode::LINE_DOTTED)
    };

    return result;
}

void SlurAndTieSettingsModel::createProperties()
{
    m_lineStyle = buildPropertyItem(mu::engraving::Pid::SLUR_STYLE_TYPE);
    m_direction = buildPropertyItem(mu::engraving::Pid::SLUR_DIRECTION);
}

void SlurAndTieSettingsModel::loadProperties()
{
    loadPropertyItem(m_lineStyle);
    loadPropertyItem(m_direction);
}

void SlurAndTieSettingsModel::resetProperties()
{
    m_lineStyle->resetToDefault();
    m_direction->resetToDefault();
}
