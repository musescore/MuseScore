/*
MIT License

Copyright (c) 2020 Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef KORS_MODULARITY_MODULESIOC_H
#define KORS_MODULARITY_MODULESIOC_H

#include <memory>
#include <map>
#include <string>
#include <cassert>
#include <iostream>

#include "imoduleinterface.h"

// Temporary disabled
//#define IOC_CHECK_INTERFACE_TYPE

namespace kors::modularity {
class ModulesIoCBase
{
public:

#ifndef IOC_CHECK_INTERFACE_TYPE
    // Register Export
    template<class I>
    void registerExport(const std::string& module, std::shared_ptr<I> p)
    {
        registerService(module, I::modularity_interfaceInfo(), std::static_pointer_cast<IModuleInterface>(p));
    }

    template<class I>
    void registerExport(const std::string& module, I* p)
    {
        registerExport<I>(module, std::shared_ptr<I>(p));
    }

    template<class I>
    void registerExportNoDelete(const std::string& module, I* p)
    {
        registerExport<I>(module, std::shared_ptr<I>(p, [](I*) {}));
    }

    // Unregister
    template<class I>
    void unregister(const std::string& /*module*/)
    {
        unregisterService(I::modularity_interfaceInfo());
    }

    template<class I>
    void unregisterIfRegistered(const std::string& module, std::shared_ptr<I> p)
    {
        if (resolve<I>(module, std::string_view()) == p) {
            unregister<I>(module);
        }
    }

    // Resolve
    template<class I>
    std::shared_ptr<I> resolve(const std::string_view& module, const std::string_view& callInfo = std::string_view())
    {
        std::shared_ptr<IModuleInterface> p = doResolvePtrByInfo(module, I::modularity_interfaceInfo(), callInfo);
#ifndef NDEBUG
        return std::dynamic_pointer_cast<I>(p);
#else
        return std::static_pointer_cast<I>(p);
#endif
    }

#endif

    void reset()
    {
        m_map.clear();
    }

    ModulesIoCBase() = default;

protected:

    void unregisterService(const InterfaceInfo& info)
    {
        m_map.erase(info.id);
    }

    void registerService(const std::string& module,
                         const InterfaceInfo& info,
                         std::shared_ptr<IModuleInterface> p)
    {
        if (!p) {
            assert(p);
            return;
        }

        auto foundIt = m_map.find(info.id);
        if (foundIt != m_map.end()) {
            std::cerr << module << ": double register:"
                      << info.id << ", first register in" << m_map[info.id].sourceModule << std::endl;
            assert(false);
            return;
        }

        Service inj;
        inj.sourceModule = module;
        inj.p = p;
        m_map[info.id] = inj;
    }

    std::shared_ptr<IModuleInterface> doResolvePtrByInfo(const std::string_view& /*usageModule*/,
                                                         const InterfaceInfo& info,
                                                         const std::string_view& /*callInfo*/)
    {
        //! TODO add statistics collection / monitoring, who resolves what

        auto it = m_map.find(info.id);
        if (it == m_map.end()) {
            return nullptr;
        }

        Service& inj = it->second;
        if (inj.p) {
            return inj.p;
        }

        return nullptr;
    }

    struct Service {
        std::string sourceModule;
        std::shared_ptr<IModuleInterface> p;
    };

    std::map<std::string_view, Service > m_map;
};

class ModulesGlobalIoC : public ModulesIoCBase
{
public:

    // Register Export
    template<class I>
    void registerExport(const std::string& module, std::shared_ptr<I> p)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        registerService(module, I::modularity_interfaceInfo(), std::static_pointer_cast<IModuleInterface>(p));
    }

    template<class I>
    void registerExport(const std::string& module, I* p)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        registerExport<I>(module, std::shared_ptr<I>(p));
    }

    template<class I>
    void registerExportNoDelete(const std::string& module, I* p)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        registerExport<I>(module, std::shared_ptr<I>(p, [](I*) {}));
    }

    // Unregister
    template<class I>
    void unregister(const std::string& /*module*/)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        unregisterService(I::modularity_interfaceInfo());
    }

    template<class I>
    void unregisterIfRegistered(const std::string& module, std::shared_ptr<I> p)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        if (resolve<I>(module, std::string_view()) == p) {
            unregister<I>(module);
        }
    }

    // Resolve
    template<class I>
    std::shared_ptr<I> resolve(const std::string_view& module, const std::string_view& callInfo = std::string_view())
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        std::shared_ptr<IModuleInterface> p = doResolvePtrByInfo(module, I::modularity_interfaceInfo(), callInfo);
#ifndef NDEBUG
        return std::dynamic_pointer_cast<I>(p);
#else
        return std::static_pointer_cast<I>(p);
#endif
    }
};

class ModulesContextIoC : public ModulesIoCBase
{
public:

    // Register Export
    template<class I>
    void registerExport(const std::string& module, std::shared_ptr<I> p)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        registerService(module, I::modularity_interfaceInfo(), std::static_pointer_cast<IModuleInterface>(p));
    }

    template<class I>
    void registerExport(const std::string& module, I* p)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        registerExport<I>(module, std::shared_ptr<I>(p));
    }

    template<class I>
    void registerExportNoDelete(const std::string& module, I* p)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        registerExport<I>(module, std::shared_ptr<I>(p, [](I*) {}));
    }

    // Unregister
    template<class I>
    void unregister(const std::string& /*module*/)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        unregisterService(I::modularity_interfaceInfo());
    }

    template<class I>
    void unregisterIfRegistered(const std::string& module, std::shared_ptr<I> p)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        if (resolve<I>(module, std::string_view()) == p) {
            unregister<I>(module);
        }
    }

    // Resolve
    template<class I>
    std::shared_ptr<I> resolve(const std::string_view& module, const std::string_view& callInfo = std::string_view())
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        std::shared_ptr<IModuleInterface> p = doResolvePtrByInfo(module, I::modularity_interfaceInfo(), callInfo);
#ifndef NDEBUG
        return std::dynamic_pointer_cast<I>(p);
#else
        return std::static_pointer_cast<I>(p);
#endif
    }
};
}

#endif // KORS_MODULARITY_MODULESIOC_H
