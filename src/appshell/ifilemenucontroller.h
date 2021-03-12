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
#ifndef MU_APPSHELL_IFILEMENUCONTROLLER_H
#define MU_APPSHELL_IFILEMENUCONTROLLER_H

#include "modularity/imoduleexport.h"
#include "uicomponents/imenucontroller.h"

namespace mu::appshell {
class IFileMenuController : public uicomponents::IMenuController, MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFileMenuController)
};
}

#endif // MU_APPSHELL_IFILEMENUCONTROLLER_H
