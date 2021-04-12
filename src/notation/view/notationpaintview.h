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

#include <QQuickPaintedItem>

#include "modularity/ioc.h"

#include "notation/inotationconfiguration.h"
#include "inotationcontextmenu.h"

#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "playback/iplaybackcontroller.h"

#include "notationviewinputcontroller.h"
#include "noteinputcursor.h"
#include "playbackcursor.h"
#include "loopmarker.h"

namespace mu::notation {
class NotationPaintView : public QQuickPaintedItem, public IControlledView, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, INotationContextMenu, notationContextMenu)

    Q_PROPERTY(qreal startHorizontalScrollPosition READ startHorizontalScrollPosition NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal horizontalScrollSize READ horizontalScrollSize NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal startVerticalScrollPosition READ startVerticalScrollPosition NOTIFY verticalScrollChanged)
    Q_PROPERTY(qreal verticalScrollSize READ verticalScrollSize NOTIFY verticalScrollChanged)

    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QRect viewport READ viewport NOTIFY viewportChanged)

public:
    explicit NotationPaintView(QQuickItem* parent = nullptr);

    Q_INVOKABLE virtual void load();

    Q_INVOKABLE void scrollHorizontal(qreal position);
    Q_INVOKABLE void scrollVertical(qreal position);

    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();

    qreal width() const override;
    qreal height() const override;

    QPoint toLogical(const QPoint& point) const override;

    Q_INVOKABLE void moveCanvas(int dx, int dy) override;
    void moveCanvasVertical(int dy) override;
    void moveCanvasHorizontal(int dx) override;

    qreal currentScaling() const override;
    void scale(qreal scaling, const QPoint& pos) override;

    bool isNoteEnterMode() const override;
    void showShadowNote(const QPointF& pos) override;

    void showContextMenu(const ElementType& elementType, const QPoint& pos) override;
    Q_INVOKABLE void handleAction(const QString& actionCode);

    INotationInteractionPtr notationInteraction() const override;
    INotationPlaybackPtr notationPlayback() const override;

    qreal startHorizontalScrollPosition() const;
    qreal horizontalScrollSize() const;
    qreal startVerticalScrollPosition() const;
    qreal verticalScrollSize() const;

    QColor backgroundColor() const;
    QRect viewport() const;

signals:
    void openContextMenuRequested(const QVariantList& items, const QPoint& pos);
    void textEdittingStarted();

    void horizontalScrollChanged();
    void verticalScrollChanged();

    void backgroundColorChanged(QColor color);
    void viewportChanged(QRect viewport);

protected:
    void setNotation(INotationPtr notation);
    void setReadonly(bool readonly);

    void moveCanvasToCenter();
    void moveCanvasToPosition(const QPoint& logicPos);
    double guiScaling() const;

    QRectF notationContentRect() const;

    QRect toLogical(const QRect& rect) const;

    // Draw
    void paint(QPainter* painter) override;

protected slots:
    virtual void onViewSizeChanged();

private:
    INotationPtr notation() const;
    INotationNoteInputPtr notationNoteInput() const;
    INotationElementsPtr notationElements() const;
    INotationStylePtr notationStyle() const;
    INotationSelectionPtr notationSelection() const;

    void clear();
    void initBackground();
    void initNavigatorOrientation();

    bool canReceiveAction(const actions::ActionCode& actionCode) const override;
    void onCurrentNotationChanged();
    bool isInited() const;

    // Input
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    QRectF canvasRect() const;

    qreal horizontalScrollableAreaSize() const;
    qreal horizontalScrollableSize() const;
    qreal verticalScrollableAreaSize() const;
    qreal verticalScrollableSize() const;

    void adjustCanvasPosition(const QRectF& logicRect);

    void onNoteInputChanged();
    void onSelectionChanged();

    void onPlayingChanged();
    void movePlaybackCursor(uint32_t tick);

    void updateLoopMarkers(const LoopBoundaries& boundaries);

    const Page* pointToPage(const QPointF& point) const;
    QPointF alignToCurrentPageBorder(const QRectF& showRect, const QPointF& pos) const;

    void paintBackground(const QRect& rect, mu::draw::Painter* painter);

    QPoint canvasCenter() const;
    std::pair<int, int> constraintCanvas(int dx, int dy) const;

    notation::INotationPtr m_notation;
    QTransform m_matrix;
    std::unique_ptr<NotationViewInputController> m_inputController;
    std::unique_ptr<PlaybackCursor> m_playbackCursor;
    std::unique_ptr<NoteInputCursor> m_noteInputCursor;
    std::unique_ptr<LoopMarker> m_loopInMarker;
    std::unique_ptr<LoopMarker> m_loopOutMarker;

    qreal m_previousVerticalScrollPosition = 0;
    qreal m_previousHorizontalScrollPosition = 0;
};
}

#endif // MU_NOTATION_NOTATIONPAINTVIEW_H
