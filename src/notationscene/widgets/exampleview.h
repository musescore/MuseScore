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

#pragma once

#include <QPaintEvent>
#include <QFrame>

#include "engraving/editing/mscoreview.h"

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "draw/types/transform.h"

namespace mu::engraving {
class EngravingItem;
class Score;
}

namespace mu::notation {
class ExampleView : public QFrame, public engraving::MuseScoreView
{
    Q_OBJECT

    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;

public:
    ExampleView(QWidget* parent = 0);
    ~ExampleView();
    void resetMatrix();
    void layoutChanged() override;
    void dataChanged(const muse::RectF&) override;
    void updateAll() override;
    void adjustCanvasPosition(const engraving::EngravingItem* el, int staff = -1) override;
    void setScore(engraving::Score*) override;
    void removeScore() override;

    void changeEditElement(engraving::EngravingItem*) override;
    void setDropRectangle(const muse::RectF&) override;
    void drawBackground(muse::draw::Painter*, const muse::RectF&) const override;
    void dragExampleView(QMouseEvent* ev);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

    muse::PointF toLogical(const QPointF& point);

private:
    void drawElements(muse::draw::Painter& painter, const std::vector<EngravingItem*>& el);
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void constraintCanvas(int* dxx);
    QSize sizeHint() const override;

    muse::draw::Transform m_matrix;
    QColor m_backgroundColor;
    QPixmap* m_backgroundPixmap;

    bool m_isDragging = false;
    muse::PointF m_moveStartPoint;

    double m_defaultScaling = 0;
};
}
