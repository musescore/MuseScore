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
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Quit"),
             QT_TRANSLATE_NOOP("action", "Quit")
             ),
    UiAction("restart",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Restart")
             ),
    UiAction("fullscreen",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Full screen"),
             QT_TRANSLATE_NOOP("action", "Full screen"),
             Checkable::Yes
             ),
    UiAction("about",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&About…")
             ),
    UiAction("about-qt",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "About &Qt…")
             ),
    UiAction("about-musicxml",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "About &MusicXML…")
             ),
    UiAction("online-handbook",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Online &handbook"),
             QT_TRANSLATE_NOOP("action", "Go to online handbook")
             ),
    UiAction("ask-help",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "As&k for help")
             ),
    UiAction("report-bug",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Report a bug")
             ),
    UiAction("leave-feedback",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "F&eedback")
             ),
    UiAction("revert-factory",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Revert to &factory settings"),
             QT_TRANSLATE_NOOP("action", "Revert to factory settings")
             ),

    // Docking
    UiAction("dock-restore-default-layout",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Restore the &default layout"),
             QT_TRANSLATE_NOOP("action", "Restore the default layout")
             ),

    // Toolbars
    UiAction("toggle-transport",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             QT_TRANSLATE_NOOP("action", "&Playback controls"),
             QT_TRANSLATE_NOOP("action", "Show/hide playback controls"),
             Checkable::Yes
             ),
    UiAction("toggle-noteinput",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Note input"),
             QT_TRANSLATE_NOOP("action", "Show/hide note input toolbar"),
             Checkable::Yes
             ),

    // Vertical panels
    UiAction("toggle-palettes",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Palettes"),
             QT_TRANSLATE_NOOP("action", "Show/hide palettes"),
             Checkable::Yes
             ),
    UiAction("toggle-instruments",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Instr&uments"),
             QT_TRANSLATE_NOOP("action", "Open instruments dialog…"),
             Checkable::Yes
             ),
    UiAction("inspector",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Propert&ies"),
             QT_TRANSLATE_NOOP("action", "Show/hide properties"),
             Checkable::Yes
             ),
    UiAction("toggle-selection-filter",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             QT_TRANSLATE_NOOP("action", "Se&lection filter"),
             QT_TRANSLATE_NOOP("action", "Show/hide selection filter"),
             Checkable::Yes
             ),

    // Navigator
    UiAction("toggle-navigator",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Navigator"),
             QT_TRANSLATE_NOOP("action", "Show/hide navigator"),
             Checkable::Yes
             ),

    // Horizontal panels
    UiAction("toggle-timeline",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Tim&eline"),
             QT_TRANSLATE_NOOP("action", "Show/hide timeline"),
             Checkable::Yes
             ),
    UiAction("toggle-mixer",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Mixer"),
             QT_TRANSLATE_NOOP("action", "Show/hide mixer"),
             IconCode::Code::MIXER,
             Checkable::Yes
             ),
    UiAction("toggle-piano-keyboard",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Piano &keyboard"),
             QT_TRANSLATE_NOOP("action", "Show/hide piano keyboard"),
             Checkable::Yes
             ),
    UiAction("toggle-scorecmp-tool",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             QT_TRANSLATE_NOOP("action", "Score comparison tool"),
             Checkable::Yes
             ),

    // Status bar
    UiAction("toggle-statusbar",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             QT_TRANSLATE_NOOP("action", "&Status bar"),
             QT_TRANSLATE_NOOP("action", "Show/hide status bar"),
             Checkable::Yes
             ),

    UiAction("preference-dialog",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Preferences"),
             QT_TRANSLATE_NOOP("action", "Preferences…")
             ),
    UiAction("check-update",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Check for &update")
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
