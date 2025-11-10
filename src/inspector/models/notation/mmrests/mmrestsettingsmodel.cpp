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
#include "mmrestsettingsmodel.h"
#include "dom/mmrest.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

MMRestSettingsModel::MMRestSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_MMREST);
    setTitle(muse::qtrc("inspector", "Multimeasure rest"));
    setIcon(muse::ui::IconCode::Code::MULTIMEASURE_REST);
    createProperties();

    if (notation::INotationStylePtr notationStyle = style()) {
        notationStyle->styleChanged().onNotify(this, [this]() {
            updateNumberOptionsEnabled();
        });
    }
}

void MMRestSettingsModel::createProperties()
{
    m_isNumberVisible = buildPropertyItem(mu::engraving::Pid::MMREST_NUMBER_VISIBLE);
    m_numberPosition = buildPropertyItem(mu::engraving::Pid::MMREST_NUMBER_OFFSET);
}

void MMRestSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::MMREST);
}

void MMRestSettingsModel::loadProperties()
{
    loadPropertyItem(m_isNumberVisible);
    loadPropertyItem(m_numberPosition);

    updateNumberOptionsEnabled();
}

void MMRestSettingsModel::resetProperties()
{
    m_isNumberVisible->resetToDefault();
    m_numberPosition->resetToDefault();
}

PropertyItem* MMRestSettingsModel::isNumberVisible() const
{
    return m_isNumberVisible;
}

PropertyItem* MMRestSettingsModel::numberPosition() const
{
    return m_numberPosition;
}

bool MMRestSettingsModel::areNumberOptionsEnabled() const
{
    return m_areNumberOptionsEnabled;
}

void MMRestSettingsModel::updateNumberOptionsEnabled()
{
    bool enabled = true;
    for (EngravingItem* item : m_elementList) {
        IF_ASSERT_FAILED(item->isMMRest()) {
            continue;
        }
        MMRest* mmRest = toMMRest(item);
        if (!mmRest->shouldShowNumberByDefault()) {
            enabled = false;
            break;
        }
    }

    if (enabled != m_areNumberOptionsEnabled) {
        m_areNumberOptionsEnabled = enabled;
        emit areNumberOptionsEnabledChanged(m_areNumberOptionsEnabled);
    }
}
