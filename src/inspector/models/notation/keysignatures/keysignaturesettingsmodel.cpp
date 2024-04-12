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
#include "keysignaturesettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

KeySignatureSettingsModel::KeySignatureSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_KEYSIGNATURE);
    setTitle(muse::qtrc("inspector", "Key signature"));
    setIcon(muse::ui::IconCode::Code::KEY_SIGNATURE);
    createProperties();
}

void KeySignatureSettingsModel::createProperties()
{
    m_hasToShowCourtesy = buildPropertyItem(mu::engraving::Pid::SHOW_COURTESY);
    m_mode = buildPropertyItem(mu::engraving::Pid::KEYSIG_MODE);
}

void KeySignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::KEYSIG);
}

void KeySignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_hasToShowCourtesy);
    loadPropertyItem(m_mode);

    bool enabled = true;

    for (const mu::engraving::EngravingItem* element : m_elementList) {
        if (element->generated()) {
            enabled = false;
            break;
        }
    }

    m_hasToShowCourtesy->setIsEnabled(enabled);
    m_mode->setIsEnabled(enabled);
}

void KeySignatureSettingsModel::resetProperties()
{
    m_hasToShowCourtesy->resetToDefault();
    m_mode->resetToDefault();
}

PropertyItem* KeySignatureSettingsModel::hasToShowCourtesy() const
{
    return m_hasToShowCourtesy;
}

PropertyItem* KeySignatureSettingsModel::mode() const
{
    return m_mode;
}
