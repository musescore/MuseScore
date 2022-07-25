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
#include "projectuiactions.h"

using namespace mu::project;
using namespace mu::ui;

const UiActionList ProjectUiActions::m_actions = {
    UiAction("file-open",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Open…"),
             QT_TRANSLATE_NOOP("action", "Open…"),
             IconCode::Code::OPEN_FILE
             ),
    UiAction("file-new",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&New…"),
             QT_TRANSLATE_NOOP("action", "New…"),
             IconCode::Code::NEW_FILE
             ),
    UiAction("file-close",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Close"),
             QT_TRANSLATE_NOOP("action", "Close")
             ),
    UiAction("file-save",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Save"),
             QT_TRANSLATE_NOOP("action", "Save"),
             IconCode::Code::SAVE
             ),
    UiAction("file-save-as",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Save &as…"),
             QT_TRANSLATE_NOOP("action", "Save as…")
             ),
    UiAction("file-save-a-copy",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Save a cop&y…"),
             QT_TRANSLATE_NOOP("action", "Save a copy…")
             ),
    UiAction("file-save-selection",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Save se&lection…"),
             QT_TRANSLATE_NOOP("action", "Save selection…")
             ),
    UiAction("file-save-to-cloud",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Save to clo&ud…"),
             QT_TRANSLATE_NOOP("action", "Save to cloud…"),
             IconCode::Code::CLOUD_FILE
             ),
    UiAction("file-publish",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             QT_TRANSLATE_NOOP("action", "Pu&blish to MuseScore.com…"),
             QT_TRANSLATE_NOOP("action", "Publish to MuseScore.com…"),
             IconCode::Code::CLOUD_FILE
             ),
    UiAction("file-export",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             QT_TRANSLATE_NOOP("action", "Export…"),
             QT_TRANSLATE_NOOP("action", "Export…"),
             IconCode::Code::SHARE_FILE
             ),
    UiAction("file-import-pdf",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Import P&DF…"),
             IconCode::Code::IMPORT
             ),
    UiAction("project-properties",
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             QT_TRANSLATE_NOOP("action", "Project propert&ies…"),
             QT_TRANSLATE_NOOP("action", "Project properties…")
             ),
    UiAction("print",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "Print…"),
             QT_TRANSLATE_NOOP("action", "Print…"),
             IconCode::Code::PRINT
             ),
    UiAction("clear-recent",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             QT_TRANSLATE_NOOP("action", "&Clear recent files")
             )
};

ProjectUiActions::ProjectUiActions(std::shared_ptr<ProjectActionsController> controller)
    : m_controller(controller)
{
}

const UiActionList& ProjectUiActions::actionsList() const
{
    return m_actions;
}

bool ProjectUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool ProjectUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> ProjectUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> ProjectUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
