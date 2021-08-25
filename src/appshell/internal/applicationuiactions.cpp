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

#include "log.h"

using namespace mu::appshell;
using namespace mu::ui;

const UiActionList ApplicationUiActions::m_actions = {
    UiAction("quit",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Quit")
             ),
    UiAction("fullscreen",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Full screen"),
             QT_TRANSLATE_NOOP("action", "Full screen"),
             Checkable::Yes
             ),
    UiAction("about",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "About…")
             ),
    UiAction("about-qt",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "About Qt…")
             ),
    UiAction("about-musicxml",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "About MusicXML…")
             ),
    UiAction("online-handbook",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Online handbook")
             ),
    UiAction("ask-help",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Ask for help")
             ),
    UiAction("report-bug",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Report a bug"),
             QT_TRANSLATE_NOOP("action", "Report a bug")
             ),
    UiAction("leave-feedback",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Feedback"),
             QT_TRANSLATE_NOOP("action", "Leave feedback")
             ),
    UiAction("revert-factory",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Revert to factory settings"),
             QT_TRANSLATE_NOOP("action", "Revert to factory settings")
             ),
    UiAction("toggle-mixer",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Mixer"),
             QT_TRANSLATE_NOOP("action", "Toggle mixer"),
             IconCode::Code::MIXER,
             Checkable::Yes
             ),
    UiAction("toggle-navigator",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Navigator"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Navigator'"),
             Checkable::Yes
             ),
    UiAction("toggle-palettes",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Palettes"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Palettes'"),
             Checkable::Yes
             ),
    UiAction("toggle-instruments",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Instruments"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Instruments'"),
             Checkable::Yes
             ),
    UiAction("inspector",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Properties"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Properties'"),
             Checkable::Yes
             ),
    UiAction("toggle-selection-filter",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Selection filter"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Selection filter'"),
             Checkable::Yes
             ),
    UiAction("toggle-statusbar",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Status bar"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Status bar'"),
             Checkable::Yes
             ),
    UiAction("toggle-noteinput",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Note Input"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Note Input' toolbar"),
             Checkable::Yes
             ),
    UiAction("toggle-notationtoolbar",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Notation toolbar"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Notation' toolbar"),
             Checkable::Yes
             ),
    UiAction("toggle-undoredo",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Undo/redo toolbar"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Undo/Redo' toolbar"),
             Checkable::Yes
             ),
    UiAction("toggle-transport",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Playback Controls"),
             QT_TRANSLATE_NOOP("action", "Toggle Playback Controls toolbar"),
             Checkable::Yes
             ),
    UiAction("toggle-timeline",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Timeline"),
             QT_TRANSLATE_NOOP("action", "Toggle timeline"),
             Checkable::Yes
             ),
    UiAction("synth-control",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Synthesizer"),
             QT_TRANSLATE_NOOP("action", "Toggle synthesizer"),
             Checkable::Yes
             ),
    UiAction("toggle-piano",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Piano"),
             QT_TRANSLATE_NOOP("action", "Toggle piano"),
             Checkable::Yes
             ),
    UiAction("toggle-scorecmp-tool",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Score comparison tool"),
             QT_TRANSLATE_NOOP("action", "Toggle score comparison tool"),
             Checkable::Yes
             ),
    UiAction("preference-dialog",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Preferences"),
             QT_TRANSLATE_NOOP("action", "Open preferences dialog")
             ),
    UiAction("check-update",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Check for updates"),
             QT_TRANSLATE_NOOP("action", "Check for updates")
             )
};

const std::vector<std::pair<mu::actions::ActionCode, PanelType> > ApplicationUiActions::m_panels = {
    { "toggle-palettes", PanelType::Palettes },
    { "toggle-instruments", PanelType::Instruments },
    { "inspector", PanelType::Inspector },
    { "toggle-selection-filter", PanelType::SelectionFilter },
    { "toggle-notationtoolbar", PanelType::NotationToolBar },
    { "toggle-noteinput", PanelType::NoteInputBar },
    { "toggle-undoredo", PanelType::UndoRedoToolBar },
    { "toggle-navigator", PanelType::NotationNavigator },
    { "toggle-statusbar", PanelType::NotationStatusBar },
    { "toggle-transport", PanelType::PlaybackToolBar },
    { "toggle-mixer", PanelType::Mixer },
    { "toggle-timeline", PanelType::Timeline },
    { "synth-control", PanelType::Synthesizer },
    { "toggle-piano", PanelType::Piano },
    { "toggle-scorecmp-tool", PanelType::ComparisonTool }
};

ApplicationUiActions::ApplicationUiActions(std::shared_ptr<ApplicationActionController> controller)
    : m_controller(controller)
{
}

void ApplicationUiActions::init()
{
    notationPageState()->panelsVisibleChanged().onReceive(this, [this](const PanelTypeList& types) {
        actions::ActionCodeList alist;
        for (PanelType t : types) {
            actions::ActionCode code = panelTypeToAction(t);
            if (!code.empty()) {
                alist.push_back(std::move(code));
            }
        }
        if (!alist.empty()) {
            m_actionCheckedChanged.send(alist);
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

PanelType ApplicationUiActions::panelType(const actions::ActionCode& code) const
{
    for (const auto& p : m_panels) {
        if (p.first == code) {
            return p.second;
        }
    }
    return PanelType::Undefined;
}

mu::actions::ActionCode ApplicationUiActions::panelTypeToAction(const PanelType& type) const
{
    for (const auto& p : m_panels) {
        if (p.second == type) {
            return p.first;
        }
    }
    return actions::ActionCode();
}

bool ApplicationUiActions::actionChecked(const UiAction& act) const
{
    PanelType panel = panelType(act.code);
    if (panel != PanelType::Undefined) {
        return notationPageState()->isPanelVisible(panel);
    }

    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> ApplicationUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> ApplicationUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
