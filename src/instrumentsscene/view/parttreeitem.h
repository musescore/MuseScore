/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#pragma once

#include "abstractlayoutpaneltreeitem.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "notation/iselectinstrumentscenario.h"

namespace mu::instrumentsscene {
class PartTreeItem : public AbstractLayoutPanelTreeItem, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<notation::ISelectInstrumentsScenario> selectInstrumentsScenario { this };
    muse::Inject<muse::IInteractive> interactive { this };

public:
    PartTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent);

    void init(const notation::Part* masterPart);

    const notation::Part* part() const;

    MoveParams buildMoveParams(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent, int destinationRow) const override;

    void moveChildren(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent, int destinationRow,
                      bool updateNotation) override;

    void moveChildrenOnScore(const MoveParams& params) override;

    void removeChildren(int row, int count, bool deleteChild) override;

    Q_INVOKABLE bool canAcceptDrop(const QVariant& item) const override;

    Q_INVOKABLE QString instrumentId() const;
    Q_INVOKABLE void replaceInstrument();
    Q_INVOKABLE void resetAllFormatting();

private:
    void onScoreChanged(const mu::engraving::ScoreChangesRange& changes) override;

    void listenVisibilityChanged();
    void createAndAddPart(const muse::ID& masterPartId);

    size_t resolveNewPartIndex(const muse::ID& partId) const;

    const notation::Part* m_part = nullptr;
    bool m_ignoreVisibilityChange = true;
    bool m_partExists = false;
};
}
