/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "automationwidgetmodel.h"

#include "types/translatablestring.h"

#include "automationuiutils.h"
#include "notationcommands.h"

#include "log.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;

using mu::engraving::AutomationType;
using namespace mu::notation::automationui;

static const QString VOICE_ITEM_ID_PREFIX("automation-widget-voice-");
static const QString ALL_VOICES_ITEM_ID("automation-widget-voice-all");
static const QString SET_TEMPO_RANGE_ITEM_ID("automation-widget-set-tempo-range");

AutomationWidgetModel::AutomationWidgetModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void AutomationWidgetModel::load()
{
    AbstractMenuModel::load();

    if (!m_subscribed) {
        m_subscribed = true;

        commandsController()->automationModeEnabledChanged().onNotify(this, [this]() {
            emit isVisibleChanged();
        });

        notationConfiguration()->automationUiVariantChanged().onNotify(this, [this]() {
            emit isVisibleChanged();
        });

        notationConfiguration()->currentAutomationTypeChanged().onNotify(this, [this]() {
            buildItems();
            emit currentTypeChanged();
        });
    }

    buildItems();
}

bool AutomationWidgetModel::isVisible() const
{
    return notationConfiguration()->automationUiVariant() == AutomationUiVariant::Widget
           && commandsController()->isAutomationModeEnabled();
}

QString AutomationWidgetModel::currentTypeTitle() const
{
    return typeTitle(notationConfiguration()->currentAutomationType()).toQString();
}

void AutomationWidgetModel::buildItems()
{
    const AutomationType type = notationConfiguration()->currentAutomationType();

    MenuItemList items {
        makeTypeItem(AutomationType::Dynamics, "dynamics"),
        makeTypeItem(AutomationType::Volume, "volume"),
        makeTypeItem(AutomationType::Pan, "pan"),
        makeTypeItem(AutomationType::Tempo, "tempo"),
        makeSeparator(),
    };

    if (type == AutomationType::Tempo) {
        items << makeItem(SET_TEMPO_RANGE_ITEM_ID, u"Set tempo range…", this);
    } else {
        // Voice selection is only available for Dynamics
        if (type == AutomationType::Dynamics) {
            items << makeMenu(TranslatableString::untranslatable(voiceTitle(m_voiceIdx)), makeVoiceItems(), "automation-widget-voices")
                  << makeSeparator();
        }

        MenuItem* resetMenu = makeMenu(TranslatableString::untranslatable("Reset"), makeResetItems(type, this), "automation-widget-reset");
        resetMenu->setIcon(IconCode::Code::UNDO);
        items << resetMenu;
    }

    setItems(items);
}

MenuItem* AutomationWidgetModel::makeTypeItem(AutomationType type, const std::string& queryTypeParam)
{
    rcommand::CommandQuery query(SELECT_AUTOMATION_TYPE_COMMAND);
    query.addParam("type", Val(queryTypeParam));

    MenuItem* item = makeMenuItem(query, TranslatableString::untranslatable(typeTitle(type)));
    if (!item) {
        return item;
    }

    item->setChecked(notationConfiguration()->currentAutomationType() == type);

    return item;
}

MenuItemList AutomationWidgetModel::makeVoiceItems()
{
    MenuItemList items {
        makeItem(ALL_VOICES_ITEM_ID, voiceTitle(std::nullopt), this, true, !m_voiceIdx.has_value())
    };

    for (int voiceIdx = 0; voiceIdx < VOICES_COUNT; ++voiceIdx) {
        items << makeItem(VOICE_ITEM_ID_PREFIX + QString::number(voiceIdx), voiceTitle(voiceIdx), this, true, m_voiceIdx == voiceIdx);
    }

    return items;
}

void AutomationWidgetModel::onCommandStateChanged(const rcommand::Command& command, const rcommand::CommandState& state)
{
    //! NOTE: the type items share one (parameterized) command, whose generic state is always unchecked;
    //! their checked state is managed here instead (see makeTypeItem)
    if (command == SELECT_AUTOMATION_TYPE_COMMAND) {
        return;
    }

    AbstractMenuModel::onCommandStateChanged(command, state);
}

void AutomationWidgetModel::handleMenuItem(const QString& itemId)
{
    if (itemId == ALL_VOICES_ITEM_ID) {
        m_voiceIdx.reset();
        buildItems();
        return;
    }

    if (itemId.startsWith(VOICE_ITEM_ID_PREFIX)) {
        m_voiceIdx = itemId.mid(VOICE_ITEM_ID_PREFIX.size()).toInt();
        buildItems();
        return;
    }

    if (isResetItem(itemId)) {
        handleResetItem(itemId);
        return;
    }

    if (itemId == SET_TEMPO_RANGE_ITEM_ID) {
        interactive()->open(SET_TEMPO_RANGE_DIALOG_URI);
        return;
    }

    AbstractMenuModel::handleMenuItem(itemId);
}
