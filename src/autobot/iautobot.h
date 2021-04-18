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
#ifndef MU_AUTOBOT_IAUTOBOT_H
#define MU_AUTOBOT_IAUTOBOT_H

#include <vector>

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"
#include "abtypes.h"
#include "itestcase.h"

namespace mu::autobot {
class IAutobot : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAutobot)
public:
    virtual ~IAutobot() = default;

    enum class Status {
        Stoped = 0,
        RunningAll,
        RunningFile
    };

    virtual std::vector<ITestCasePtr> testCases() const = 0;
    virtual ITestCasePtr testCase(const std::string& name) const = 0;

    virtual void setCurrentTestCase(const std::string& name) = 0;
    virtual const ValCh<ITestCasePtr>& currentTestCase() const = 0;

    virtual void runAllFiles() = 0;
    virtual void runFile(int fileIndex) = 0;
    virtual void stop() = 0;
    virtual const ValCh<Status>& status() const = 0;

    virtual const ValNt<Files>& files() const = 0;
    virtual async::Channel<File> fileFinished() const = 0;
    virtual const ValCh<int>& currentFileIndex() const = 0;
};
}

#endif // MU_AUTOBOT_IAUTOBOT_H
