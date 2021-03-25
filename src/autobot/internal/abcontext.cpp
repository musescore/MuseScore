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
#include "abcontext.h"

#include "log.h"

using namespace mu::autobot;

const std::vector<IAbContext::StepContext>& AbContext::steps() const
{
    return m_steps;
}

void AbContext::addStep(const std::string& name)
{
    StepContext c;
    c.name = name;
    m_steps.push_back(std::move(c));
}

const IAbContext::StepContext& AbContext::currentStep() const
{
    if (!m_steps.empty()) {
        return m_steps.back();
    }

    static StepContext dummy;
    return dummy;
}

const IAbContext::StepContext& AbContext::step(const std::string& name) const
{
    // search from back to front
    auto it = std::find_if(m_steps.rbegin(), m_steps.rend(), [name](const StepContext& c) {
        return c.name == name;
    });

    if (it != m_steps.rend()) {
        return *it;
    }

    static StepContext dummy;
    return dummy;
}

void AbContext::setGlobalVal(const Key& key, const Val& val)
{
    m_globalVals[key] = val;
}

IAbContext::Val AbContext::globalVal(const Key& key) const
{
    auto it = m_globalVals.find(key);
    if (it != m_globalVals.end()) {
        return it->second;
    }
    return Val();
}

void AbContext::setStepVal(const Key& key, const Val& val)
{
    IF_ASSERT_FAILED(!m_steps.empty()) {
        return;
    }
    m_steps.back().vals[key] = val;
}

IAbContext::Val AbContext::stepVal(const std::string& stepName, const Key& key) const
{
    const StepContext& s = step(stepName);
    if (s.name.empty()) {
        LOGW() << "step not found, name: " << stepName;
        return Val();
    }

    auto it = s.vals.find(key);
    if (it != s.vals.end()) {
        return it->second;
    }

    return Val();
}

void AbContext::setStepRet(const Ret& ret)
{
    IF_ASSERT_FAILED(!m_steps.empty()) {
        return;
    }
    m_steps.back().ret = ret;
}

mu::Ret AbContext::stepRet(const std::string& stepName) const
{
    const StepContext& s = step(stepName);
    if (s.name.empty()) {
        LOGW() << "step not found, name: " << stepName;
        return Ret();
    }

    return s.ret;
}

IAbContext::Val AbContext::findVal(const Key& key) const
{
    for (int i = m_steps.size() - 1; i >= 0; --i) {
        const StepContext& s = m_steps.at(i);
        auto it = s.vals.find(key);
        if (it != s.vals.end()) {
            return it->second;
        }
    }

    return globalVal(key);
}

mu::Ret AbContext::completeRet() const
{
    for (const StepContext& s : m_steps) {
        if (!s.ret) {
            return s.ret;
        }
    }

    return make_ret(Ret::Code::Ok);
}
