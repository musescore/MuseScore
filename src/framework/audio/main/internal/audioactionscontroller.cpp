/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "audioactionscontroller.h"

using namespace muse;
using namespace muse::audio;

void AudioActionsController::init()
{
    dispatcher()->reg(this, "action://audio/dev/use-workermode", [this]() { setMode(workmode::WorkerMode); });
    dispatcher()->reg(this, "action://audio/dev/use-drivermode", [this]() { setMode(workmode::DriverMode); });
    dispatcher()->reg(this, "action://audio/dev/use-workerrpcmode", [this]() { setMode(workmode::WorkerRpcMode); });
}

void AudioActionsController::setMode(workmode::Mode m)
{
    workmode::setMode(m);
    m_actionCheckedChanged.send({ "action://audio/dev/use-workermode",
                                  "action://audio/dev/use-drivermode",
                                  "action://audio/dev/use-workerrpcmode" });

    auto promise = interactive()->question("Changing the audio mode",
                                           "Restart required, do you want to perform it?",
                                           { IInteractive::Button::Yes, IInteractive::Button::No });

    promise.onResolve(nullptr, [this](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::Yes)) {
            application()->restart();
        }
    });
}

bool AudioActionsController::actionChecked(const actions::ActionCode& act) const
{
    if (act == "action://audio/dev/use-workermode") {
        return workmode::desiredMode() == workmode::WorkerMode;
    } else if (act == "action://audio/dev/use-drivermode") {
        return workmode::desiredMode() == workmode::DriverMode;
    } else if (act == "action://audio/dev/use-workerrpcmode") {
        return workmode::desiredMode() == workmode::WorkerRpcMode;
    }

    return false;
}

async::Channel<actions::ActionCodeList> AudioActionsController::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
