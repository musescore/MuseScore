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

#include "projectcommandsstate.h"

#include "global/containers.h"

#include "../projectcommands.h"
#include "rcommand/commandtypes.h"

using namespace muse;
using namespace muse::rcommand;
using namespace mu::project;

enum class PCondition {
    Any,
    HasProject,
    NeedSave,
    NotBusy,
    HasSelection,
};

static std::map<Command, PCondition> PROJECT_COMMAND_CONDITIONS = {
    { PROJECT_NEW_COMMAND, PCondition::Any },
    { PROJECT_OPEN_COMMAND, PCondition::Any },
    { PROJECT_CLOSE_COMMAND, PCondition::NotBusy },
    { PROJECT_SAVE_COMMAND, PCondition::NeedSave },
    { PROJECT_SAVE_AS_COMMAND, PCondition::NotBusy },
    { PROJECT_SAVE_A_COPY_COMMAND, PCondition::NotBusy },
    { PROJECT_SAVE_SELECTION_COMMAND, PCondition::HasSelection },
    { PROJECT_SAVE_TO_CLOUD_COMMAND, PCondition::NotBusy },
    { PROJECT_SAVE_AT_COMMAND, PCondition::NotBusy },
    { PROJECT_PUBLISH_COMMAND, PCondition::NotBusy },
    { PROJECT_SHARED_AUDIO_COMMAND, PCondition::NotBusy },
    { PROJECT_EXPORT_COMMAND, PCondition::HasProject },
    { PROJECT_IMPORT_PDF_COMMAND, PCondition::Any },
    { PROJECT_IMPORT_AUDIO_TO_SCORE_COMMAND, PCondition::Any },
    { PROJECT_PRINT_COMMAND, PCondition::HasProject },
    { PROJECT_CLEAR_RECENT_COMMAND, PCondition::Any },
    { PROJECT_CONTINUE_LAST_SESSION_COMMAND, PCondition::Any },
    { PROJECT_PROPERTIES_COMMAND, PCondition::HasProject },
};

using BusyStatus = IProjectCommandsController::BusyStatus;
static std::map<Command, BusyStatus> PROJECT_COMMAND_BUSY_STATUSES = {
    { PROJECT_CLOSE_COMMAND, BusyStatus::Closing },
    { PROJECT_SAVE_COMMAND, BusyStatus::Saving },
    { PROJECT_SAVE_AS_COMMAND, BusyStatus::Saving },
    { PROJECT_SAVE_A_COPY_COMMAND, BusyStatus::Saving },
    { PROJECT_SAVE_SELECTION_COMMAND, BusyStatus::Saving },
    { PROJECT_SAVE_TO_CLOUD_COMMAND, BusyStatus::Uploading },
    { PROJECT_SAVE_AT_COMMAND, BusyStatus::Uploading },
    { PROJECT_PUBLISH_COMMAND, BusyStatus::Publishing },
    { PROJECT_SHARED_AUDIO_COMMAND, BusyStatus::AudioSharing },
};

static inline std::vector<Command> commands(const PCondition& condition)
{
    std::vector<Command> result;
    for (const auto& p : PROJECT_COMMAND_CONDITIONS) {
        if (p.second == condition) {
            result.push_back(p.first);
        }
    }
    return result;
}

std::string ProjectCommandsState::moduleName() const
{
    return "project";
}

void ProjectCommandsState::init()
{
    m_moduleRegister = commandsRegister()->moduleRegister(moduleName());
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    controller()->hasProjectChanged().onNotify(this, [this]() {
        updateCommandStates();
    });

    controller()->needSaveChanged().onNotify(this, [this]() {
        updateCommandStates(commands(PCondition::NeedSave));
    });

    controller()->busyChanged().onNotify(this, [this]() {
        updateCommandStates(commands(PCondition::NotBusy));
    });

    controller()->hasSelectionChanged().onNotify(this, [this]() {
        updateCommandStates(commands(PCondition::HasSelection));
    });

    updateCommandStates();
}

void ProjectCommandsState::deinit()
{
    controller()->hasProjectChanged().disconnect(this);
    controller()->needSaveChanged().disconnect(this);
    controller()->busyChanged().disconnect(this);
    controller()->hasSelectionChanged().disconnect(this);
}

void ProjectCommandsState::updateCommandStates(const std::vector<Command>& commands)
{
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    const auto& commandList = commands.empty() ? m_moduleRegister->commandList() : commands;

    for (const auto& command : commandList) {
        CommandState newState = commandState(command);
        if (m_commandStates[command] != newState) {
            m_commandStates[command] = newState;
            m_commandStateChanged.send(command, newState);
        }
    }
}

CommandState ProjectCommandsState::commandState(const Command& command) const
{
    PCondition condition = muse::value(PROJECT_COMMAND_CONDITIONS, command, PCondition::Any);
    switch (condition) {
    case PCondition::Any: return CommandState(true, false);
    case PCondition::HasProject: return CommandState(controller()->hasProject(), false);
    case PCondition::NeedSave: return CommandState(controller()->hasProject() && controller()->needSave(), false);
    case PCondition::NotBusy: {
        return CommandState(controller()->hasProject() && !controller()->isBusy(PROJECT_COMMAND_BUSY_STATUSES.at(command)), false);
    }
    case PCondition::HasSelection: return CommandState(controller()->hasProject() && controller()->hasSelection(), false);
    }
    return CommandState(true, false);
}

async::Channel<Command, CommandState> ProjectCommandsState::commandStateChanged() const
{
    return m_commandStateChanged;
}
