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

using namespace mu::commandline;


mu::Ret CommandLineRegister::apply(const std::string& opt, const CommandLineValues& vals)
{
    auto found = m_handlers.find(opt);
    if (found == m_handlers.end()) {
        return make_ret(Ret::Code::UnknownError, "not found handler");
    }

    Handler& h = found->second;
    h.callback(vals);
    return make_ret(Ret::Code::Ok);
}

void CommandLineRegister::reg(ICommandLineHandler* handler, const CommandLineOption& opt, const CallBackWithVals& call)
{
    for (const std::string& op : opt) {
        doReg(handler, op, call);
    }
}

void CommandLineRegister::doReg(ICommandLineHandler* handler, const std::string& opt, const CallBackWithVals& call)
{
    auto found = m_handlers.find(opt);
    IF_ASSERT_FAILED(found == m_handlers.end()) {
        return;
    }

    Handler h;
    h.h = handler;
    h.callback = call;
    m_handlers.insert({opt, h});
}

void CommandLineRegister::unReg(ICommandLineHandler* handler)
{
    std::list<std::string> opts;
    for (auto const& it : m_handlers) {
        if (it.second.h == handler) {
            opts.push_back(it.first);
        }
    }

    for (const std::string& opt : opts) {
        m_handlers.erase(opt);
    }
}
