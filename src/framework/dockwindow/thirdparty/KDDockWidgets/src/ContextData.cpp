#include "ContextData.h"

#include <map>

#include "Config.h"
#include "private/DockRegistry_p.h"
#include "private/DragController_p.h"

namespace KDDockWidgets {
static std::map<int, ContextData*> s_data = {};

ContextData* ContextData::context(int ctx)
{
    //! FIXME Temporary for compatibility
    ctx = 0;

    //qDebug() << "ctx: " << ctx;

    auto it = s_data.find(ctx);
    if (it != s_data.end()) {
        return it->second;
    }

    ContextData* d = new ContextData();
    d->config = new Config(ctx);
    d->reg = new DockRegistry(ctx);
    d->dctrl = new DragController(ctx);

    s_data.insert({ ctx, d });

    return d;
}

void ContextData::destroyContext(int ctx)
{
    auto it = s_data.find(ctx);
    if (it == s_data.end()) {
        return;
    }

    ContextData* d = it->second;
    delete d->config;
    delete d->reg;
    delete d->dctrl;
    delete d;

    s_data.erase(ctx);
}
}
