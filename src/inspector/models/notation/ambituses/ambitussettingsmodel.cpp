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
#include "ambitussettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

AmbitusSettingsModel::AmbitusSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_AMBITUS);
    setTitle(qtrc("inspector", "Ambitus"));
    setIcon(ui::IconCode::Code::AMBITUS);
    createProperties();
}

void AmbitusSettingsModel::createProperties()
{
    m_noteheadGroup = buildPropertyItem(Ms::Pid::HEAD_GROUP);
    m_noteheadType = buildPropertyItem(Ms::Pid::HEAD_TYPE);

    m_topTpc = buildPropertyItem(Ms::Pid::TPC1);
    m_bottomTpc = buildPropertyItem(Ms::Pid::FBPARENTHESIS1);
    m_topOctave = buildPropertyItem(Ms::Pid::FBPARENTHESIS3);
    m_bottomOctave = buildPropertyItem(Ms::Pid::FBPARENTHESIS4);

    m_topPitch = buildPropertyItem(Ms::Pid::PITCH, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_bottomPitch = buildPropertyItem(Ms::Pid::FBPARENTHESIS2, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_direction = buildPropertyItem(Ms::Pid::MIRROR_HEAD);
    m_lineThickness = buildPropertyItem(Ms::Pid::LINE_WIDTH_SPATIUM);
}

void AmbitusSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::AMBITUS);
}

void AmbitusSettingsModel::loadProperties()
{
    loadPropertyItem(m_noteheadGroup);
    loadPropertyItem(m_noteheadType);

    loadPropertyItem(m_topTpc);
    loadPropertyItem(m_bottomTpc);
    loadPropertyItem(m_topOctave);
    loadPropertyItem(m_bottomOctave);
    loadPropertyItem(m_topPitch);
    loadPropertyItem(m_bottomPitch);

    loadPropertyItem(m_direction);
    loadPropertyItem(m_lineThickness, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble());
    });
}

void AmbitusSettingsModel::resetProperties()
{
    m_noteheadGroup->resetToDefault();
    m_noteheadType->resetToDefault();

    m_direction->resetToDefault();
    m_lineThickness->resetToDefault();

    m_topTpc->resetToDefault();
    m_bottomTpc->resetToDefault();
    m_topPitch->resetToDefault();
    m_bottomPitch->resetToDefault();
}

void AmbitusSettingsModel::matchRangesToStaff()
{
    // TODO: The "default values" are not always actual and correct
    // That is because the default value gets set to a fixed value
    // only when loadProperties() is called, but for an Ambitus, it
    // is crucial that the default value is recalculated every time.
    m_topTpc->resetToDefault();
    m_bottomTpc->resetToDefault();
    m_topPitch->resetToDefault();
    m_bottomPitch->resetToDefault();
}

PropertyItem* AmbitusSettingsModel::noteheadGroup() const
{
    return m_noteheadGroup;
}

PropertyItem* AmbitusSettingsModel::noteheadType() const
{
    return m_noteheadType;
}

PropertyItem* AmbitusSettingsModel::topTpc() const
{
    return m_topTpc;
}

PropertyItem* AmbitusSettingsModel::bottomTpc() const
{
    return m_bottomTpc;
}

PropertyItem* AmbitusSettingsModel::topOctave() const
{
    return m_topOctave;
}

PropertyItem* AmbitusSettingsModel::bottomOctave() const
{
    return m_bottomOctave;
}

PropertyItem* AmbitusSettingsModel::direction() const
{
    return m_direction;
}

PropertyItem* AmbitusSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

PropertyItem* AmbitusSettingsModel::topPitch() const
{
    return m_topPitch;
}

PropertyItem* AmbitusSettingsModel::bottomPitch() const
{
    return m_bottomPitch;
}
