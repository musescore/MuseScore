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

#ifndef MU_FRAMEWORK_MODULESIOC_H
#define MU_FRAMEWORK_MODULESIOC_H

#include <memory>
#include <map>
#include <string>
#include <cassert>

#include "imoduleexport.h"
#include "log.h"

namespace mu {
namespace framework {
class ModulesIoC
{
public:

    static ModulesIoC* instance()
    {
        static ModulesIoC p;
        return &p;
    }

    template<class I>
    void registerExportCreator(const std::string& module, IModuleExportCreator* c)
    {
        if (!c) {
            assert(c);
        }
        registerInjection(module, I::interfaceId(), std::shared_ptr<IModuleExportInterface>(), c);
    }

    template<class I>
    void registerExport(const std::string& module, I* p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerExport<I>(module, std::shared_ptr<I>(p));
    }

    template<class I>
    void registerExport(const std::string& module, std::shared_ptr<I> p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerInjection(module, I::interfaceId(), std::static_pointer_cast<IModuleExportInterface>(p), nullptr);
    }

    template<class I>
    std::shared_ptr<I> resolve(const std::string& module, bool required = true)
    {
        std::shared_ptr<IModuleExportInterface> p = doResolvePtrById(module, I::interfaceId());
        if (required) {
            assert(p);
        }
        return std::static_pointer_cast<I>(p);
    }

    template<class I>
    std::shared_ptr<I> resolveRequiredImport(const std::string& module)
    {
        std::shared_ptr<IModuleExportInterface> p = doResolvePtrById(module, I::interfaceId());
        if (!p) {
            //LOGE() << "not found implementation for interface: " << I::interfaceId();
            assert(false);
        }
#ifdef DEBUG
        return std::dynamic_pointer_cast<I>(p);
#else
        return std::static_pointer_cast<I>(p);
#endif
    }

    void reset()
    {
        _map.clear();
    }

private:

    ModulesIoC() {}

    void removeInjection(const std::string& id)
    {
        _map.erase(id);
    }

    void registerInjection(const std::string& registerModule,
                           const std::string& id,
                           std::shared_ptr<IModuleExportInterface> p,
                           IModuleExportCreator* c)
    {
        auto foundIt = _map.find(id);
        if (foundIt != _map.end()) {
            //LOGE() << registerModule << ": double register:" << id << ", first register in" << _map[id].registerModule;
            assert(false);
            return;
        }

        Injection inj;
        inj.registerModule = registerModule;
        inj.c = c;
        inj.p = p;
        _map[id] = inj;
    }

    std::shared_ptr<IModuleExportInterface> doResolvePtrById(const std::string& resolveModule, const std::string& id)
    {
        (void)(resolveModule); //! TODO add statistics collection / monitoring, who resolves what
        Injection& inj = _map[id];
        if (inj.p) {
            return inj.p;
        }

        if (inj.c) {
            return inj.c->create();
        }

        return nullptr;
    }

    struct Injection {
        IModuleExportCreator* c;
        std::string registerModule;
        std::shared_ptr<IModuleExportInterface> p;
        Injection() : c(nullptr) {}
    };

    std::map<std::string, Injection > _map;
};

template<class T>
struct Creator : MODULE_EXPORT_CREATOR
{
    std::shared_ptr<IModuleExportInterface> create() { return std::make_shared<T>(); }
};
}
}

#endif // MU_FRAMEWORK_MODULESIOC_H
