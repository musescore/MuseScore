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
#include <functional>

#include "imoduleinterface.h"

// Temporary disabled
//#define IOC_CHECK_INTERFACE_TYPE

namespace kors::modularity {
template<class I>
struct Subscriber
{
    using OnChanged = std::function<void (const std::shared_ptr<I>&)>;
    using OnUnSubscribe = std::function<void ()>;

    ~Subscriber()
    {
        unsubscribe();
    }

    // set in client
    OnChanged onChanged = nullptr;

    // set in ioc
    OnUnSubscribe onUnSubscribe = nullptr;

    // called from ioc
    void changed(const std::shared_ptr<I>& p)
    {
        if (onChanged) {
            onChanged(p);
        }
    }

    // called from client
    void unsubscribe()
    {
        if (onUnSubscribe) {
            onUnSubscribe();
        }
    }
};

class ModulesIoCBase
{
public:

    virtual ~ModulesIoCBase()
    {
        reset();
    }

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
        if (resolve<I>(module) == p) {
            unregister<I>(module);
        }
    }

    // Resolve
    template<class I>
    std::shared_ptr<I> resolve(const std::string_view& module)
    {
        std::shared_ptr<IModuleInterface> p = doResolvePtrByInfo(module, I::modularity_interfaceInfo());
        return pointer_cast<I>(p);
    }

    template<class I>
    void subscribe(Subscriber<I>* s)
    {
        int key = doSubscribe(I::modularity_interfaceInfo(), [s](const std::shared_ptr<IModuleInterface>& p) {
            s->changed(pointer_cast<I>(p));
        });

        s->onUnSubscribe = [this, key, s]() {
            doUnsubscribe(I::modularity_interfaceInfo(), key);
            s->onUnSubscribe = nullptr;
        };
    }

#endif

    void reset()
    {
        for (auto& s : m_map) {
            Service& inj = s.second;
            inj.p = nullptr;

            for (const auto& c : inj.onChanges) {
                c.second(inj.p);
            }
        }
        m_map.clear();
    }

    ModulesIoCBase() = default;

protected:

    void unregisterService(const InterfaceInfo& info)
    {
        auto it = m_map.find(info.id);
        if (it == m_map.end()) {
            return;
        }

        Service& inj = it->second;
        inj.p = nullptr;

        for (const auto& c : inj.onChanges) {
            c.second(inj.p);
        }
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
            Service& inj = foundIt->second;
            if (inj.p) {
                std::cerr << module << ": double register:"
                          << info.id << ", first register in" << m_map[info.id].sourceModule << std::endl;
                assert(false);
            } else {
                inj.sourceModule = module;
                inj.p = p;
                for (const auto& c : inj.onChanges) {
                    c.second(inj.p);
                }
            }
        } else {
            Service inj;
            inj.sourceModule = module;
            inj.p = p;
            m_map[info.id] = inj;
        }
    }

    std::shared_ptr<IModuleInterface> doResolvePtrByInfo(const std::string_view& /*usageModule*/,
                                                         const InterfaceInfo& info)
    {
        //! TODO add statistics collection / monitoring, who resolves what

        auto it = m_map.find(info.id);
        if (it == m_map.end()) {
            return nullptr;
        }

        return it->second.p;
    }

    using OnChangedInternal = std::function<void (const std::shared_ptr<IModuleInterface>&)>;
    int doSubscribe(const InterfaceInfo& info, const OnChangedInternal& onChanged)
    {
        auto it = m_map.find(info.id);
        if (it == m_map.end()) {
            return -1;
        }

        Service& inj = it->second;
        int key = ++inj.lastKey;
        inj.onChanges.insert({ key, onChanged });

        return key;
    }

    void doUnsubscribe(const InterfaceInfo& info, int key)
    {
        auto it = m_map.find(info.id);
        if (it == m_map.end()) {
            return;
        }

        Service& inj = it->second;
        inj.onChanges.erase(key);
    }

    template<class I>
    static inline std::shared_ptr<I> pointer_cast(const std::shared_ptr<IModuleInterface>& p)
    {
#ifndef NDEBUG
        return std::dynamic_pointer_cast<I>(p);
#else
        return std::static_pointer_cast<I>(p);
#endif
    }

    struct Service {
        std::string sourceModule;
        std::shared_ptr<IModuleInterface> p;
        int lastKey = 0;
        std::map<int, OnChangedInternal> onChanges;
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
        if (resolve<I>(module) == p) {
            unregister<I>(module);
        }
    }

    // Resolve
    template<class I>
    std::shared_ptr<I> resolve(const std::string_view& module)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        std::shared_ptr<IModuleInterface> p = doResolvePtrByInfo(module, I::modularity_interfaceInfo());
        return pointer_cast<I>(p);
    }

    template<class I>
    void subscribe(Subscriber<I>* s)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
        int key = doSubscribe(I::modularity_interfaceInfo(), [s](const std::shared_ptr<IModuleInterface>& p) {
            s->changed(pointer_cast<I>(p));
        });

        s->onUnSubscribe = [this, key]() {
            doUnsubscribe(I::modularity_interfaceInfo(), key);
        };
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
        if (resolve<I>(module) == p) {
            unregister<I>(module);
        }
    }

    // Resolve
    template<class I>
    std::shared_ptr<I> resolve(const std::string_view& module)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        std::shared_ptr<IModuleInterface> p = doResolvePtrByInfo(module, I::modularity_interfaceInfo());
        return pointer_cast<I>(p);
    }

    template<class I>
    void subscribe(Subscriber<I>* s)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
        int key = doSubscribe(I::modularity_interfaceInfo(), [s](const std::shared_ptr<IModuleInterface>& p) {
            s->changed(pointer_cast<I>(p));
        });

        s->onUnSubscribe = [this, key]() {
            doUnsubscribe(I::modularity_interfaceInfo(), key);
        };
    }
};
}

#endif // KORS_MODULARITY_MODULESIOC_H
