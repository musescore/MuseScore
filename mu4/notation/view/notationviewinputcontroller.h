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

#include <QWheelEvent>
#include "modularity/ioc.h"
#include "../inotationconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationplayback.h"
#include "playback/iplaybackcontroller.h"

namespace mu {
namespace notation {
class IControlledView
{
public:
    virtual ~IControlledView() = default;

    virtual qreal width() const = 0;
    virtual qreal height() const = 0;
    virtual qreal scale() const = 0;
    virtual QPoint toLogical(const QPoint& p) const = 0;

    virtual void moveCanvas(int dx, int dy) = 0;
    virtual void scrollVertical(int dy) = 0;
    virtual void scrollHorizontal(int dx) = 0;
    virtual void setZoom(int zoomPercentage, const QPoint& pos) = 0;

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

    void wheelEvent(QWheelEvent* ev);
    void mousePressEvent(QMouseEvent* ev);
    void mouseMoveEvent(QMouseEvent* ev);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent* ev);
    void hoverMoveEvent(QHoverEvent* ev);
    void keyPressEvent(QKeyEvent* event);

    void dragEnterEvent(QDragEnterEvent* ev);
    void dragLeaveEvent(QDragLeaveEvent* ev);
    void dragMoveEvent(QDragMoveEvent* ev);
    void dropEvent(QDropEvent* ev);

private:
    std::shared_ptr<INotation> currentNotation() const;

    void zoomIn();
    void zoomOut();

    int currentZoomIndex() const;
    void setZoom(int zoomPercentage, const QPoint& pos = QPoint());

    void setViewMode(const ViewMode& viewMode);

    bool canReceiveAction(const actions::ActionName& actionName) const override;

    struct InteractData {
        QPoint beginPoint;
        Element* hitElement = nullptr;
    };

    bool isDragAllowed() const;
    void startDragElements(ElementType etype, const QPointF& eoffset);

    float hitWidth() const;

    IControlledView* m_view = nullptr;
    InteractData m_interactData;

    QList<int> m_possibleZoomsPercentage;
};
}
}
#endif // MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H
