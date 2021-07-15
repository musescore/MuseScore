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
#include "timesignaturesettingsmodel.h"

#include <QSizeF>

#include "dataformatter.h"
#include "log.h"
#include "translation.h"

using namespace mu::inspector;

TimeSignatureSettingsModel::TimeSignatureSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TIME_SIGNATURE);
    setTitle(qtrc("inspector", "Time signature"));
    createProperties();
}

void TimeSignatureSettingsModel::createProperties()
{
    m_horizontalScale = buildPropertyItem(Ms::Pid::SCALE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QSizeF(newValue.toDouble() / 100, m_verticalScale->value().toDouble() / 100));
    });

    m_verticalScale = buildPropertyItem(Ms::Pid::SCALE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QSizeF(m_horizontalScale->value().toDouble() / 100, newValue.toDouble() / 100));
    });

    m_shouldShowCourtesy = buildPropertyItem(Ms::Pid::SHOW_COURTESY);
}

void TimeSignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TIMESIG);
}

void TimeSignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_horizontalScale, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toSizeF().width()) * 100;
    });

    loadPropertyItem(m_verticalScale, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toSizeF().height()) * 100;
    });

    loadPropertyItem(m_shouldShowCourtesy);
}

void TimeSignatureSettingsModel::resetProperties()
{
    m_horizontalScale->resetToDefault();
    m_verticalScale->resetToDefault();
    m_shouldShowCourtesy->resetToDefault();
}

void TimeSignatureSettingsModel::showTimeSignatureProperties()
{
    NOT_IMPLEMENTED;
}

PropertyItem* TimeSignatureSettingsModel::horizontalScale() const
{
    return m_horizontalScale;
}

PropertyItem* TimeSignatureSettingsModel::verticalScale() const
{
    return m_verticalScale;
}

PropertyItem* TimeSignatureSettingsModel::shouldShowCourtesy() const
{
    return m_shouldShowCourtesy;
}
