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
#ifndef MUSE_TESTING_ENVIRONMENT_H
#define MUSE_TESTING_ENVIRONMENT_H

#include <functional>
#include <vector>

#include "modularity/imodulesetup.h"

namespace muse::testing {
class Environment
{
public:

    using Modules = std::vector<modularity::IModuleSetup*>;
    using PreInit = std::function<void ()>;
    using PostInit = std::function<void ()>;
    using DeInit = std::function<void ()>;

    Environment() = default;

    static void setDependency(const Modules& modules);
    static void setPreInit(const PreInit& preInit);
    static void setPostInit(const PostInit& postInit);
    static void setDeInit(const DeInit& deInit);

    static void setup();
    static void deinit();

private:
    static Modules m_dependencyModules;
    static PreInit m_preInit;
    static PostInit m_postInit;
    static DeInit m_deInit;
};

class SuiteEnvironment
{
public:

    SuiteEnvironment(const Environment::Modules& dependencyModules
                     , const Environment::PreInit& preInit = nullptr
                     , const Environment::PostInit& postInit = nullptr
                     , const Environment::DeInit& deInit = nullptr)
    {
        Environment::setDependency(dependencyModules);
        Environment::setPreInit(preInit);
        Environment::setPostInit(postInit);
        Environment::setDeInit(deInit);
    }

    SuiteEnvironment() = default;

    SuiteEnvironment& setDependencyModules(const Environment::Modules& dependencyModules)
    {
        Environment::setDependency(dependencyModules);
        return *this;
    }

    SuiteEnvironment& setPreInit(const Environment::PreInit& preInit)
    {
        Environment::setPreInit(preInit);
        return *this;
    }

    SuiteEnvironment& setPostInit(const Environment::PostInit& postInit)
    {
        Environment::setPostInit(postInit);
        return *this;
    }

    SuiteEnvironment& setDeInit(const Environment::DeInit& deInit)
    {
        Environment::setDeInit(deInit);
        return *this;
    }
};
}

#endif // MUSE_TESTING_ENVIRONMENT_H
