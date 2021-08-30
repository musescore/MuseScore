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
    PartTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent);

    void init(const notation::Part* masterPart);

    bool isSelectable() const override;

    Q_INVOKABLE QString instrumentId() const;

    void moveChildren(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent, int destinationRow) override;
    void removeChildren(int row, int count, bool deleteChild) override;

private:
    void listenVisibilityChanged();
    void createAndAddPart(const ID& masterPartId);

    size_t resolveNewPartIndex(const ID& partId) const;

    QString m_instrumentId;
    bool m_isInited = false;
};
}

#endif // MU_INSTRUMENTSSCENE_PARTTREEITEM_H
