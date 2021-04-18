//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_NOTATIONNAVIGATOR_H
#define MU_NOTATION_NOTATIONNAVIGATOR_H

#include <QObject>
#include <QQuickPaintedItem>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "inotationconfiguration.h"
#include "context/iglobalcontext.h"
#include "ui/iuiconfiguration.h"
#include "notationpaintview.h"

namespace mu::notation {
class NotationNavigator : public NotationPaintView
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, ui::IUiConfiguration, uiConfiguration)

    Q_PROPERTY(int orientation READ orientation NOTIFY orientationChanged)

public:
    NotationNavigator(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load() override;
    Q_INVOKABLE void setCursorRect(const QRect& rect);

    int orientation() const;

signals:
    void moveNotationRequested(int dx, int dy);
    void orientationChanged();

private:
    INotationPtr currentNotation() const;

    void initOrientation();
    void initVisible();

    ViewMode notationViewMode() const;

    void rescale();

    void paint(QPainter* painter) override;

    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void paintCursor(QPainter* painter);
    void paintPageNumbers(QPainter* painter);

    void moveCanvasToRect(const QRect& viewRect);

    bool isVerticalOrientation() const;

    QRectF notationContentRect() const;
    PageList pages() const;

    QRect m_cursorRect;
    QPoint m_startMove;
};
}

#endif // MU_NOTATION_NOTATIONNAVIGATOR_H
