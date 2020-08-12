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
#ifndef MU_FRAMEWORK_INTERACTIVEPROVIDER_H
#define MU_FRAMEWORK_INTERACTIVEPROVIDER_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QStack>

#include "modularity/ioc.h"
#include "../iinteractiveprovider.h"
#include "../iinteractiveuriregister.h"
#include "../imainwindow.h"
#include "retval.h"

namespace mu {
namespace framework {
class QmlLaunchData : public QObject
{
    Q_OBJECT
public:
    explicit QmlLaunchData(QObject* parent = nullptr);

    Q_INVOKABLE QVariant value(const QString& key) const;
    Q_INVOKABLE void setValue(const QString& key, const QVariant& val);
    Q_INVOKABLE QVariant data() const;

private:
    QVariantMap m_data;
};

class InteractiveProvider : public QObject, public IInteractiveProvider
{
    Q_OBJECT

    INJECT(ui, IInteractiveUriRegister, uriRegister)
    INJECT(ui, IMainWindow, mainWindow)

public:
    explicit InteractiveProvider();

    RetVal<Val> open(const UriQuery& uri) override;
    ValCh<Uri> currentUri() const override;

    Q_INVOKABLE QString objectID(const QVariant& val) const;

    Q_INVOKABLE void onOpen(const QVariant& type);
    Q_INVOKABLE void onPopupClose(const QString& objectID, const QVariant& rv);

signals:
    void fireOpen(QmlLaunchData* data);

private:
    struct OpenData
    {
        bool sync = false;
        QString objectID;
    };

    void fillData(QmlLaunchData* data, const UriQuery& q) const;
    void fillData(QObject* object, const UriQuery& q) const;
    Ret toRet(const QVariant& jsr) const;
    RetVal<Val> toRetVal(const QVariant& jsrv) const;

    RetVal<OpenData> openWidgetDialog(const UriQuery& q);
    RetVal<OpenData> openQml(const UriQuery& q);

    UriQuery m_openingUriQuery;
    QStack<UriQuery> m_stack;
    async::Channel<Uri> m_currentUriChanged;
    QMap<QString, RetVal<Val> > m_retvals;
};
}
}

#endif // MU_FRAMEWORK_INTERACTIVEPROVIDER_H
