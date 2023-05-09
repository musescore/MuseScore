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
#include "applicationuiactions.h"

#include "ui/view/iconcodes.h"
#include "context/uicontext.h"

#include "view/dockwindow/idockwindow.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::ui;
using namespace mu::actions;
using namespace mu::dock;

const ActionCode TOGGLE_NAVIGATOR_ACTION_CODE("toggle-navigator");

const UiActionList ApplicationUiActions::m_actions = {
    UiAction("quit",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Quit"),
             TranslatableString("action", "Quit")
             ),
    UiAction("restart",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Restart")
             ),
    UiAction("fullscreen",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Full screen"),
             TranslatableString("action", "Full screen"),
             Checkable::Yes
             ),
    UiAction("about-musescore",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&About MuseScore…")
             ),
    UiAction("about-qt",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "About &Qt…")
             ),
    UiAction("about-musicxml",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "About &MusicXML…")
             ),
    UiAction("online-handbook",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Online &handbook"),
             TranslatableString("action", "Open online handbook")
             ),
    UiAction("ask-help",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "As&k for help")
             ),
    UiAction("revert-factory",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Revert to &factory settings"),
             TranslatableString("action", "Revert to factory settings")
             ),

    // Docking
    UiAction("dock-restore-default-layout",
             ActionCategory::LayoutFormatting,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Restore the &default layout"),
             TranslatableString("action", "Restore the default layout")
             ),

    // Toolbars
    UiAction("toggle-transport",
             ActionCategory::Playback,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Playback controls"),
             TranslatableString("action", "Show/hide playback controls"),
             Checkable::Yes
             ),
    UiAction("toggle-noteinput",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Note input"),
             TranslatableString("action", "Show/hide note input toolbar"),
             Checkable::Yes
             ),

    // Vertical panels
    UiAction("toggle-palettes",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Palettes"),
             TranslatableString("action", "Show/hide palettes"),
             Checkable::Yes
             ),
    UiAction("toggle-instruments",
             ActionCategory::File,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Instr&uments"),
             TranslatableString("action", "Open instruments dialog…"),
             Checkable::Yes
             ),
    UiAction("inspector",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Propert&ies"),
             TranslatableString("action", "Show/hide properties"),
             Checkable::Yes
             ),
    UiAction("toggle-selection-filter",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Se&lection filter"),
             TranslatableString("action", "Show/hide selection filter"),
             Checkable::Yes
             ),

    // Navigator
    UiAction("toggle-navigator",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Navigator"),
             TranslatableString("action", "Show/hide navigator"),
             Checkable::Yes
             ),

    // Horizontal panels
    UiAction("toggle-timeline",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Tim&eline"),
             TranslatableString("action", "Show/hide timeline"),
             Checkable::Yes
             ),
    UiAction("toggle-mixer",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Mixer"),
             TranslatableString("action", "Show/hide mixer"),
             IconCode::Code::MIXER,
             Checkable::Yes
             ),
    UiAction("toggle-piano-keyboard",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Piano &keyboard"),
             TranslatableString("action", "Show/hide piano keyboard"),
             Checkable::Yes
             ),
    UiAction("toggle-scorecmp-tool",
             ActionCategory::Undefined,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Score comparison tool"),
             Checkable::Yes
             ),

    // Status bar
    UiAction("toggle-statusbar",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Status bar"),
             TranslatableString("action", "Show/hide status bar"),
             Checkable::Yes
             ),

    UiAction("preference-dialog",
             ActionCategory::Application,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Preferences"),
             TranslatableString("action", "Preferences…")
             )
};

ApplicationUiActions::ApplicationUiActions(std::shared_ptr<ApplicationActionController> controller)
    : m_controller(controller)
{
}

void ApplicationUiActions::init()
{
    configuration()->isNotationNavigatorVisibleChanged().onNotify(this, [this]() {
        m_actionCheckedChanged.send({ TOGGLE_NAVIGATOR_ACTION_CODE });
    });

    dockWindowProvider()->windowChanged().onNotify(this, [this]() {
        listenOpenedDocksChanged(dockWindowProvider()->window());
    });
}

void ApplicationUiActions::listenOpenedDocksChanged(IDockWindow* window)
{
    if (!window) {
        return;
    }

    window->docksOpenStatusChanged().onReceive(this, [this](const QStringList& dockNames) {
        ActionCodeList actions;

        for (const ActionCode& toggleDockAction : toggleDockActions().keys()) {
            const DockName& dockName = toggleDockActions()[toggleDockAction];

            if (dockNames.contains(dockName)) {
                actions.push_back(toggleDockAction);
            }
        }

        if (!actions.empty()) {
            m_actionCheckedChanged.send(actions);
        }
    });
}

const mu::ui::UiActionList& ApplicationUiActions::actionsList() const
{
    return m_actions;
}

bool ApplicationUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool ApplicationUiActions::actionChecked(const UiAction& act) const
{
    QMap<ActionCode, DockName> toggleDockActions = ApplicationUiActions::toggleDockActions();
    DockName dockName = toggleDockActions.value(act.code, DockName());

    if (dockName.isEmpty()) {
        return false;
    }

    if (dockName == NOTATION_NAVIGATOR_PANEL_NAME) {
        return configuration()->isNotationNavigatorVisible();
    }

    const IDockWindow* window = dockWindowProvider()->window();
    return window ? window->isDockOpen(dockName) : false;
}

mu::async::Channel<mu::actions::ActionCodeList> ApplicationUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> ApplicationUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

const QMap<mu::actions::ActionCode, DockName>& ApplicationUiActions::toggleDockActions()
{
    static const QMap<mu::actions::ActionCode, DockName> actionsMap {
        { "toggle-transport", PLAYBACK_TOOLBAR_NAME },
        { "toggle-noteinput", NOTE_INPUT_BAR_NAME },

        { "toggle-palettes", PALETTES_PANEL_NAME },
        { "toggle-instruments", INSTRUMENTS_PANEL_NAME },
        { "inspector", INSPECTOR_PANEL_NAME },
        { "toggle-selection-filter", SELECTION_FILTERS_PANEL_NAME },

        { TOGGLE_NAVIGATOR_ACTION_CODE, NOTATION_NAVIGATOR_PANEL_NAME },

        { "toggle-timeline", TIMELINE_PANEL_NAME },
        { "toggle-mixer", MIXER_PANEL_NAME },
        { "toggle-piano-keyboard", PIANO_KEYBOARD_PANEL_NAME },

        { "toggle-statusbar", NOTATION_STATUSBAR_NAME },
    };

    return actionsMap;
}
