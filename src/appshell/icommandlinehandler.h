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
#ifndef MU_APPSHELL_ICOMMANDLINEHANDLER_H
#define MU_APPSHELL_ICOMMANDLINEHANDLER_H

#include <utility>
#include <string>
#include <vector>
#include <memory>

namespace mu::appshell {
class ICommandLineHandler
{
public:
    virtual ~ICommandLineHandler() = default;

    using Option = std::pair<std::string /*short*/, std::string /*long*/>;
    using Value = std::string;
    using Values = std::vector<Value>;

    virtual Option option() const = 0;
    virtual bool exec(const Values& vals) = 0;
};

using ICommandLineHandlerPtr = std::shared_ptr<ICommandLineHandler>;

inline bool operator ==(const ICommandLineHandler::Option& opt, const std::string& str)
{
    return opt.first == str || opt.second == str;
}
}

#endif // MU_APPSHELL_ICOMMANDLINEHANDLER_H
