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
#ifndef MU_NOTATION_NOTATIONPAINTVIEW_H
#define MU_NOTATION_NOTATIONPAINTVIEW_H

#include <QObject>
#include <QQuickPaintedItem>
#include <QTransform>

#include "modularity/ioc.h"
#include "inotationconfiguration.h"
#include "internal/inotationactionsrepositoryfactory.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "playback/iplaybackcontroller.h"
#include "shortcuts/ishortcutsregister.h"

#include "notationviewinputcontroller.h"
#include "playbackcursor.h"

namespace mu {
namespace notation {
class NotationViewInputController;
class NotationPaintView : public QQuickPaintedItem, public IControlledView, public async::Asyncable,public actions::Actionable
{
    Q_OBJECT

    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, INotationActionsRepositoryFactory, actionsFactory)
    INJECT(notation, shortcuts::IShortcutsRegister, shortcutsRegister)

public:
    explicit NotationPaintView(QQuickItem* parent = nullptr);
    ~NotationPaintView() override;

    Q_INVOKABLE void handleAction(const QString& actionName);

    qreal width() const override;
    qreal height() const override;
    qreal scale() const override;

    QPoint toLogical(const QPoint& point) const override;

    void moveCanvas(int dx, int dy) override;
    void scrollVertical(int dy) override;
    void scrollHorizontal(int dx) override;
    void setZoom(int zoomPercentage, const QPoint& pos) override;

    bool isNoteEnterMode() const override;
    void showShadowNote(const QPointF& pos) override;

    void showContextMenu(const ElementType& elementType, const QPoint& pos) override;

    INotationInteractionPtr notationInteraction() const override;
    INotationPlaybackPtr notationPlayback() const override;

private slots:
    void onViewSizeChanged();

signals:
    void openContextMenuRequested(const QVariantList& items, const QPoint& pos);

private:
    bool canReceiveAction(const actions::ActionName& action) const override;
    void onCurrentNotationChanged();
    bool isInited() const;

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

    QRect toLogical(const QRect& rect) const;
    QRect viewport() const;

    void adjustCanvasPosition(const QRectF& logicRect);
    void moveCanvasToPosition(const QPoint& logicPos);

    void onInputStateChanged();
    void onSelectionChanged();

    void onPlayingChanged();
    void movePlaybackCursor(uint32_t tick);

    QColor m_backgroundColor;
    notation::INotationPtr m_notation;
    QTransform m_matrix;
    NotationViewInputController* m_inputController = nullptr;
    PlaybackCursor* m_playbackCursor = nullptr;
};
}
}

#endif // MU_NOTATION_NOTATIONPAINTVIEW_H
