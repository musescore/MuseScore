#ifndef KORS_MODULARITY_INJECTABLE_H
#define KORS_MODULARITY_INJECTABLE_H

#include <cassert>
#include "context.h"

namespace kors::modularity {
class Injectable
{
public:
    virtual ~Injectable() = default;

    Injectable(const ContextPtr& ctx = nullptr)
        : m_ctx(ctx) {}

    Injectable(const Injectable* inj = nullptr)
        : m_inj(inj) {}

    const ContextPtr& iocContext() const
    {
        if (m_ctx) {
            return m_ctx;
        }

        assert(m_inj);
        return m_inj->iocContext();
    }

private:
    const ContextPtr m_ctx;
    const Injectable* m_inj = nullptr;
};
}

#endif // KORS_MODULARITY_INJECTABLE_H
