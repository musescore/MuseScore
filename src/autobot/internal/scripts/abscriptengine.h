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
#ifndef MU_AUTOBOT_ABSCRIPTENGINE_H
#define MU_AUTOBOT_ABSCRIPTENGINE_H

#include <QString>
#include <QJSValue>
#include <QJSEngine>
#include <QVariantMap>

#include "modularity/ioc.h"
#include "system/ifilesystem.h"
#include "ret.h"

#include "../api/iapiengine.h"
#include "../api/scriptapi.h"

namespace mu::autobot {
class AbScriptEngine : public api::IApiEngine
{
    INJECT(autobot, system::IFileSystem, fileSystem)
public:
    AbScriptEngine();
    ~AbScriptEngine();

    struct CallData {
        QVariantMap context;
        QStringList args;
    };

    void setScriptPath(const io::path& arg);
    io::path scriptPath() const;

    Ret call(const QString& funcName, QJSValue* retVal = nullptr);
    Ret call(const QString& funcName, const CallData& data, QJSValue* retVal = nullptr);

    // IApiEngine
    QJSValue newQObject(QObject* o) override;

private:

    RetVal<QByteArray> readScriptContent(const io::path& scriptPath) const;
    RetVal<QJSValue> evaluateContent(const QByteArray& fileContent, const io::path& filePath);
    Ret jsValueToRet(const QJSValue& val) const;
    Ret doCall(const QString& funcName, const CallData& data, QJSValue* retVal);

    struct FuncData {
        QString funcName;
        QJSValue func;
    };

    QJSEngine* m_engine = nullptr;
    api::ScriptApi* m_api = nullptr;
    io::path m_scriptPath;
    QByteArray m_lastEvalScript;
    FuncData m_lastCallFunc;
};
}

#endif // MU_AUTOBOT_ABSCRIPTENGINE_H
