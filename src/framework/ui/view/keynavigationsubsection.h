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
#ifndef MU_UI_KEYNAVIGATIONSUBSECTION_H
#define MU_UI_KEYNAVIGATIONSUBSECTION_H

#include <QObject>
#include <QQmlParserStatus>
#include "../ikeynavigationsection.h"
#include "keynavigationsection.h"

namespace mu::ui {
class KeyNavigationSubSection : public QObject, public IKeyNavigationSubSection, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(KeyNavigationSection * section READ section WRITE setSection)

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(int order READ order WRITE setOrder)

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    explicit KeyNavigationSubSection(QObject* parent = nullptr);

    KeyNavigationSection* section() const;

    QString name() const override;
    int order() const override;
    bool enabled() const override;
    bool active() const override;
    void setActive(bool active) override;

    // QQmlParserStatus
    void classBegin() override;
    void componentComplete() override;

public slots:
    void setSection(KeyNavigationSection* section);
    void setName(QString name);
    void setOrder(int order);
    void setEnabled(bool enabled);

signals:
    void enabledChanged(bool enabled);
    void activeChanged(bool active);

private:
    KeyNavigationSection* m_section = nullptr;
    QString m_name;
    int m_order = -1;
    bool m_active = false;
    bool m_enabled = true;
};
}

#endif // MU_UI_KEYNAVIGATIONSUBSECTION_H
