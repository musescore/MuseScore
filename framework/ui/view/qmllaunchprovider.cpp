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
#include "qmllaunchprovider.h"
#include "log.h"

using namespace mu::framework;
using namespace mu;

static const QString PAGE_TYPE_DOCK("dock");
static const QString PAGE_TYPE_POPUP("popup");
static const QString PAGE_TYPE_WIDGET("widget");

QmlLaunchProvider::QmlLaunchProvider()
    : QObject()
{
}

RetVal<Val> QmlLaunchProvider::open(const UriQuery& q)
{
    m_openingUriQuery = q;

    UriType openType = uriRegister()->uriType(QString::fromStdString(q.uri().toString()));
    RetVal<OpenData> openedRet;
    switch (openType) {
    case UriType::Widget:
        openedRet = openWidget(q);
        break;
    default: // TODO remove after register qml uri through uriRegister
//    case UriType::Qml:
        openedRet = openQml(q);
        break;
//    case UriType::Undefined:
//        openedRet.ret = make_ret(Ret::Code::UnknownError);
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

void QmlLaunchProvider::fillData(QmlLaunchData* data, const UriQuery& q) const
{
    data->setValue("uri", QString::fromStdString(q.uri().toString()));

    QVariantMap params;
    const UriQuery::Params& p = q.params();
    for (auto it = p.cbegin(); it != p.cend(); ++it) {
        params[QString::fromStdString(it->first)] = it->second.toQVariant();
    }

    data->setValue("params", params);
    data->setValue("sync", params.value("sync", false));
}

ValCh<Uri> QmlLaunchProvider::currentUri() const
{
    ValCh<Uri> v;
    if (!m_stack.empty()) {
        v.val = m_stack.last().uri();
    }
    v.ch = m_currentUriChanged;
    return v;
}

QString QmlLaunchProvider::objectID(const QVariant& val) const
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

Ret QmlLaunchProvider::toRet(const QVariant& jsr) const
{
    QVariantMap jsobj = jsr.toMap();
    IF_ASSERT_FAILED(jsobj.contains("errcode")) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret;
    ret.setCode(jsobj.value("errcode").toInt());
    return ret;
}

RetVal<Val> QmlLaunchProvider::toRetVal(const QVariant& jsrv) const
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

RetVal<QmlLaunchProvider::OpenData> QmlLaunchProvider::openWidget(const UriQuery &q)
{
    RetVal<OpenData> result;

    QString uri = QString::fromStdString(q.uri().toString());

    int widgetMetaTypeId = uriRegister()->widgetDialogMetaTypeId(uri);
    QString objectId = QString::number(widgetMetaTypeId); // unque?

    void *widgetClassPtr = QMetaType::create(widgetMetaTypeId);
    QDialog* dialog = static_cast<QDialog*>(widgetClassPtr);

    if (!dialog) {
        result.ret = make_ret(Ret::Code::UnknownError);
        return result;
    }

    QVariantMap status;
    connect(dialog, &QDialog::finished, [=, &status, &objectId](int finishStatus){
        status["errcode"] = static_cast<int>(Ret::Code::Ok);
        status["value"] = finishStatus;

        onPopupClose(objectId, status);
    });

    onOpen(PAGE_TYPE_WIDGET);

    dialog->exec();

    QMetaType::destroy(widgetMetaTypeId, widgetClassPtr);

    result.ret = toRet(status);
    result.val.sync = false; // TODO
    result.val.objectID = QString::number(widgetMetaTypeId);

    return result;
}

RetVal<QmlLaunchProvider::OpenData> QmlLaunchProvider::openQml(const UriQuery &q)
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

void QmlLaunchProvider::onOpen(QString pageType)
{
    IF_ASSERT_FAILED(!pageType.isEmpty()) {
        pageType = PAGE_TYPE_POPUP;
    }

    if (PAGE_TYPE_DOCK == pageType) {
        m_stack.clear();
        m_stack.push(m_openingUriQuery);
    } else if (PAGE_TYPE_POPUP == pageType) {
        m_stack.push(m_openingUriQuery);
    } else if (PAGE_TYPE_WIDGET == pageType) {
        m_stack.push(m_openingUriQuery);
    } else {
        IF_ASSERT_FAILED_X(false, "unknown page type") {
            m_stack.push(m_openingUriQuery);
        }
    }

    m_currentUriChanged.send(currentUri().val);
    m_openingUriQuery = UriQuery();
}

void QmlLaunchProvider::onPopupClose(const QString& objectID, const QVariant& jsrv)
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
