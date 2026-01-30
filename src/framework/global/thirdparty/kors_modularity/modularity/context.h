/*
MIT License

Copyright (c) 2024 Igor Korsukov

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
#ifndef KORS_MODULARITY_CONTEXT_H
#define KORS_MODULARITY_CONTEXT_H

#include <memory>
#include <functional>

namespace kors::modularity {
using IoCID = int;
struct Context
{
    IoCID id = -1;

    Context(IoCID id = -1)
        : id(id) {}
};

using ContextPtr = std::shared_ptr<Context>;

static const IoCID globalId = 0;
static const ContextPtr globalCtx;

class Contextable
{
public:
    virtual ~Contextable() = default;

    using GetContext = std::function<ContextPtr ()>;

    Contextable(const ContextPtr& ctx)
        : m_ctx(ctx) {}

    Contextable(const Contextable* inj)
        : m_inj(inj) {}

    Contextable(const GetContext& getCtx)
        : m_getCtx(getCtx) {}

    Contextable(const Contextable& i) = default;

    Contextable& operator=(const Contextable& i) = default;

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
    const Contextable* m_inj = nullptr;
    GetContext m_getCtx;
};
}

#endif // KORS_MODULARITY_CONTEXT_H
