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
#ifndef MU_TESTING_ENVIRONMENT_H
#define MU_TESTING_ENVIRONMENT_H

#include <functional>
#include <vector>

#include "framework/global/modularity/imodulesetup.h"

namespace mu::testing {
class Environment
{
public:

    using Modules = std::vector<modularity::IModuleSetup*>;
    using PreInit = std::function<void ()>;
    using PostInit = std::function<void ()>;

    Environment() = default;

    static void setDependency(const Modules& modules);
    static void setPreInit(const PreInit& preInit);
    static void setPostInit(const PostInit& postInit);

    static void setup();

private:
    static Modules m_dependencyModules;
    static PreInit m_preInit;
    static PostInit m_postInit;
};

class SuiteEnvironment
{
public:

    SuiteEnvironment(const Environment::Modules& dependencyModules
                     , const Environment::PreInit& preInit = nullptr
                     , const Environment::PostInit& postInit = nullptr)
    {
        Environment::setDependency(dependencyModules);
        Environment::setPreInit(preInit);
        Environment::setPostInit(postInit);
    }
};
}

#endif // MU_TESTING_ENVIRONMENT_H
