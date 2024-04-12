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

#include "ottavasettingsmodel.h"

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::inspector;

using IconCode = muse::ui::IconCode::Code;

OttavaSettingsModel::OttavaSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : TextLineSettingsModel(parent, repository, mu::engraving::ElementType::OTTAVA)
{
    setTitle(muse::qtrc("inspector", "Ottava"));
    setModelType(InspectorModelType::TYPE_OTTAVA);
    setIcon(muse::ui::IconCode::Code::OTTAVA);

    setPossibleStartHookTypes({});

    static const QList<HookTypeInfo> endHookTypes {
        { mu::engraving::HookType::NONE, IconCode::LINE_NORMAL, muse::qtrc("inspector", "Normal") },
        { mu::engraving::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK, muse::qtrc("inspector", "Hooked 90°") },
        { mu::engraving::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK, muse::qtrc("inspector", "Hooked 45°") }
    };

    setPossibleEndHookTypes(endHookTypes);

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
    QMap<mu::engraving::OttavaType, QString> ottavaTypes {
        { mu::engraving::OttavaType::OTTAVA_8VA, muse::qtrc("inspector", "8va alta") },
        { mu::engraving::OttavaType::OTTAVA_8VB, muse::qtrc("inspector", "8va bassa") },
        { mu::engraving::OttavaType::OTTAVA_15MA, muse::qtrc("inspector", "15ma alta") },
        { mu::engraving::OttavaType::OTTAVA_15MB, muse::qtrc("inspector", "15ma bassa") },
        { mu::engraving::OttavaType::OTTAVA_22MA, muse::qtrc("inspector", "22ma alta") },
        { mu::engraving::OttavaType::OTTAVA_22MB, muse::qtrc("inspector", "22ma bassa") }
    };

    QVariantList result;

    for (mu::engraving::OttavaType type : ottavaTypes.keys()) {
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

    m_ottavaType = buildPropertyItem(mu::engraving::Pid::OTTAVA_TYPE);
    m_showNumbersOnly = buildPropertyItem(mu::engraving::Pid::NUMBERS_ONLY);

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
