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
#include "keysignaturesettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

KeySignatureSettingsModel::KeySignatureSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_KEYSIGNATURE);
    setTitle(qtrc("inspector", "Key signature"));
    createProperties();
}

void KeySignatureSettingsModel::createProperties()
{
    m_hasToShowCourtesy = buildPropertyItem(Ms::Pid::SHOW_COURTESY);
    m_mode = buildPropertyItem(Ms::Pid::KEYSIG_MODE);
}

void KeySignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::KEYSIG);
}

void KeySignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_hasToShowCourtesy, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toBool();
    });

    loadPropertyItem(m_mode);
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
