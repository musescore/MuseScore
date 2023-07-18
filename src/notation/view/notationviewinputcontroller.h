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

#include <QtEvents>

#include "modularity/ioc.h"

#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "async/asyncable.h"

#include "context/iglobalcontext.h"

#include "notation/inotationinteraction.h"
#include "notation/inotationplayback.h"
#include "notation/inotationconfiguration.h"

#include "playback/iplaybackcontroller.h"

class QQuickItem;

namespace mu::notation {
class IControlledView
{
public:
    virtual ~IControlledView() = default;

    virtual qreal width() const = 0;
    virtual qreal height() const = 0;

    virtual PointF viewportTopLeft() const = 0;

    //! Returns true if the canvas has been moved
    virtual bool moveCanvas(qreal dx, qreal dy) = 0;
    virtual void moveCanvasHorizontal(qreal dx) = 0;
    virtual void moveCanvasVertical(qreal dy) = 0;

    virtual RectF notationContentRect() const = 0;
    virtual qreal currentScaling() const = 0;
    virtual void setScaling(qreal scaling, const PointF& pos, bool overrideZoomType = true) = 0;

    virtual PointF toLogical(const PointF& p) const = 0;
    virtual PointF toLogical(const QPointF& p) const = 0;
    virtual PointF fromLogical(const PointF& r) const = 0;
    virtual RectF fromLogical(const RectF& r) const = 0;

    virtual bool isNoteEnterMode() const = 0;
    virtual void showShadowNote(const PointF& pos) = 0;

    virtual void showContextMenu(const ElementType& elementType, const QPointF& pos) = 0;
    virtual void hideContextMenu() = 0;

    virtual void showElementPopup(const ElementType& elementType, const QPointF& pos, const RectF& size) = 0;
    virtual void hideElementPopup() = 0;
    virtual void toggleElementPopup(const ElementType& elementType, const QPointF& pos, const RectF& size) = 0;

    virtual INotationInteractionPtr notationInteraction() const = 0;
    virtual INotationPlaybackPtr notationPlayback() const = 0;

    virtual QQuickItem* asItem() = 0;
};

class NotationViewInputController : public actions::Actionable, public async::Asyncable
{
    INJECT(INotationConfiguration, configuration)
    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(playback::IPlaybackController, playbackController)
    INJECT(context::IGlobalContext, globalContext)

public:
    NotationViewInputController(IControlledView* view);

    void init();

    void initZoom();
    void updateZoomAfterSizeChange();
    void zoomIn();
    void zoomOut();
    void nextScreen();
    void previousScreen();
    void nextPage();
    void previousPage();
    void startOfScore();
    void endOfScore();

    bool readonly() const;
    void setReadonly(bool readonly);

    void pinchToZoom(qreal scaleFactor, const QPointF& pos);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void hoverMoveEvent(QHoverEvent* event);
    bool shortcutOverrideEvent(QKeyEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void inputMethodEvent(QInputMethodEvent* event);

    bool canHandleInputMethodQuery(Qt::InputMethodQuery query) const;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

    ElementType selectionType() const;
    PointF selectionElementPos() const;

private:
    INotationPtr currentNotation() const;
    INotationStylePtr notationStyle() const;
    INotationInteractionPtr viewInteraction() const;
    const INotationInteraction::HitElementContext& hitElementContext() const;

    void onNotationChanged();

    void zoomToPageWidth();
    void doZoomToPageWidth();
    void zoomToWholePage();
    void doZoomToWholePage();
    void zoomToTwoPages();
    void doZoomToTwoPages();
    void moveScreen(int direction);
    void movePage(int direction);

    int currentZoomIndex() const;
    int currentZoomPercentage() const;
    PointF findZoomFocusPoint() const;
    void setScaling(qreal scaling, const PointF& pos = PointF(), bool overrideZoomType = true);
    void setZoom(int zoomPercentage, const PointF& pos = PointF());

    qreal scalingFromZoomPercentage(int zoomPercentage) const;
    int zoomPercentageFromScaling(qreal scaling) const;

    void setViewMode(const ViewMode& viewMode);

    void startDragElements(ElementType elementsType, const PointF& elementsOffset);

    void togglePopupForItemIfSupports(const EngravingItem* item);

    float hitWidth() const;

    struct ClickContext {
        PointF logicClickPos;
        const QMouseEvent* event = nullptr;
        mu::engraving::EngravingItem* hitElement = nullptr;
        bool isHitGrip = false;
    };

    bool needSelect(const ClickContext& ctx) const;
    void handleLeftClick(const ClickContext& ctx);
    void handleRightClick(const ClickContext& ctx);
    void handleLeftClickRelease(const QPointF& releasePoint);

    bool startTextEditingAllowed() const;
    void updateTextCursorPosition();

    IControlledView* m_view = nullptr;

    QList<int> m_possibleZoomPercentages;

    bool m_readonly = false;
    bool m_isCanvasDragged = false;
    bool m_tripleClickPending = false;

    PointF m_beginPoint;

    mu::engraving::EngravingItem* m_prevHitElement = nullptr;
    bool m_shouldTogglePopupOnLeftClickRelease = false;
};
}

#endif // MU_NOTATION_NOTATIONVIEWINPUTCONTROLLER_H
