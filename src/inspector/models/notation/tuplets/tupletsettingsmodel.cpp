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

#include "tupletsettingsmodel.h"

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::inspector;

using Icon = mu::ui::IconCode::Code;

template<typename T>
static QVariant object(T type, QString title, Icon iconCode = Icon::NONE)
{
    QVariantMap obj;
    obj["value"] = static_cast<int>(type);
    obj["text"] = title;
    obj["iconCode"] = static_cast<int>(iconCode);

    return obj;
}

TupletSettingsModel::TupletSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, Ms::ElementType::TUPLET)
{
    setModelType(InspectorModelType::TYPE_TUPLET);
    setTitle(qtrc("inspector", "Tuplet"));
    setIcon(ui::IconCode::Code::TUPLET_NUMBER_WITH_BRACKETS);
    createProperties();
}

PropertyItem* TupletSettingsModel::directionType() const
{
    return m_directionType;
}

PropertyItem* TupletSettingsModel::numberType() const
{
    return m_numberType;
}

PropertyItem* TupletSettingsModel::bracketType() const
{
    return m_bracketType;
}

PropertyItem* TupletSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

QVariantList TupletSettingsModel::possibleDirectionTypes() const
{
    using Type = Ms::DirectionV;

    QVariantList types {
        object(Type::AUTO, qtrc("inspector", "Auto")),
        object(Type::DOWN, qtrc("inspector", "Down"), Icon::ARROW_DOWN),
        object(Type::UP, qtrc("inspector", "Up"), Icon::ARROW_UP)
    };

    return types;
}

QVariantList TupletSettingsModel::possibleNumberTypes() const
{
    using Type = Ms::TupletNumberType;

    QVariantList types {
        object(Type::SHOW_NUMBER, qtrc("inspector", "Number")),
        object(Type::SHOW_RELATION, qtrc("inspector", "Ratio")),
        object(Type::NO_TEXT, qtrc("inspector", "None"))
    };

    return types;
}

QVariantList TupletSettingsModel::possibleBracketTypes() const
{
    using Type = Ms::TupletBracketType;

    QVariantList types {
        object(Type::AUTO_BRACKET, qtrc("inspector", "Auto")),
        object(Type::SHOW_BRACKET, qtrc("inspector", "Bracket"), Icon::TUPLET_NUMBER_WITH_BRACKETS),
        object(Type::SHOW_NO_BRACKET, qtrc("inspector", "None"), Icon::TUPLET_NUMBER_ONLY)
    };

    return types;
}

void TupletSettingsModel::createProperties()
{
    m_directionType = buildPropertyItem(Ms::Pid::DIRECTION);
    m_numberType = buildPropertyItem(Ms::Pid::NUMBER_TYPE);
    m_bracketType = buildPropertyItem(Ms::Pid::BRACKET_TYPE);
    m_lineThickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
}

void TupletSettingsModel::loadProperties()
{
    loadPropertyItem(m_directionType);
    loadPropertyItem(m_numberType);
    loadPropertyItem(m_bracketType);
    loadPropertyItem(m_lineThickness);
}

void TupletSettingsModel::resetProperties()
{
    m_directionType->resetToDefault();
    m_numberType->resetToDefault();
    m_bracketType->resetToDefault();
    m_lineThickness->resetToDefault();
}
