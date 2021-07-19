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
#include "bracketsettingsmodel.h"

#include "log.h"
#include "libmscore/bracket.h"

#include "translation.h"

using namespace mu::inspector;

BracketSettingsModel::BracketSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BRACKET);
    setTitle(qtrc("inspector", "Bracket"));
    setIcon(ui::IconCode::Code::BRACKET);
    createProperties();
}

void BracketSettingsModel::createProperties()
{
    m_bracketColumnPosition = buildPropertyItem(Ms::Pid::BRACKET_COLUMN);
    m_bracketSpanStaves = buildPropertyItem(Ms::Pid::BRACKET_SPAN);
}

void BracketSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BRACKET, [](const Ms::Element* element) -> bool {
        IF_ASSERT_FAILED(element) {
            return false;
        }

        const Ms::Bracket* bracket = Ms::toBracket(element);
        IF_ASSERT_FAILED(bracket) {
            return false;
        }

        return bracket->bracketType() != Ms::BracketType::BRACE;
    });
}

void BracketSettingsModel::loadProperties()
{
    loadPropertyItem(m_bracketColumnPosition);
    loadPropertyItem(m_bracketSpanStaves);
}

void BracketSettingsModel::resetProperties()
{
    m_bracketColumnPosition->resetToDefault();
    m_bracketSpanStaves->resetToDefault();
}

PropertyItem* BracketSettingsModel::bracketColumnPosition() const
{
    return m_bracketColumnPosition;
}

PropertyItem* BracketSettingsModel::bracketSpanStaves() const
{
    return m_bracketSpanStaves;
}
