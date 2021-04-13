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
#ifndef MU_UI_ABSTRACTKEYNAVDEVITEM_H
#define MU_UI_ABSTRACTKEYNAVDEVITEM_H

#include <QObject>
#include "ui/ikeynavigation.h"
#include "async/asyncable.h"

namespace mu::ui {
class AbstractKeyNavDevItem : public QObject, public async::Asyncable
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant index READ index NOTIFY indexChanged)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    explicit AbstractKeyNavDevItem(IKeyNavigation* keynav);

    QString name() const;
    QVariant index() const;
    bool enabled() const;
    bool active() const;

signals:
    void indexChanged();
    void enabledChanged();
    void activeChanged();

private:

    IKeyNavigation* m_keynav = nullptr;
};
}

#endif // MU_UI_ABSTRACTKEYNAVDEVITEM_H
