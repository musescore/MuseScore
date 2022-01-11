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
#include "pedalsettingsmodel.h"

#include "translation.h"
#include "ui/view/iconcodes.h"

#include "libmscore/pedal.h"

using namespace mu::inspector;

using IconCode = mu::ui::IconCode::Code;

static constexpr int HOOK_STAR = static_cast<int>(Ms::HookType::HOOK_90T) + 1;

PedalSettingsModel::PedalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : TextLineSettingsModel(parent, repository, Ms::ElementType::PEDAL)
{
    setModelType(InspectorModelType::TYPE_PEDAL);
    setTitle(qtrc("inspector", "Pedal"));
    setIcon(ui::IconCode::Code::PEDAL_MARKING);

    static const QList<HookTypeInfo> startHookTypes {
        { Ms::HookType::NONE, IconCode::LINE_NORMAL, qtrc("inspector", "Normal") },
        { Ms::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_START_HOOK, qtrc("inspector", "Hooked 45") },
        { Ms::HookType::HOOK_90T, IconCode::LINE_WITH_T_LINE_START_HOOK, qtrc("inspector", "Hooked 90 T-style") }
    };

    setPossibleStartHookTypes(startHookTypes);

    static const QList<HookTypeInfo> endHookTypes {
        { Ms::HookType::NONE, IconCode::LINE_NORMAL, qtrc("inspector", "Normal") },
        { Ms::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK, qtrc("inspector", "Hooked 90") },
        { Ms::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK, qtrc("inspector", "Hooked 45") },
        { Ms::HookType::HOOK_90T, IconCode::LINE_WITH_T_LIKE_END_HOOK, qtrc("inspector", "Hooked 90 T-style") },
        { HOOK_STAR, IconCode::LINE_PEDAL_STAR_ENDING, qtrc("inspector", "Asterisk") }
    };

    setPossibleEndHookTypes(endHookTypes);

    createProperties();
}

PropertyItem* PedalSettingsModel::lineType() const
{
    return m_lineType;
}

bool PedalSettingsModel::pedalSymbolVisible() const
{
    return beginingText()->value().toString() == Ms::Pedal::PEDAL_SYMBOL;
}

bool PedalSettingsModel::isChangingLineVisibilityAllowed() const
{
    return isStarSymbolVisible();
}

bool PedalSettingsModel::isStarSymbolVisible() const
{
    return endText()->value().toString() == Ms::Pedal::STAR_SYMBOL;
}

void PedalSettingsModel::setPedalSymbolVisible(bool visible)
{
    beginingText()->setValue(visible ? Ms::Pedal::PEDAL_SYMBOL : "");
}

void PedalSettingsModel::createProperties()
{
    TextLineSettingsModel::createProperties();

    connect(beginingText(), &PropertyItem::isModifiedChanged, this, [this]() {
        emit pedalSymbolVisibleChanged();
    });

    connect(endText(), &PropertyItem::isModifiedChanged, this, [this]() {
        emit isChangingLineVisibilityAllowedChanged();
    });

    m_lineType = buildPropertyItem(Ms::Pid::END, [this](const Ms::Pid, const QVariant& newValue) {
        setLineType(newValue.toInt());
    });

    isLineVisible()->setIsVisible(false);
    allowDiagonal()->setIsVisible(false);
    placement()->setIsVisible(false);
}

void PedalSettingsModel::loadProperties()
{
    TextLineSettingsModel::loadProperties();

    m_lineType->setIsEnabled(true);

    if (isStarSymbolVisible()) {
        m_lineType->setValue(HOOK_STAR);
    } else {
        m_lineType->setValue(endHookType()->value());
    }
}

void PedalSettingsModel::setLineType(int newType)
{
    bool rosetteHookSelected = (newType == HOOK_STAR);
    int hookType = newType;
    QString text = QString();

    if (rosetteHookSelected) {
        hookType = static_cast<int>(Ms::HookType::NONE);
        text = Ms::Pedal::STAR_SYMBOL;
        startHookType()->setValue(hookType);
    }

    endHookType()->setValue(hookType);
    endText()->setValue(text);
    isLineVisible()->setValue(!rosetteHookSelected);

    m_lineType->setValue(newType);
}

bool PedalSettingsModel::isTextVisible(TextType) const
{
    return true;
}
