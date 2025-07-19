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
#ifndef MUSE_AUTOBOT_TESTCASEREPORT_H
#define MUSE_AUTOBOT_TESTCASEREPORT_H

#include <QFile>
#include <QTextStream>

#include "modularity/ioc.h"
#include "../iautobotconfiguration.h"
#include "io/ifilesystem.h"

#include "../autobottypes.h"
#include "../itestcasecontext.h"

namespace muse::autobot {
class TestCaseReport : public Injectable
{
    Inject<IAutobotConfiguration> configuration = { this };
    Inject<io::IFileSystem> fileSystem = { this };

public:
    TestCaseReport(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    Ret beginReport(const TestCase& testCase);
    void endReport(bool aborted);

    void onStepStatusChanged(const StepInfo& stepInfo, const ITestCaseContextPtr& ctx);

private:

    QFile m_file;
    QTextStream m_stream;
    bool m_opened = false;
};
}

#endif // MUSE_AUTOBOT_TESTCASEREPORT_H
