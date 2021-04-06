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
#ifndef MU_UI_KEYNAVIGATIONSECTION_H
#define MU_UI_KEYNAVIGATIONSECTION_H

#include <QObject>
#include <QQmlParserStatus>
#include "../ikeynavigationsection.h"

#include "modularity/ioc.h"
#include "../ikeynavigationcontroller.h"

namespace mu::ui {
class KeyNavigationSection : public QObject, public IKeyNavigationSection, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int order READ order WRITE setOrder NOTIFY orderChanged)

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

    INJECT(ui, IKeyNavigationController, keyNavigationController)

public:
    explicit KeyNavigationSection(QObject* parent = nullptr);
    ~KeyNavigationSection();

    QString name() const override;
    int order() const override;
    bool enabled() const override;
    bool active() const override;
    void setActive(bool active) override;

    // QQmlParserStatus
    void classBegin() override;
    void componentComplete() override;

public slots:
    void setName(QString name);
    void setOrder(int order);
    void setEnabled(bool enabled);

signals:
    void nameChanged(QString name);
    void activeChanged(bool active);
    void orderChanged(int order);
    void enabledChanged(bool enabled);

private:
    QString m_name;
    int m_order = -1;
    bool m_active = false;
    bool m_enabled = true;
};
}

#endif // MU_UI_KEYNAVIGATIONSECTION_H
