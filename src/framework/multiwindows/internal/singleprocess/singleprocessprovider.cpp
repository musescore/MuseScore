/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include "singleprocessprovider.h"

 #include "../../iprojectprovider.h"
 #include "ui/imainwindow.h"
 #include "actions/iactionsdispatcher.h"

 #include "log.h"

using namespace muse;
using namespace muse::mi;

static const std::string pname = "mi";

int SingleProcessProvider::windowCount() const
{
    return std::max(1, application()->contextCount());
}

bool SingleProcessProvider::isFirstWindow() const
{
    return windowCount() <= 1;
}

std::shared_ptr<IProjectProvider> SingleProcessProvider::projectProvider(const modularity::ContextPtr& ctx) const
{
    return modularity::ioc(ctx)->resolve<IProjectProvider>(pname);
}

std::shared_ptr<ui::IMainWindow> SingleProcessProvider::mainWindow(const modularity::ContextPtr& ctx) const
{
    return modularity::ioc(ctx)->resolve<ui::IMainWindow>(pname);
}

std::shared_ptr<actions::IActionsDispatcher> SingleProcessProvider::dispatcher(const modularity::ContextPtr& ctx) const
{
    return modularity::ioc(ctx)->resolve<actions::IActionsDispatcher>(pname);
}

bool SingleProcessProvider::isProjectAlreadyOpened(const muse::io::path_t& projectPath) const
{
    for (const auto& ctx : application()->contexts()) {
        std::shared_ptr<IProjectProvider> pp = projectProvider(ctx);
        if (!pp) {
            LOGW() << "Not found implementation of IProjectProvider for context: " << ctx->id;
            continue;
        }

        if (pp->isProjectOpened(projectPath)) {
            return true;
        }
    }
    return false;
}

void SingleProcessProvider::activateWindowWithProject(const muse::io::path_t& projectPath)
{
    for (const auto& ctx : application()->contexts()) {
        std::shared_ptr<IProjectProvider> pp = projectProvider(ctx);
        if (!pp) {
            LOGW() << "Not found implementation of IProjectProvider for context: " << ctx->id;
            continue;
        }

        if (!pp->isProjectOpened(projectPath)) {
            continue;
        }

        std::shared_ptr<ui::IMainWindow> w = mainWindow(ctx);
        IF_ASSERT_FAILED(w) {
            continue;
        }

        w->requestShowOnFront();
        break;
    }
}

bool SingleProcessProvider::isHasWindowWithoutProject() const
{
    for (const auto& ctx : application()->contexts()) {
        std::shared_ptr<IProjectProvider> pp = projectProvider(ctx);
        if (!pp) {
            LOGW() << "Not found implementation of IProjectProvider for context: " << ctx->id;
            continue;
        }

        if (!pp->isAnyProjectOpened()) {
            return true;
        }
    }
    return false;
}

void SingleProcessProvider::activateWindowWithoutProject(const QStringList& args)
{
    for (const auto& ctx : application()->contexts()) {
        std::shared_ptr<IProjectProvider> pp = projectProvider(ctx);
        if (!pp) {
            LOGW() << "Not found implementation of IProjectProvider for context: " << ctx->id;
            continue;
        }

        if (pp->isAnyProjectOpened()) {
            continue;
        }

        std::shared_ptr<ui::IMainWindow> w = mainWindow(ctx);
        IF_ASSERT_FAILED(w) {
            continue;
        }

        w->requestShowOnFront();
        if (args.count() > 0 && !args.at(0).isEmpty()) {
            std::shared_ptr<actions::IActionsDispatcher> d = dispatcher(ctx);
            IF_ASSERT_FAILED(d) {
                break;
            }
            d->dispatch(args.at(0).toStdString(), actions::ActionData::make_arg1<bool>(false));
        }

        break;
    }
}

bool SingleProcessProvider::openNewWindow(const QStringList& args)
{
    LOGDA() << args;

    application()->setupNewContext(args);

    return true;
}
