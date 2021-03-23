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

#include "abrunner.h"

namespace mu::autobot {
class Autobot : public IAutobot, public async::Asyncable
{
public:
    Autobot();

    void init();

    std::vector<ITestCasePtr> testCases() const override;
    ITestCasePtr testCase(const std::string& name) const override;

    void runAll(const std::string& testCaseName) override;
    void runFile(const std::string& testCaseName, int fileIndex) override;
    void stop() override;
    const ValCh<Status>& status() const override;

    const ValNt<Files>& files() const override;
    const ValCh<int>& currentFileIndex() const override;

private:

    void nextScore();

    std::vector<ITestCasePtr> m_testCases;

    ITestCasePtr m_currentTC;

    ValNt<Files> m_files;
    ValCh<int> m_fileIndex;

    ValCh<Status> m_status;
    AbRunner m_runner;
};
}

#endif // MU_AUTOBOT_AUTOBOT_H
