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
#include <mutex>
#include <shared_mutex>
#include <string_view>

#include "context.h"
#include "modulesioc.h"
#include "conf.h"

namespace kors::modularity {
ModulesIoC* globalIoc();
ModulesIoC* ioc(const ContextPtr& ctx);
void removeIoC(const ContextPtr& ctx = nullptr);

//! NOTE Internal base class
template<class I>
class InjectBase
{
public:
    virtual ~InjectBase() = default;

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
            static std::string_view module = "";
            m_i = ioc(iocContext())->template resolve<I>(module);

            if (!m_i && conf::FALLBACK_TO_GLOBAL) {
                m_i = globalIoc()->template resolve<I>(module);
            }
        }
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
    InjectBase(const ContextPtr& ctx)
        : m_ctx(ctx)
    {
    }

    InjectBase(const Injectable* inj)
        : m_inj(inj)
    {
    }

    const ContextPtr m_ctx;
    const Injectable* m_inj = nullptr;
    mutable std::shared_ptr<I> m_i = nullptr;
};

template<class I>
class ThreadSafeInjectBase : public InjectBase<I>
{
public:
    using InjectBase<I>::InjectBase;

    const std::shared_ptr<I>& get() const
    {
        {
            std::shared_lock lock(m_mutex);
            if (InjectBase<I>::m_i) {
                return InjectBase<I>::m_i;
            }
        }

        std::unique_lock lock(m_mutex);
        return InjectBase<I>::get();
    }

    /// For testing purposes.
    void set(std::shared_ptr<I> impl)
    {
        std::unique_lock lock(m_mutex);
        InjectBase<I>::set(impl);
    }

    const std::shared_ptr<I>& operator()() const
    {
        return get();
    }

private:
    mutable std::shared_mutex m_mutex;
};

//! NOTE Global Inject
template<class I>
class GlobalInject : public InjectBase<I>
{
public:
    GlobalInject()
        : InjectBase<I>(globalCtx)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
    }
};

//! NOTE Global Inject, Thread-safe (locking) variant of Inject.
template<class I>
class GlobalThreadSafeInject : public ThreadSafeInjectBase<I>
{
public:
    GlobalThreadSafeInject()
        : ThreadSafeInjectBase<I>(globalCtx)
    {
        static_assert(I::modularity_isGlobalInterface(), "The interface must be global.");
    }
};

//! NOTE Context Inject
template<class I>
class ContextInject : public InjectBase<I>
{
public:

    ContextInject(const ContextPtr& ctx)
        : InjectBase<I>(ctx)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
    }

    ContextInject(const Injectable* inj)
        : InjectBase<I>(inj)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
    }
};

//! NOTE Temporary for compatibility
template<typename I>
using Inject = ContextInject<I>;

//! NOTE State less Inject, Thread-safe (locking) variant of Inject.
template<class I>
class ContextThreadSafeInject : public ThreadSafeInjectBase<I>
{
public:
    ContextThreadSafeInject(const ContextPtr& ctx)
        : ThreadSafeInjectBase<I>(ctx)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
    }

    ContextThreadSafeInject(const Injectable* inj)
        : ThreadSafeInjectBase<I>(inj)
    {
        static_assert(!I::modularity_isGlobalInterface(), "The interface must be contextual.");
    }
};

//! NOTE Temporary for compatibility
template<typename I>
using ThreadSafeInject = ContextThreadSafeInject<I>;
}
