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
#ifndef MUSE_AUTOBOT_TESTCASECONTEXT_H
#define MUSE_AUTOBOT_TESTCASECONTEXT_H

#include "../itestcasecontext.h"

namespace muse::autobot {
struct TestCaseContext : public ITestCaseContext
{
public:
    TestCaseContext() = default;

    void setGlobalVal(const Key& key, const Val& val) override;
    Val globalVal(const Key& key) const override;

    const std::vector<StepContext>& steps() const override;
    const StepContext& step(const QString& name) const override;

    void addStep(const QString& name) override;
    const StepContext& currentStep() const override;
    void setStepVal(const Key& key, const Val& val) override;

    Val stepVal(const QString& stepName, const Key& key) const override;
    Val findVal(const Key& key) const override;

private:
    std::map<Key, Val > m_globalVals;
    std::vector<StepContext> m_steps;
};
}
#endif // MUSE_AUTOBOT_TESTCASECONTEXT_H
