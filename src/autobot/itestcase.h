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
#ifndef MU_AUTOBOT_ITESTCASE_H
#define MU_AUTOBOT_ITESTCASE_H

#include <string>
#include <memory>
#include <vector>

#include "iteststep.h"
#include "autobottypes.h"

namespace mu::autobot {
class ITestCase
{
public:
    virtual ~ITestCase() = default;

    virtual std::string name() const = 0;
    virtual const std::vector<ITestStepPtr>& steps() const = 0;
};

using ITestCasePtr = std::shared_ptr<ITestCase>;
}

#endif // MU_AUTOBOT_ITESTCASE_H
