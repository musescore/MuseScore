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
#ifndef MU_INSTRUMENTS_STAFFTREEITEM_H
#define MU_INSTRUMENTS_STAFFTREEITEM_H

#include "abstractinstrumentpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu {
namespace instruments {
class StaffTreeItem : public AbstractInstrumentPanelTreeItem
{
    Q_OBJECT

public:
    explicit StaffTreeItem(notation::INotationParts* notationParts, QObject* parent = nullptr);

    Q_INVOKABLE int staffIndex() const;
    Q_INVOKABLE bool isSmall() const;
    Q_INVOKABLE bool cutawayEnabled() const;
    Q_INVOKABLE int staffType() const;
    Q_INVOKABLE QVariantList voicesVisibility() const;

    void setPartId(const QString& id);
    void setInstrumentId(const QString& id);
    void setIsSmall(bool value);
    void setCutawayEnabled(bool value);
    void setStaffType(int type);
    void setVoicesVisibility(const QVariantList& visibility);

private:
    QString m_partId;
    QString m_instrumentId;
    bool m_isSmall = false;
    bool m_cutawayEnabled = false;
    int m_staffType = 0;
    QVariantList m_voicesVisibility;
};
}
}

#endif // MU_INSTRUMENTS_STAFFTREEITEM_H
