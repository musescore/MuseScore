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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTCONTROLTREEITEM_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTCONTROLTREEITEM_H

#include "abstractinstrumentspaneltreeitem.h"

#include "modularity/ioc.h"
#include "notation/inotationparts.h"
#include "notation/iselectinstrumentscenario.h"

namespace mu::instrumentsscene {
class InstrumentControlTreeItem : public AbstractInstrumentsPanelTreeItem
{
    Q_OBJECT

    INJECT(instruments, notation::ISelectInstrumentsScenario, selectInstrumentsScenario)

public:
    explicit InstrumentControlTreeItem(notation::INotationPartsPtr notationParts, QObject* parent = nullptr);

    Q_INVOKABLE void appendNewItem() override;

    QString partId() const;
    void setPartId(const QString& id);

private:
    QString m_partId;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTCONTROLTREEITEM_H
