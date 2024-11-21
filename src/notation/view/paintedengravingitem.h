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

#include <QQuickPaintedItem>

#include "modularity/ioc.h"
#include "palette/ipaletteconfiguration.h"

#include "engraving/dom/engravingitem.h"

#include "notation/utilities/engravingitempreviewpainter.h"

namespace mu::notation {
class PaintedEngravingItem : public QQuickPaintedItem
{
    // TODO: don't use the palette for this
    INJECT_STATIC(palette::IPaletteConfiguration, configuration)

    Q_OBJECT

    Q_PROPERTY(QVariant engravingItem READ engravingItemVariant WRITE setEngravingItemVariant NOTIFY engravingItemVariantChanged)

public:
    explicit PaintedEngravingItem(QQuickItem* parent = nullptr);

    QVariant engravingItemVariant() const;
    void setEngravingItemVariant(QVariant engravingItemVariant);

    void paint(QPainter* painter) override;

signals:
    void engravingItemVariantChanged();

private:
    void paintNotationPreview(muse::draw::Painter& painter, qreal dpi) const;

    mu::engraving::ElementPtr m_item;
};
}
