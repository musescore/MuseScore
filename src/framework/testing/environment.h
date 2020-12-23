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
#ifndef MU_TESTING_ENVIRONMENT_H
#define MU_TESTING_ENVIRONMENT_H

#include <functional>
#include <vector>

#include "framework/global/modularity/imodulesetup.h"

namespace mu::testing {
class Environment
{
public:

    using Modules = std::vector<framework::IModuleSetup*>;
    using PostInit = std::function<void ()>;

    Environment() = default;

    static void setDependency(const Modules& modules);
    static void setPostInit(const PostInit& postInit);

    static void setup();

private:
    static Modules m_dependencyModules;
    static PostInit m_postInit;
};

class SuiteEnvironment
{
public:

    SuiteEnvironment(const Environment::Modules& dependencyModules, const Environment::PostInit& postInit = nullptr)
    {
        Environment::setDependency(dependencyModules);
        Environment::setPostInit(postInit);
    }
};
}

#endif // MU_TESTING_ENVIRONMENT_H
