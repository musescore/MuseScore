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
#include <functional>
#include "modularity/imoduleexport.h"
#include "icommandlinehandler.h"
#include "ret.h"

namespace mu::commandline {
class ICommandLineRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ICommandLineRegister)

public:
    virtual ~ICommandLineRegister() = default;

    using CallBack = std::function<void ()>;
    using CallBackWithVal = std::function<void (const CommandLineValue&)>;
    using CallBackWithVals = std::function<void (const CommandLineValues&)>;

    virtual Ret apply(const std::string& opt, const CommandLineValues& vals) = 0;

    virtual void unReg(ICommandLineHandler* handler) = 0;
    virtual void reg(ICommandLineHandler* handler, const CommandLineOption& opt, const CallBackWithVals& call) = 0;

    void reg(ICommandLineHandler* handler, const CommandLineOption& opt, const CallBackWithVal& call)
    {
        reg(handler, opt, [call](const CommandLineValues& vals) { call(!vals.empty() ? vals.front() : CommandLineValue()); });
    }

    void reg(ICommandLineHandler* handler, const CommandLineOption& opt, const CallBack& call)
    {
        reg(handler, opt, [call](const CommandLineValues&) { call(); });
    }
};
}

#endif // MU_COMMANDLINE_ICOMMANDLINEREGISTER_H
