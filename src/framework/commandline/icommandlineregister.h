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
#ifndef MU_COMMANDLINE_ICOMMANDLINEREGISTER_H
#define MU_COMMANDLINE_ICOMMANDLINEREGISTER_H

#include <memory>
#include "modularity/imoduleexport.h"
#include "icommandlinehandler.h"

namespace mu::commandline {
class ICommandLineRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ICommandLineRegister)

public:
    virtual ~ICommandLineRegister() = default;

    virtual bool reg(const ICommandLineControllerPtr& h) = 0;
    virtual bool reg(const ICommandLineController::Option& opt, const std::function<void()>& f) = 0;
    virtual bool reg(const ICommandLineController::Option& opt, const std::function<void(const ICommandLineHandler::Values& vals)>& f) = 0;

    virtual ICommandLineControllerPtr handler(const ICommandLineController::Option& opt) const = 0;
    virtual ICommandLineControllerPtr handler(const std::string& opt) const = 0;
};
}

#endif // MU_COMMANDLINE_ICOMMANDLINEREGISTER_H
