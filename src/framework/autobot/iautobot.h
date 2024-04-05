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
#ifndef MUSE_AUTOBOT_IAUTOBOT_H
#define MUSE_AUTOBOT_IAUTOBOT_H

#include <vector>
#include <QJSValue>

#include "modularity/imoduleinterface.h"
#include "io/path.h"
#include "async/channel.h"
#include "autobottypes.h"
#include "itestcasecontext.h"
#include "internal/autobotinteractive.h"

namespace muse::autobot {
class IAutobot : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAutobot)
public:
    virtual ~IAutobot() = default;

    enum class Status {
        Undefined = 0,
        Running,
        Paused,
        Aborted,
        Error,
        Finished
    };

    static QString statusToString(Status st)
    {
        switch (st) {
        case IAutobot::Status::Undefined: return "";
        case IAutobot::Status::Running: return "Running";
        case IAutobot::Status::Paused: return "Paused";
        case IAutobot::Status::Aborted: return "Aborted";
        case IAutobot::Status::Error: return "Error";
        case IAutobot::Status::Finished: return "Finished";
        }
        return QString();
    }

    virtual Status status() const = 0;
    virtual async::Channel<io::path_t, Status> statusChanged() const = 0;
    virtual async::Channel<StepInfo, Ret> stepStatusChanged() const = 0;

    virtual SpeedMode speedMode() const = 0;
    virtual void setSpeedMode(SpeedMode mode) = 0;
    virtual async::Channel<SpeedMode> speedModeChanged() const = 0;
    virtual void setDefaultIntervalMsec(int msec) = 0;
    virtual int defaultIntervalMsec() const = 0;
    virtual int intervalMsec() const = 0;

    struct Options {
        io::path_t context;
        std::string contextVal;
        std::string func;
        std::string funcArgs;
    };

    virtual void execScript(const io::path_t& path, const Options& opt = Options()) = 0;

    virtual void runTestCase(const TestCase& testCase) = 0;
    virtual void sleep(int msec) = 0;
    virtual void pause() = 0;
    virtual void unpause() = 0;
    virtual void abort() = 0;
    virtual void fatal(const QString& msg) = 0;

    virtual ITestCaseContextPtr context() const = 0;
    virtual AutobotInteractivePtr autobotInteractive() const = 0;
};
}

#endif // MUSE_AUTOBOT_IAUTOBOT_H
