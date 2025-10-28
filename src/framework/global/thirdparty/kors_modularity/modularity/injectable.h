#pragma once

#include <cassert>
#include <functional>

#include "context.h"

namespace kors::modularity {
class Injectable
{
public:
    virtual ~Injectable() = default;

    Injectable(const modularity::ContextPtr& ctx = nullptr)
        : m_ctx(ctx) {}

    Injectable(const Injectable* inj)
        : m_ctx(inj->iocContext()) {}

    Injectable(const Injectable& i) = default;

    Injectable& operator=(const Injectable& i) = default;

    const modularity::ContextPtr& iocContext() const
    {
        return m_ctx;
    }

private:
    modularity::ContextPtr m_ctx;
};

class LazyInjectable
{
public:
    virtual ~LazyInjectable() = default;

    using GetContext = std::function<modularity::ContextPtr ()>;

    LazyInjectable(const modularity::ContextPtr& ctx = nullptr)
        : m_ctx(ctx) {}

    LazyInjectable(const Injectable* inj)
        : m_ctx(inj->iocContext()) {}

    LazyInjectable(const LazyInjectable* inj)
        : m_inj(inj) {}

    LazyInjectable(const GetContext& getCtx)
        : m_getCtx(getCtx) {}

    LazyInjectable(const LazyInjectable& i) = default;

    LazyInjectable& operator=(const LazyInjectable& i) = default;

    const modularity::ContextPtr& iocContext() const
    {
        if (m_ctx) {
            return m_ctx;
        }

        if (m_inj) {
            m_ctx = m_inj->iocContext();
        } else if (m_getCtx) {
            m_ctx = m_getCtx();
        }

        return m_ctx;
    }

private:
    mutable modularity::ContextPtr m_ctx;
    const LazyInjectable* m_inj = nullptr;
    GetContext m_getCtx;
};
}
