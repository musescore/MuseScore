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
#include "tremolobarsettingsmodel.h"

#include "types/tremolobartypes.h"
#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

TremoloBarSettingsModel::TremoloBarSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TREMOLOBAR);
    setTitle(muse::qtrc("inspector", "Tremolo bar"));
    setIcon(muse::ui::IconCode::Code::GUITAR_TREMOLO_BAR);
    createProperties();
}

void TremoloBarSettingsModel::createProperties()
{
    m_type = buildPropertyItem(mu::engraving::Pid::TREMOLOBAR_TYPE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        if (newValue.toInt() != static_cast<int>(TremoloBarTypes::TremoloBarType::TYPE_CUSTOM)) {
            emit requestReloadPropertyItems();
        }
    });

    m_curve = buildPropertyItem(mu::engraving::Pid::TREMOLOBAR_CURVE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_lineThickness = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);
    m_scale = buildPropertyItem(mu::engraving::Pid::MAG);
}

void TremoloBarSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::TREMOLOBAR);

    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void TremoloBarSettingsModel::loadProperties()
{
    loadPropertyItem(m_type);
    loadPropertyItem(m_curve);
    loadPropertyItem(m_lineThickness, formatDoubleFunc);
    loadPropertyItem(m_scale, formatDoubleFunc);
}

void TremoloBarSettingsModel::resetProperties()
{
    m_type->resetToDefault();
    m_curve->resetToDefault();
    m_lineThickness->resetToDefault();
    m_scale->resetToDefault();
}

PropertyItem* TremoloBarSettingsModel::type() const
{
    return m_type;
}

PropertyItem* TremoloBarSettingsModel::curve() const
{
    return m_curve;
}

PropertyItem* TremoloBarSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

PropertyItem* TremoloBarSettingsModel::scale() const
{
    return m_scale;
}

bool TremoloBarSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // TremoloBar inspector doesn't support multiple selection
}
