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
#ifndef MU_NOTATION_NOTATIONNAVIGATOR_H
#define MU_NOTATION_NOTATIONNAVIGATOR_H

#include <QObject>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickPaintedItem>

#include "draw/types/geometry.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "inotationconfiguration.h"
#include "context/iglobalcontext.h"
#include "ui/iuiconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "abstractnotationpaintview.h"

namespace mu::notation {
class NotationNavigatorViewRect : public QQuickPaintedItem
{
    Q_OBJECT

    INJECT(INotationConfiguration, configuration)

public:
    NotationNavigatorViewRect(QQuickItem* parent = nullptr);

    void setRect(const RectF& cursorRect);

private:
    virtual void paint(QPainter* painter) override;

    RectF m_cursorRect;
};

class NotationNavigator : public AbstractNotationPaintView
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)
    INJECT(INotationConfiguration, configuration)
    INJECT(ui::IUiConfiguration, uiConfiguration)
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration)

    Q_PROPERTY(int orientation READ orientation NOTIFY orientationChanged)

public:
    NotationNavigator(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void setCursorRect(const QRectF& rect);

    int orientation() const;

signals:
    void moveNotationRequested(qreal dx, qreal dy);
    void orientationChanged();

private:
    INotationPtr currentNotation() const;

    void initOrientation();
    void initVisible();

    ViewMode notationViewMode() const;

    void rescale();

    void paint(QPainter* painter) override;
    void onViewSizeChanged() override;

    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void paintPageNumbers(QPainter* painter);

    bool moveCanvasToRect(const RectF& viewRect);

    bool isVerticalOrientation() const;

    PageList pages() const;

    RectF m_cursorRect;
    NotationNavigatorViewRect* m_cursorRectView;
    PointF m_startMove;
};
}

#endif // MU_NOTATION_NOTATIONNAVIGATOR_H
