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

#include "ottavasettingsmodel.h"

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::inspector;

using IconCode = mu::ui::IconCode::Code;

OttavaSettingsModel::OttavaSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : TextLineSettingsModel(parent, repository, Ms::ElementType::OTTAVA)
{
    setTitle(qtrc("inspector", "Ottava"));
    setModelType(InspectorModelType::TYPE_OTTAVA);
    setIcon(ui::IconCode::Code::OTTAVA);

    static const QList<HookTypeInfo> hookTypes {
        { Ms::HookType::NONE, IconCode::LINE_NORMAL, qtrc("inspector", "Normal") },
        { Ms::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK, qtrc("inspector", "Hooked 90") },
        { Ms::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK, qtrc("inspector", "Hooked 45") }
    };

    setPossibleEndHookTypes(hookTypes);

    createProperties();
}

PropertyItem* OttavaSettingsModel::ottavaType() const
{
    return m_ottavaType;
}

PropertyItem* OttavaSettingsModel::showNumbersOnly() const
{
    return m_showNumbersOnly;
}

QVariantList OttavaSettingsModel::possibleOttavaTypes() const
{
    QMap<Ms::OttavaType, QString> ottavaTypes {
        { Ms::OttavaType::OTTAVA_8VA, mu::qtrc("inspector", "8va alta") },
        { Ms::OttavaType::OTTAVA_8VB, mu::qtrc("inspector", "8va bassa") },
        { Ms::OttavaType::OTTAVA_15MA, mu::qtrc("inspector", "15ma alta") },
        { Ms::OttavaType::OTTAVA_15MB, mu::qtrc("inspector", "15ma bassa") },
        { Ms::OttavaType::OTTAVA_22MA, mu::qtrc("inspector", "22ma alta") },
        { Ms::OttavaType::OTTAVA_22MB, mu::qtrc("inspector", "22ma bassa") }
    };

    QVariantList result;

    for (Ms::OttavaType type : ottavaTypes.keys()) {
        QVariantMap obj;
        obj["value"] = static_cast<int>(type);
        obj["text"] = ottavaTypes[type];

        result << obj;
    }

    return result;
}

void OttavaSettingsModel::createProperties()
{
    TextLineSettingsModel::createProperties();

    m_ottavaType = buildPropertyItem(Ms::Pid::OTTAVA_TYPE);
    m_showNumbersOnly = buildPropertyItem(Ms::Pid::NUMBERS_ONLY);

    isLineVisible()->setIsVisible(true);
    allowDiagonal()->setIsVisible(true);
    placement()->setIsVisible(false);
}

void OttavaSettingsModel::loadProperties()
{
    TextLineSettingsModel::loadProperties();

    loadPropertyItem(m_ottavaType);
    loadPropertyItem(m_showNumbersOnly);
}

void OttavaSettingsModel::resetProperties()
{
    TextLineSettingsModel::resetProperties();

    m_ottavaType->resetToDefault();
    m_showNumbersOnly->resetToDefault();
}

bool OttavaSettingsModel::isTextVisible(TextType) const
{
    return true;
}
