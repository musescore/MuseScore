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
#ifndef MU_CLOUD_IACCOUNTCONTROLLER_H
#define MU_CLOUD_IACCOUNTCONTROLLER_H

#include "modularity/imoduleexport.h"
#include "cloudtypes.h"

#include "retval.h"

namespace mu {
namespace cloud {
class IAccountController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAccountController)

public:
    virtual ~IAccountController() = default;

    virtual void createAccount() = 0;
    virtual void signIn() = 0;
    virtual void signOut() = 0;

    virtual ValCh<bool> userAuthorized() const = 0;
    virtual ValCh<AccountInfo> accountInfo() const = 0;
};
}
}

#endif // MU_CLOUD_IACCOUNTCONTROLLER_H
