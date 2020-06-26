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

QmlLaunchProvider::QmlLaunchProvider(QObject* parent)
    : QObject(parent)
{
}

RetVal<Val> QmlLaunchProvider::open(const UriQuery& q)
{
    QmlLaunchData* data = new QmlLaunchData();
    fillData(data, q);

    emit fireOpen(data);

    RetVal<Val> rv = toRetVal(data->value("ret"));

    QString pageType = data->value("page_type").toString();
    IF_ASSERT_FAILED(!pageType.isEmpty()) {
        pageType = PAGE_TYPE_POPUP;
    }

    if (PAGE_TYPE_DOCK == pageType) {
        m_stack.clear();
        m_stack.push(q);
    } else if (PAGE_TYPE_POPUP == pageType) {
        m_stack.push(q);
    } else {
        IF_ASSERT_FAILED_X(false, "unknown page type") {
            m_stack.push(q);
        }
    }

    delete data;

    return rv;
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

Uri QmlLaunchProvider::currentUri() const
{
    return m_stack.last().uri();
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

RetVal<Val> QmlLaunchProvider::toRetVal(const QVariant& jsrv) const
{
    RetVal<Val> rv;
    QVariantMap jsobj = jsrv.toMap();

    IF_ASSERT_FAILED(jsobj.contains("errcode")) {
        rv.ret = make_ret(Ret::Code::Error);
        return rv;
    }

    int errcode = jsobj.value("errcode").toInt();
    QVariant val = jsobj.value("value");

    rv.ret.setCode(errcode);
    rv.val = Val::fromQVariant(val);

    return rv;
}

void QmlLaunchProvider::onClose(const QString& objectID, const QVariant& jsrv)
{
    m_retvals[objectID] = toRetVal(jsrv);
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
