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
#include "hooksettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

HookSettingsModel::HookSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_HOOK);
    setTitle(qtrc("inspector", "Flag")); // internally called "Hook", but "Flag" in SMuFL, so here externally too

    createProperties();
}

void HookSettingsModel::createProperties()
{
    m_horizontalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QPointF(m_horizontalOffset->value().toDouble(), newValue.toDouble()));
    });
}

void HookSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HOOK);
}

void HookSettingsModel::loadProperties()
{
    loadPropertyItem(m_horizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().x());
    });

    loadPropertyItem(m_verticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });
}

void HookSettingsModel::resetProperties()
{
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

PropertyItem* HookSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* HookSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}
