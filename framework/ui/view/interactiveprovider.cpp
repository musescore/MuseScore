//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "interactiveprovider.h"
#include "log.h"

#include <QMetaType>
#include <QMetaProperty>

using namespace mu::framework;
using namespace mu;

static const QString PAGE_TYPE_DOCK("dock");
static const QString PAGE_TYPE_POPUP("popup");
static const QString PAGE_TYPE_WIDGET("widget");

InteractiveProvider::InteractiveProvider()
    : QObject()
{
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

    object->setParent(mainWindow()->qMainWindow());
}

ValCh<Uri> InteractiveProvider::currentUri() const
{
    ValCh<Uri> v;
    if (!m_stack.empty()) {
        v.val = m_stack.last().uri();
    }
    v.ch = m_currentUriChanged;
    return v;
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

        objectID = QString(obj->metaObject()->className()) + "_" + QString::number(count);
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

    connect(dialog, &QDialog::finished, [this, objectId, dialog](int finishStatus) {
        QVariantMap status;
        status["errcode"] = static_cast<int>(Ret::Code::Ok);
        status["value"] = finishStatus;

        onPopupClose(objectId, status);
        dialog->deleteLater();
    });

    onOpen(ContainerType::QWidgetDialog);

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

void InteractiveProvider::onOpen(const QVariant& type)
{
    ContainerType::Type containerType = type.value<ContainerType::Type>();

    IF_ASSERT_FAILED(!(containerType == ContainerType::Undefined)) {
        containerType = ContainerType::QmlDialog;
    }

    if (ContainerType::PrimaryPage == containerType) {
        m_stack.clear();
        m_stack.push(m_openingUriQuery);
    } else if (ContainerType::QmlDialog == containerType) {
        m_stack.push(m_openingUriQuery);
    } else if (ContainerType::QWidgetDialog == containerType) {
        m_stack.push(m_openingUriQuery);
    } else {
        IF_ASSERT_FAILED_X(false, "unknown page type") {
            m_stack.push(m_openingUriQuery);
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
