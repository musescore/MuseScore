#pragma once

#include <cassert>
#include <functional>

#include "context.h"

namespace kors::modularity {
class Injectable
{
public:
    virtual ~Injectable() = default;

    using GetContext = std::function<modularity::ContextPtr ()>;

    Injectable(const modularity::ContextPtr& ctx = nullptr)
        : m_ctx(ctx) {}

    Injectable(const Injectable* inj)
        : m_inj(inj) {}

    Injectable(const GetContext& getCtx)
        : m_getCtx(getCtx) {}

    Injectable(const Injectable& i) = default;

    Injectable& operator=(const Injectable& i) = default;

    const modularity::ContextPtr& iocContext() const
    {
        if (m_ctx) {
            return m_ctx;
        }

        if (m_getCtx) {
            m_ctx = m_getCtx();
        } else if (m_inj) {
            m_ctx = m_inj->iocContext();
        }

        return m_ctx;
    }

private:
    mutable modularity::ContextPtr m_ctx;
    const Injectable* m_inj = nullptr;
    GetContext m_getCtx;
};
}
