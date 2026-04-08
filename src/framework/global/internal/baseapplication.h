/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#pragma once

#include <vector>
#include <memory>

#include "../iapplication.h"

#include "global/modularity/ioc.h"
#include "global/itickerprovider.h"
#include "global/modularity/imodulesetup.h"

#include "cmdoptions.h"

namespace muse {
class GlobalModule;
class BaseApplication : public IApplication, public std::enable_shared_from_this<BaseApplication>
{
    GlobalInject<ITickerProvider> tickerProvider;
public:

    BaseApplication(const std::shared_ptr<CmdOptions>& appOptions);

    static String appName();
    static String appTitle();
    static bool appUnstable();
    static Version appVersion();
    static Version appFullVersion();
    static String appBuild();
    static String appRevision();

    String name() const override { return appName(); }
    String title() const override { return appTitle(); }
    bool unstable() const override { return appUnstable(); }
    Version version() const override { return appVersion(); }
    Version fullVersion() const override { return appFullVersion(); }
    String build() const override { return appBuild(); }
    String revision() const override { return appRevision(); }

    void setRunMode(const RunMode& mode);
    RunMode runMode() const override;
    bool noGui() const override;

    void addModule(modularity::IModuleSetup* module);

    // application lifecycle
    void setup() override;
    void restart() override;
    void finish() override;

    // context management
    muse::modularity::ContextPtr setupNewContext(const muse::StringList& args = {}) override;
    void destroyContext(const muse::modularity::ContextPtr& ctx) override;
    size_t contextCount() const override;
    std::vector<muse::modularity::ContextPtr> contexts() const override;

    // runtime
    void processEvents() override;

#ifndef NO_QT_SUPPORT
    QWindow* focusWindow() const override;
    bool notify(QObject* object, QEvent* event) override;

    Qt::KeyboardModifiers keyboardModifiers() const override;
#endif

protected:

    struct ContextData {
        muse::modularity::ContextPtr ctxId;
        bool initializing = false;
        std::shared_ptr<CmdOptions> options;
        std::vector<muse::modularity::IContextSetup*> setups;

        bool isValid() const { return ctxId != nullptr && !setups.empty(); }
    };

    virtual void doSetup(const std::shared_ptr<CmdOptions>& options);
    virtual void doFinish();
    virtual void applyCommandLineOptions(const std::shared_ptr<CmdOptions>& options);

    ContextData& contextData(const muse::modularity::ContextPtr& ctx);
    virtual std::shared_ptr<CmdOptions> makeContextOptions(const muse::StringList& args) const;
    virtual void showContextSplash(const muse::modularity::ContextPtr& ctx);
    virtual void setupContext(const muse::modularity::ContextPtr& ctx);
    virtual void startupScenario(const muse::modularity::ContextPtr& ctx) = 0;
    virtual void doDestroyContext(const ContextData& data);

    RunMode m_runMode = RunMode::GuiApp;

    //! NOTE Separately to initialize logger and profiler as early as possible
    muse::GlobalModule* m_globalModule = nullptr;
    std::vector<muse::modularity::IModuleSetup*> m_modules;
    std::shared_ptr<CmdOptions> m_appOptions;

    std::vector<ContextData> m_contexts;

    FinishMode m_finishMode = FinishMode::Default;
};
}
