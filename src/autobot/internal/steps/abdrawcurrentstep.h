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
#ifndef MU_AUTOBOT_ABDRAWCURRENTSTEP_H
#define MU_AUTOBOT_ABDRAWCURRENTSTEP_H

#include "../abbasestep.h"
#include "modularity/ioc.h"
#include "iautobotconfiguration.h"
#include "system/ifilesystem.h"

namespace mu::autobot {
class AbDrawCurrentStep : public AbBaseStep
{
    INJECT(autobot, IAutobotConfiguration, configuration)
    INJECT(autobot, system::IFileSystem, fileSystem)

public:
    AbDrawCurrentStep(bool isDoSave = false);

    std::string name() const override;

protected:
    void doRun(IAbContextPtr ctx) override;

private:

    bool m_isDoSave = false;
};
}

#endif // MU_AUTOBOT_ABDRAWCURRENTSTEP_H
