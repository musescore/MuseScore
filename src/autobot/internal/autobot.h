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
#include "system/ifilesystem.h"
#include "ui/inavigationcontroller.h"
#include "shortcuts/ishortcutsregister.h"

#include "scriptengine.h"
#include "testcasecontext.h"
#include "testcaserunner.h"
#include "testcasereport.h"

namespace mu::autobot {
class Autobot : public IAutobot, public async::Asyncable
{
    INJECT(autobot, IAutobotConfiguration, configuration)
    INJECT(autobot, system::IFileSystem, fileSystem)
    INJECT(autobot, ui::INavigationController, navigation)
    INJECT(autobot, shortcuts::IShortcutsRegister, shortcutsRegister)

public:
    Autobot() = default;

    void init();

    Status status() const override;
    async::Channel<Status> statusChanged() const override;

    Ret execScript(const io::path& path) override;

    void setStepsInterval(int msec) override;
    void runTestCase(const TestCase& testCase) override;
    void sleep(int msec) override;
    void pause() override;
    void unpause() override;
    void abort() override;

    async::Channel<QString, StepStatus> stepStatusChanged() const override;

    ITestCaseContextPtr context() const override;

private:

    void affectOnServices();
    void restoreAffectOnServices();

    void setStatus(Status st);

    Status m_status = Status::Undefined;
    async::Channel<Status> m_statusChanged;
    async::Channel<QString, StepStatus> m_stepStatusChanged;
    ScriptEngine* m_engine = nullptr;
    ITestCaseContextPtr m_context = nullptr;
    TestCaseRunner m_runner;
    TestCaseReport m_report;

    QEventLoop m_sleepLoop;
};
}

#endif // MU_AUTOBOT_AUTOBOT_H
