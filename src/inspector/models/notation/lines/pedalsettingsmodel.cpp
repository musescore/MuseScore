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
#include "pedalsettingsmodel.h"

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::inspector;

using IconCode = mu::ui::IconCode::Code;

PedalSettingsModel::PedalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : LineSettingsModel(parent, repository, Ms::ElementType::PEDAL)
{
    setModelType(InspectorModelType::TYPE_PEDAL);
    setTitle(qtrc("inspector", "Pedal"));
    setIcon(ui::IconCode::Code::PEDAL_MARKING);

    static const QList<HookTypeInfo> hookTypes {
        { Ms::HookType::NONE, IconCode::LINE_NORMAL },
        { Ms::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK },
        { Ms::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK },
        { Ms::HookType::HOOK_90T, IconCode::LINE_PEDAL_STAR_ENDING }
    };

    setPossibleHookTypes(hookTypes);

    createProperties();
}

PropertyItem* PedalSettingsModel::showPedalSymbol() const
{
    return m_showPedalSymbol;
}

PropertyItem* PedalSettingsModel::showLineWithRosette() const
{
    return m_showLineWithRosette;
}

bool PedalSettingsModel::showLineWithRosetteVisible() const
{
    return m_showLineWithRosetteVisible;
}

void PedalSettingsModel::createProperties()
{
    LineSettingsModel::createProperties();

    //! TODO: determine suitable properties
    m_showPedalSymbol = buildPropertyItem(Ms::Pid::SYMBOL);
    m_showLineWithRosette = buildPropertyItem(Ms::Pid::SYMBOL);
}

void PedalSettingsModel::loadProperties()
{
    LineSettingsModel::loadProperties();

    loadPropertyItem(m_showPedalSymbol);
    loadPropertyItem(m_showLineWithRosette);
}

void PedalSettingsModel::resetProperties()
{
    LineSettingsModel::resetProperties();

    m_showPedalSymbol->resetToDefault();
    m_showLineWithRosette->resetToDefault();
}
