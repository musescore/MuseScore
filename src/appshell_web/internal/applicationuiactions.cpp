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
#include "applicationuiactions.h"

#include "ui/view/iconcodes.h"
#include "context/uicontext.h"
#include "context/shortcutcontext.h"

#include "dockwindow/idockwindow.h"
#include "async/notification.h"

#include "log.h"

using namespace muse;
using namespace mu::appshell;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::dock;

static const ActionCode TOGGLE_NAVIGATOR_ACTION_CODE("toggle-navigator");
static const ActionCode TOGGLE_BRAILLE_ACTION_CODE("toggle-braille-panel");
static const ActionCode TOGGLE_PERCUSSION_PANEL_ACTION_CODE("toggle-percussion-panel");

const UiActionList ApplicationUiActions::m_actions = {
    // Toolbars
    UiAction("toggle-transport",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Playback controls"),
             TranslatableString("action", "Show/hide playback controls"),
             Checkable::Yes
             ),
    UiAction("toggle-noteinput",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Note input"),
             TranslatableString("action", "Show/hide note input toolbar"),
             Checkable::Yes
             ),

    // Vertical panels
    UiAction("toggle-palettes",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Palettes"),
             TranslatableString("action", "Show/hide palettes"),
             Checkable::Yes
             ),
    UiAction("toggle-instruments",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Layout"),
             TranslatableString("action", "Show/hide layout panel"),
             Checkable::Yes
             ),
    UiAction("inspector",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Propert&ies"),
             TranslatableString("action", "Show/hide properties"),
             Checkable::Yes
             ),
    UiAction("toggle-selection-filter",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Se&lection filter"),
             TranslatableString("action", "Show/hide selection filter"),
             Checkable::Yes
             ),
    UiAction("toggle-undo-history-panel",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&History"),
             TranslatableString("action", "Show/hide undo history"),
             Checkable::Yes
             ),

    // Navigator
    UiAction(TOGGLE_NAVIGATOR_ACTION_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Navigator"),
             TranslatableString("action", "Show/hide navigator"),
             Checkable::Yes
             ),

    // Braille panel
    UiAction(TOGGLE_BRAILLE_ACTION_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Braille"),
             TranslatableString("action", "Show/hide braille panel"),
             Checkable::Yes
             ),

    // Horizontal panels
    UiAction("toggle-timeline",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Tim&eline"),
             TranslatableString("action", "Show/hide timeline"),
             Checkable::Yes
             ),
    UiAction("toggle-mixer",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Mixer"),
             TranslatableString("action", "Show/hide mixer"),
             IconCode::Code::MIXER,
             Checkable::Yes
             ),
    UiAction("toggle-piano-keyboard",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Piano &keyboard"),
             TranslatableString("action", "Show/hide piano keyboard"),
             Checkable::Yes
             ),
    UiAction(TOGGLE_PERCUSSION_PANEL_ACTION_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Percussion"),
             TranslatableString("action", "Show/hide percussion panel"),
             Checkable::Yes
             ),
    UiAction("toggle-scorecmp-tool",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Score comparison tool"),
             Checkable::Yes
             ),

    // Status bar
    UiAction("toggle-statusbar",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Status bar"),
             TranslatableString("action", "Show/hide status bar"),
             Checkable::Yes
             )
};

ApplicationUiActions::ApplicationUiActions(std::shared_ptr<ApplicationActionController> controller, const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx), m_controller(controller)
{
}

void ApplicationUiActions::init()
{
    configuration()->isNotationNavigatorVisibleChanged().onNotify(this, [this]() {
        m_actionCheckedChanged.send({ TOGGLE_NAVIGATOR_ACTION_CODE });
    });

    brailleConfiguration()->braillePanelEnabledChanged().onNotify(this, [this]() {
        m_actionCheckedChanged.send({ TOGGLE_BRAILLE_ACTION_CODE });
    });

    notationConfiguration()->useNewPercussionPanelChanged().onNotify(this, [this]() {
        m_actionEnabledChanged.send({ TOGGLE_PERCUSSION_PANEL_ACTION_CODE });
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

const muse::ui::UiActionList& ApplicationUiActions::actionsList() const
{
    return m_actions;
}

bool ApplicationUiActions::actionEnabled(const UiAction& act) const
{
    if (act.code == TOGGLE_PERCUSSION_PANEL_ACTION_CODE) {
        return notationConfiguration()->useNewPercussionPanel();
    }

    return m_controller->canReceiveAction(act.code);
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

    if (dockName == NOTATION_BRAILLE_PANEL_NAME) {
        return brailleConfiguration()->braillePanelEnabled();
    }

    const IDockWindow* window = dockWindowProvider()->window();
    return window ? window->isDockOpenAndCurrentInFrame(dockName) : false;
}

muse::async::Channel<ActionCodeList> ApplicationUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

muse::async::Channel<ActionCodeList> ApplicationUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

const QMap<ActionCode, DockName>& ApplicationUiActions::toggleDockActions()
{
    static const QMap<ActionCode, DockName> actionsMap {
        { "toggle-transport", PLAYBACK_TOOLBAR_NAME },
        { "toggle-noteinput", NOTE_INPUT_BAR_NAME },

        { "toggle-palettes", PALETTES_PANEL_NAME },
        { "toggle-instruments", LAYOUT_PANEL_NAME },
        { "inspector", INSPECTOR_PANEL_NAME },
        { "toggle-selection-filter", SELECTION_FILTERS_PANEL_NAME },
        { "toggle-undo-history-panel", UNDO_HISTORY_PANEL_NAME },

        { TOGGLE_NAVIGATOR_ACTION_CODE, NOTATION_NAVIGATOR_PANEL_NAME },
        { TOGGLE_BRAILLE_ACTION_CODE, NOTATION_BRAILLE_PANEL_NAME },

        { "toggle-timeline", TIMELINE_PANEL_NAME },
        { "toggle-mixer", MIXER_PANEL_NAME },
        { "toggle-piano-keyboard", PIANO_KEYBOARD_PANEL_NAME },
        { TOGGLE_PERCUSSION_PANEL_ACTION_CODE, PERCUSSION_PANEL_NAME },

        { "toggle-statusbar", NOTATION_STATUSBAR_NAME },
    };

    return actionsMap;
}
