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
#include "bracketsettingsmodel.h"

#include "engraving/dom/bracketItem.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

BracketSettingsModel::BracketSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, mu::engraving::ElementType::BRACKET)
{
    setModelType(InspectorModelType::TYPE_BRACKET);
    setTitle(muse::qtrc("inspector", "Bracket"));
    setIcon(muse::ui::IconCode::Code::BRACKET);
    createProperties();

    connect(this, &BracketSettingsModel::selectionChanged, this, &BracketSettingsModel::maxBracketColumnPositionChanged);
    connect(m_bracketSpanStaves, &PropertyItem::propertyModified, this, &BracketSettingsModel::maxBracketColumnPositionChanged);
}

void BracketSettingsModel::createProperties()
{
    m_bracketColumnPosition = buildPropertyItem(mu::engraving::Pid::BRACKET_COLUMN);
    m_bracketSpanStaves = buildPropertyItem(mu::engraving::Pid::BRACKET_SPAN);
}

void BracketSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::BRACKET);

    emit selectionChanged();
}

void BracketSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::BRACKET_COLUMN,
        Pid::BRACKET_SPAN,
    };

    loadProperties(propertyIdSet);
}

void BracketSettingsModel::resetProperties()
{
    m_bracketColumnPosition->resetToDefault();
    m_bracketSpanStaves->resetToDefault();
}

void BracketSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void BracketSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::BRACKET_COLUMN)) {
        loadPropertyItem(m_bracketColumnPosition);
    }

    if (muse::contains(propertyIdSet, Pid::BRACKET_SPAN)) {
        loadPropertyItem(m_bracketSpanStaves);
    }
}

PropertyItem* BracketSettingsModel::bracketColumnPosition() const
{
    return m_bracketColumnPosition;
}

PropertyItem* BracketSettingsModel::bracketSpanStaves() const
{
    return m_bracketSpanStaves;
}

bool BracketSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // Brackets inspector doesn't support multiple selection
}

int BracketSettingsModel::maxBracketColumnPosition() const
{
    if (m_elementList.count() != 1) {
        return 0;
    }

    const BracketItem* bracketItem = toBracketItem(m_elementList.front());
    const Score* score = bracketItem->score();
    int bracketStartIndex = static_cast<int>(bracketItem->staff()->idx());
    int bracketEndIndex = bracketStartIndex + static_cast<int>(bracketItem->bracketSpan()) - 1;

    int count = 0;

    // Count the number of brackets that overlap with the selected one
    for (const Staff* staff : score->staves()) {
        int otherBracketStartIndex = static_cast<int>(staff->idx());
        for (const BracketItem* otherBracketItem : staff->brackets()) {
            int otherBracketEndIndex = otherBracketStartIndex + static_cast<int>(otherBracketItem->bracketSpan()) - 1;
            if (otherBracketStartIndex <= bracketEndIndex
                && otherBracketEndIndex >= bracketStartIndex) {
                ++count;
            }
        }
    }

    // The maximum column index equals the total minus 1
    return count - 1;
}

int BracketSettingsModel::maxBracketSpanStaves() const
{
    if (m_elementList.count() != 1) {
        return 0;
    }

    const BracketItem* bracketItem = toBracketItem(m_elementList.front());
    return static_cast<int>(bracketItem->score()->nstaves()) - static_cast<int>(bracketItem->staff()->idx());
}
