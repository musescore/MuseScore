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
#include <QWindow>

#include "widgetdialog.h"

using namespace mu;
using namespace mu::ui;
using namespace mu::framework;

static const QString PAGE_TYPE_DOCK("dock");
static const QString PAGE_TYPE_POPUP("popup");
static const QString PAGE_TYPE_WIDGET("widget");

InteractiveProvider::InteractiveProvider()
    : QObject()
{
}

RetVal<Val> InteractiveProvider::question(const std::string& title, const framework::IInteractive::Text& text,
                                          const framework::IInteractive::ButtonDatas& buttons, int defBtn,
                                          const framework::IInteractive::Options& options)
{
    return openStandardDialog("QUESTION", QString::fromStdString(title), text, buttons, defBtn, options);
}

RetVal<Val> InteractiveProvider::info(const std::string& title, const std::string& text, const IInteractive::ButtonDatas& buttons,
                                      int defBtn,
                                      const framework::IInteractive::Options& options)
{
    return openStandardDialog("INFO", QString::fromStdString(title), text, buttons, defBtn, options);
}

RetVal<Val> InteractiveProvider::warning(const std::string& title, const std::string& text, const IInteractive::ButtonDatas& buttons,
                                         int defBtn,
                                         const framework::IInteractive::Options& options)
{
    return openStandardDialog("WARNING", QString::fromStdString(title), text, buttons, defBtn, options);
}

RetVal<Val> InteractiveProvider::error(const std::string& title, const std::string& text, const IInteractive::ButtonDatas& buttons,
                                       int defBtn,
                                       const framework::IInteractive::Options& options)
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
    if (openedRet.val.sync && !openedRet.val.objectID.isEmpty()) {
        RetVal<Val> rv = m_retvals.take(openedRet.val.objectID);
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

void InteractiveProvider::close(const Uri& uri)
{
    for (const ObjectInfo& objectInfo: m_stack) {
        if (objectInfo.uriQuery.uri() != uri) {
            continue;
        }

        ContainerMeta openMeta = uriRegister()->meta(objectInfo.uriQuery.uri());
        switch (openMeta.type) {
        case ContainerType::QWidgetDialog:
            closeWidgetDialog(objectInfo.objectId);
            break;
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

void InteractiveProvider::fillStandatdDialogData(QmlLaunchData* data, const QString& type, const QString& title,
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

    if (options.testFlag(framework::IInteractive::Option::WithIcon)) {
        params["withIcon"] = true;
    }

    if (options.testFlag(framework::IInteractive::Option::WithShowAgain)) {
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

QString InteractiveProvider::objectID(const QVariant& val) const
{
    static int count(0);

    ++count;

    QString objectID;
    if (val.canConvert<QObject*>()) {
        QObject* obj = val.value<QObject*>();
        IF_ASSERT_FAILED(obj) {
            return QString();
        }

        objectID = QString(obj->metaObject()->className());
    } else {
        objectID = "unknown_" + QString::number(count);
    }
    return "object://" + objectID;
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

    QString objectId = QString::number(widgetMetaTypeId);

    void* widgetClassPtr = QMetaType::create(widgetMetaTypeId);
    QDialog* dialog = static_cast<QDialog*>(widgetClassPtr);

    if (!dialog) {
        result.ret = make_ret(Ret::Code::UnknownError);
        return result;
    }

    fillData(dialog, q);

    if (dialog->result()) {
        QMetaType::destroy(widgetMetaTypeId, widgetClassPtr);
        result.ret = make_ret(Ret::Code::Cancel);
        return result;
    }

    connect(dialog, &QDialog::finished, [this, objectId, dialog](int code) {
        QVariantMap status;

        QDialog::DialogCode dialogCode = static_cast<QDialog::DialogCode>(code);
        switch (dialogCode) {
        case QDialog::Rejected: {
            status["errcode"] = static_cast<int>(Ret::Code::Cancel);
            break;
        }
        case QDialog::Accepted: {
            status["errcode"] = static_cast<int>(Ret::Code::Ok);
            break;
        }
        }

        onPopupClose(objectId, status);
        dialog->deleteLater();
    });

    onOpen(ContainerType::QWidgetDialog, widgetMetaTypeId);

    bool sync = q.param("sync", Val(false)).toBool();
    if (sync) {
        dialog->exec();
    } else {
        dialog->show();
    }

    result.ret = make_ret(Ret::Code::Ok);
    result.val.sync = sync;
    result.val.objectID = QString::number(widgetMetaTypeId);
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
    result.val.objectID = data->value("objectID").toString();

    delete data;

    return result;
}

RetVal<Val> InteractiveProvider::openStandardDialog(const QString& type, const QString& title, const framework::IInteractive::Text& text,
                                                    const framework::IInteractive::ButtonDatas& buttons, int defBtn,
                                                    const framework::IInteractive::Options& options)
{
    QmlLaunchData* data = new QmlLaunchData();
    fillStandatdDialogData(data, type, title, text, buttons, defBtn, options);

    emit fireOpenStandardDialog(data);

    Ret ret = toRet(data->value("ret"));
    QString objectID = data->value("objectID").toString();

    delete data;

    RetVal<Val> result;
    if (!ret) {
        result.ret = ret;
        return result;
    }

    if (!objectID.isEmpty()) {
        RetVal<Val> rv = m_retvals.take(objectID);
        if (rv.ret.valid()) {
            result = rv;
        }
    }

    return result;
}

void InteractiveProvider::closeWidgetDialog(const QVariant& dialogMetaTypeId)
{
    const QWindow* window = mainWindow()->qWindow();
    if (!window) {
        return;
    }

    int _dialogMetaTypeId = dialogMetaTypeId.toInt();

    for (QObject* object : window->children()) {
        WidgetDialog* dialog = dynamic_cast<WidgetDialog*>(object);
        if (dialog && dialog->metaTypeId() == _dialogMetaTypeId) {
            dialog->close();
        }
    }
}

void InteractiveProvider::closeQml(const QVariant& objectID)
{
    emit fireClose(objectID);
}

void InteractiveProvider::onOpen(const QVariant& type, const QVariant& objectId)
{
    ContainerType::Type containerType = type.value<ContainerType::Type>();

    IF_ASSERT_FAILED(!(containerType == ContainerType::Undefined)) {
        containerType = ContainerType::QmlDialog;
    }

    ObjectInfo objectInfo;
    objectInfo.uriQuery = m_openingUriQuery;
    objectInfo.objectId = objectId;

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

    m_currentUriChanged.send(currentUri().val);
    m_openingUriQuery = UriQuery();
}

void InteractiveProvider::onPopupClose(const QString& objectID, const QVariant& jsrv)
{
    m_retvals[objectID] = toRetVal(jsrv);

    IF_ASSERT_FAILED(m_stack.size() > 1) {
        return;
    }

    m_stack.pop();
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
