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
#include "commandlineregister.h"

#include "log.h"

using namespace mu::appshell;

CommandLineRegister::CommandLineRegister()
{
}

bool CommandLineRegister::reg(const ICommandLineControllerPtr& h)
{
    IF_ASSERT_FAILED(h) {
        return false;
    }

    IF_ASSERT_FAILED(handler(h->option()) == nullptr) {
        return false;
    }

    m_handlers.push_back(h);
    return true;
}

ICommandLineControllerPtr CommandLineRegister::handler(const ICommandLineController::Option& opt) const
{
    for (const ICommandLineControllerPtr& h : m_handlers) {
        if (h->option() == opt) {
            return h;
        }
    }
    return nullptr;
}

ICommandLineControllerPtr CommandLineRegister::handler(const std::string& opt) const
{
    for (const ICommandLineControllerPtr& h : m_handlers) {
        if (h->option() == opt) {
            return h;
        }
    }
    return nullptr;
}
