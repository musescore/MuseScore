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
#include "vibratosettingsmodel.h"

#include "translation.h"

#include "engraving/dom/vibrato.h"

using namespace mu::inspector;

VibratoSettingsModel::VibratoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, mu::engraving::ElementType::VIBRATO)
{
    setModelType(InspectorModelType::TYPE_VIBRATO);
    setTitle(muse::qtrc("inspector", "Vibrato"));
    setIcon(muse::ui::IconCode::Code::VIBRATO);

    createProperties();
}

PropertyItem* VibratoSettingsModel::lineType() const
{
    return m_lineType;
}

PropertyItem* VibratoSettingsModel::placement() const
{
    return m_placement;
}

QVariantList VibratoSettingsModel::possibleLineTypes() const
{
    QMap<mu::engraving::VibratoType, QString> types {
        { mu::engraving::VibratoType::GUITAR_VIBRATO, muse::qtrc("inspector", "Vibrato") },
        { mu::engraving::VibratoType::GUITAR_VIBRATO_WIDE, muse::qtrc("inspector", "Vibrato wide") },
        { mu::engraving::VibratoType::VIBRATO_SAWTOOTH, muse::qtrc("inspector", "Vibrato sawtooth") },
        { mu::engraving::VibratoType::VIBRATO_SAWTOOTH_WIDE, muse::qtrc("inspector", "Vibrato sawtooth wide") }
    };

    QVariantList result;

    for (mu::engraving::VibratoType type : types.keys()) {
        QVariantMap obj;

        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}

void VibratoSettingsModel::createProperties()
{
    m_lineType = buildPropertyItem(mu::engraving::Pid::VIBRATO_TYPE);
    m_placement = buildPropertyItem(mu::engraving::Pid::PLACEMENT);
}

void VibratoSettingsModel::loadProperties()
{
    loadPropertyItem(m_lineType);
    loadPropertyItem(m_placement);
}

void VibratoSettingsModel::resetProperties()
{
    m_lineType->resetToDefault();
    m_placement->resetToDefault();
}
