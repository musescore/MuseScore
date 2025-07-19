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
#include "hairpinsettingsmodel.h"

#include "engraving/dom/hairpin.h"

#include "types/commontypes.h"
#include "types/hairpintypes.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

HairpinSettingsModel::HairpinSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : TextLineSettingsModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_HAIRPIN);
    setTitle(muse::qtrc("inspector", "Hairpin"));
    setIcon(muse::ui::IconCode::Code::HAIRPIN);

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

PropertyItem* HairpinSettingsModel::continuousHeight() const
{
    return m_continuousHeight;
}

PropertyItem* HairpinSettingsModel::snapBefore() const
{
    return m_snapBefore;
}

PropertyItem* HairpinSettingsModel::snapAfter() const
{
    return m_snapAfter;
}

void HairpinSettingsModel::createProperties()
{
    TextLineSettingsModel::createProperties();

    m_isNienteCircleVisible = buildPropertyItem(mu::engraving::Pid::HAIRPIN_CIRCLEDTIP);
    m_height = buildPropertyItem(mu::engraving::Pid::HAIRPIN_HEIGHT);
    m_continuousHeight = buildPropertyItem(mu::engraving::Pid::HAIRPIN_CONT_HEIGHT);

    m_snapBefore = buildPropertyItem(mu::engraving::Pid::SNAP_BEFORE);
    m_snapAfter = buildPropertyItem(mu::engraving::Pid::SNAP_AFTER);

    isLineVisible()->setIsVisible(false);
    allowDiagonal()->setIsVisible(true);
    placement()->setIsVisible(true);
}

void HairpinSettingsModel::loadProperties()
{
    TextLineSettingsModel::loadProperties();

    const static PropertyIdSet propertyIdSet {
        Pid::HAIRPIN_CIRCLEDTIP,
        Pid::HAIRPIN_HEIGHT,
        Pid::HAIRPIN_CONT_HEIGHT,
        Pid::SNAP_BEFORE,
        Pid::SNAP_AFTER
    };

    loadProperties(propertyIdSet);
}

void HairpinSettingsModel::resetProperties()
{
    TextLineSettingsModel::resetProperties();

    m_isNienteCircleVisible->resetToDefault();
    m_height->resetToDefault();
    m_continuousHeight->resetToDefault();
    m_snapBefore->resetToDefault();
    m_snapAfter->resetToDefault();
}

void HairpinSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::HAIRPIN, [](const mu::engraving::EngravingItem* element) -> bool {
        const mu::engraving::Hairpin* hairpin = mu::engraving::toHairpin(
            element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == mu::engraving::HairpinType::CRESC_HAIRPIN || hairpin->hairpinType() == mu::engraving::HairpinType::DECRESC_HAIRPIN;
    });
}

void HairpinSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet& changedStyleIdSet)
{
    TextLineSettingsModel::onNotationChanged(changedPropertyIdSet, changedStyleIdSet);
    loadProperties(changedPropertyIdSet);
}

void HairpinSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::HAIRPIN_CIRCLEDTIP)) {
        loadPropertyItem(m_isNienteCircleVisible);
    }

    if (muse::contains(propertyIdSet, Pid::HAIRPIN_HEIGHT)) {
        loadPropertyItem(m_height, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::HAIRPIN_CONT_HEIGHT)) {
        loadPropertyItem(m_continuousHeight, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::SNAP_BEFORE)) {
        loadPropertyItem(m_snapBefore);
    }

    if (muse::contains(propertyIdSet, Pid::SNAP_AFTER)) {
        loadPropertyItem(m_snapAfter);
    }
}
