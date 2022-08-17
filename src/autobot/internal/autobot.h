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
#ifndef MU_AUTOBOT_AUTOBOT_H
#define MU_AUTOBOT_AUTOBOT_H

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

#include "scriptengine.h"
#include "testcasecontext.h"
#include "testcaserunner.h"
#include "testcasereport.h"
#include "autobotinteractive.h"

namespace mu::autobot {
class Autobot : public IAutobot, public async::Asyncable
{
    INJECT(autobot, IAutobotConfiguration, configuration)
    INJECT(autobot, io::IFileSystem, fileSystem)
    INJECT(autobot, ui::INavigationController, navigation)
    INJECT(autobot, shortcuts::IShortcutsRegister, shortcutsRegister)
    INJECT(autobot, framework::IInteractive, interactive)
    INJECT(autobot, ui::IMainWindow, mainWindow)

public:
    Autobot() = default;

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

    void execScript(const io::path_t& path) override;
    void runTestCase(const TestCase& testCase) override;
    void sleep(int msec) override;
    void pause() override;
    void unpause() override;
    void abort() override;
    void fatal(const QString& msg) override;

    ITestCaseContextPtr context() const override;
    AutobotInteractivePtr autobotInteractive() const override;

private:

    void affectOnServices();
    void restoreAffectOnServices();

    void setStatus(Status st);

    Status m_status = Status::Undefined;
    async::Channel<io::path_t, Status> m_statusChanged;
    async::Channel<StepInfo, Ret> m_stepStatusChanged;
    ScriptEngine* m_engine = nullptr;
    ITestCaseContextPtr m_context = nullptr;
    TestCaseRunner m_runner;
    TestCaseReport m_report;
    AutobotInteractivePtr m_autobotInteractive = nullptr;

    QEventLoop m_sleepLoop;
};
}

#endif // MU_AUTOBOT_AUTOBOT_H
