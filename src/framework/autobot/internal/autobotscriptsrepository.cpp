/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "autobotscriptsrepository.h"

#include "scriptengine.h"
#include "autobottypes.h"

#include "log.h"

using namespace muse;
using namespace muse::autobot;

Scripts AutobotScriptsRepository::scripts() const
{
    io::paths_t scriptsDirs = configuration()->scriptsDirPaths();
    io::paths_t scriptsPaths;
    for (const io::path_t& dir : scriptsDirs) {
        RetVal<io::paths_t> paths = fileSystem()->scanFiles(dir, { "*.js" }, io::ScanMode::FilesInCurrentDir);
        scriptsPaths.insert(scriptsPaths.end(), paths.val.begin(), paths.val.end());
    }

    std::sort(scriptsPaths.begin(), scriptsPaths.end());

    Scripts scripts;
    for (const io::path_t& p : scriptsPaths) {
        Script s;
        s.path = p;

        //! NOTE Get script info
        ScriptEngine engine(iocContext());
        engine.setScriptPath(p);
        if (!engine.evaluate()) {
            LOGW() << "Bad script: " << p;
            s.type = ScriptType::Undefined;
            s.title = io::completeBasename(p).toQString();
            scripts.push_back(std::move(s));
            continue;
        }

        QJSValue tcVal = engine.globalProperty(TESTCASE_JS_GLOBALNAME);
        TestCase tc(tcVal);
        if (!tc.isValid()) {
            s.type = ScriptType::Custom;
            s.title = io::completeBasename(p).toQString();
            scripts.push_back(std::move(s));
            continue;
        }

        s.type = ScriptType::TestCase;
        s.title = tc.name();
        s.description = tc.description();
        scripts.push_back(std::move(s));
    }

    return scripts;
}
