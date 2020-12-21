//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_FRAMEWORK_APPLICATION_H
#define MU_FRAMEWORK_APPLICATION_H

#include "../iapplication.h"

namespace mu::framework {
class Application : public IApplication
{
public:

    Application() = default;

    void setRunMode(const RunMode& mode) override;
    RunMode runMode() const override;
    bool noGui() const override;

private:

    RunMode m_runMode = RunMode::Editor;
};
}

#endif // MU_FRAMEWORK_APPLICATION_H
