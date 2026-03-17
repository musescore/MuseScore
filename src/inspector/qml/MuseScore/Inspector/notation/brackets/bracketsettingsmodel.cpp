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

#include "engraving/dom/bracket.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

BracketSettingsModel::BracketSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                           IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, iocCtx, repository, mu::engraving::ElementType::BRACKET)
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
    m_bracketColumnPosition = buildPropertyItem(mu::engraving::Pid::BRACKET_COLUMN, [&](Pid pid, const QVariant val) {
        if (!areSettingsAvailable()) {
            return;
        }
        BracketItem* b = toBracketItem(m_elementList.front());
        Staff* staff = b->staff();
        beginCommand(TranslatableString("undoableAction", "Edit %1").arg(propertyUserName(pid)));
        staff->changeBracketColumn(b->column(), val.toInt());
        staff->score()->setLayoutAll();
        updateNotation();
        endCommand();
    });

    m_bracketSpanStaves = buildPropertyItem(mu::engraving::Pid::BRACKET_SPAN);
    m_longName = buildPropertyItem(mu::engraving::Pid::STAFF_LONG_NAME);
    m_shortName = buildPropertyItem(mu::engraving::Pid::STAFF_SHORT_NAME);
    m_showText = buildPropertyItem(mu::engraving::Pid::GROUP_BRACKET_SHOW_TEXT);
    m_showBracket = buildPropertyItem(mu::engraving::Pid::GROUP_BRACKET_SHOW_BRACKET);

    updateIsGroupBracket();
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
        Pid::STAFF_LONG_NAME,
        Pid::STAFF_SHORT_NAME,
        Pid::GROUP_BRACKET_SHOW_TEXT,
        Pid::GROUP_BRACKET_SHOW_BRACKET
    };

    loadProperties(propertyIdSet);

    updateIsGroupBracket();
}

void BracketSettingsModel::resetProperties()
{
    m_bracketColumnPosition->resetToDefault();
    m_bracketSpanStaves->resetToDefault();
    m_longName->resetToDefault();
    m_shortName->resetToDefault();
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

    if (muse::contains(propertyIdSet, Pid::STAFF_LONG_NAME)) {
        loadPropertyItem(m_longName);
    }

    if (muse::contains(propertyIdSet, Pid::STAFF_SHORT_NAME)) {
        loadPropertyItem(m_shortName);
    }

    if (muse::contains(propertyIdSet, Pid::GROUP_BRACKET_SHOW_TEXT)) {
        loadPropertyItem(m_showText);
    }

    if (muse::contains(propertyIdSet, Pid::GROUP_BRACKET_SHOW_BRACKET)) {
        loadPropertyItem(m_showBracket);
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

bool BracketSettingsModel::isGroupBracket() const
{
    return m_isGroupBracket;
}

void BracketSettingsModel::updateIsGroupBracket()
{
    bool isGroupBracket = false;
    if (m_elementList.count() == 1) {
        isGroupBracket = toBracketItem(m_elementList.front())->bracketType() == BracketType::GROUP;
    }

    if (m_isGroupBracket != isGroupBracket) {
        m_isGroupBracket = isGroupBracket;
        emit isGroupBracketChanged(m_isGroupBracket);
    }
}

PropertyItem* BracketSettingsModel::longName() const
{
    return m_longName;
}

PropertyItem* BracketSettingsModel::shortName() const
{
    return m_shortName;
}

PropertyItem* BracketSettingsModel::showText() const
{
    return m_showText;
}

PropertyItem* BracketSettingsModel::showBracket() const
{
    return m_showBracket;
}
