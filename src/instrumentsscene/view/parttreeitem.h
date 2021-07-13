/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_INSTRUMENTSSCENE_PARTTREEITEM_H
#define MU_INSTRUMENTSSCENE_PARTTREEITEM_H

#include "abstractinstrumentspaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu::instrumentsscene {
class PartTreeItem : public AbstractInstrumentsPanelTreeItem
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

    void moveChildren(const int sourceRow, const int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                      const int destinationRow) override;
    void removeChildren(const int row, const int count, const bool deleteChild) override;

private:
    QString m_instrumentId;
    QString m_instrumentName;
    QString m_instrumentAbbreviature;
};
}

#endif // MU_INSTRUMENTSSCENE_PARTTREEITEM_H
