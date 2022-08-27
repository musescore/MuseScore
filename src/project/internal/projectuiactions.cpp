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

#include "types/translatablestring.h"

using namespace mu::project;
using namespace mu::ui;

const UiActionList ProjectUiActions::m_actions = {
    UiAction("file-open",
             ActionCategory::FILE,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Open…"),
             TranslatableString("action", "Open…"),
             IconCode::Code::OPEN_FILE
             ),
    UiAction("file-new",
             ActionCategory::FILE,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&New…"),
             TranslatableString("action", "New…"),
             IconCode::Code::NEW_FILE
             ),
    UiAction("file-close",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Close"),
             TranslatableString("action", "Close")
             ),
    UiAction("file-save",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Save"),
             TranslatableString("action", "Save"),
             IconCode::Code::SAVE
             ),
    UiAction("file-save-as",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Save &as…"),
             TranslatableString("action", "Save as…")
             ),
    UiAction("file-save-a-copy",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Save a cop&y…"),
             TranslatableString("action", "Save a copy…")
             ),
    UiAction("file-save-selection",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Save se&lection…"),
             TranslatableString("action", "Save selection…")
             ),
    UiAction("file-save-to-cloud",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Save to clo&ud…"),
             TranslatableString("action", "Save to cloud…"),
             IconCode::Code::CLOUD_FILE
             ),
    UiAction("file-publish",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Pu&blish to MuseScore.com…"),
             TranslatableString("action", "Publish to MuseScore.com…"),
             IconCode::Code::CLOUD_FILE
             ),
    UiAction("file-export",
             ActionCategory::File,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Export…"),
             IconCode::Code::SHARE_FILE
             ),
    UiAction("file-import-pdf",
             ActionCategory::UNDEFINED,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Import P&DF…"),
             TranslatableString("action", "Import PDF…"),
             IconCode::Code::IMPORT
             ),
    UiAction("project-properties",
             ActionCategory::Dialogspanels,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Project propert&ies…"),
             TranslatableString("action", "Project properties…")
             ),
    UiAction("print",
             ActionCategory::FILE,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Print…"),
             TranslatableString("action", "Print…"),
             IconCode::Code::PRINT
             ),
    UiAction("clear-recent",
             ActionCategory::FILE,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Clear recent files"),
             TranslatableString("action", "Clear recent files")
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
