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
#ifndef MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H
#define MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H

#include "modularity/ioc.h"

#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"

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

    virtual QPoint toLogical(const QPoint& p) const = 0;

    virtual bool isNoteEnterMode() const = 0;
    virtual void showShadowNote(const QPointF& pos) = 0;

    virtual void showContextMenu(const ElementType& elementType, const QPoint& pos) = 0;

    virtual INotationInteractionPtr notationInteraction() const = 0;
    virtual INotationPlaybackPtr notationPlayback() const = 0;
};

class NotationViewInputController : public actions::Actionable
{
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, context::IGlobalContext, globalContext)

public:
    NotationViewInputController(IControlledView* view);

    void setReadonly(bool readonly);

    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void hoverMoveEvent(QHoverEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

private:
    INotationPtr currentNotation() const;

    void zoomIn();
    void zoomOut();
    void zoomToPageWidth();

    int currentZoomIndex() const;
    int currentZoomPercentage() const;
    qreal notationScaling() const;
    void setZoom(int zoomPercentage, const QPoint& pos = QPoint());

    void setViewMode(const ViewMode& viewMode);

    struct InteractData {
        QPoint beginPoint;
        Element* hitElement = nullptr;
        int hitStaffIndex = 0;
    };

    bool isDragAllowed() const;
    void startDragElements(ElementType elementsType, const QPointF& elementsOffset);

    float hitWidth() const;

    ElementType selectionType() const;

    IControlledView* m_view = nullptr;
    InteractData m_interactData;

    QList<int> m_possibleZoomsPercentage;

    bool m_readonly = false;
    bool m_isCanvasDragged = false;
};
}

#endif // MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H
