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

 #include "multiwindowprovider.h"

 #include "log.h"

 #include "../appfactory.h"

using namespace mu::app;

bool MultiWindowProvider::openNewAppInstance(const QStringList& args)
{
    LOGDA() << args;

    AppFactory f;
    CmdOptions opt;
    opt.runMode = muse::IApplication::RunMode::GuiApp;
    std::shared_ptr<muse::IApplication> app = f.newApp(opt);

    app->perform();

    return true;
}
