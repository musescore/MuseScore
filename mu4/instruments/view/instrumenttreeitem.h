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
#ifndef MU_INSTRUMENTS_INSTRUMENTTREEITEM_H
#define MU_INSTRUMENTS_INSTRUMENTTREEITEM_H

#include "abstractinstrumentpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu {
namespace instruments {
class InstrumentTreeItem : public AbstractInstrumentPanelTreeItem
{
    Q_OBJECT

public:
    explicit InstrumentTreeItem(notation::INotationParts* notationParts, QObject* parent = nullptr);

    Q_INVOKABLE QString partId() const;
    Q_INVOKABLE QString partName() const;
    Q_INVOKABLE QString abbreviature() const;

    void setPartId(const QString& partId);
    void setPartName(const QString& partName);
    void setAbbreviature(const QString& abbreviature);

    void moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                      const int destinationRow) override;
    void removeChildren(const int row, const int count, const bool deleteChild) override;

private:
    QString m_partId;
    QString m_partName;
    QString m_abbreviature;
};
}
}

#endif // MU_INSTRUMENTS_INSTRUMENTTREEITEM_H
