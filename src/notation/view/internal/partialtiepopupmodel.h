/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "view/abstractelementpopupmodel.h"
#include "uicomponents/view/menuitem.h"

namespace mu::engraving {
class Tie;
class TieJumpPoint;
}

namespace mu::notation {
class PartialTiePopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)
    Q_PROPERTY(bool tieDirection READ tieDirection NOTIFY tieDirectionChanged)
    Q_PROPERTY(QPointF dialogPosition READ dialogPosition CONSTANT)

public:
    explicit PartialTiePopupModel(QObject* parent = nullptr);

    QVariantList items() const;
    bool tieDirection() const;
    static bool canOpen(const EngravingItem* element);
    QPointF dialogPosition() const;

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void toggleItemChecked(const QString& id);

    Q_INVOKABLE void onClosed();

signals:
    void tieDirectionChanged(bool direction);
    void itemsChanged();

private:
    void load();
    muse::uicomponents::MenuItemList makeMenuItems();
    muse::uicomponents::MenuItem* makeMenuItem(const engraving::TieJumpPoint* jumpPoint);
    mu::engraving::Tie* tie() const;

    muse::uicomponents::MenuItemList m_items;
};
}
