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
#ifndef MU_AUTOBOT_JSMODULELOADER_H
#define MU_AUTOBOT_JSMODULELOADER_H

#include <QObject>
#include <QJSValue>
#include <QList>

#include "modularity/ioc.h"
#include "../iautobotconfiguration.h"
#include "io/ifilesystem.h"

namespace mu::autobot {
class ScriptEngine;
class JsModuleLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QJSValue exports READ exports WRITE setExports)

    INJECT(IAutobotConfiguration, configuration)
    INJECT(io::IFileSystem, fileSystem)
public:
    explicit JsModuleLoader(QObject* parent = 0);

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

#endif // MU_AUTOBOT_JSMODULELOADER_H
