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

#pragma once

#include <memory>
#include <shared_mutex>
#include <string_view>

#include "context.h"
#include "modulesioc.h"
#include "injectable.h"

namespace kors::modularity {
ModulesIoC* _ioc(const ContextPtr& ctx = nullptr);
void removeIoC(const ContextPtr& ctx = nullptr);

template<class I>
class Inject
{
public:
    virtual ~Inject() = default;

#ifdef MUSE_MODULE_GLOBAL_MULTI_IOC
    Inject(const ContextPtr& ctx)
#else
    Inject(const ContextPtr& ctx = nullptr)
    #endif
        : m_ctx(ctx)
    {
        static std::string_view module = "";
        m_i = _ioc(m_ctx)->template resolve<I>(module);
    }

    Inject(const Injectable* inj)
        : Inject(inj->iocContext()) {}

    const std::shared_ptr<I>& get() const
    {
        return m_i;
    }

    /// For testing purposes only. Not thread-safe.
    void set(std::shared_ptr<I> impl)
    {
        m_i = impl;
    }

    const std::shared_ptr<I>& operator()() const
    {
        return get();
    }

protected:
    const ContextPtr m_ctx;
    mutable std::shared_ptr<I> m_i = nullptr;
};

template<class I>
class GlobalInject : public Inject<I>
{
public:
    GlobalInject()
        : Inject<I>(ContextPtr()) {}
};

/// Variant of Inject that resolves the dependency lazily on first use. Useful
/// when the context of the Injectable is not yet known at construction time.
/// Not thread-safe.
template<class I>
class LazyInject
{
public:
    LazyInject(const LazyInjectable* inj)
        : m_inj(inj) {}

    const std::shared_ptr<I>& get() const
    {
        if (!m_i) {
            static std::string_view module = "";
            m_i = _ioc(m_inj->iocContext())->template resolve<I>(module);
        }
        return m_i;
    }

    /// For testing purposes only. Not thread-safe.
    void set(std::shared_ptr<I> impl) { m_i = impl; }

    const std::shared_ptr<I>& operator()() const { return get(); }

private:
    const LazyInjectable* m_inj = nullptr;
    mutable std::shared_ptr<I> m_i = nullptr;
};

/// Thread-safe (locking) variant of LazyInject.
template<class I>
class ThreadSafeLazyInject : public LazyInject<I>
{
public:
    using LazyInject<I>::LazyInject;

    const std::shared_ptr<I>& get() const
    {
        {
            std::shared_lock lock(m_mutex);
            if (LazyInject<I>::m_i) {
                return LazyInject<I>::m_i;
            }
        }

        std::unique_lock lock(m_mutex);
        return LazyInject<I>::get();
    }

    void set(std::shared_ptr<I> impl)
    {
        std::unique_lock lock(m_mutex);
        LazyInject<I>::set(impl);
    }

    const std::shared_ptr<I>& operator()() const
    {
        return get();
    }

private:
    mutable std::shared_mutex m_mutex;
};
}
