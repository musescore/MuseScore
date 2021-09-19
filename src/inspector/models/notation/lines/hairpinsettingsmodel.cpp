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
#include "hairpinsettingsmodel.h"

#include "libmscore/hairpin.h"

#include "dataformatter.h"
#include "translation.h"
#include "types/hairpintypes.h"

using namespace mu::inspector;

HairpinSettingsModel::HairpinSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : LineSettingsModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_HAIRPIN);
    setTitle(qtrc("inspector", "Hairpin"));
    setIcon(ui::IconCode::Code::HAIRPIN);

    createProperties();
}

PropertyItem* HairpinSettingsModel::isNienteCircleVisible() const
{
    return m_isNienteCircleVisible;
}

PropertyItem* HairpinSettingsModel::height() const
{
    return m_height;
}

PropertyItem* HairpinSettingsModel::continiousHeight() const
{
    return m_continiousHeight;
}

void HairpinSettingsModel::createProperties()
{
    LineSettingsModel::createProperties();

    m_isNienteCircleVisible = buildPropertyItem(Ms::Pid::HAIRPIN_CIRCLEDTIP);
    m_height = buildPropertyItem(Ms::Pid::HAIRPIN_HEIGHT);
    m_continiousHeight = buildPropertyItem(Ms::Pid::HAIRPIN_CONT_HEIGHT);
}

void HairpinSettingsModel::loadProperties()
{
    auto formatDoubleFunc = [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble());
    };

    LineSettingsModel::loadProperties();

    loadPropertyItem(m_isNienteCircleVisible);
    loadPropertyItem(m_height, formatDoubleFunc);
    loadPropertyItem(m_continiousHeight, formatDoubleFunc);
}

void HairpinSettingsModel::resetProperties()
{
    LineSettingsModel::resetProperties();

    m_isNienteCircleVisible->resetToDefault();
    m_height->resetToDefault();
    m_continiousHeight->resetToDefault();
}

void HairpinSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HAIRPIN, [](const Ms::EngravingItem* element) -> bool {
        const Ms::Hairpin* hairpin = Ms::toHairpin(element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == Ms::HairpinType::CRESC_HAIRPIN || hairpin->hairpinType() == Ms::HairpinType::DECRESC_HAIRPIN;
    });
}
