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
#include "log.h"

#include <QMetaType>
#include <QMetaProperty>
#include <QDialog>
#include <QGuiApplication>
#include <QWindow>

using namespace mu;
using namespace mu::ui;
using namespace mu::framework;

class WidgetDialogEventFilter : public QObject
{
public:
    WidgetDialogEventFilter(QObject* parent, const std::function<void()>& onShownCallBack,
                            const std::function<void()>& onHideCallBack)
        : QObject(parent), m_onShownCallBack(onShownCallBack), m_onHideCallBack(onHideCallBack)
    {
    }

private:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::Show && m_onShownCallBack) {
            m_onShownCallBack();
        }

        if (event->type() == QEvent::Hide && m_onHideCallBack) {
            m_onHideCallBack();
        }

        return QObject::eventFilter(watched, event);
    }

    std::function<void()> m_onShownCallBack;
    std::function<void()> m_onHideCallBack;
};

InteractiveProvider::InteractiveProvider()
    : QObject()
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
        if (m_stack[i].window->isWidgetType()) {
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

RetVal<Val> InteractiveProvider::question(const std::string& title, const IInteractive::Text& text,
                                          const IInteractive::ButtonDatas& buttons, int defBtn,
                                          const IInteractive::Options& options)
{
    return openStandardDialog("QUESTION", QString::fromStdString(title), text, buttons, defBtn, options);
}

RetVal<Val> InteractiveProvider::info(const std::string& title, const std::string& text, const IInteractive::ButtonDatas& buttons,
                                      int defBtn,
                                      const IInteractive::Options& options)
{
    return openStandardDialog("INFO", QString::fromStdString(title), text, buttons, defBtn, options);
}

RetVal<Val> InteractiveProvider::warning(const std::string& title, const std::string& text, const IInteractive::ButtonDatas& buttons,
                                         int defBtn,
                                         const IInteractive::Options& options)
{
    return openStandardDialog("WARNING", QString::fromStdString(title), text, buttons, defBtn, options);
}

RetVal<Val> InteractiveProvider::error(const std::string& title, const std::string& text, const IInteractive::ButtonDatas& buttons,
                                       int defBtn,
                                       const IInteractive::Options& options)
{
    return openStandardDialog("ERROR", QString::fromStdString(title), text, buttons, defBtn, options);
}

RetVal<Val> InteractiveProvider::open(const UriQuery& q)
{
    m_openingUriQuery = q;

    ContainerMeta openMeta = uriRegister()->meta(q.uri());
    RetVal<OpenData> openedRet;
    switch (openMeta.type) {
    case ContainerType::QWidgetDialog:
        openedRet = openWidgetDialog(q);
        break;
    case ContainerType::PrimaryPage:
    case ContainerType::QmlDialog:
        openedRet = openQml(q);
        break;
    case ContainerType::Undefined:
        openedRet.ret = make_ret(Ret::Code::UnknownError);
    }

    if (!openedRet.ret) {
        LOGE() << "failed open err: " << openedRet.ret.toString() << ", uri: " << q.toString();
        return RetVal<Val>(openedRet.ret);
    }

    RetVal<Val> returnedRV;
    returnedRV.ret = make_ret(Ret::Code::Ok);
    if (openedRet.val.sync && !openedRet.val.objectId.isEmpty()) {
        RetVal<Val> rv = m_retvals.take(openedRet.val.objectId);
        if (rv.ret.valid()) {
            returnedRV = rv;
        }
    }

    return returnedRV;
}

RetVal<bool> InteractiveProvider::isOpened(const Uri& uri) const
{
    for (const ObjectInfo& objectInfo: m_stack) {
        if (objectInfo.uriQuery.uri() == uri) {
            return RetVal<bool>::make_ok(true);
        }
    }

    return RetVal<bool>::make_ok(false);
}

RetVal<bool> InteractiveProvider::isOpened(const UriQuery& uri) const
{
    for (const ObjectInfo& objectInfo: m_stack) {
        if (objectInfo.uriQuery == uri) {
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
    for (const ObjectInfo& objectInfo: m_stack) {
        if (objectInfo.uriQuery != uri) {
            continue;
        }

        ContainerMeta openMeta = uriRegister()->meta(objectInfo.uriQuery.uri());
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
    for (const ObjectInfo& objectInfo : m_stack) {
        if (objectInfo.uriQuery.uri() != uri) {
            continue;
        }

        ContainerMeta openMeta = uriRegister()->meta(objectInfo.uriQuery.uri());
        switch (openMeta.type) {
        case ContainerType::QWidgetDialog: {
            if (auto window = dynamic_cast<QWidget*>(objectInfo.window)) {
                window->close();
            }
        } break;
        case ContainerType::QmlDialog:
            closeQml(objectInfo.objectId);
            break;
        case ContainerType::PrimaryPage:
        case ContainerType::Undefined:
            break;
        }
    }
}

void InteractiveProvider::fillData(QmlLaunchData* data, const UriQuery& q) const
{
    ContainerMeta meta = uriRegister()->meta(q.uri());
    data->setValue("path", meta.qmlPath);
    data->setValue("type", meta.type);
    data->setValue("uri", QString::fromStdString(q.uri().toString()));

    QVariantMap params;
    const UriQuery::Params& p = q.params();
    for (auto it = p.cbegin(); it != p.cend(); ++it) {
        params[QString::fromStdString(it->first)] = it->second.toQVariant();
    }

    data->setValue("params", params);
    data->setValue("sync", params.value("sync", false));
    data->setValue("modal", params.value("modal", ""));
}

void InteractiveProvider::fillData(QObject* object, const UriQuery& q) const
{
    QVariantMap params;
    const UriQuery::Params& p = q.params();
    for (auto it = p.cbegin(); it != p.cend(); ++it) {
        params[QString::fromStdString(it->first)] = it->second.toQVariant();
    }

    const QMetaObject* metaObject = object->metaObject();
    for (int i = 0; i < metaObject->propertyCount(); i++) {
        QMetaProperty metaProperty = metaObject->property(i);
        if (params.contains(metaProperty.name())) {
            object->setProperty(metaProperty.name(), params[metaProperty.name()]);
        }
    }
}

void InteractiveProvider::fillStandardDialogData(QmlLaunchData* data, const QString& type, const QString& title,
                                                 const IInteractive::Text& text, const IInteractive::ButtonDatas& buttons, int defBtn,
                                                 const IInteractive::Options& options) const
{
    auto format = [](IInteractive::TextFormat f) {
        switch (f) {
        case IInteractive::TextFormat::PlainText: return Qt::PlainText;
        case IInteractive::TextFormat::RichText:  return Qt::RichText;
        }
        return Qt::PlainText;
    };

    QVariantMap params;
    params["type"] = type;
    params["title"] = title;
    params["text"] = QString::fromStdString(text.text);
    params["textFormat"] = format(text.format);
    params["defaultButtonId"] = defBtn;

    QVariantList buttonList;
    for (const IInteractive::ButtonData& buttonData: buttons) {
        QVariantMap buttonObj;
        buttonObj["buttonId"] = QVariant::fromValue(buttonData.btn);
        buttonObj["title"] = QVariant::fromValue(QString::fromStdString(buttonData.text));
        buttonObj["accent"] = QVariant::fromValue(buttonData.accent);

        buttonList << buttonObj;
    }

    if (!buttonList.empty()) {
        params["buttons"] = buttonList;
    }

    if (options.testFlag(IInteractive::Option::WithIcon)) {
        params["withIcon"] = true;
    }

    if (options.testFlag(IInteractive::Option::WithShowAgain)) {
        params["withShowAgain"] = true;
    }

    data->setValue("params", params);
}

ValCh<Uri> InteractiveProvider::currentUri() const
{
    ValCh<Uri> v;
    if (!m_stack.empty()) {
        v.val = m_stack.last().uriQuery.uri();
    }
    v.ch = m_currentUriChanged;
    return v;
}

std::vector<Uri> InteractiveProvider::stack() const
{
    std::vector<Uri> uris;
    for (const ObjectInfo& info : m_stack) {
        uris.push_back(info.uriQuery.uri());
    }
    return uris;
}

QWindow* InteractiveProvider::topWindow() const
{
    if (m_stack.empty()) {
        LOGE() << "stack is empty";
        return mainWindow()->qWindow();
    }

    const ObjectInfo& last = m_stack.last();
    if (!last.window) {
        return mainWindow()->qWindow();
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

        objectId = QString(obj->metaObject()->className());
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

RetVal<InteractiveProvider::OpenData> InteractiveProvider::openWidgetDialog(const UriQuery& q)
{
    RetVal<OpenData> result;

    ContainerMeta meta = uriRegister()->meta(q.uri());
    int widgetMetaTypeId = meta.widgetMetaTypeId;

    static int count(0);
    QString objectId = QString("%1_%2").arg(widgetMetaTypeId).arg(++count);

    void* widgetClassPtr = QMetaType::create(widgetMetaTypeId);
    QDialog* dialog = static_cast<QDialog*>(widgetClassPtr);

    if (!dialog) {
        result.ret = make_ret(Ret::Code::UnknownError);
        return result;
    }

    fillData(dialog, q);

    dialog->installEventFilter(new WidgetDialogEventFilter(dialog,
                                                           [this, dialog, objectId]() {
        if (dialog) {
            onOpen(ContainerType::QWidgetDialog, objectId, dialog->window());
        }
    },
                                                           [this, dialog, objectId]() {
        if (!dialog) {
            return;
        }

        QVariantMap status;

        QDialog::DialogCode dialogCode = static_cast<QDialog::DialogCode>(dialog->result());
        Ret::Code errorCode = dialogCode == QDialog::Accepted ? Ret::Code::Ok : Ret::Code::Cancel;
        status["errcode"] = static_cast<int>(errorCode);

        onClose(objectId, status);
        dialog->deleteLater();
    }));

    bool sync = q.param("sync", Val(false)).toBool();
    if (sync) {
        dialog->exec();
    } else {
        dialog->show();
    }

    result.ret = make_ret(Ret::Code::Ok);
    result.val.sync = sync;
    result.val.objectId = objectId;

    return result;
}

RetVal<InteractiveProvider::OpenData> InteractiveProvider::openQml(const UriQuery& q)
{
    QmlLaunchData* data = new QmlLaunchData();
    fillData(data, q);

    emit fireOpen(data);

    RetVal<OpenData> result;
    result.ret = toRet(data->value("ret"));
    result.val.sync = data->value("sync").toBool();
    result.val.objectId = data->value("objectId").toString();

    delete data;

    return result;
}

RetVal<Val> InteractiveProvider::openStandardDialog(const QString& type, const QString& title, const IInteractive::Text& text,
                                                    const IInteractive::ButtonDatas& buttons, int defBtn,
                                                    const IInteractive::Options& options)
{
    QmlLaunchData* data = new QmlLaunchData();
    fillStandardDialogData(data, type, title, text, buttons, defBtn, options);

    emit fireOpenStandardDialog(data);

    Ret ret = toRet(data->value("ret"));
    QString objectId = data->value("objectId").toString();

    delete data;

    RetVal<Val> result;
    if (!ret) {
        result.ret = ret;
        return result;
    }

    if (!objectId.isEmpty()) {
        RetVal<Val> rv = m_retvals.take(objectId);
        if (rv.ret.valid()) {
            result = rv;
        }
    }

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

    if (m_openingUriQuery.param("floating").toBool()) {
        m_openingUriQuery = UriQuery();
        return;
    }

    ObjectInfo objectInfo;
    objectInfo.uriQuery = m_openingUriQuery;
    objectInfo.objectId = objectId;
    objectInfo.window = window;
    if (!objectInfo.window) {
        objectInfo.window = (containerType == ContainerType::PrimaryPage) ? mainWindow()->qWindow() : qApp->focusWindow();
    }

    if (ContainerType::PrimaryPage == containerType) {
        m_stack.clear();
        m_stack.push(objectInfo);
    } else if (ContainerType::QmlDialog == containerType) {
        m_stack.push(objectInfo);
    } else if (ContainerType::QWidgetDialog == containerType) {
        m_stack.push(objectInfo);
    } else {
        IF_ASSERT_FAILED_X(false, "unknown page type") {
            m_stack.push(objectInfo);
        }
    }

    notifyAboutCurrentUriChanged();
    m_opened.send(m_openingUriQuery.uri());

    m_openingUriQuery = UriQuery();
}

void InteractiveProvider::onClose(const QString& objectId, const QVariant& jsrv)
{
    m_retvals[objectId] = toRetVal(jsrv);

    bool found = false;
    for (int i = 0; i < m_stack.size(); ++i) {
        if (m_stack[i].objectId == objectId) {
            m_stack.remove(i);
            found = true;
            break;
        }
    }

    //! NOTE We may not find an object in the stack if it's,
    //! for example, a floating dialog (usually diagnostic dialogs)
    if (found) {
        notifyAboutCurrentUriChanged();
    }
}

void InteractiveProvider::notifyAboutCurrentUriChanged()
{
    m_currentUriChanged.send(currentUri().val);
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
