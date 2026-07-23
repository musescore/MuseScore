/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/dom/ottava.h"

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::propertiespanel;

using IconCode = muse::ui::IconCode::Code;

OttavaSettingsModel::OttavaSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository)
    : TextLineSettingsModel(parent, iocCtx, repository, mu::engraving::ElementType::OTTAVA)
{
    setTitle(muse::qtrc("propertiespanel", "Ottava"));
    setModelType(PropertiesPanelModelType::TYPE_OTTAVA);
    setIcon(muse::ui::IconCode::Code::OTTAVA);

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
        { mu::engraving::OttavaType::OTTAVA_8VA, muse::qtrc("propertiespanel", "8va alta") },
        { mu::engraving::OttavaType::OTTAVA_8VB, muse::qtrc("propertiespanel", "8va bassa") },
        { mu::engraving::OttavaType::OTTAVA_15MA, muse::qtrc("propertiespanel", "15ma alta") },
        { mu::engraving::OttavaType::OTTAVA_15MB, muse::qtrc("propertiespanel", "15ma bassa") },
        { mu::engraving::OttavaType::OTTAVA_22MA, muse::qtrc("propertiespanel", "22ma alta") },
        { mu::engraving::OttavaType::OTTAVA_22MB, muse::qtrc("propertiespanel", "22ma bassa") }
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

void OttavaSettingsModel::updateStartAndEndHookTypes()
{
    setPossibleStartHookTypes({});

    static const QList<HookTypeInfo> endHookTypes {
        { mu::engraving::HookType::NONE, IconCode::LINE_NORMAL, muse::qtrc("propertiespanel", "Normal", "hook type") },
        { mu::engraving::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK, muse::qtrc("propertiespanel", "Hooked 90°", "hook type") },
        { mu::engraving::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK, muse::qtrc("propertiespanel", "Hooked 45°", "hook type") },
        { mu::engraving::HookType::ARROW, IconCode::LINE_ARROW_RIGHT, muse::qtrc("propertiespanel", "Line arrow", "hook type") },
        { mu::engraving::HookType::ARROW_FILLED, IconCode::FILLED_ARROW_RIGHT, muse::qtrc("propertiespanel", "Filled arrow", "hook type") }
    };

    setPossibleEndHookTypes(endHookTypes);
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
    updateStartAndEndHookTypes();
}
