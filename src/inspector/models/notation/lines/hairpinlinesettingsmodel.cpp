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

#include "hairpinlinesettingsmodel.h"

#include "engraving/dom/hairpin.h"

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::inspector;

using IconCode = muse::ui::IconCode::Code;

HairpinLineSettingsModel::HairpinLineSettingsModel(QObject* parent, IElementRepositoryService* repository, HairpinLineType lineType)
    : TextLineSettingsModel(parent, repository)
{
    if (lineType == Diminuendo) {
        setModelType(InspectorModelType::TYPE_DIMINUENDO);
        setTitle(muse::qtrc("inspector", "Diminuendo"));
        setIcon(muse::ui::IconCode::Code::DIMINUENDO);
    } else {
        setModelType(InspectorModelType::TYPE_CRESCENDO);
        setTitle(muse::qtrc("inspector", "Crescendo"));
        setIcon(muse::ui::IconCode::Code::CRESCENDO);
    }

    m_hairpinType = lineType == Crescendo ? engraving::HairpinType::CRESC_LINE : engraving::HairpinType::DECRESC_LINE;

    createProperties();
}

PropertyItem* HairpinLineSettingsModel::snapBefore() const
{
    return m_snapBefore;
}

PropertyItem* HairpinLineSettingsModel::snapAfter() const
{
    return m_snapAfter;
}

void HairpinLineSettingsModel::createProperties()
{
    TextLineSettingsModel::createProperties();

    m_snapBefore = buildPropertyItem(mu::engraving::Pid::SNAP_BEFORE);
    m_snapAfter = buildPropertyItem(mu::engraving::Pid::SNAP_AFTER);

    isLineVisible()->setIsVisible(true);
    allowDiagonal()->setIsVisible(false);
    placement()->setIsVisible(true);
}

void HairpinLineSettingsModel::loadProperties()
{
    TextLineSettingsModel::loadProperties();

    loadPropertyItem(m_snapBefore);
    loadPropertyItem(m_snapAfter);
}

void HairpinLineSettingsModel::resetProperties()
{
    TextLineSettingsModel::resetProperties();

    m_snapBefore->resetToDefault();
    m_snapAfter->resetToDefault();
}

void HairpinLineSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::HAIRPIN, [this](const mu::engraving::EngravingItem* element) -> bool {
        const mu::engraving::Hairpin* hairpin = mu::engraving::toHairpin(
            element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == m_hairpinType;
    });
}
