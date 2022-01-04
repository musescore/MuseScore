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

#ifndef MU_MODULARITY_MODULESIOC_H
#define MU_MODULARITY_MODULESIOC_H

#include <memory>
#include <map>
#include <string>
#include <cassert>
#include <iostream>
#include "imoduleexport.h"

namespace mu::modularity {
class ModulesIoC
{
public:

    static ModulesIoC* instance();

    template<class I>
    void registerExportCreator(const std::string& module, IModuleExportCreator* c)
    {
        if (!c) {
            assert(c);
            return;
        }
        registerService(module, I::interfaceId(), std::shared_ptr<IModuleExportInterface>(), c);
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
    void registerExportNoDelete(const std::string& module, I* p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerExport<I>(module, std::shared_ptr<I>(p, [](I*) {}));
    }

    template<class I>
    void registerExport(const std::string& module, std::shared_ptr<I> p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerService(module, I::interfaceId(), std::static_pointer_cast<IModuleExportInterface>(p), nullptr);
    }

    template<class I>
    void unregisterExport(const std::string& /*module*/)
    {
        unregisterService(I::interfaceId());
    }

    template<class I>
    void unregisterExportIfRegistered(const std::string& module, std::shared_ptr<I> p)
    {
        if (resolve<I>(module) == p) {
            unregisterExport<I>(module);
        }
    }

    template<class I>
    std::shared_ptr<I> resolve(const std::string& module)
    {
        std::shared_ptr<IModuleExportInterface> p = doResolvePtrById(module, I::interfaceId());
#ifdef DEBUG
        return std::dynamic_pointer_cast<I>(p);
#else
        return std::static_pointer_cast<I>(p);
#endif
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
        m_map.clear();
    }

private:

    ModulesIoC() = default;

    void unregisterService(const std::string& id)
    {
        m_map.erase(id);
    }

    void registerService(const std::string& module,
                         const std::string& id,
                         std::shared_ptr<IModuleExportInterface> p,
                         IModuleExportCreator* c)
    {
        auto foundIt = m_map.find(id);
        if (foundIt != m_map.end()) {
            std::cout << module << ": double register:" << id << ", first register in" << m_map[id].sourceModule;
            assert(false);
            return;
        }

        Service inj;
        inj.sourceModule = module;
        inj.c = c;
        inj.p = p;
        m_map[id] = inj;
    }

    std::shared_ptr<IModuleExportInterface> doResolvePtrById(const std::string& resolveModule, const std::string& id)
    {
        (void)(resolveModule); //! TODO add statistics collection / monitoring, who resolves what
        auto it = m_map.find(id);
        if (it == m_map.end()) {
            return nullptr;
        }

        Service& inj = it->second;
        if (inj.p) {
            return inj.p;
        }

        if (inj.c) {
            return inj.c->create();
        }

        return nullptr;
    }

    struct Service {
        IModuleExportCreator* c = nullptr;
        std::string sourceModule;
        std::shared_ptr<IModuleExportInterface> p;
    };

    std::map<std::string, Service > m_map;
};

template<class T>
struct Creator : MODULE_EXPORT_CREATOR
{
    std::shared_ptr<IModuleExportInterface> create() { return std::make_shared<T>(); }
};
}

#endif // MU_MODULARITY_MODULESIOC_H
