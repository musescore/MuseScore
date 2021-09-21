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

#include "hairpinlinesettingsmodel.h"

#include "libmscore/hairpin.h"

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::inspector;

using IconCode = mu::ui::IconCode::Code;

HairpinLineSettingsModel::HairpinLineSettingsModel(QObject* parent, IElementRepositoryService* repository, HairpinLineType lineType)
    : LineSettingsModel(parent, repository)
{
    QString title = qtrc("inspector", "Crescendo");
    InspectorModelType type = InspectorModelType::TYPE_CRESCENDO;

    if (lineType == Diminuendo) {
        title = qtrc("inspector", "Diminuendo");
        type = InspectorModelType::TYPE_DIMINUENDO;
    }

    setModelType(type);
    setTitle(title);
    setIcon(ui::IconCode::Code::CRESCENDO_LINE);

    static const QList<HookTypeInfo> hookTypes {
        { Ms::HookType::NONE, IconCode::LINE_NORMAL },
        { Ms::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK },
        { Ms::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK },
        { Ms::HookType::HOOK_90T, IconCode::LINE_WITH_T_LIKE_END_HOOK }
    };

    setPossibleEndHookTypes(hookTypes);

    createProperties();
}

void HairpinLineSettingsModel::createProperties()
{
    LineSettingsModel::createProperties();

    allowDiagonal()->setIsVisible(false);
}

void HairpinLineSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HAIRPIN, [](const Ms::EngravingItem* element) -> bool {
        const Ms::Hairpin* hairpin = Ms::toHairpin(element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == Ms::HairpinType::CRESC_LINE || hairpin->hairpinType() == Ms::HairpinType::DECRESC_LINE;
    });
}
