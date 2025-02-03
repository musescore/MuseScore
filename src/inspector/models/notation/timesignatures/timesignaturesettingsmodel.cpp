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
#include "timesignaturesettingsmodel.h"

#include "dom/layoutbreak.h"
#include "dom/measure.h"
#include "dataformatter.h"
#include "translation.h"

using namespace mu::inspector;

TimeSignatureSettingsModel::TimeSignatureSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TIME_SIGNATURE);
    setTitle(muse::qtrc("inspector", "Time signature"));
    setIcon(muse::ui::IconCode::Code::TIME_SIGNATURE);
    createProperties();
}

void TimeSignatureSettingsModel::createProperties()
{
    m_horizontalScale = buildPropertyItem(mu::engraving::Pid::SCALE,
                                          [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QSizeF(newValue.toDouble() / 100, m_verticalScale->value().toDouble() / 100));
    },
                                          [this](const mu::engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, QSizeF(newValue.toDouble() / 100, m_verticalScale->value().toDouble() / 100));
        emit requestReloadPropertyItems();
    });

    m_verticalScale = buildPropertyItem(mu::engraving::Pid::SCALE,
                                        [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QSizeF(m_horizontalScale->value().toDouble() / 100, newValue.toDouble() / 100));
    },
                                        [this](const mu::engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, QSizeF(m_horizontalScale->value().toDouble() / 100, newValue.toDouble() / 100));
        emit requestReloadPropertyItems();
    });

    m_shouldShowCourtesy = buildPropertyItem(mu::engraving::Pid::SHOW_COURTESY);
}

void TimeSignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::TIMESIG);
}

void TimeSignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_horizontalScale, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.value<QSizeF>().width()) * 100;
    });

    loadPropertyItem(m_verticalScale, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.value<QSizeF>().height()) * 100;
    });

    loadPropertyItem(m_shouldShowCourtesy);

    bool enableCourtesy = true;

    for (const mu::engraving::EngravingItem* element : m_elementList) {
        if (element->generated()) {
            m_isGenerated = true;
        }

        const engraving::Measure* measure = element->findMeasure();
        const engraving::Measure* prevMeasure = measure ? measure->prevMeasure() : nullptr;
        const engraving::LayoutBreak* sectionBreak = prevMeasure ? prevMeasure->sectionBreakElement() : nullptr;
        if (sectionBreak && !sectionBreak->showCourtesy()) {
            enableCourtesy = false;
            break;
        }
    }

    m_shouldShowCourtesy->setIsEnabled(!m_isGenerated && enableCourtesy);
    m_horizontalScale->setIsEnabled(!m_isGenerated);
    m_verticalScale->setIsEnabled(!m_isGenerated);
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
