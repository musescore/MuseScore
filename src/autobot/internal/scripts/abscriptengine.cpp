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
#include "abscriptengine.h"

#include "log.h"

using namespace mu;
using namespace mu::autobot;
using namespace mu::api;

AbScriptEngine::AbScriptEngine()
{
    m_engine = new QJSEngine();
    m_api = new ScriptApi(this, m_engine);

    m_engine->globalObject().setProperty("api", newQObject(m_api));
}

AbScriptEngine::~AbScriptEngine()
{
    delete m_engine;
}

void AbScriptEngine::setScriptPath(const io::path& arg)
{
    m_scriptPath = arg;
}

io::path AbScriptEngine::scriptPath() const
{
    return m_scriptPath;
}

Ret AbScriptEngine::call(const QString& funcName, QJSValue* retVal)
{
    CallData data;
    Ret ret = doCall(funcName, data, retVal);
    return ret;
}

Ret AbScriptEngine::call(const QString& funcName, const CallData& data, QJSValue* retVal)
{
    Ret ret = doCall(funcName, data, retVal);
    return ret;
}

Ret AbScriptEngine::doCall(const QString& funcName, const CallData& data, QJSValue* retVal)
{
    TRACEFUNC;

    io::path _scriptPath = scriptPath();

    IF_ASSERT_FAILED(!_scriptPath.empty()) {
        return make_ret(Ret::Code::InternalError);
    }

    IF_ASSERT_FAILED(!funcName.isEmpty()) {
        return make_ret(Ret::Code::InternalError);
    }

    RetVal<QByteArray> content = readScriptContent(_scriptPath);
    if (!content.ret) {
        return content.ret;
    }

    QByteArray _scriptContent = content.val;

    IF_ASSERT_FAILED(!_scriptContent.isEmpty()) {
        return make_ret(Ret::Code::InternalError);
    }

    if (m_lastEvalScript != _scriptContent) {
        RetVal<QJSValue> eval = evaluateContent(_scriptContent, _scriptPath);
        if (!eval.ret) {
            return eval.ret;
        }
        m_lastEvalScript = _scriptContent;
    }

    if (m_lastCallFunc.funcName != funcName) {
        m_lastCallFunc.func = m_engine->globalObject().property(funcName);
    }

    Ret ret = m_lastCallFunc.func.isCallable();
    if (!ret) {
        m_lastCallFunc.funcName.clear();
        ret = make_ret(Ret::Code::UnknownError, QString("not found function: `%1`").arg(funcName));
        LOGE() << "Not found function " << funcName;
    }

    QJSValue value;
    if (ret) {
        // api()->context()->insert(data.context);

        QJSValueList args;
        for (const QString& arg : data.args) {
            args.append(arg);
        }

        value = m_lastCallFunc.func.call(args);

        // api()->gc();
        ret = jsValueToRet(value);
    }

    if (retVal) {
        *retVal = value;
    }

    return ret;
}

RetVal<QByteArray> AbScriptEngine::readScriptContent(const io::path& scriptPath) const
{
    TRACEFUNC;
    return fileSystem()->readFile(scriptPath);
}

RetVal<QJSValue> AbScriptEngine::evaluateContent(const QByteArray& fileContent, const io::path& filePath)
{
    TRACEFUNC;
    RetVal<QJSValue> rv;
    rv.val = m_engine->evaluate(QString(fileContent), filePath.toQString());
    rv.ret = jsValueToRet(rv.val);
    return rv;
}

Ret AbScriptEngine::jsValueToRet(const QJSValue& val) const
{
    TRACEFUNC;
    if (val.isError()) {
        QString fileName = val.property("fileName").toString();
        int line = val.property("lineNumber").toInt();
        Ret ret = make_ret(Ret::Code::UnknownError,
                           QString("File: %1, Exception at line: %2, %3").arg(fileName).arg(line).arg(val.toString()));

        LOGE() << ret.toString();
    }

    return Ret(Ret::Code::Ok);
}

QJSValue AbScriptEngine::newQObject(QObject* o)
{
    if (!o->parent()) {
        o->setParent(m_engine);
    }
    return m_engine->newQObject(o);
}
