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

std::recursive_mutex kors::modularity::StaticMutex::mutex;

static std::map<kors::modularity::IoCID, kors::modularity::ModulesIoC*> s_map;

kors::modularity::ModulesIoC* kors::modularity::_ioc(const ContextPtr& ctx)
{
    if (!ctx || ctx->id < 0) {
        static ModulesIoC global;
        return &global;
    }

    auto it = s_map.find(ctx->id);
    if (it != s_map.end()) {
        return it->second;
    }

    return s_map.insert({ ctx->id, new ModulesIoC() }).first->second;
}

void kors::modularity::removeIoC(const ContextPtr& ctx)
{
    if (!ctx || ctx->id < 0) {
        //! NOTE Can't remove global ioc
        return;
    }

    auto it = s_map.find(ctx->id);
    if (it != s_map.end()) {
        s_map.erase(it);
    }
}
