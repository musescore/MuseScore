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
#include "crescendosettingsmodel.h"

#include "libmscore/hairpin.h"

#include "translation.h"
#include "types/crescendotypes.h"

using namespace mu::inspector;

CrescendoSettingsModel::CrescendoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : LineSettingsModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_CRESCENDO);
    setTitle(qtrc("inspector", "Crescendo"));
    setIcon(ui::IconCode::Code::CRESCENDO_LINE);

    createProperties();
}

void CrescendoSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HAIRPIN, [](const Ms::EngravingItem* element) -> bool {
        const Ms::Hairpin* hairpin = Ms::toHairpin(element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == Ms::HairpinType::CRESC_LINE || hairpin->hairpinType() == Ms::HairpinType::DECRESC_LINE;
    });
}
