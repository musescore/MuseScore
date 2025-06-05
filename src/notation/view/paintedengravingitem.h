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
#include "engraving/iengravingconfiguration.h"

#include "engraving/dom/engravingitem.h"

#include "notation/utilities/engravingitempreviewpainter.h"

namespace mu::notation {
class PaintedEngravingItem : public QQuickPaintedItem
{
    INJECT_STATIC(engraving::IEngravingConfiguration, configuration)

    Q_OBJECT

    Q_PROPERTY(QVariant engravingItem READ engravingItemVariant WRITE setEngravingItemVariant NOTIFY engravingItemVariantChanged)
    Q_PROPERTY(int numStaffLines READ numStaffLines WRITE setNumStaffLines NOTIFY numStaffLinesChanged)

    Q_PROPERTY(double spatium READ spatium WRITE setSpatium NOTIFY spatiumChanged)

public:
    explicit PaintedEngravingItem(QQuickItem* parent = nullptr);

    QVariant engravingItemVariant() const;
    void setEngravingItemVariant(QVariant engravingItemVariant);

    int numStaffLines() const;
    void setNumStaffLines(int numStaffLines);

    double spatium() const;
    void setSpatium(double spatium);

    void paint(QPainter* painter) override;

signals:
    void engravingItemVariantChanged();
    void numStaffLinesChanged();
    void spatiumChanged();

private:
    void paintNotationPreview(muse::draw::Painter& painter, qreal dpi) const;

    mu::engraving::ElementPtr m_item;
    int m_numStaffLines = 0;

    double m_spatium = 1.0;
};
}
