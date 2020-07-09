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
#ifndef MU_NOTATIONSCENE_NOTATIONVIEWINPUTCONTROLLER_H
#define MU_NOTATIONSCENE_NOTATIONVIEWINPUTCONTROLLER_H

#include <QWheelEvent>
#include "modularity/ioc.h"
#include "../iscenenotationconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "domain/notation/inotationinteraction.h"

namespace mu {
namespace scene {
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
    virtual void zoomStep(qreal step, const QPoint& pos) = 0;

    virtual bool isNoteEnterMode() const = 0;
    virtual void showShadowNote(const QPointF& pos) = 0;

    virtual domain::notation::INotationInteraction* notationInteraction() const = 0;
};

class NotationViewInputController
{
    INJECT(notation_scene, ISceneNotationConfiguration, configuration)
    INJECT(notation_scene, actions::IActionsDispatcher, dispatcher)
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

    struct InteractData {
        QPoint beginPoint;
        domain::notation::Element* hitElement = nullptr;
    };

    void startDragElements(domain::notation::ElementType etype, const QPointF& eoffset);

    float hitWidth() const;

    IControlledView* m_view = nullptr;
    InteractData m_interactData;
};
}
}
}
#endif // MU_NOTATIONSCENE_NOTATIONVIEWINPUTCONTROLLER_H
