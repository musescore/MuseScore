/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/dom/tuplet.h"

#include "translation.h"

#include "types/commontypes.h"

using namespace mu::propertiespanel;

using Icon = muse::ui::IconCode::Code;

TupletSettingsModel::TupletSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository, mu::engraving::ElementType::TUPLET)
{
    setModelType(PropertiesPanelModelType::TYPE_TUPLET);
    setTitle(muse::qtrc("propertiespanel", "Tuplet"));
    setIcon(muse::ui::IconCode::Code::TUPLET_NUMBER_WITH_BRACKETS);
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

QVariantList TupletSettingsModel::possibleNumberTypes() const
{
    using Type = mu::engraving::TupletNumberType;

    QVariantList types {
        object(Type::SHOW_NUMBER, muse::qtrc("propertiespanel", "Number", "tuplet number type")),
        object(Type::SHOW_RELATION, muse::qtrc("propertiespanel", "Ratio", "tuplet number type")),
        object(Type::NO_TEXT, muse::qtrc("propertiespanel", "None", "tuplet number type"))
    };

    return types;
}

QVariantList TupletSettingsModel::possibleBracketTypes() const
{
    using Type = mu::engraving::TupletBracketType;

    QVariantList types {
        object(Type::AUTO_BRACKET, muse::qtrc("propertiespanel", "Auto", "tuplet bracket type")),
        object(Type::SHOW_BRACKET, muse::qtrc("propertiespanel", "Bracket", "tuplet bracket type"), Icon::TUPLET_NUMBER_WITH_BRACKETS),
        object(Type::SHOW_NO_BRACKET, muse::qtrc("propertiespanel", "None", "tuplet bracket type"), Icon::TUPLET_NUMBER_ONLY)
    };

    return types;
}

void TupletSettingsModel::createProperties()
{
    m_directionType = buildPropertyItem(mu::engraving::Pid::DIRECTION);
    m_numberType = buildPropertyItem(mu::engraving::Pid::NUMBER_TYPE);
    m_bracketType = buildPropertyItem(mu::engraving::Pid::BRACKET_TYPE);
    m_lineThickness = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);
}

void TupletSettingsModel::loadProperties()
{
    loadPropertyItem(m_directionType);
    loadPropertyItem(m_numberType);
    loadPropertyItem(m_bracketType);
    loadPropertyItem(m_lineThickness, formatDoubleFunc);
}
