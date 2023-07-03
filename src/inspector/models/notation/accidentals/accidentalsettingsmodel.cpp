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
#include "accidentalsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

AccidentalSettingsModel::AccidentalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_ACCIDENTAL);
    setTitle(qtrc("inspector", "Accidental"));
    setIcon(ui::IconCode::Code::ACCIDENTAL_SHARP);
    createProperties();
}

void AccidentalSettingsModel::createProperties()
{
    m_bracketType = buildPropertyItem(mu::engraving::Pid::ACCIDENTAL_BRACKET);
    m_isSmall = buildPropertyItem(mu::engraving::Pid::SMALL);
}

void AccidentalSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::ACCIDENTAL);
}

void AccidentalSettingsModel::loadProperties()
{
    loadPropertyItem(m_bracketType);
    loadPropertyItem(m_isSmall);
    updateIsSmallAvailable();
}

void AccidentalSettingsModel::resetProperties()
{
    m_bracketType->resetToDefault();
}

PropertyItem* AccidentalSettingsModel::bracketType() const
{
    return m_bracketType;
}

PropertyItem* AccidentalSettingsModel::isSmall() const
{
    return m_isSmall;
}

bool AccidentalSettingsModel::isSmallAvailable() const
{
    return m_isSmallAvailable;
}

void AccidentalSettingsModel::updateIsSmallAvailable()
{
    bool available = true;
    for (mu::engraving::EngravingItem* item : m_elementList) {
        if (item && item->parent() && item->parent()->isOrnament()) {
            available = false;
            break;
        }
    }
    setIsSmallAvailable(available);
}

void AccidentalSettingsModel::setIsSmallAvailable(bool available)
{
    if (m_isSmallAvailable == available) {
        return;
    }

    m_isSmallAvailable = available;
    emit isSmallAvailableChanged(m_isSmallAvailable);
}
