/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "notation/iselectinstrumentscenario.h"

namespace mu::instrumentsscene {
class PartTreeItem : public AbstractInstrumentsPanelTreeItem, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<notation::ISelectInstrumentsScenario> selectInstrumentsScenario { this };
    muse::Inject<muse::IInteractive> interactive { this };

public:
    PartTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent);

    void init(const notation::Part* masterPart);

    bool isSelectable() const override;

    MoveParams buildMoveParams(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                               int destinationRow) const override;

    void moveChildren(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent, int destinationRow,
                      bool updateNotation) override;

    void removeChildren(int row, int count, bool deleteChild) override;

    Q_INVOKABLE QString instrumentId() const;
    Q_INVOKABLE void replaceInstrument();
    Q_INVOKABLE void resetAllFormatting();

private:
    void listenVisibilityChanged();
    void createAndAddPart(const muse::ID& masterPartId);

    size_t resolveNewPartIndex(const muse::ID& partId) const;

    QString m_instrumentId;
    bool m_isInited = false;
};
}

#endif // MU_INSTRUMENTSSCENE_PARTTREEITEM_H
