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
#ifndef MU_FRAMEWORK_QMLDIALOG_H
#define MU_FRAMEWORK_QMLDIALOG_H

#include <QQuickItem>
#include <QQmlComponent>
#include <QQuickView>
#include <QJSValue>

#include "ret.h"

class QDialog;

namespace mu {
namespace framework {
class QmlDialog : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent * content READ content WRITE setContent NOTIFY contentChanged)

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString objectID READ objectID WRITE setObjectID NOTIFY objectIDChanged)
    Q_PROPERTY(bool modal READ modal WRITE setModal NOTIFY modalChanged)

    Q_PROPERTY(QVariantMap ret READ ret WRITE setRet NOTIFY retChanged)

    Q_CLASSINFO("DefaultProperty", "content")

public:
    QmlDialog(QQuickItem* parent = nullptr);

    QQmlComponent* content() const;
    QString title() const;
    QString objectID() const;
    bool modal() const;
    QVariantMap ret() const;

    Q_INVOKABLE void exec();
    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void reject();

public slots:
    void setContent(QQmlComponent* component);
    void setTitle(QString title);
    void setObjectID(QString objectID);
    void setModal(bool modal);
    void setRet(QVariantMap ret);

signals:
    void contentChanged();
    void titleChanged(QString title);
    void objectIDChanged(QString objectID);
    void modalChanged(bool modal);
    void retChanged(QVariantMap ret);

    void closed();

private:
    void setErrCode(Ret::Code code);

    void componentComplete() override;

    QQuickView* m_view = nullptr;
    QQmlComponent* m_content = nullptr;
    QDialog* m_dialog = nullptr;
    QString m_objectID;
    QVariantMap m_ret;
};
}
}

#endif // MU_FRAMEWORK_QMLDIALOG_H
