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

#include "projectcommandsregister.h"

using namespace mu::project;

#include "../projectcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace mu::project;

static const std::vector<CommandInfo> s_commandInfos = {
    CommandInfo{
        PROJECT_NEW_COMMAND,
        TranslatableString("project", "&New…"),
        TranslatableString("project", "Create a new project"),
        InputSchema(),
        Decoration(IconCode::Code::NEW_FILE)
    },
    CommandInfo{
        PROJECT_OPEN_COMMAND,
        TranslatableString("project", "&Open…"),
        TranslatableString("project", "Open a project"),
        InputSchema({
            { "url", Arg(DataType::String, u"URL of the project to open (optional)") },
            { "display_name", Arg(DataType::String, u"Display name of the project (optional)") },
        }),
        Decoration(IconCode::Code::OPEN_FILE)
    },
    CommandInfo{
        PROJECT_CLOSE_COMMAND,
        TranslatableString("project", "&Close"),
        TranslatableString("project", "Close the project"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        PROJECT_SAVE_COMMAND,
        TranslatableString("project", "&Save"),
        TranslatableString("project", "Save the project"),
        InputSchema(),
        Decoration(IconCode::Code::SAVE)
    },
    CommandInfo{
        PROJECT_SAVE_AS_COMMAND,
        TranslatableString("project", "Save &as…"),
        TranslatableString("project", "Save the project as"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PROJECT_SAVE_A_COPY_COMMAND,
        TranslatableString("project", "Save a &copy…"),
        TranslatableString("project", "Save a copy of the project"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PROJECT_SAVE_SELECTION_COMMAND,
        TranslatableString("project", "Save &selection…"),
        TranslatableString("project", "Save the selection of the project"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PROJECT_SAVE_TO_CLOUD_COMMAND,
        TranslatableString("project", "Save to clo&ud…"),
        TranslatableString("project", "Save the project to cloud"),
        InputSchema(),
        Decoration(IconCode::Code::CLOUD_FILE)
    },
    CommandInfo{
        PROJECT_SAVE_AT_COMMAND,
        TranslatableString("project", "Save at…"),
        TranslatableString("project", "Save the project at"),
        InputSchema({
            { "path", Arg(DataType::String, u"Path of the project to save") }
        }),
        Decoration()
    },

    CommandInfo{
        PROJECT_PUBLISH_COMMAND,
        TranslatableString("project", "Publish to &MuseScore.com…"),
        TranslatableString("project", "Publish the project to MuseScore.com"),
        InputSchema(),
        Decoration(IconCode::Code::CLOUD_FILE)
    },
    CommandInfo{
        PROJECT_SHARED_AUDIO_COMMAND,
        TranslatableString("project", "Share on &Audio.com…"),
        TranslatableString("project", "Share the project on Audio.com"),
        InputSchema(),
        Decoration(IconCode::Code::SHARE_AUDIO)
    },

    CommandInfo{
        PROJECT_EXPORT_COMMAND,
        TranslatableString("project", "&Export…"),
        TranslatableString("project", "Export the project"),
        InputSchema(),
        Decoration(IconCode::Code::SHARE_FILE)
    },
    CommandInfo{
        PROJECT_IMPORT_PDF_COMMAND,
        TranslatableString("project", "Import P&DF…"),
        TranslatableString("project", "Import the PDF file"),
        InputSchema(),
        Decoration(IconCode::Code::OPEN_LINK)
    },
    CommandInfo{
        PROJECT_IMPORT_AUDIO_TO_SCORE_COMMAND,
        TranslatableString("project", "Import A&udio to Score…"),
        TranslatableString("project", "Import the audio file to the score"),
        InputSchema(),
        Decoration(IconCode::Code::OPEN_LINK)
    },

    CommandInfo{
        PROJECT_PROPERTIES_COMMAND,
        TranslatableString("project", "Project propert&ies…"),
        TranslatableString("project", "Project properties"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PROJECT_PRINT_COMMAND,
        TranslatableString("project", "&Print…"),
        TranslatableString("project", "Print the project"),
        InputSchema(),
        Decoration(IconCode::Code::PRINT)
    },
    CommandInfo{
        PROJECT_CLEAR_RECENT_COMMAND,
        TranslatableString("project", "&Clear list of recent files"),
        TranslatableString("project", "Clear list of recent files"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PROJECT_CONTINUE_LAST_SESSION_COMMAND,
        TranslatableString("project", "Continue last session"),
        TranslatableString("project", "Continue the last session"),
        InputSchema(),
        Decoration()
    },
};

std::string ProjectCommandsRegister::moduleName() const
{
    return "project";
}

const std::vector<muse::rcommand::Command>& ProjectCommandsRegister::commandList() const
{
    static std::vector<muse::rcommand::Command> commands;
    if (commands.empty()) {
        commands.reserve(s_commandInfos.size());
        for (const auto& info : s_commandInfos) {
            commands.push_back(info.command);
        }
    }
    return commands;
}

const std::vector<muse::rcommand::CommandInfo>& ProjectCommandsRegister::commandInfoList() const
{
    return s_commandInfos;
}
