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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTTREEITEM_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTTREEITEM_H

#include "abstractinstrumentspaneltreeitem.h"

#include "notation/inotationparts.h"
#include "async/asyncable.h"

namespace mu::instrumentsscene {
class InstrumentsTreeItem : public AbstractInstrumentsPanelTreeItem, public async::Asyncable
{
    Q_OBJECT

public:
    explicit InstrumentsTreeItem(notation::INotationPartsPtr notationParts, QObject* parent = nullptr);

    Q_INVOKABLE QString partId() const;
    Q_INVOKABLE QString partName() const;
    Q_INVOKABLE QString abbreviature() const;

    void setPartId(const QString& partId);
    void setPartName(const QString& partName);
    void setAbbreviature(const QString& abbreviature);

    void moveChildren(const int sourceRow, const int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                      const int destinationRow) override;
    void removeChildren(const int row, const int count, const bool deleteChild) override;

    void updateCanChangeVisibility();

private:
    notation::ID staffId(int row) const;

    QString m_partId;
    QString m_partName;
    QString m_abbreviature;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTTREEITEM_H
