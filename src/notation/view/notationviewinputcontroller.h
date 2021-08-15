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
#ifndef MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H
#define MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H

#include "modularity/ioc.h"

#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "async/asyncable.h"

#include "context/iglobalcontext.h"

#include "notation/inotationinteraction.h"
#include "notation/inotationplayback.h"
#include "notation/inotationconfiguration.h"

#include "playback/iplaybackcontroller.h"

namespace mu::notation {
class IControlledView
{
public:
    virtual ~IControlledView() = default;

    virtual qreal width() const = 0;
    virtual qreal height() const = 0;

    virtual void moveCanvas(int dx, int dy) = 0;
    virtual void moveCanvasHorizontal(int dx) = 0;
    virtual void moveCanvasVertical(int dy) = 0;

    virtual qreal currentScaling() const = 0;
    virtual void scale(qreal scaling, const QPoint& pos) = 0;

    virtual PointF toLogical(const QPoint& p) const = 0;

    virtual bool isNoteEnterMode() const = 0;
    virtual void showShadowNote(const PointF& pos) = 0;

    virtual void showContextMenu(const ElementType& elementType, const QPoint& pos) = 0;
    virtual void hideContextMenu() = 0;

    virtual INotationInteractionPtr notationInteraction() const = 0;
    virtual INotationPlaybackPtr notationPlayback() const = 0;
};

class NotationViewInputController : public actions::Actionable, public async::Asyncable
{
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, context::IGlobalContext, globalContext)

public:
    NotationViewInputController(IControlledView* view);

    void init();

    bool isZoomInited();
    void initZoom();
    void zoomIn();
    void zoomOut();

    void setReadonly(bool readonly);

    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void hoverMoveEvent(QHoverEvent* event);
    void keyPressEvent(QKeyEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

    ElementType selectionType() const;
    mu::PointF hitElementPos() const;

private:
    INotationPtr currentNotation() const;
    INotationStylePtr notationStyle() const;
    INotationInteractionPtr viewInteraction() const;
    Element* hitElement() const;

    void zoomToPageWidth();
    void zoomToWholePage();
    void zoomToTwoPages();

    int currentZoomIndex() const;
    int currentZoomPercentage() const;
    qreal notationScaling() const;
    void setZoom(int zoomPercentage, const QPoint& pos = QPoint());

    void setViewMode(const ViewMode& viewMode);

    bool isDragAllowed() const;
    void startDragElements(ElementType elementsType, const PointF& elementsOffset);

    float hitWidth() const;

    bool needSelect(const QMouseEvent* event, const PointF& clickLogicPos) const;

    double guiScalling() const;

    IControlledView* m_view = nullptr;

    QList<int> m_possibleZoomsPercentage;

    bool m_readonly = false;
    bool m_isCanvasDragged = false;

    bool m_isZoomInited = false;
    PointF m_beginPoint;
};
}

#endif // MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H
