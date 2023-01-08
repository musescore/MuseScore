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

#include "dataformatter.h"
#include "translation.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::engraving;

TimeSignatureSettingsModel::TimeSignatureSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TIME_SIGNATURE);
    setTitle(qtrc("inspector", "Time signature"));
    setIcon(ui::IconCode::Code::TIME_SIGNATURE);
    createProperties();
}

void TimeSignatureSettingsModel::createProperties()
{
    m_horizontalScale = buildPropertyItem(
        Pid::SCALE,
        [](const QVariant& value, const EngravingItem* element) {
        double hor = value.toDouble() / 100;
        double ver = element->getProperty(Pid::SCALE).value<ScaleF>().height();
        return ScaleF(hor, ver);
    },
        [this](const QVariant& value) {
        double hor = value.toDouble() / 100;
        double ver = m_verticalScale->value().toDouble() / 100; // TODO: What if m_verticalScale->value() is invalid?
        return ScaleF(hor, ver);
    });

    m_verticalScale = buildPropertyItem(
        Pid::SCALE,
        [](const QVariant& value, const EngravingItem* element) {
        double hor = element->getProperty(Pid::SCALE).value<ScaleF>().width();
        double ver = value.toDouble() / 100;
        return ScaleF(hor, ver);
    },
        [this](const QVariant& value) {
        double hor = m_horizontalScale->value().toDouble() / 100; // TODO: What if m_horizontalScale->value() is invalid?
        double ver = value.toDouble() / 100;
        return ScaleF(hor, ver);
    });

    m_shouldShowCourtesy = buildPropertyItem(mu::engraving::Pid::SHOW_COURTESY);
}

void TimeSignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::TIMESIG);
}

void TimeSignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_horizontalScale, [](const engraving::PropertyValue& propertyValue) -> QVariant {
        return DataFormatter::roundDouble(propertyValue.value<ScaleF>().width()) * 100;
    });

    loadPropertyItem(m_verticalScale, [](const engraving::PropertyValue& propertyValue) -> QVariant {
        return DataFormatter::roundDouble(propertyValue.value<ScaleF>().height()) * 100;
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
    dispatcher()->dispatch("time-signature-properties");
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
