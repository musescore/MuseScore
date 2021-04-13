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
#ifndef MU_UI_KEYNAVDEVMODEL_H
#define MU_UI_KEYNAVDEVMODEL_H

#include <QObject>
#include <QTimer>
#include "modularity/ioc.h"
#include "ui/ikeynavigationcontroller.h"
#include "async/asyncable.h"

namespace mu::ui {
class AbstractKeyNavDevItem;
class KeyNavDevModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantList sections READ sections NOTIFY sectionsChanged)

    INJECT(ui, IKeyNavigationController, keynavController)
public:
    explicit KeyNavDevModel(QObject* parent = nullptr);

    QVariantList sections() const;

    Q_INVOKABLE void reload();

signals:
    void sectionsChanged();

private:

    QVariant toWrap(IKeyNavigationSection* s);
    QVariant toWrap(IKeyNavigationSubSection* sub);
    QVariant toWrap(IKeyNavigationControl* ctrl);

    QVariantList m_sections;
    mutable QList<AbstractKeyNavDevItem*> m_memstore;
    QTimer m_reloadDelayer;
};
}

#endif // MU_UI_KEYNAVDEVMODEL_H
