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
#ifndef MU_NOTATIONSCENE_NOTATIONPAINTVIEW_H
#define MU_NOTATIONSCENE_NOTATIONPAINTVIEW_H

#include <QObject>
#include <QQuickPaintedItem>
#include <QTransform>
#include <QTimer>

#include "modularity/ioc.h"
#include "../iscenenotationconfiguration.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "scenes/playback/iplaybackcontroller.h"

#include "notationviewinputcontroller.h"
#include "playbackcursor.h"

namespace mu {
namespace scene {
namespace notation {
class NotationViewInputController;
class NotationPaintView : public QQuickPaintedItem, public IControlledView, public async::Asyncable,
    public actions::Actionable
{
    Q_OBJECT

    INJECT(notation_scene, ISceneNotationConfiguration, configuration)
    INJECT(notation_scene, actions::IActionsDispatcher, dispatcher)
    INJECT(notation_scene, context::IGlobalContext, globalContext)
    INJECT(notation_scene, playback::IPlaybackController, playbackController)

public:
    NotationPaintView();
    ~NotationPaintView();

    // IControlledView
    qreal width() const override;
    qreal height() const override;
    qreal scale() const override;

    QPoint toLogical(const QPoint& p) const override;

    void moveCanvas(int dx, int dy) override;
    void scrollVertical(int dy) override;
    void scrollHorizontal(int dx) override;
    void zoomStep(qreal step, const QPoint& pos) override;

    bool isNoteEnterMode() const override;
    void showShadowNote(const QPointF& pos);

    domain::notation::INotationInteraction* notationInteraction() const;
    // -----

private slots:
    void onViewSizeChanged();

private:

    bool canReceiveAction(const actions::ActionName& action) const override;
    void onCurrentNotationChanged();
    bool isInited() const;
    std::shared_ptr<domain::notation::INotation> notation() const;

    // Draw
    void paint(QPainter* painter) override;

    // Input
    void wheelEvent(QWheelEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void hoverMoveEvent(QHoverEvent* ev) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dragLeaveEvent(QDragLeaveEvent* ev) override;
    void dragMoveEvent(QDragMoveEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;

    QRect toLogical(const QRect& r) const;
    QPoint toPhysical(const QPoint& p) const;

    void zoom(qreal mag, const QPoint& pos);

    // ---

    qreal xoffset() const;
    qreal yoffset() const;
    QRect viewport() const;

    void adjustCanvasPosition(const QRectF& logicRect);
    void moveCanvasToPosition(const QPoint& logicPos);

    void onInputStateChanged();
    void onSelectionChanged();

    void onPlayingChanged();
    void updatePlaybackCursor();

    QColor m_backgroundColor;
    std::shared_ptr<domain::notation::INotation> m_notation;
    QTransform m_matrix;
    NotationViewInputController* m_inputController = nullptr;
    PlaybackCursor* m_playbackCursor = nullptr;
    QTimer m_playbackUpdateTimer;
};
}
}
}

#endif // MU_NOTATIONSCENE_NOTATIONPAINTVIEW_H
