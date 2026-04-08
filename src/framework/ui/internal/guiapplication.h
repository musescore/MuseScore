/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <map>
#include <memory>

#include <QTimer>

#include "global/internal/baseapplication.h"
#include "global/internal/cmdoptions.h"

class QQuickWindow;

namespace muse::ui {
class GuiApplication : public BaseApplication
{
public:
    GuiApplication(const std::shared_ptr<CmdOptions>& options);

protected:

    void doSetup(const std::shared_ptr<CmdOptions>& options) override;
    void doFinish() override;
    void setupGraphicsApi();

    void startupScenario(const muse::modularity::ContextPtr& ctxId) override;

    virtual QString mainWindowQmlPath(const QString& platform) const = 0;
    virtual bool loadMainWindow(const muse::modularity::ContextPtr& ctxId);

    virtual void doStartupScenario(const muse::modularity::ContextPtr& ctxId) = 0;

    void doDestroyContext(const ContextData& data) override;

    QTimer m_delayedInitTimer;

    std::map<muse::modularity::IoCID, QQuickWindow*> m_windows;
};
}
