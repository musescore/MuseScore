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
#ifndef MU_FRAMEWORK_LAUNCHER_H
#define MU_FRAMEWORK_LAUNCHER_H

#include "../ilauncher.h"
#include "modularity/ioc.h"
#include "ui/iqmllaunchprovider.h"
#include "retval.h"

namespace mu {
namespace framework {
class Launcher : public ILauncher
{
    INJECT(ui, IQmlLaunchProvider, qmlprovider)

public:
    RetVal<Val> open(const std::string& uri) override;
    RetVal<Val> open(const UriQuery& uri) override;
    ValCh<Uri> currentUri() const override;

    Ret openUrl(const std::string& url) override;
};
}
}

#endif // MU_FRAMEWORK_LAUNCHER_H
