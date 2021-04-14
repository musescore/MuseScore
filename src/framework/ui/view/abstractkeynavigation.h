//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_UI_ABSTRACTKEYNAVIGATION_H
#define MU_UI_ABSTRACTKEYNAVIGATION_H

#include <QObject>
#include <QQmlParserStatus>

#include "../ikeynavigation.h"

namespace mu::ui {
class AbstractKeyNavigation : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    // see IKeyNavigation::Index
    Q_PROPERTY(int order READ order WRITE setOrder NOTIFY orderChanged)
    Q_PROPERTY(int column READ column WRITE setColumn NOTIFY columnChanged)
    Q_PROPERTY(int row READ row WRITE setRow NOTIFY rowChanged)

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    explicit AbstractKeyNavigation(QObject* parent = nullptr);

    int order() const;
    int column() const;
    int row() const;

    QString name() const;

    const IKeyNavigation::Index& index() const;
    async::Channel<IKeyNavigation::Index> indexChanged() const;

    bool enabled() const;
    async::Channel<bool> enabledChanged() const;

    bool active() const;
    async::Channel<bool> activeChanged() const;

    // QQmlParserStatus
    void classBegin() override;
    void componentComplete() override;

public slots:
    void setName(QString name);
    void setOrder(int order);
    void setColumn(int column);
    void setRow(int row);
    void setEnabled(bool enabled);
    void setActive(bool active);

signals:
    void nameChanged(QString name);
    void orderChanged(int order);
    void columnChanged(int column);
    void rowChanged(int row);
    void enabledChanged(bool enabled);
    void activeChanged(bool active);

protected:

    QString m_name;
    IKeyNavigation::Index m_index;
    async::Channel<IKeyNavigation::Index> m_indexChanged;

    bool m_enabled = true;
    async::Channel<bool> m_enabledChanged;

    bool m_active = false;
    async::Channel<bool> m_activeChanged;
};
}

#endif // MU_UI_ABSTRACTKEYNAVIGATION_H
