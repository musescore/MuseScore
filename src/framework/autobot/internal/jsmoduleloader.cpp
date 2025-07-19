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
#include <QFileInfo>

#include "jsmoduleloader.h"

#include "scriptengine.h"

#include "log.h"

using namespace muse::autobot;

JsModuleLoader::JsModuleLoader(const modularity::ContextPtr& iocCtx, QObject* parent)
    : QObject(parent), Injectable(iocCtx)
{
}

void JsModuleLoader::pushEngine(ScriptEngine* engine)
{
    m_engineList.append(engine);
}

void JsModuleLoader::popEngine()
{
    if (!m_engineList.isEmpty()) {
        m_engineList.removeLast();
    }
}

ScriptEngine* JsModuleLoader::engine() const
{
    ScriptEngine* engine = !m_engineList.isEmpty() ? m_engineList.last() : nullptr;
    return engine;
}

QJSValue JsModuleLoader::require(QString module)
{
    //!TODO Make recursion protection
    IF_ASSERT_FAILED(engine()) {
        return QJSValue();
    }

    bool ok = false;
    QString filePath = resolvePath(module, &ok);
    if (!ok) {
        return QJSValue();
    }

    return engine()->require(filePath);
}

QJSValue JsModuleLoader::exports() const
{
    IF_ASSERT_FAILED(engine()) {
        return QJSValue();
    }

    return engine()->exports();
}

void JsModuleLoader::setExports(QJSValue exports)
{
    IF_ASSERT_FAILED(engine()) {
        return;
    }

    return engine()->setExports(exports);
}

QString JsModuleLoader::scriptPath() const
{
    IF_ASSERT_FAILED(engine()) {
        return QString();
    }
    return QFileInfo(engine()->scriptPath().toQString()).absolutePath();
}

QString JsModuleLoader::resolvePath(const QString& module, bool* ok)
{
    QString basePath = scriptPath();
    return resolvePath(basePath, module, ok);
}

QString JsModuleLoader::resolvePath(const QString& basePath, const QString& module, bool* _ok)
{
    TRACEFUNC;
    QString moduleFile = module.endsWith(".js") ? module : (module + ".js");

    //! NOTE Check relative path
    io::path_t path = basePath + "/" + moduleFile;
    bool ok = fileSystem()->exists(path);
    if (!ok) {
        //! NOTE Search module in default paths
        static const io::paths_t defPaths = configuration()->scriptsDirPaths();

        for (const io::path_t& defPath : defPaths) {
            path = defPath + "/" + moduleFile;
            ok = fileSystem()->exists(path);
            if (ok) {
                break;
            }
        }
    }

    if (!ok) {
        LOGE() << "Not found module: " << module;
    }

    if (_ok) {
        *_ok = ok;
    }

    return ok ? path.toQString() : "";
}
