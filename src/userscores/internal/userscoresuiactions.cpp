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
#include "userscoresuiactions.h"

using namespace mu::userscores;
using namespace mu::ui;

const UiActionList UserScoresUiActions::m_actions = {
    UiAction("file-open",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Open..."),
             QT_TRANSLATE_NOOP("action", "Load score from file")
             ),
    UiAction("file-new",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "New..."),
             QT_TRANSLATE_NOOP("action", "Create new score")
             ),
    UiAction("file-close",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Close"),
             QT_TRANSLATE_NOOP("action", "Close current score")
             ),
    UiAction("file-save",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Save"),
             QT_TRANSLATE_NOOP("action", "Save score to file")
             ),
    UiAction("file-save-online",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Upload to MuseScore.com"),
             QT_TRANSLATE_NOOP("action", "Save score on MuseScore.com"),
             IconCode::Code::CLOUD_FILE
             ),
    UiAction("file-save-as",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Save As..."),
             QT_TRANSLATE_NOOP("action", "Save score under a new file name")
             ),
    UiAction("file-save-a-copy",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Save a Copy..."),
             QT_TRANSLATE_NOOP("action", "Save a copy of the score in addition to the current file")
             ),
    UiAction("file-save-selection",
             mu::context::UiCtxNotationFocused,
             QT_TRANSLATE_NOOP("action", "Save Selection..."),
             QT_TRANSLATE_NOOP("action", "Save current selection as new score")
             ),
    UiAction("file-export",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Export"),
             QT_TRANSLATE_NOOP("action", "Save a copy of the score in various formats"),
             IconCode::Code::SHARE_FILE
             ),
    UiAction("file-import",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Import..."),
             QT_TRANSLATE_NOOP("action", "Import score from file")
             ),
    UiAction("file-import-pdf",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Import PDF..."),
             QT_TRANSLATE_NOOP("action", "Import a PDF file with an experimental service on musescore.com")
             ),
    UiAction("print",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Print..."),
             QT_TRANSLATE_NOOP("action", "Print score/part")
             ),
    UiAction("clear-recent",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Clear Recent Files")
             )
};

UserScoresUiActions::UserScoresUiActions(std::shared_ptr<FileScoreController> controller)
    : m_controller(controller)
{
}

const UiActionList& UserScoresUiActions::actionsList() const
{
    return m_actions;
}

bool UserScoresUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool UserScoresUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> UserScoresUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> UserScoresUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
