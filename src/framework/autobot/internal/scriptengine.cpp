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
#include "scriptengine.h"

#include <QJSValueIterator>

#include "jsmoduleloader.h"
#include "../autobotutils.h"

#include "log.h"

using namespace muse;
using namespace muse::autobot;
using namespace muse::api;

ScriptEngine::ScriptEngine(const modularity::ContextPtr& iocContext)
    : m_iocContext(iocContext)
{
    m_engine = new QJSEngine();
    m_api = new ScriptApi(this, m_engine);

    m_engine->globalObject().setProperty("api", newQObject(m_api));

    m_moduleLoader = new JsModuleLoader(m_iocContext, m_engine);
    m_moduleLoader->pushEngine(this);
    QJSValue loaderObj = newQObject(m_moduleLoader);
    QJSValue requireFn = loaderObj.property("require");
    m_engine->globalObject().setProperty("require", requireFn);
    m_engine->globalObject().setProperty("exports", m_engine->newObject());
    m_engine->globalObject().setProperty("module", loaderObj);
}

ScriptEngine::ScriptEngine(ScriptEngine* engine)
    : m_iocContext(engine->m_iocContext), m_engine(engine->m_engine), m_api(engine->m_api), m_moduleLoader(engine->m_moduleLoader),
    m_isRequireMode(true)
{
    m_moduleLoader->pushEngine(this);
}

ScriptEngine::~ScriptEngine()
{
    m_moduleLoader->popEngine();

    if (!m_isRequireMode) {
        delete m_moduleLoader;
        delete m_api;
        delete m_engine;
    }
}

void ScriptEngine::setScriptPath(const io::path_t& arg)
{
    m_scriptPath = arg;
}

io::path_t ScriptEngine::scriptPath() const
{
    return m_scriptPath;
}

QJSValue ScriptEngine::require(const QString& filePath)
{
    TRACEFUNC;

    RetVal<ByteArray> data = readScriptContent(filePath);
    if (!data.ret) {
        LOGE() << "failed read file, err: " << data.ret.toString() << ", file: " << filePath;
        return QJSValue();
    }

    QByteArray content = QByteArray("(function() { \n") + data.val.toQByteArrayNoCopy() + QByteArray("}());");

    ScriptEngine requireEngine(this);
    requireEngine.setScriptPath(filePath);
    RetVal<QJSValue> val = requireEngine.evaluateContent(content, filePath);
    if (!val.ret) {
        LOGE() << "failed evaluate content ret: " << val.ret.toString();
        return QJSValue();
    }

    QJSValue exports = requireEngine.globalProperty("exports");
    return exports;
}

void ScriptEngine::setGlobalProperty(const QString& name, const QJSValue& val)
{
    m_engine->globalObject().setProperty(name, val);
}

QJSValue ScriptEngine::globalProperty(const QString& name) const
{
    return m_engine->globalObject().property(name);
}

QJSValue ScriptEngine::exports() const
{
    return globalProperty("exports");
}

void ScriptEngine::setExports(const QJSValue& obj)
{
    setGlobalProperty("exports", obj);
}

Ret ScriptEngine::evaluate()
{
    io::path_t path = scriptPath();

    IF_ASSERT_FAILED(!path.empty()) {
        return make_ret(Ret::Code::InternalError);
    }

    RetVal<ByteArray> content = readScriptContent(path);
    if (!content.ret) {
        return content.ret;
    }

    const QByteArray contentData = content.val.toQByteArray();

    IF_ASSERT_FAILED(!contentData.isEmpty()) {
        return make_ret(Ret::Code::InternalError);
    }

    if (m_lastEvalScript != contentData) {
        RetVal<QJSValue> eval = evaluateContent(contentData, path);
        if (!eval.ret) {
            return eval.ret;
        }
        m_lastEvalScript = contentData;
    }

    return Ret(Ret::Code::Ok);
}

Ret ScriptEngine::call(const QString& funcName, QJSValue* retVal)
{
    Ret ret = doCall(funcName, QJSValueList(), retVal);
    return ret;
}

Ret ScriptEngine::call(const QString& funcName, const QJSValueList& args, QJSValue* retVal)
{
    Ret ret = doCall(funcName, args, retVal);
    return ret;
}

Ret ScriptEngine::doCall(const QString& funcName, const QJSValueList& args, QJSValue* retVal)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(!funcName.isEmpty()) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = evaluate();
    if (!ret) {
        return ret;
    }

    if (m_lastCallFunc.funcName != funcName) {
        m_lastCallFunc.func = m_engine->globalObject().property(funcName);
    }

    ret = m_lastCallFunc.func.isCallable();
    if (!ret) {
        m_lastCallFunc.funcName.clear();
        ret = make_ret(Ret::Code::UnknownError, QString("not found function: `%1`").arg(funcName));
        LOGE() << "Not found function: " << funcName;
        //dump("globalObject", m_engine->globalObject());
    }

    QJSValue value;
    if (ret) {
        value = m_lastCallFunc.func.call(args);
        ret = jsValueToRet(value);
    }

    if (retVal) {
        *retVal = value;
    }

    return ret;
}

void ScriptEngine::dump(const QString& name, const QJSValue& val)
{
    QStringList props;
    QJSValueIterator it(val);
    while (it.next()) {
        props << it.name();
    }

    QString str = name + "\n";
    for (const QString& prop : props) {
        str += "  " + prop + ": " + val.property(prop).toString() + "\n";
    }

    LOGD() << str;
}

void ScriptEngine::throwError(const QString& message)
{
    m_engine->throwError(message);
}

RetVal<ByteArray> ScriptEngine::readScriptContent(const io::path_t& scriptPath) const
{
    TRACEFUNC;
    return fileSystem()->readFile(scriptPath);
}

RetVal<QJSValue> ScriptEngine::evaluateContent(const QByteArray& fileContent, const io::path_t& filePath)
{
    TRACEFUNC;
    RetVal<QJSValue> rv;
    rv.val = m_engine->evaluate(QString(fileContent), filePath.toQString());
    rv.ret = jsValueToRet(rv.val);
    return rv;
}

const modularity::ContextPtr& ScriptEngine::iocContext() const
{
    return m_iocContext;
}

QJSValue ScriptEngine::newQObject(QObject* o)
{
    if (!o->parent()) {
        o->setParent(m_engine);
    }
    return m_engine->newQObject(o);
}

QJSValue ScriptEngine::newObject()
{
    return m_engine->newObject();
}

QJSValue ScriptEngine::newArray(size_t length)
{
    return m_engine->newArray(uint(length));
}
