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

#ifndef MU_FRAMEWORK_IOC_H
#define MU_FRAMEWORK_IOC_H

#include "modulesioc.h"

#define INJECT(Module, Interface, getter) \
private: \
    mutable std::shared_ptr<Interface> _##getter; \
public: \
    std::shared_ptr<Interface> getter() const {  \
        if (!_##getter) { \
            _##getter = mu::framework::ioc()->resolve<Interface>(#Module); \
        } \
        return _##getter; \
    } \
    void set##getter(std::shared_ptr<Interface> impl) { _##getter = impl; } \

#define INJECT_STATIC(Module, Interface, getter) \
public: \
    static std::shared_ptr<Interface> getter() {  \
        static std::shared_ptr<Interface> _static##getter; \
        if (!_static##getter) { \
            _static##getter = mu::framework::ioc()->resolve<Interface>(#Module); \
        } \
        return _static##getter; \
    } \

namespace mu {
namespace framework {
inline ModulesIoC* ioc()
{
    return ModulesIoC::instance();
}
}
}

#endif // MU_FRAMEWORK_IOC_H
