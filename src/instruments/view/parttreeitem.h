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
#ifndef MU_INSTRUMENTS_PARTTREEITEM_H
#define MU_INSTRUMENTS_PARTTREEITEM_H

#include "abstractinstrumentpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu::instruments {
class PartTreeItem : public AbstractInstrumentPanelTreeItem
{
    Q_OBJECT

public:
    explicit PartTreeItem(notation::INotationPartsPtr notationParts, QObject* parent = nullptr);

    Q_INVOKABLE QString instrumentId() const;
    Q_INVOKABLE QString instrumentName() const;
    Q_INVOKABLE QString instrumentAbbreviature() const;

    void setInstrumentId(const QString& instrumentId);
    void setInstrumentName(const QString& name);
    void setInstrumentAbbreviature(const QString& abbreviature);

    void moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                      const int destinationRow) override;
    void removeChildren(const int row, const int count, const bool deleteChild) override;

private:
    QString m_instrumentId;
    QString m_instrumentName;
    QString m_instrumentAbbreviature;
};
}

#endif // MU_INSTRUMENTS_PARTTREEITEM_H
