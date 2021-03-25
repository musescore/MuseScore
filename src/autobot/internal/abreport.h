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
#ifndef MU_AUTOBOT_ABREPORT_H
#define MU_AUTOBOT_ABREPORT_H

#include "modularity/ioc.h"
#include "../iautobotconfiguration.h"

#include "../itestcase.h"
#include "../abtypes.h"
#include "../iabcontext.h"

namespace mu::autobot {
class AbReport
{
    INJECT(autobot, IAutobotConfiguration, configuration)
public:
    AbReport() = default;

    void beginReport(const ITestCasePtr& testCase);
    void endReport();

    void beginFile(const File& file);
    void endFile(const IAbContextPtr& ctx);

    void beginStep(const IAbContextPtr& ctx);
    void endStep(const IAbContextPtr& ctx);
};
}

#endif // MU_AUTOBOT_ABREPORT_H
