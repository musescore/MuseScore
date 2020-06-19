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
#ifndef MU_SHORTCUTS_SHORTCUTSINSTANCEMODEL_H
#define MU_SHORTCUTS_SHORTCUTSINSTANCEMODEL_H

#include <QObject>
#include <QString>
#include <QList>

#include "modularity/ioc.h"
#include "ishortcutsregister.h"
#include "ishortcutscontroller.h"

namespace mu {
namespace shortcuts {
class ShortcutsInstanceModel : public QObject
{
    Q_OBJECT

    INJECT(shortcuts, IShortcutsRegister, shortcutsRegister)
    INJECT(shortcuts, IShortcutsController, controller)

    Q_PROPERTY(QStringList shortcuts READ shortcuts NOTIFY shortcutsChanged)

public:
    explicit ShortcutsInstanceModel(QObject* parent = nullptr);

    QStringList shortcuts() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void activate(const QString& key);

signals:
    void shortcutsChanged();

private:
    QStringList m_shortcuts;
};
}
}

#endif // MU_SHORTCUTS_SHORTCUTSINSTANCEMODEL_H
