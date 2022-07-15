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
#ifndef MU_NOTATION_ABSTRACTNOTATIONPAINTVIEW_H
#define MU_NOTATION_ABSTRACTNOTATIONPAINTVIEW_H

#include <QQuickPaintedItem>
#include <QTimer>

#include "modularity/ioc.h"

#include "notation/inotationconfiguration.h"

#include "actions/iactionsdispatcher.h"
#include "ui/iuiconfiguration.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "playback/iplaybackcontroller.h"
#include "ui/iuicontextresolver.h"
#include "ui/imainwindow.h"
#include "ui/iuiactionsregister.h"
#include "uicomponents/view/abstractmenumodel.h"

#include "notationviewinputcontroller.h"
#include "noteinputcursor.h"
#include "playbackcursor.h"
#include "loopmarker.h"
#include "continuouspanel.h"

namespace mu::notation {
class AbstractNotationPaintView : public QQuickPaintedItem, public IControlledView, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, engraving::IEngravingConfiguration, engravingConfiguration)
    INJECT(notation, ui::IUiConfiguration, uiConfiguration)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, ui::IUiContextResolver, uiContextResolver)
    INJECT(notation, ui::IMainWindow, mainWindow)
    INJECT(notation, ui::IUiActionsRegister, actionsRegister)

    Q_PROPERTY(qreal startHorizontalScrollPosition READ startHorizontalScrollPosition NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal horizontalScrollbarSize READ horizontalScrollbarSize NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal startVerticalScrollPosition READ startVerticalScrollPosition NOTIFY verticalScrollChanged)
    Q_PROPERTY(qreal verticalScrollbarSize READ verticalScrollbarSize NOTIFY verticalScrollChanged)

    Q_PROPERTY(QRectF viewport READ viewport_property NOTIFY viewportChanged)

    Q_PROPERTY(bool publishMode READ publishMode WRITE setPublishMode NOTIFY publishModeChanged)

    Q_PROPERTY(bool accessibilityEnabled READ accessibilityEnabled WRITE setAccessibilityEnabled NOTIFY accessibilityEnabledChanged)

public:
    explicit AbstractNotationPaintView(QQuickItem* parent = nullptr);
    ~AbstractNotationPaintView() override;

    Q_INVOKABLE void load();

    Q_INVOKABLE void scrollHorizontal(qreal position);
    Q_INVOKABLE void scrollVertical(qreal position);

    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();

    Q_INVOKABLE void selectOnNavigationActive();

    Q_INVOKABLE void forceFocusIn();

    qreal width() const override;
    qreal height() const override;

    PointF toLogical(const PointF& point) const override;
    PointF toLogical(const QPointF& point) const override;
    RectF toLogical(const RectF& rect) const;

    PointF fromLogical(const PointF& point) const override;
    RectF fromLogical(const RectF& rect) const override;

    Q_INVOKABLE bool moveCanvas(qreal dx, qreal dy) override;
    void moveCanvasVertical(qreal dy) override;
    void moveCanvasHorizontal(qreal dx) override;

    qreal currentScaling() const override;
    void setScaling(qreal scaling, const PointF& pos) override;
    void scale(qreal factor, const PointF& pos);

    Q_INVOKABLE void pinchToZoom(qreal scaleFactor, const QPointF& pos);

    bool isNoteEnterMode() const override;
    void showShadowNote(const PointF& pos) override;

    void showContextMenu(const ElementType& elementType, const QPointF& pos, bool activateFocus = false) override;
    void hideContextMenu() override;

    INotationInteractionPtr notationInteraction() const override;
    INotationPlaybackPtr notationPlayback() const override;

    qreal startHorizontalScrollPosition() const;
    qreal horizontalScrollbarSize() const;
    qreal startVerticalScrollPosition() const;
    qreal verticalScrollbarSize() const;

    PointF viewportTopLeft() const override;
    RectF viewport() const;
    QRectF viewport_property() const;

    bool publishMode() const;
    void setPublishMode(bool arg);

    bool accessibilityEnabled() const;
    void setAccessibilityEnabled(bool accessibilityEnabled);

signals:
    void showContextMenuRequested(int elementType, const QPointF& viewPos);
    void hideContextMenuRequested();

    void horizontalScrollChanged();
    void verticalScrollChanged();

    void backgroundColorChanged(QColor color);
    void viewportChanged();
    void publishModeChanged();

    void activeFocusRequested();

    void accessibilityEnabledChanged(bool accessibilityEnabled);

protected:
    INotationPtr notation() const;
    void setNotation(INotationPtr notation);
    void setReadonly(bool readonly);
    void setMatrix(const draw::Transform& matrix);

    void moveCanvasToCenter();
    bool moveCanvasToPosition(const PointF& logicPos);

    RectF notationContentRect() const override;

    // Draw
    void paint(QPainter* painter) override;

    virtual void onNotationSetup();

    virtual void onLoadNotation(INotationPtr notation);
    virtual void onUnloadNotation(INotationPtr notation);

    virtual void onMatrixChanged(const draw::Transform& matrix);

protected slots:
    virtual void onViewSizeChanged();

private:
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

    bool doMoveCanvas(qreal dx, qreal dy);

    // Input
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    bool event(QEvent* event) override;
    bool shortcutOverride(QKeyEvent* event);
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void inputMethodEvent(QInputMethodEvent* event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

    bool ensureViewportInsideScrollableArea();

    RectF scrollableAreaRect() const;

    qreal horizontalScrollableSize() const;
    qreal verticalScrollableSize() const;

    bool adjustCanvasPosition(const RectF& logicRect, bool adjustVertically = true);

    void onNoteInputStateChanged();

    void onShowItemRequested(const INotationInteraction::ShowItemRequest& request);

    void onPlayingChanged();
    void movePlaybackCursor(midi::tick_t tick);
    bool needAdjustCanvasVerticallyWhilePlayback(const RectF& cursorRect);

    void updateLoopMarkers(const LoopBoundaries& boundaries);

    const Page* pointToPage(const PointF& point) const;
    PointF alignToCurrentPageBorder(const RectF& showRect, const PointF& pos) const;

    void paintBackground(const RectF& rect, draw::Painter* painter);

    PointF canvasCenter() const;
    std::pair<qreal, qreal> constraintCanvas(qreal dx, qreal dy) const;

    INotationPtr m_notation;
    draw::Transform m_matrix;

    std::unique_ptr<NotationViewInputController> m_inputController;
    std::unique_ptr<PlaybackCursor> m_playbackCursor;
    std::unique_ptr<NoteInputCursor> m_noteInputCursor;
    std::unique_ptr<LoopMarker> m_loopInMarker;
    std::unique_ptr<LoopMarker> m_loopOutMarker;
    std::unique_ptr<ContinuousPanel> m_continuousPanel;

    qreal m_previousVerticalScrollPosition = 0;
    qreal m_previousHorizontalScrollPosition = 0;

    bool m_publishMode = false;
    int m_lastAcceptedKey = -1;
    bool m_accessibilityEnabled = false;

    bool m_autoScrollEnabled = true;
    QTimer m_enableAutoScrollTimer;
};
}

#endif // MU_NOTATION_ABSTRACTNOTATIONPAINTVIEW_H
