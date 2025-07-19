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
#include "tremolosettingsmodel.h"

#include <QList>

#include "translation.h"

#include "engraving/dom/tremolotwochord.h"

using namespace mu::inspector;
using namespace mu::engraving;

TremoloSettingsModel::TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TREMOLO);
    setTitle(muse::qtrc("inspector", "Tremolos"));
    setIcon(muse::ui::IconCode::Code::TREMOLO_TWO_NOTES);
    createProperties();
}

void TremoloSettingsModel::createProperties()
{
    m_style = buildPropertyItem(mu::engraving::Pid::TREMOLO_STYLE);
    m_direction = buildPropertyItem(mu::engraving::Pid::STEM_DIRECTION);
}

void TremoloSettingsModel::requestElements()
{
    // the tremolo section currently only has a style setting
    // so only tremolos which can have custom styles make it appear

    m_elementList.clear();
    for (EngravingItem* it : m_repository->findElementsByType(ElementType::TREMOLO_TWOCHORD)) {
        if (item_cast<TremoloTwoChord*>(it)->customStyleApplicable()) {
            m_elementList << it;
        }
    }
}

void TremoloSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::TREMOLO_STYLE)) {
        loadPropertyItem(m_style);
    }
    if (muse::contains(propertyIdSet, Pid::STEM_DIRECTION)) {
        loadPropertyItem(m_direction);
    }
}

void TremoloSettingsModel::loadProperties()
{
    loadProperties(PropertyIdSet { Pid::TREMOLO_STYLE, Pid::STEM_DIRECTION });
}

void TremoloSettingsModel::resetProperties()
{
    m_style->resetToDefault();
    m_direction->resetToDefault();
}

void TremoloSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

PropertyItem* TremoloSettingsModel::style() const
{
    return m_style;
}

PropertyItem* TremoloSettingsModel::direction() const
{
    return m_direction;
}
