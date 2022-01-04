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
#include "vibratosettingsmodel.h"

#include "translation.h"

#include "libmscore/vibrato.h"

using namespace mu::inspector;

VibratoSettingsModel::VibratoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, Ms::ElementType::VIBRATO)
{
    setModelType(InspectorModelType::TYPE_VIBRATO);
    setTitle(qtrc("inspector", "Vibrato"));
    setIcon(ui::IconCode::Code::VIBRATO);

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
    QMap<Ms::Vibrato::Type, QString> types {
        { Ms::Vibrato::Type::GUITAR_VIBRATO, mu::qtrc("inspector", "Vibrato") },
        { Ms::Vibrato::Type::GUITAR_VIBRATO_WIDE, mu::qtrc("inspector", "Vibrato wide") },
        { Ms::Vibrato::Type::VIBRATO_SAWTOOTH, mu::qtrc("inspector", "Vibrato sawtooth") },
        { Ms::Vibrato::Type::VIBRATO_SAWTOOTH_WIDE, mu::qtrc("inspector", "Vibrato sawtooth wide") }
    };

    QVariantList result;

    for (Ms::Vibrato::Type type : types.keys()) {
        QVariantMap obj;

        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}

void VibratoSettingsModel::createProperties()
{
    m_lineType = buildPropertyItem(Ms::Pid::VIBRATO_TYPE);
    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);
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
