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
#ifndef MUSE_AUTOBOT_ITESTCASECONTEXT_H
#define MUSE_AUTOBOT_ITESTCASECONTEXT_H

#include <memory>
#include <vector>
#include <map>
#include <QJSValue>
#include <QString>

namespace muse::autobot {
class ITestCaseContext
{
public:
    virtual ~ITestCaseContext() = default;

    using Key = QString;
    using Val = QJSValue;

    struct StepContext
    {
        QString name;
        std::map<Key, Val> vals;
    };

    virtual const std::vector<StepContext>& steps() const = 0;
    virtual const StepContext& step(const QString& name) const = 0;

    // global
    virtual void setGlobalVal(const Key& key, const Val& val) = 0;
    virtual Val globalVal(const Key& key) const = 0;

    // work with current (last) step
    virtual void addStep(const QString& name) = 0;  // become current
    virtual const StepContext& currentStep() const = 0; // last step
    virtual void setStepVal(const Key& key, const Val& val) = 0;

    virtual Val stepVal(const QString& stepName, const Key& key) const = 0;
    virtual Val findVal(const Key& key) const = 0;
};
using ITestCaseContextPtr = std::shared_ptr<ITestCaseContext>;
}

#endif // MUSE_AUTOBOT_ITESTCASECONTEXT_H
