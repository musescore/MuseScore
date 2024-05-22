#ifndef KORS_MODULARITY_INJECTABLE_H
#define KORS_MODULARITY_INJECTABLE_H

#include <cassert>
#include <functional>

#include "context.h"

namespace kors::modularity {
class Injectable
{
public:
    virtual ~Injectable() = default;

    using GetContext = std::function<modularity::ContextPtr()>;

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

        if (m_inj) {
            m_ctx = m_inj->iocContext();
        }

        if (m_getCtx) {
            m_ctx = m_getCtx();
        }

        return m_ctx;
    }

private:
    mutable modularity::ContextPtr m_ctx;
    const Injectable* m_inj = nullptr;
    GetContext m_getCtx;
};
}

#endif // KORS_MODULARITY_INJECTABLE_H
