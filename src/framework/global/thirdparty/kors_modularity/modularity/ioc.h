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
#ifndef KORS_MODULARITY_IOC_H
#define KORS_MODULARITY_IOC_H

#include <memory>
#include <mutex>
#include <string_view>

#include "context.h"
#include "modulesioc.h"
#include "injectable.h"

namespace kors::modularity {
ModulesIoC* _ioc(const ContextPtr& ctx = nullptr);
void removeIoC(const ContextPtr& ctx = nullptr);

struct StaticMutex
{
    static std::recursive_mutex mutex;
};

template<class I>
class Inject
{
public:

#ifdef MUSE_MODULE_GLOBAL_MULTI_IOC
    Inject(const ContextPtr& ctx)
        : m_ctx(ctx) {}
#else
    Inject(const ContextPtr& ctx = nullptr)
        : m_ctx(ctx) {}
#endif

    Inject(const Injectable* o)
        : m_inj(o) {}

    const ContextPtr& iocContext() const
    {
        if (m_ctx) {
            return m_ctx;
        }

        //assert(m_inj);
        if (m_inj) {
            return m_inj->iocContext();
        }

        // null
        return m_ctx;
    }

    const std::shared_ptr<I>& get() const
    {
        if (!m_i) {
            //! NOTE In resolve, a new object can be created using a creator,
            //! in this object injects can be used in the constructor,
            //! this will lead to a double mutex lock, so the mutex must be recursive.
            const std::lock_guard<std::recursive_mutex> lock(StaticMutex::mutex);
            if (!m_i) {
                static std::string_view module = "";
                m_i = _ioc(iocContext())->template resolve<I>(module);
            }
        }
        return m_i;
    }

    void set(std::shared_ptr<I> impl)
    {
        m_i = impl;
    }

    const std::shared_ptr<I>& operator()() const
    {
        return get();
    }

private:

    const ContextPtr m_ctx;
    const Injectable* m_inj = nullptr;
    mutable std::shared_ptr<I> m_i = nullptr;
};

template<class I>
class GlobalInject : public Inject<I>
{
public:
    GlobalInject()
        : Inject<I>(ContextPtr()) {}
};
}

#endif // KORS_MODULARITY_IOC_H
