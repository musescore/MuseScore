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
#include "glissandosettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

GlissandoSettingsModel::GlissandoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, Ms::ElementType::GLISSANDO)
{
    setModelType(InspectorModelType::TYPE_GLISSANDO);
    setTitle(qtrc("inspector", "Glissando"));
    setIcon(ui::IconCode::Code::GLISSANDO);

    createProperties();
}

PropertyItem* GlissandoSettingsModel::lineType() const
{
    return m_lineType;
}

PropertyItem* GlissandoSettingsModel::showText() const
{
    return m_showText;
}

PropertyItem* GlissandoSettingsModel::text() const
{
    return m_text;
}

QVariantList GlissandoSettingsModel::possibleLineTypes() const
{
    QMap<Ms::GlissandoType, QString> types {
        { Ms::GlissandoType::STRAIGHT, mu::qtrc("inspector", "Straight") },
        { Ms::GlissandoType::WAVY, mu::qtrc("inspector", "Wavy") }
    };

    QVariantList result;

    for (Ms::GlissandoType type : types.keys()) {
        QVariantMap obj;

        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}

void GlissandoSettingsModel::createProperties()
{
    m_lineType = buildPropertyItem(Ms::Pid::GLISS_TYPE);
    m_showText = buildPropertyItem(Ms::Pid::GLISS_SHOW_TEXT);
    m_text = buildPropertyItem(Ms::Pid::GLISS_TEXT);
}

void GlissandoSettingsModel::loadProperties()
{
    loadPropertyItem(m_lineType);
    loadPropertyItem(m_showText);
    loadPropertyItem(m_text);
}

void GlissandoSettingsModel::resetProperties()
{
    m_lineType->resetToDefault();
    m_showText->resetToDefault();
    m_text->resetToDefault();
}
