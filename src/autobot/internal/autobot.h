//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUTOBOT_AUTOBOT_H
#define MU_AUTOBOT_AUTOBOT_H

#include "../iautobot.h"
#include "io/path.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "../iautobotconfiguration.h"
#include "system/ifilesystem.h"

#include "abrunner.h"
#include "abreport.h"

namespace mu::autobot {
class Autobot : public IAutobot, public async::Asyncable
{
    INJECT(autobot, IAutobotConfiguration, configuration)
    INJECT(autobot, system::IFileSystem, fileSystem)

public:
    Autobot();

    void init();

    std::vector<ITestCasePtr> testCases() const override;
    ITestCasePtr testCase(const std::string& name) const override;

    void setCurrentTestCase(const std::string& name) override;
    const ValCh<ITestCasePtr>& currentTestCase() const override;

    void runAllFiles() override;
    void runFile(int fileIndex) override;
    void stop() override;
    const ValCh<Status>& status() const override;

    const ValNt<Files>& files() const override;
    async::Channel<File> fileFinished() const override;
    const ValCh<int>& currentFileIndex() const override;

private:

    mu::RetVal<mu::io::paths> filesList() const;
    void nextFile();
    void onFileFinished(const IAbContextPtr& ctx);
    void doStop();

    std::vector<ITestCasePtr> m_testCases;
    ValCh<ITestCasePtr> m_currentTestCase;

    ValNt<Files> m_files;
    ValCh<int> m_fileIndex;

    async::Channel<File> m_fileFinished;

    ValCh<Status> m_status;
    AbRunner m_runner;
    AbReport m_report;
};
}

#endif // MU_AUTOBOT_AUTOBOT_H
