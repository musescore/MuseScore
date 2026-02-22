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
#include "ioc.h"

#include <map>
#include <utility>

// Context
const kors::modularity::ContextPtr globalCtx
    = std::make_shared<kors::modularity::Context>(kors::modularity::globalId);


#ifdef IOC_CHECK_INTERFACE_TYPE   
// IoC
static std::map<kors::modularity::IoCID, kors::modularity::ModulesContextIoC*> s_contexts;

kors::modularity::ModulesGlobalIoC* kors::modularity::globalIoc()
{
    static ModulesGlobalIoC global;
    return &global;
}

kors::modularity::ModulesContextIoC* kors::modularity::ioc(const ContextPtr& ctx)
{
    assert(ctx && ctx->id > 0);
    if (!(ctx && ctx->id > 0)) {
        return nullptr;
    }

    auto it = s_contexts.find(ctx->id);
    if (it != s_contexts.end()) {
        return it->second;
    }

    return s_contexts.insert({ ctx->id, new ModulesContextIoC() }).first->second;
}

#else 
// IoC
static std::map<kors::modularity::IoCID, kors::modularity::ModulesIoCBase*> s_contexts;

kors::modularity::ModulesIoCBase* kors::modularity::globalIoc()
{
    static ModulesIoCBase global;
    return &global;
}

kors::modularity::ModulesIoCBase* kors::modularity::ioc(const ContextPtr& ctx)
{
    if (!(ctx && ctx->id > 0)) {
        return globalIoc();
    }

    auto it = s_contexts.find(ctx->id);
    if (it != s_contexts.end()) {
        return it->second;
    }

    return s_contexts.insert({ ctx->id, new ModulesIoCBase() }).first->second;
}

#endif

void kors::modularity::removeIoC(const ContextPtr& ctx)
{
    if (!(ctx && ctx->id > 0)) {
        return;
    }

    auto it = s_contexts.find(ctx->id);
    if (it != s_contexts.end()) {
        delete it->second;
        s_contexts.erase(it);
    }
}
