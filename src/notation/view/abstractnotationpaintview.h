/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "uicomponents/view/quickpaintedview.h"

#include "notationviewinputcontroller.h"
#include "noteinputcursor.h"
#include "notationruler.h"
#include "playbackcursor.h"
#include "loopmarker.h"
#include "continuouspanel.h"
#include "abstractelementpopupmodel.h"

namespace mu::notation {
class AbstractNotationPaintView : public muse::uicomponents::QuickPaintedView, public IControlledView, public muse::Injectable,
    public muse::async::Asyncable, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(qreal startHorizontalScrollPosition READ startHorizontalScrollPosition NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal horizontalScrollbarSize READ horizontalScrollbarSize NOTIFY horizontalScrollChanged)
    Q_PROPERTY(qreal startVerticalScrollPosition READ startVerticalScrollPosition NOTIFY verticalScrollChanged)
    Q_PROPERTY(qreal verticalScrollbarSize READ verticalScrollbarSize NOTIFY verticalScrollChanged)

    Q_PROPERTY(QRectF viewport READ viewport_property NOTIFY viewportChanged)

    Q_PROPERTY(bool publishMode READ publishMode WRITE setPublishMode NOTIFY publishModeChanged)

    Q_PROPERTY(bool isMainView READ isMainView WRITE setIsMainView NOTIFY isMainViewChanged)

    muse::Inject<INotationConfiguration> configuration = { this };
    muse::Inject<engraving::IEngravingConfiguration> engravingConfiguration = { this };
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<muse::ui::IUiContextResolver> uiContextResolver = { this };
    muse::Inject<muse::ui::IMainWindow> mainWindow = { this };
    muse::Inject<muse::ui::IUiActionsRegister> actionsRegister = { this };

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

    Q_INVOKABLE void onContextMenuIsOpenChanged(bool open);
    Q_INVOKABLE void onElementPopupIsOpenChanged(const PopupModelType& popupType = PopupModelType::TYPE_UNDEFINED);

    Q_INVOKABLE void setPlaybackCursorItem(QQuickItem* cursor);

    qreal width() const override;
    qreal height() const override;

    muse::PointF toLogical(const muse::PointF& point) const override;
    muse::PointF toLogical(const QPointF& point) const override;
    muse::RectF toLogical(const muse::RectF& rect) const;

    muse::PointF fromLogical(const muse::PointF& point) const override;
    muse::RectF fromLogical(const muse::RectF& rect) const override;

    Q_INVOKABLE bool moveCanvas(
        qreal dx, qreal dy, bool userTriggeredMove = true, bool overrideZoomType = false);
    Q_INVOKABLE bool moveCanvasToPosition(
        qreal x, qreal y, bool userTriggeredMove = true, bool overrideZoomType = false);

    qreal currentScaling() const override;
    void setScaling(qreal scaling, const muse::PointF& pos, bool overrideZoomType = true) override;
    void scale(qreal factor, const muse::PointF& pos, bool overrideZoomType = true);

    Q_INVOKABLE void pinchToZoom(qreal scaleFactor, const QPointF& pos);

    bool isNoteEnterMode() const override;
    void showShadowNote(const muse::PointF& pos) override;

    void showContextMenu(const ElementType& elementType, const QPointF& pos) override;
    void hideContextMenu() override;

    void showElementPopup(const ElementType& elementType, const muse::RectF& elementRect) override;
    void hideElementPopup(const ElementType& elementType = ElementType::INVALID) override;
    void toggleElementPopup(const ElementType& elementType, const muse::RectF& elementRect) override;

    bool elementPopupIsOpen(const ElementType& elementType) const override;

    INotationInteractionPtr notationInteraction() const override;
    INotationPlaybackPtr notationPlayback() const override;

    QQuickItem* asItem() override;

    qreal startHorizontalScrollPosition() const;
    qreal horizontalScrollbarSize() const;
    qreal startVerticalScrollPosition() const;
    qreal verticalScrollbarSize() const;

    muse::PointF viewportTopLeft() const override;
    muse::RectF viewport() const override;
    QRectF viewport_property() const;

    bool publishMode() const;
    void setPublishMode(bool arg);

    bool isMainView() const;
    void setIsMainView(bool isMainView);

signals:
    void showContextMenuRequested(int elementType, const QPointF& viewPos);
    void hideContextMenuRequested();

    void showElementPopupRequested(mu::notation::PopupModelType modelType, const QRectF& elementRect);
    void hideElementPopupRequested();
    void isPopupOpenChanged(bool isPopupOpen);

    void horizontalScrollChanged();
    void verticalScrollChanged();

    void backgroundColorChanged(QColor color);
    void viewportChanged();
    void publishModeChanged();

    void activeFocusRequested();

    void isMainViewChanged(bool isMainView);

protected:
    INotationPtr notation() const;
    void setNotation(INotationPtr notation);

    NotationViewInputController* inputController() const;

    void setReadonly(bool readonly);
    void setMatrix(const muse::draw::Transform& matrix);

    void moveCanvasToCenter();

    muse::RectF notationContentRect() const override;

    // Draw
    void paint(QPainter* painter) override;

    virtual void onNotationSetup();

    virtual void onLoadNotation(INotationPtr notation);
    virtual void onUnloadNotation(INotationPtr notation);

    virtual void initZoomAndPosition();

    virtual void onMatrixChanged(const muse::draw::Transform& oldMatrix, const muse::draw::Transform& newMatrix, bool overrideZoomType);

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

    bool canReceiveAction(const muse::actions::ActionCode& actionCode) const override;
    void onCurrentNotationChanged();
    bool isInited() const;

    void scheduleRedraw(const muse::RectF& rect = muse::RectF());
    muse::RectF correctDrawRect(const muse::RectF& rect) const;

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
    void keyReleaseEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void inputMethodEvent(QInputMethodEvent* event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

    bool ensureViewportInsideScrollableArea();

    qreal horizontalScrollableSize() const;
    qreal verticalScrollableSize() const;

    bool adjustCanvasPosition(const muse::RectF& logicRect, bool adjustVertically = true);
    bool adjustCanvasPositionSmoothPan(const muse::RectF& cursorRect);

    void onNoteInputStateChanged();

    void onShowItemRequested(const INotationInteraction::ShowItemRequest& request);

    void onPlayingChanged();
    void movePlaybackCursor(muse::midi::tick_t tick);
    bool needAdjustCanvasVerticallyWhilePlayback(const muse::RectF& cursorRect);

    void onPlaybackCursorRectChanged();

    void updateLoopMarkers();

    const Page* pageByPoint(const muse::PointF& point) const;
    muse::PointF alignToCurrentPageBorder(const muse::RectF& showRect, const muse::PointF& pos) const;

    void paintBackground(const muse::RectF& rect, muse::draw::Painter* painter);

    muse::PointF canvasCenter() const;
    muse::PointF constrainedCanvasMoveDelta(qreal x, qreal y);

    INotationPtr m_notation;
    muse::draw::Transform m_matrix;

    bool m_loadCalled = false;
    std::unique_ptr<NotationViewInputController> m_inputController;
    std::unique_ptr<PlaybackCursor> m_playbackCursor;
    std::unique_ptr<NoteInputCursor> m_noteInputCursor;
    std::unique_ptr<NotationRuler> m_ruler;
    std::unique_ptr<LoopMarker> m_loopInMarker;
    std::unique_ptr<LoopMarker> m_loopOutMarker;
    std::unique_ptr<ContinuousPanel> m_continuousPanel;

    qreal m_previousVerticalScrollPosition = 0;
    qreal m_previousHorizontalScrollPosition = 0;

    bool m_readonly = false;
    bool m_publishMode = false;
    int m_lastAcceptedKey = -1;
    bool m_isMainView = false;

    bool m_autoScrollEnabled = true;
    QTimer m_enableAutoScrollTimer;

    PopupModelType m_currentElementPopupType = PopupModelType::TYPE_UNDEFINED;
    bool m_isContextMenuOpen = false;

    muse::RectF m_shadowNoteRect;

    QQuickItem* m_playbackCursorItem = nullptr;
};
}

#endif // MU_NOTATION_ABSTRACTNOTATIONPAINTVIEW_H
