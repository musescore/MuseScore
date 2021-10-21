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

#include "../iautobot.h"
#include "io/path.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "../iautobotconfiguration.h"
#include "system/ifilesystem.h"
#include "iinteractive.h"

#include "scriptengine.h"
#include "testcasecontext.h"
#include "testcaserunner.h"
#include "testcasereport.h"

namespace mu::autobot {
class Autobot : public IAutobot, public async::Asyncable
{
    INJECT(autobot, IAutobotConfiguration, configuration)
    INJECT(autobot, system::IFileSystem, fileSystem)
    INJECT(api, framework::IInteractive, interactive)

public:
    Autobot() = default;

    void init();

    Ret loadScript(const Script& script) override;

    void setStepsInterval(int msec) override;
    void runTestCase(const TestCase& testCase) override;
    bool pauseTestCase() override;
    void abortTestCase() override;

    ITestCaseContextPtr context() const override;

private:
    ITestCaseContextPtr m_context = nullptr;
    TestCaseRunner m_runner;
    TestCaseReport m_report;
};
}

#endif // MU_AUTOBOT_AUTOBOT_H
