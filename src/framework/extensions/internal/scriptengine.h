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
#ifndef MUSE_EXTENSIONS_SCRIPTENGINE_H
#define MUSE_EXTENSIONS_SCRIPTENGINE_H

#include <QString>
#include <QJSValue>
#include <QJSEngine>
#include <QVariantMap>
#include <QByteArray>

#include "modularity/ioc.h"
#include "global/io/ifilesystem.h"
#include "global/types/ret.h"

#include "global/api/iapiengine.h"

namespace muse::extensions {
class JsModuleLoader;
class ScriptEngine : public muse::api::IApiEngine
{
    GlobalInject<io::IFileSystem> fileSystem;

public:
    ScriptEngine(const modularity::ContextPtr& iocCtx, int apiverion);
    ~ScriptEngine();

    struct CallData {
        QVariantMap context;
        QStringList args;
    };

    void setScriptPath(const io::path_t& arg);
    io::path_t scriptPath() const;

    Ret evaluate();

    Ret call(const QString& funcName, QJSValue* retVal = nullptr);
    Ret call(const QString& funcName, const QJSValueList& args, QJSValue* retVal = nullptr);

    void setGlobalProperty(const QString& name, const QJSValue& val);
    QJSValue globalProperty(const QString& name) const;

    void throwError(const QString& message);

    // js modules
    QJSValue require(const QString& filePath);
    QJSValue exports() const;
    void setExports(const QJSValue& obj);

    // IApiEngine
    const modularity::ContextPtr& iocContext() const override;
    QJSValue newQObject(QObject* o) override;
    QJSValue newObject() override;
    QJSValue newArray(size_t length = 0) override;

    static void dump(const QString& name, const QJSValue& val);
    static Ret jsValueToRet(const QJSValue& val);

private:

    ScriptEngine(ScriptEngine* engine);

    RetVal<ByteArray> readScriptContent(const io::path_t& scriptPath) const;
    RetVal<QJSValue> evaluateContent(const QByteArray& fileContent, const io::path_t& filePath);
    Ret doCall(const QString& funcName, const QJSValueList& args, QJSValue* retVal);

    struct FuncData {
        QString funcName;
        QJSValue func;
    };

    const modularity::ContextPtr m_iocContext;
    QJSEngine* m_engine = nullptr;
    QObject* m_api = nullptr;
    JsModuleLoader* m_moduleLoader = nullptr;
    bool m_isRequireMode = false;
    io::path_t m_scriptPath;
    QByteArray m_lastEvalScript;
    FuncData m_lastCallFunc;
};
}

#endif // MUSE_EXTENSIONS_SCRIPTENGINE_H
