/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "testcasecontext.h"

#include "log.h"

using namespace muse::autobot;

void TestCaseContext::setGlobalVal(const Key& key, const Val& val)
{
    m_globalVals[key] = val;
}

TestCaseContext::Val TestCaseContext::globalVal(const Key& key) const
{
    auto it = m_globalVals.find(key);
    if (it != m_globalVals.end()) {
        return it->second;
    }
    return Val();
}

const std::vector<TestCaseContext::StepContext>& TestCaseContext::steps() const
{
    return m_steps;
}

const TestCaseContext::StepContext& TestCaseContext::step(const QString& name) const
{
    // search from back to front
    auto it = std::find_if(m_steps.rbegin(), m_steps.rend(), [name](const StepContext& c) {
        return c.name == name;
    });

    if (it != m_steps.rend()) {
        return *it;
    }

    static StepContext dummyCtx;
    return dummyCtx;
}

void TestCaseContext::addStep(const QString& name)
{
    StepContext c;
    c.name = name;
    m_steps.push_back(std::move(c));
}

const TestCaseContext::StepContext& TestCaseContext::currentStep() const
{
    if (!m_steps.empty()) {
        return m_steps.back();
    }

    static StepContext dummyCtx;
    return dummyCtx;
}

void TestCaseContext::setStepVal(const Key& key, const Val& val)
{
    IF_ASSERT_FAILED(!m_steps.empty()) {
        return;
    }
    m_steps.back().vals[key] = val;
}

TestCaseContext::Val TestCaseContext::stepVal(const QString& stepName, const Key& key) const
{
    const StepContext& s = step(stepName);
    if (s.name.isEmpty()) {
        LOGW() << "step not found, name: " << stepName;
        return Val();
    }

    auto it = s.vals.find(key);
    if (it != s.vals.end()) {
        return it->second;
    }

    return Val();
}

TestCaseContext::Val TestCaseContext::findVal(const Key& key) const
{
    for (size_t i = m_steps.size(); i > 0; --i) {
        const StepContext& s = m_steps.at(i - 1);
        auto it = s.vals.find(key);
        if (it != s.vals.end()) {
            return it->second;
        }
    }

    return globalVal(key);
}
