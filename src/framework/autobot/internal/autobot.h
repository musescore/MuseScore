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
#ifndef MUSE_AUTOBOT_AUTOBOT_H
#define MUSE_AUTOBOT_AUTOBOT_H

#include <QEventLoop>

#include "../iautobot.h"
#include "io/path.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "../iautobotconfiguration.h"
#include "io/ifilesystem.h"
#include "ui/inavigationcontroller.h"
#include "shortcuts/ishortcutsregister.h"
#include "iinteractive.h"
#include "ui/imainwindow.h"
#include "global/iapplication.h"

#include "scriptengine.h"
#include "testcaserunner.h"
#include "testcasereport.h"
#include "autobotinteractive.h"

namespace muse::autobot {
class Autobot : public IAutobot, public Injectable, public async::Asyncable
{
    Inject<IApplication> application = { this };
    Inject<IAutobotConfiguration> configuration= { this };
    Inject<io::IFileSystem> fileSystem= { this };
    Inject<muse::ui::INavigationController> navigation= { this };
    Inject<shortcuts::IShortcutsRegister> shortcutsRegister= { this };
    Inject<IInteractive> interactive= { this };
    Inject<muse::ui::IMainWindow> mainWindow= { this };

public:
    Autobot(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx), m_report(iocCtx) {}

    void init();

    Status status() const override;
    async::Channel<io::path_t, Status> statusChanged() const override;
    async::Channel<StepInfo, Ret> stepStatusChanged() const override;

    SpeedMode speedMode() const override;
    void setSpeedMode(SpeedMode mode) override;
    async::Channel<SpeedMode> speedModeChanged() const override;
    void setDefaultIntervalMsec(int msec) override;
    int defaultIntervalMsec() const override;
    int intervalMsec() const override;

    void execScript(const io::path_t& path, const Options& opt = Options()) override;

    void runTestCase(const TestCase& testCase) override;
    void sleep(int msec) override;
    void pause() override;
    void unpause() override;
    void abort() override;
    void fatal(const QString& msg) override;

    ITestCaseContextPtr context() const override;
    AutobotInteractivePtr autobotInteractive() const override;

private:

    void loadContext(ITestCaseContextPtr ctx, const io::path_t& context, const std::string& contextVal, ScriptEngine* e);
    QJSValueList parseFuncArgs(const std::string& funcArgs, ScriptEngine* e) const;

    void affectOnServices();
    void restoreAffectOnServices();

    void setStatus(Status st);

    struct AffectedServiceState {
        bool fontDisabledMerging = false;
    };

    Status m_status = Status::Undefined;
    async::Channel<io::path_t, Status> m_statusChanged;
    async::Channel<StepInfo, Ret> m_stepStatusChanged;
    ScriptEngine* m_engine = nullptr;
    ITestCaseContextPtr m_context = nullptr;
    TestCaseRunner m_runner;
    TestCaseReport m_report;
    AutobotInteractivePtr m_autobotInteractive = nullptr;
    AffectedServiceState m_affectedServiceState;

    QEventLoop m_sleepLoop;
};
}

#endif // MUSE_AUTOBOT_AUTOBOT_H
