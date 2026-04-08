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

 #include "global/iapplication.h"

namespace muse {
class ApplicationStub : public IApplication
{
public:
    String name() const override { return u"stub"; }
    String title() const override { return u"stub"; }
    bool unstable() const override { return true; }
    Version version() const override { return Version(4, 0, 0); }
    Version fullVersion() const override { return Version(4, 0, 0); }

    String build() const override { return u"stub"; }
    String revision() const override { return u"stub"; }

    RunMode runMode() const override { return RunMode::ConsoleApp; }
    bool noGui() const override { return true; }

    void showSplash() override {}
    void setup() override {}
    void finish() override {}
    void restart() override {}

    modularity::ContextPtr setupNewContext(const StringList&) override { return nullptr; }
    void destroyContext(const modularity::ContextPtr&) override {}
    size_t contextCount() const override { return 0; }
    std::vector<modularity::ContextPtr> contexts() const override { return {}; }

    void processEvents() override {}

#ifndef NO_QT_SUPPORT
    QWindow* focusWindow() const override { return nullptr; }
    bool notify(QObject*, QEvent*) override { return true; }

    Qt::KeyboardModifiers keyboardModifiers() const override { return Qt::KeyboardModifiers(); }
#endif
};
}
