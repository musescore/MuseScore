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
#ifndef MU_COMMANDLINE_COMMANDLINEREGISTER_H
#define MU_COMMANDLINE_COMMANDLINEREGISTER_H

#include <map>

#include "../icommandlineregister.h"

namespace mu::commandline {
class CommandLineRegister : public ICommandLineRegister
{
public:
    CommandLineRegister() = default;

     Ret apply(const std::string& opt, const CommandLineValues& vals) override;

     void unReg(ICommandLineHandler* handler) override;
     void reg(ICommandLineHandler* handler, const CommandLineOption& opt, const CallBackWithVals& call) override;

private:

     void doReg(ICommandLineHandler* handler, const std::string& opt, const CallBackWithVals& call);

     struct Handler
     {
         ICommandLineHandler* h = nullptr;
         CallBackWithVals callback;
     };

     std::map<std::string/*option*/, Handler > m_handlers;
};
}

#endif // MU_COMMANDLINE_COMMANDLINEREGISTER_H
