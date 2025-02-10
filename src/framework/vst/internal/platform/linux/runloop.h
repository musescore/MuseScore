/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include <vector>

#include "pluginterfaces/gui/iplugview.h"

class QSocketNotifier;
class QTimer;
namespace muse::vst {
class RunLoop : public Steinberg::Linux::IRunLoop
{
    DECLARE_FUNKNOWN_METHODS

public:
    RunLoop();
    virtual ~RunLoop();

    Steinberg::tresult PLUGIN_API registerEventHandler(Steinberg::Linux::IEventHandler* handler,
                                                       Steinberg::Linux::FileDescriptor fd) override;
    Steinberg::tresult PLUGIN_API unregisterEventHandler(Steinberg::Linux::IEventHandler* handler) override;

    Steinberg::tresult PLUGIN_API registerTimer(Steinberg::Linux::ITimerHandler* handler,
                                                Steinberg::Linux::TimerInterval milliseconds) override;
    Steinberg::tresult PLUGIN_API unregisterTimer(Steinberg::Linux::ITimerHandler* handler) override;

    void stop();

private:

    struct Handler {
        Steinberg::Linux::FileDescriptor fd;
        Steinberg::Linux::IEventHandler* handler = nullptr;
        QSocketNotifier* readSN = nullptr;
        QSocketNotifier* writeSN = nullptr;

        ~Handler();
    };

    struct Timer {
        Steinberg::Linux::TimerInterval interval;
        Steinberg::Linux::ITimerHandler* handler = nullptr;
        QTimer* timer = nullptr;

        ~Timer();
    };

    std::vector<Handler*> m_handlers;
    std::vector<Timer*> m_timers;
};
}
