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
#pragma once

#include "actions/actionable.h"
#include "global/async/channel.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "global/iapplication.h"
#include "global/iinteractive.h"

#include "audio/common/workmode.h"

namespace muse::audio {
class AudioActionsController : public actions::Actionable
{
    Inject<actions::IActionsDispatcher> dispatcher;
    Inject<IApplication> application;
    Inject<IInteractive> interactive;

public:
    AudioActionsController() = default;

    void init();

    bool actionChecked(const actions::ActionCode& act) const;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const;

private:

    void setMode(workmode::Mode m);

    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}
