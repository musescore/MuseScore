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
#include "async/asyncable.h"

#include "notation/inotationparts.h"

#include "layoutpanelutils.h"

namespace mu::instrumentsscene {
class SystemObjectsLayerTreeItem : public AbstractLayoutPanelTreeItem, public muse::async::Asyncable
{
    Q_OBJECT

public:
    SystemObjectsLayerTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent);

    void init(const mu::engraving::Staff* staff, const SystemObjectGroups& systemObjects);

    const mu::engraving::Staff* staff() const;
    void setStaff(const mu::engraving::Staff* staff);

    Q_INVOKABLE QString staffId() const;
    Q_INVOKABLE bool canAcceptDrop(const QVariant& item) const override;

private:
    void onUndoStackChanged(const mu::engraving::ScoreChangesRange& changes);
    void onVisibleChanged(bool isVisible);

    bool addSystemObject(mu::engraving::EngravingItem* obj);
    bool removeSystemObject(mu::engraving::EngravingItem* obj);

    void updateStaff();
    void updateState();

    const mu::engraving::Staff* m_staff = nullptr;
    mu::engraving::staff_idx_t m_staffIdx = muse::nidx;
    SystemObjectGroups m_systemObjectGroups;
    bool m_ignoreVisibilityChanges = false;
};
}
