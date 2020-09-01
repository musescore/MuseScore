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
#ifndef MU_INSTRUMENTS_INSTRUMENTCONTROLTREEITEM_H
#define MU_INSTRUMENTS_INSTRUMENTCONTROLTREEITEM_H

#include "abstractinstrumentpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu {
namespace instruments {
class InstrumentControlTreeItem : public AbstractInstrumentPanelTreeItem
{
    Q_OBJECT
public:
    explicit InstrumentControlTreeItem(notation::INotationParts* notationParts, QObject* parent = nullptr);

    Q_INVOKABLE void appendNewItem() override;

    QString partId() const;
    void setPartId(const QString& id);

private:
    QString m_partId;
};
}
}

#endif // MU_INSTRUMENTS_INSTRUMENTCONTROLTREEITEM_H
