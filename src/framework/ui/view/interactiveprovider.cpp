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
#include "interactiveprovider.h"

#include <QMetaType>
#include <QMetaProperty>
#include <QDialog>
#include <QFileDialog>
#include <QColorDialog>
#include <QGuiApplication>
#include <QWindow>

#include "global/async/async.h"
#include "diagnostics/diagnosticutils.h"

#include "internal/widgetdialogadapter.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::async;

InteractiveProvider::InteractiveProvider(const modularity::ContextPtr& iocCtx)
    : QObject(), Injectable(iocCtx)
{
    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](QWindow* window) {
        raiseWindowInStack(window);
    });

    connect(qApp, &QGuiApplication::focusObjectChanged, this, [this](QObject* obj) {
        auto widget = dynamic_cast<QWidget*>(obj);

        if (widget && widget->isWindow()) {
            raiseWindowInStack(widget);
        }
    });
}

void InteractiveProvider::raiseWindowInStack(QObject* newActiveWindow)
{
    if (!newActiveWindow || m_stack.isEmpty() || m_stack.top().window == newActiveWindow) {
        return;
    }

    for (int i = 0; i < m_stack.size(); ++i) {
        bool found = m_stack[i].window == newActiveWindow;
        if (m_stack[i].window && m_stack[i].window->isWidgetType()) {
            found = newActiveWindow->objectName() == (m_stack[i].window->objectName() + "Window");
        }

        if (found) {
            ObjectInfo info = m_stack.takeAt(i);
            m_stack.push(info);
            notifyAboutCurrentUriChanged();
            return;
        }
    }
}

async::Promise<Color> InteractiveProvider::selectColor(const Color& color, const std::string& title)
{
    if (m_isSelectColorOpened) {
        LOGW() << "already opened";
        return async::make_promise<Color>([](auto, auto reject) {
            Ret ret = muse::make_ret(Ret::Code::UnknownError);
            return reject(ret.code(), ret.text());
        });
    }

    m_isSelectColorOpened = true;

    return async::make_promise<Color>([this, color, title](auto resolve, auto reject) {
        //! FIX https://github.com/musescore/MuseScore/issues/23208
        shortcutsRegister()->setActive(false);

        QColorDialog* dlg = new QColorDialog();
        if (!title.empty()) {
            dlg->setWindowTitle(QString::fromStdString(title));
        }

        dlg->setCurrentColor(color.toQColor());

        QObject::connect(dlg, &QFileDialog::finished, [this, dlg, resolve, reject](int result) {
            dlg->deleteLater();

            m_isSelectColorOpened = false;
            shortcutsRegister()->setActive(true);

            if (result != QDialog::Accepted) {
                Ret ret = muse::make_ret(Ret::Code::Cancel);
                (void)reject(ret.code(), ret.text());
                return;
            }

            QColor selectedColor = dlg->selectedColor();
            (void)resolve(Color::fromQColor(selectedColor));
        });

        dlg->open();

        return async::Promise<Color>::dummy_result();
    }, async::PromiseType::AsyncByBody);
}

bool InteractiveProvider::isSelectColorOpened() const
{
    return m_isSelectColorOpened;
}

RetVal<Val> InteractiveProvider::openSync(const UriQuery& q_)
{
#ifndef MUSE_MODULE_UI_SYNCINTERACTIVE_SUPPORTED
    NOT_SUPPORTED;
    std::abort();
    {
        RetVal<Val> rv;
        rv.ret = muse::make_ret(Ret::Code::NotSupported);
        return rv;
    }
#endif

    UriQuery q = q_;

    //! NOTE Disable Dialog.exec()
    q.set("sync", false);

    RetVal<Val> rv;
    QEventLoop loop;
    Promise<Val>::Resolve resolve;
    Promise<Val>::Reject reject;
    Promise<Val> promise = make_promise<Val>([&resolve, &reject](auto res, auto rej) {
        resolve = res;
        reject = rej;
        return Promise<Val>::Result::unchecked();
    }, PromiseType::AsyncByBody);

    promise.onResolve(this, [&rv, &loop](const Val& val) {
        rv = RetVal<Val>::make_ok(val);
        loop.quit();
    });

    promise.onReject(this, [&rv, &loop](int code, const std::string& err) {
        LOGE() << code << " " << err;
        rv.ret = make_ret(code, err);
        loop.quit();
    });

    auto func = openFunc(q);
    func(resolve, reject);

    ContainerMeta openMeta = uriRegister()->meta(q.uri());
    if (openMeta.type == ContainerType::PrimaryPage) {
        LOGW() << "Primary pages should not open in synchronous mode, please fix this.";
        return rv;
    }

    loop.exec();

    return rv;
}

Promise<Val> InteractiveProvider::openAsync(const UriQuery& q)
{
    return make_promise<Val>(openFunc(q), PromiseType::AsyncByBody);
}

async::Promise<Val> InteractiveProvider::openAsync(const Uri& uri, const QVariantMap& params)
{
    return make_promise<Val>(openFunc(UriQuery(uri), params), PromiseType::AsyncByBody);
}

Promise<Val>::Body InteractiveProvider::openFunc(const UriQuery& q)
{
    QVariantMap params;
    const UriQuery::Params& p = q.params();
    for (auto it = p.cbegin(); it != p.cend(); ++it) {
        params[QString::fromStdString(it->first)] = it->second.toQVariant();
    }

    return openFunc(q, params);
}

Promise<Val>::Body InteractiveProvider::openFunc(const UriQuery& q, const QVariantMap& params)
{
    auto func = [this, q, params](Promise<Val>::Resolve resolve, Promise<Val>::Reject reject) {
        IF_ASSERT_FAILED(!m_openingObject.objectId.isValid()) {
            LOGE() << "The opening of the previous object has not been completed"
                   << ", objectId: " << m_openingObject.objectId.toString()
                   << ", query: " << m_openingObject.query.toString();
        }

        m_openingObject = { q, resolve, reject, QVariant(), nullptr };

        RetVal<OpenData> openedRet;

        notifyAboutCurrentUriWillBeChanged();

        //! NOTE Currently, extensions do not replace the default functionality
        //! But in the future, we may allow extensions to replace the current functionality
        //! (first check for the presence of an extension with this uri,
        //! and if it is found, then open it)

        ContainerMeta openMeta = uriRegister()->meta(q.uri());
        switch (openMeta.type) {
        case ContainerType::QWidgetDialog:
            openedRet = openWidgetDialog(q.uri(), params);
            break;
        case ContainerType::PrimaryPage:
        case ContainerType::QmlDialog:
            openedRet = openQml(q.uri(), params);
            break;
        case ContainerType::Undefined: {
            //! NOTE Not found default, try extension
            extensions::Manifest ext = extensionsProvider()->manifest(q.uri());
            if (ext.isValid()) {
                openedRet = openExtensionDialog(q, params);
            } else {
                openedRet.ret = make_ret(Ret::Code::UnknownError);
            }
        }
        }

        if (!openedRet.ret) {
            LOGE() << "failed open err: " << openedRet.ret.toString() << ", uri: " << q.toString();
            return reject(openedRet.ret.code(), openedRet.ret.text());
        }

        return Promise<Val>::Result::unchecked();
    };

    return func;
}

RetVal<bool> InteractiveProvider::isOpened(const Uri& uri) const
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        if (objectInfo.query.uri() == uri) {
            return RetVal<bool>::make_ok(true);
        }
    }

    return RetVal<bool>::make_ok(false);
}

RetVal<bool> InteractiveProvider::isOpened(const UriQuery& uri) const
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        if (objectInfo.query == uri) {
            return RetVal<bool>::make_ok(true);
        }
    }

    return RetVal<bool>::make_ok(false);
}

async::Channel<Uri> InteractiveProvider::opened() const
{
    return m_opened;
}

void InteractiveProvider::raise(const UriQuery& uri)
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        if (objectInfo.query != uri) {
            continue;
        }

        ContainerMeta openMeta = uriRegister()->meta(objectInfo.query.uri());
        switch (openMeta.type) {
        case ContainerType::QWidgetDialog: {
            if (auto window = dynamic_cast<QWidget*>(objectInfo.window)) {
                window->raise();
                window->activateWindow();
            }
        } break;
        case ContainerType::QmlDialog:
            raiseQml(objectInfo.objectId);
            break;
        case ContainerType::PrimaryPage:
        case ContainerType::Undefined:
            break;
        }
    }
}

void InteractiveProvider::close(const Uri& uri)
{
    for (const ObjectInfo& obj : allOpenObjects()) {
        if (obj.query.uri() == uri) {
            closeObject(obj);
        }
    }
}

void InteractiveProvider::close(const UriQuery& uri)
{
    for (const ObjectInfo& obj : allOpenObjects()) {
        if (obj.query == uri) {
            closeObject(obj);
        }
    }
}

void InteractiveProvider::closeAllDialogs()
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        UriQuery uriQuery = objectInfo.query;
        if (muse::diagnostics::isDiagnosticsUri(uriQuery.uri())) {
            continue;
        }
        ContainerMeta openMeta = uriRegister()->meta(uriQuery.uri());
        if (openMeta.type == ContainerType::QWidgetDialog || openMeta.type == ContainerType::QmlDialog) {
            closeObject(objectInfo);
        }
    }
}

void InteractiveProvider::closeObject(const ObjectInfo& obj)
{
    ContainerMeta openMeta = uriRegister()->meta(obj.query.uri());
    switch (openMeta.type) {
    case ContainerType::QWidgetDialog: {
        if (auto window = dynamic_cast<QWidget*>(obj.window)) {
            window->close();
        }
    } break;
    case ContainerType::QmlDialog:
        closeQml(obj.objectId);
        break;
    case ContainerType::PrimaryPage:
    case ContainerType::Undefined:
        break;
    }
}

void InteractiveProvider::fillExtData(QmlLaunchData* data, const UriQuery& q, const QVariantMap& params_) const
{
    static Uri VIEWER_URI = Uri("muse://extensions/viewer");

    ContainerMeta meta = uriRegister()->meta(VIEWER_URI);
    data->setValue("path", meta.qmlPath);
    data->setValue("type", meta.type);

    QVariantMap params = params_;
    params["uri"] = QString::fromStdString(q.toString());

    //! NOTE Extension dialogs open as non-modal by default
    //! The modal parameter must be present in the uri
    //! But here, just in case, `true` is indicated by default,
    //! since this value is set in the base class of the dialog by default
    if (!params.contains("modal")) {
        params["modal"] = q.param("modal", Val(true)).toBool();
    }

    data->setValue("uri", QString::fromStdString(VIEWER_URI.toString()));
    data->setValue("sync", params.value("sync", false));
    data->setValue("params", params);
}

void InteractiveProvider::fillData(QmlLaunchData* data, const Uri& uri, const QVariantMap& params) const
{
    ContainerMeta meta = uriRegister()->meta(uri);
    data->setValue("path", meta.qmlPath);
    data->setValue("type", meta.type);
    data->setValue("uri", QString::fromStdString(uri.toString()));
    data->setValue("params", params);
    data->setValue("sync", params.value("sync", false));
    data->setValue("modal", params.value("modal", ""));
}

void InteractiveProvider::fillData(QObject* object, const QVariantMap& params) const
{
    const QMetaObject* metaObject = object->metaObject();
    for (int i = 0; i < metaObject->propertyCount(); i++) {
        QMetaProperty metaProperty = metaObject->property(i);
        if (params.contains(metaProperty.name())) {
            object->setProperty(metaProperty.name(), params[metaProperty.name()]);
        }
    }
}

ValCh<Uri> InteractiveProvider::currentUri() const
{
    ValCh<Uri> v;
    if (!m_stack.empty()) {
        v.val = m_stack.last().query.uri();
    }
    v.ch = m_currentUriChanged;
    return v;
}

RetVal<bool> InteractiveProvider::isCurrentUriDialog() const
{
    if (m_stack.empty()) {
        return RetVal<bool>::make_ok(false);
    }

    const ObjectInfo& last = m_stack.last();
    if (!last.window) {
        return RetVal<bool>::make_ok(false);
    }

    return RetVal<bool>::make_ok(last.window != mainWindow()->qWindow());
}

async::Notification InteractiveProvider::currentUriAboutToBeChanged() const
{
    return m_currentUriAboutToBeChanged;
}

std::vector<Uri> InteractiveProvider::stack() const
{
    std::vector<Uri> uris;
    for (const ObjectInfo& info : m_stack) {
        uris.push_back(info.query.uri());
    }
    return uris;
}

QWindow* InteractiveProvider::topWindow() const
{
    QWindow* mainWin = mainWindow()->qWindow();

    if (m_stack.empty()) {
        LOGE() << "stack is empty";
        return mainWin;
    }

    const ObjectInfo& last = m_stack.last();
    if (!last.window) {
        return mainWin;
    }

    if (last.window == mainWin) {
        return mainWin;
    }

    // TODO/HACK: last.window doesn't seem to have a parent when the top window is a widget....
    if (!last.window->parent() && !topWindowIsWidget()) {
        ASSERT_X("Window must have a parent!");
    }

    return qobject_cast<QWindow*>(last.window);
}

bool InteractiveProvider::topWindowIsWidget() const
{
    if (m_stack.empty()) {
        return false;
    }

    const ObjectInfo& last = m_stack.last();
    if (!last.window) {
        return false;
    }

    return last.window->isWidgetType();
}

QString InteractiveProvider::objectId(const QVariant& val) const
{
    static int count(0);

    ++count;

    QString objectId;
    if (val.canConvert<QObject*>()) {
        QObject* obj = val.value<QObject*>();
        IF_ASSERT_FAILED(obj) {
            return QString();
        }

        objectId = QString(obj->metaObject()->className()) + "_" + QString::number(count);
    } else {
        objectId = "unknown_" + QString::number(count);
    }
    return "object://" + objectId;
}

Ret InteractiveProvider::toRet(const QVariant& jsr) const
{
    QVariantMap jsobj = jsr.toMap();
    IF_ASSERT_FAILED(jsobj.contains("errcode")) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret;
    ret.setCode(jsobj.value("errcode").toInt());
    return ret;
}

RetVal<Val> InteractiveProvider::toRetVal(const QVariant& jsrv) const
{
    RetVal<Val> rv;
    QVariantMap jsobj = jsrv.toMap();

    IF_ASSERT_FAILED(jsobj.contains("errcode")) {
        rv.ret = make_ret(Ret::Code::UnknownError);
        return rv;
    }

    int errcode = jsobj.value("errcode").toInt();
    QVariant val = jsobj.value("value");

    rv.ret.setCode(errcode);
    rv.val = Val::fromQVariant(val);

    return rv;
}

RetVal<InteractiveProvider::OpenData> InteractiveProvider::openExtensionDialog(const UriQuery& q, const QVariantMap& params)
{
    QmlLaunchData data;
    fillExtData(&data, q, params);

    emit fireOpen(&data);

    RetVal<OpenData> result;
    result.ret = toRet(data.value("ret"));
    result.val.objectId = data.value("objectId").toString();

    return result;
}

RetVal<InteractiveProvider::OpenData> InteractiveProvider::openWidgetDialog(const Uri& uri, const QVariantMap& params)
{
    RetVal<OpenData> result;

    ContainerMeta meta = uriRegister()->meta(uri);
    int widgetMetaTypeId = meta.widgetMetaTypeId;

    static int count(0);
    QString objectId = QString("%1_%2").arg(widgetMetaTypeId).arg(++count);

    QMetaType metaType = QMetaType(widgetMetaTypeId);
    QDialog* dialog = static_cast<QDialog*>(metaType.create());

    if (!dialog) {
        result.ret = make_ret(Ret::Code::UnknownError);
        return result;
    }

    fillData(dialog, params);

    //! NOTE Will be deleted with the dialog
    WidgetDialogAdapter* adapter = new WidgetDialogAdapter(dialog);
    adapter->onShow([this, objectId, dialog]() {
        async::Async::call(this, [this, objectId, dialog]() {
            onOpen(ContainerType::QWidgetDialog, objectId, dialog->window());
        });
    })
    .onHide([this, objectId, dialog]() {
        QDialog::DialogCode dialogCode = static_cast<QDialog::DialogCode>(dialog->result());
        Ret::Code errorCode = dialogCode == QDialog::Accepted ? Ret::Code::Ok : Ret::Code::Cancel;

        QVariantMap ret;
        ret["errcode"] = static_cast<int>(errorCode);

        onClose(objectId, ret);

        dialog->deleteLater();
    });

    bool modal = params.value("modal", "true") == "true";
    dialog->setWindowModality(modal ? Qt::ApplicationModal : Qt::NonModal);
    dialog->show();
    dialog->activateWindow();     // give keyboard focus to aid blind users

    result.ret = make_ret(Ret::Code::Ok);
    result.val.objectId = objectId;

    return result;
}

RetVal<InteractiveProvider::OpenData> InteractiveProvider::openQml(const Uri& uri, const QVariantMap& params)
{
    QmlLaunchData data;
    fillData(&data, uri, params);

    emit fireOpen(&data);

    RetVal<OpenData> result;
    result.ret = toRet(data.value("ret"));
    result.val.objectId = data.value("objectId").toString();

    return result;
}

void InteractiveProvider::closeQml(const QVariant& objectId)
{
    emit fireClose(objectId);
}

void InteractiveProvider::raiseQml(const QVariant& objectId)
{
    emit fireRaise(objectId);
}

void InteractiveProvider::onOpen(const QVariant& type, const QVariant& objectId, QObject* window)
{
    ContainerType::Type containerType = type.value<ContainerType::Type>();

    IF_ASSERT_FAILED(containerType != ContainerType::Undefined) {
        containerType = ContainerType::QmlDialog;
    }

    m_openingObject.objectId = objectId;
    m_openingObject.window = window;
    if (!m_openingObject.window) {
        m_openingObject.window = (containerType == ContainerType::PrimaryPage) ? mainWindow()->qWindow() : qApp->focusWindow();
    }

    if (m_openingObject.query.param("floating").toBool()) {
        m_floatingObjects.push_back(m_openingObject);
        m_openingObject = ObjectInfo(); // clear
        return;
    }

    if (ContainerType::PrimaryPage == containerType) {
        m_stack.clear();
        m_stack.push(m_openingObject);
    } else if (ContainerType::QmlDialog == containerType) {
        m_stack.push(m_openingObject);
    } else if (ContainerType::QWidgetDialog == containerType) {
        m_stack.push(m_openingObject);
    } else {
        IF_ASSERT_FAILED_X(false, "unknown page type") {
            m_stack.push(m_openingObject);
        }
    }

    notifyAboutCurrentUriChanged();

    Uri uri = m_openingObject.query.uri();
    m_openingObject = ObjectInfo();     // clear

    Async::call(this, [this, uri]() {
        m_opened.send(uri);
    });
}

void InteractiveProvider::onClose(const QString& objectId, const QVariant& jsrv)
{
    RetVal<Val> rv = toRetVal(jsrv);

    ObjectInfo obj;

    bool inStack = false;
    for (int i = 0; i < m_stack.size(); ++i) {
        if (m_stack.at(i).objectId == objectId) {
            obj = m_stack.at(i);
            inStack = true;
            m_stack.remove(i);
            break;
        }
    }

    if (!inStack) {
        for (size_t i = 0; i < m_floatingObjects.size(); ++i) {
            if (m_floatingObjects.at(i).objectId == objectId) {
                obj = m_floatingObjects.at(i);
                m_floatingObjects.erase(m_floatingObjects.begin() + i);
                break;
            }
        }
    }

    DO_ASSERT(obj.objectId.isValid());

    if (rv.ret) {
        (void)obj.resolve(rv.val);
    } else {
        (void)obj.reject(rv.ret.code(), rv.ret.text());
    }

    if (inStack) {
        notifyAboutCurrentUriChanged();
    }
}

std::vector<InteractiveProvider::ObjectInfo> InteractiveProvider::allOpenObjects() const
{
    std::vector<ObjectInfo> result;

    result.insert(result.end(), m_stack.cbegin(), m_stack.cend());
    result.insert(result.end(), m_floatingObjects.cbegin(), m_floatingObjects.cend());

    return result;
}

void InteractiveProvider::notifyAboutCurrentUriChanged()
{
    m_currentUriChanged.send(currentUri().val);
}

void InteractiveProvider::notifyAboutCurrentUriWillBeChanged()
{
    m_currentUriAboutToBeChanged.notify();
}

// === QmlLaunchData ===
QmlLaunchData::QmlLaunchData(QObject* parent)
    : QObject(parent)
{
}

QVariant QmlLaunchData::value(const QString& key) const
{
    return m_data.value(key);
}

void QmlLaunchData::setValue(const QString& key, const QVariant& val)
{
    m_data[key] = val;
}

QVariant QmlLaunchData::data() const
{
    return m_data;
}
