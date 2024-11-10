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

#include "partssettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

PartsSettingsModel::PartsSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(muse::qtrc("inspector", "Score and part synchronization"));
    setSectionType(InspectorSectionType::SECTION_PARTS);

    createProperties();
}

void PartsSettingsModel::createProperties()
{
    m_positionLinkedToMaster = buildPropertyItem(Pid::POSITION_LINKED_TO_MASTER, [this](const Pid, const QVariant& newValue) {
        setPropertyValue(m_elementsForPartLinkingOption, Pid::POSITION_LINKED_TO_MASTER, newValue.toBool());
    });
    m_appearanceLinkedToMaster = buildPropertyItem(Pid::APPEARANCE_LINKED_TO_MASTER, [this](const Pid, const QVariant& newValue) {
        setPropertyValue(m_elementsForPartLinkingOption, Pid::APPEARANCE_LINKED_TO_MASTER, newValue.toBool());
    });
    m_textLinkedToMaster = buildPropertyItem(Pid::TEXT_LINKED_TO_MASTER, [this](const Pid, const QVariant& newValue) {
        setPropertyValue(m_elementsForTextLinkingOption, Pid::TEXT_LINKED_TO_MASTER, newValue.toBool());
    });
    m_excludeFromOtherParts = buildPropertyItem(Pid::EXCLUDE_FROM_OTHER_PARTS, [this](const Pid, const QVariant& newValue) {
        setPropertyValue(m_elementsForExcludeOption, Pid::EXCLUDE_FROM_OTHER_PARTS, newValue.toBool());
    });

    updateShowPartLinkingOption();
    updateShowExcludeOption();
    updateShowTextLinkingOption();
}

void PartsSettingsModel::requestElements()
{
    m_elementList = m_repository->takeAllElements();

    m_elementsForPartLinkingOption.clear();
    m_elementsForExcludeOption.clear();
    m_elementsForTextLinkingOption.clear();

    for (EngravingItem* item : m_elementList) {
        if (!item->score()->isMaster() && !item->isLayoutBreak()) {
            m_elementsForPartLinkingOption.push_back(item);
        }
        if (item->canBeExcludedFromOtherParts()) {
            m_elementsForExcludeOption.push_back(item);
        }
        if (item->isTextBase()) {
            m_elementsForTextLinkingOption.push_back(item);
        }
    }

    updateShowPartLinkingOption();
    updateShowExcludeOption();
    updateShowTextLinkingOption();
}

void PartsSettingsModel::loadProperties()
{
    loadPropertyItem(m_positionLinkedToMaster, m_elementsForPartLinkingOption);
    loadPropertyItem(m_appearanceLinkedToMaster, m_elementsForPartLinkingOption);
    loadPropertyItem(m_textLinkedToMaster, m_elementsForTextLinkingOption);
    loadPropertyItem(m_excludeFromOtherParts, m_elementsForExcludeOption);
}

void PartsSettingsModel::resetProperties()
{
    m_positionLinkedToMaster->resetToDefault();
    m_appearanceLinkedToMaster->resetToDefault();
    m_textLinkedToMaster->resetToDefault();
}

void PartsSettingsModel::onNotationChanged(const PropertyIdSet&, const StyleIdSet&)
{
    loadProperties();
}

PropertyItem* PartsSettingsModel::positionLinkedToMaster() const
{
    return m_positionLinkedToMaster;
}

PropertyItem* PartsSettingsModel::appearanceLinkedToMaster() const
{
    return m_appearanceLinkedToMaster;
}

PropertyItem* PartsSettingsModel::textLinkedToMaster() const
{
    return m_textLinkedToMaster;
}

PropertyItem* PartsSettingsModel::excludeFromOtherParts() const
{
    return m_excludeFromOtherParts;
}

bool PartsSettingsModel::showPartLinkingOption() const
{
    return m_showPartLinkingOption;
}

bool PartsSettingsModel::showExcludeOption() const
{
    return m_showExcludeOption;
}

bool PartsSettingsModel::showTextLinkingOption() const
{
    return m_showTextLinkingOption;
}

void PartsSettingsModel::updateShowPartLinkingOption()
{
    bool showPartLinking = !m_elementsForPartLinkingOption.empty();

    if (m_showPartLinkingOption != showPartLinking) {
        m_showPartLinkingOption = showPartLinking;
        emit showPartLinkingOptionChanged(m_showPartLinkingOption);
    }
}

void PartsSettingsModel::updateShowExcludeOption()
{
    bool showExclude = !m_elementsForExcludeOption.empty();

    if (m_showExcludeOption != showExclude) {
        m_showExcludeOption = showExclude;
        emit showExcludeOptionChanged(m_showExcludeOption);
    }
}

void PartsSettingsModel::updateShowTextLinkingOption()
{
    bool showTextLink = !m_elementsForTextLinkingOption.empty();

    if (m_showTextLinkingOption != showTextLink) {
        m_showTextLinkingOption = showTextLink;
        emit showTextLinkingOptionChanged(m_showTextLinkingOption);
    }
}
