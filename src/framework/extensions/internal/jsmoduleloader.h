/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MUSE_EXTENSIONS__H
#define MUSE_EXTENSIONS__H

#include <QObject>
#include <QJSValue>
#include <QList>

#include "modularity/ioc.h"
#include "../iextensionsconfiguration.h"
#include "global/io/ifilesystem.h"

namespace muse::extensions {
class ScriptEngine;
class JsModuleLoader : public QObject, public muse::Injectable
{
    Q_OBJECT
    Q_PROPERTY(QJSValue exports READ exports WRITE setExports)

    Inject<IExtensionsConfiguration> configuration = { this };
    Inject<io::IFileSystem> fileSystem = { this };

public:
    explicit JsModuleLoader(const modularity::ContextPtr& iocCtx, QObject* parent = 0);

    void pushEngine(ScriptEngine* engine);
    void popEngine();

    QJSValue exports() const;
    Q_INVOKABLE QJSValue require(QString module);

    QString scriptPath() const;

    QString resolvePath(const QString& module, bool* ok = 0);
    QString resolvePath(const QString& basePath, const QString& module, bool* ok = nullptr);

public slots:
    void setExports(QJSValue exports);

private:

    ScriptEngine* engine() const;

    QList<ScriptEngine* > m_engineList;
    QJSValue m_exports;
};
}

#endif // MUSE_EXTENSIONS__H
