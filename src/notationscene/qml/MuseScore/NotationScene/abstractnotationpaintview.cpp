/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "abstractnotationpaintview.h"

#include <QCursor>
#include <QPainter>
#include <QMimeData>

#include "async/async.h"

#include "engraving/dom/score.h"
#include "engraving/dom/shadownote.h"

#include "notation/imasternotation.h" // IWYU pragma: keep
#include "notation/inotationaccessibility.h" // IWYU pragma: keep
#include "notation/inotationautomation.h"
#include "notation/inotationelements.h"
#include "notation/inotationnoteinput.h"
#include "notation/inotationpainting.h" // IWYU pragma: keep
#include "notation/inotationselection.h"
#include "notation/inotationstyle.h"
#include "notation/inotationviewstate.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::draw;

static constexpr qreal SCROLL_LIMIT_OFF_OVERSCROLL_FACTOR = 0.75;

AbstractNotationPaintView::AbstractNotationPaintView(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsDrops, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &AbstractNotationPaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &AbstractNotationPaintView::onViewSizeChanged);

    connect(this, &AbstractNotationPaintView::horizontalScrollChanged, [this]() {
        m_previousHorizontalScrollPosition = startHorizontalScrollPosition();
    });

    connect(this, &AbstractNotationPaintView::verticalScrollChanged, [this]() {
        m_previousVerticalScrollPosition = startVerticalScrollPosition();
    });

    m_enableAutoScrollTimer.setSingleShot(true);
    connect(&m_enableAutoScrollTimer, &QTimer::timeout, this, [this]() {
        m_autoScrollEnabled = true;
    });

    connect(&m_updatePlaybackCursorInterpolatedTimer, &QTimer::timeout, this, [this]() {
        updatePlaybackCursorInterpolated();
    });
}

AbstractNotationPaintView::~AbstractNotationPaintView()
{
    m_inputController->deinit();

    if (m_notation && isMainView()) {
        m_notation->accessibility()->setMapToScreenFunc(nullptr);
        m_notation->interaction()->setGetViewRectFunc(nullptr);
    }

    clear();
}

void AbstractNotationPaintView::load()
{
    TRACEFUNC;

    m_loadCalled = true;
    m_inputController = std::make_unique<NotationViewInputController>(this, iocContext());

    // Clip automation lines to the view bounds
    m_automationLinesContainer = new QQuickItem(this);
    m_automationLinesContainer->setClip(true);
    m_automationLinesContainer->setWidth(width());
    m_automationLinesContainer->setHeight(height());
    connect(this, &QQuickItem::widthChanged, m_automationLinesContainer, [this]() {
        m_automationLinesContainer->setWidth(width());
    });
    connect(this, &QQuickItem::heightChanged, m_automationLinesContainer, [this]() {
        m_automationLinesContainer->setHeight(height());
    });

    m_notationAutomationController = std::make_unique<NotationAutomationController>(m_automationLinesContainer, iocContext());

    m_playbackCue = std::make_unique<PlaybackCue>(iocContext());
    m_playbackCursor = std::make_unique<PlaybackCursor>(iocContext());

    m_noteInputCursor = std::make_unique<NoteInputCursor>(iocContext(), notationConfiguration()->thinNoteInputCursor());
    m_ruler = std::make_unique<NotationRuler>(iocContext());

    m_loopInMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopIn, iocContext());
    m_loopOutMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopOut, iocContext());

    m_continuousPanel = std::make_unique<ContinuousPanel>();

    m_inputController->setReadonly(m_readonly);
    m_inputController->init();

    m_elapsedTimer.start();

    onNotationSetup();

    initBackground();
    initNavigatorOrientation();

    configuration()->isLimitCanvasScrollAreaChanged().onNotify(this, [this]() {
        ensureViewportInsideScrollableArea();

        emit horizontalScrollChanged();
        emit verticalScrollChanged();
        emit viewportChanged();
    }, async::Asyncable::Mode::SetReplace);

    m_isAutomaticallyPanEnabled = notationConfiguration()->isAutomaticallyPanEnabled();
    notationConfiguration()->isAutomaticallyPanEnabledChanged().onNotify(this, [this]() {
        m_isAutomaticallyPanEnabled = notationConfiguration()->isAutomaticallyPanEnabled();
    }, async::Asyncable::Mode::SetReplace);

    m_isSmoothPanningEnabled = configuration()->isSmoothPanning();
    configuration()->isSmoothPanningChanged().onNotify(this, [this]() {
        m_isSmoothPanningEnabled = configuration()->isSmoothPanning();
    }, async::Asyncable::Mode::SetReplace);

    playbackState()->playbackPositionChanged().onReceive(this, [this](secs_t secs) {
        m_lastPlaybackPosition = secs;
        m_lastPlaybackPositionUpdateTimeNs = m_elapsedTimer.nsecsElapsed();

        if (!playbackState()->isPlaying()) {
            if (const INotationPlaybackPtr playback = notationPlayback()) {
                movePlaybackCursor(playback->secToTick(secs));
            }
        }
    }, async::Asyncable::Mode::SetReplace);

    playbackState()->isPlayingChanged().onReceive(this, [this](bool) {
        onPlayingChanged();
    }, async::Asyncable::Mode::SetReplace);

    scheduleRedraw();
}

void AbstractNotationPaintView::initBackground()
{
    emit backgroundColorChanged(notationConfiguration()->backgroundColor());

    notationConfiguration()->backgroundChanged().onNotify(this, [this]() {
        emit backgroundColorChanged(notationConfiguration()->backgroundColor());
        scheduleRedraw();
    }, async::Asyncable::Mode::SetReplace);
}

void AbstractNotationPaintView::initNavigatorOrientation()
{
    notationConfiguration()->canvasOrientation().ch.onReceive(this, [this](muse::Orientation) {
        moveCanvasToPosition(PointF(0, 0));
    }, async::Asyncable::Mode::SetReplace);
}

void AbstractNotationPaintView::moveCanvasToCenter()
{
    TRACEFUNC;

    if (!isInited()) {
        return;
    }

    PointF canvasCenter = this->canvasCenter();
    Transform oldMatrix = m_matrix;

    if (doMoveCanvas(canvasCenter.x(), canvasCenter.y())) {
        onMatrixChanged(oldMatrix, m_matrix, false);
    }
}

void AbstractNotationPaintView::scrollHorizontal(qreal position)
{
    TRACEFUNC;

    qreal scrollStep = position - m_previousHorizontalScrollPosition;
    if (qFuzzyIsNull(scrollStep)) {
        return;
    }

    qreal dx = horizontalScrollableSize() * scrollStep;
    moveCanvasHorizontal(-dx);
}

void AbstractNotationPaintView::scrollVertical(qreal position)
{
    TRACEFUNC;

    qreal scrollStep = position - m_previousVerticalScrollPosition;
    if (qFuzzyIsNull(scrollStep)) {
        return;
    }

    qreal dy = verticalScrollableSize() * scrollStep;
    moveCanvasVertical(-dy);
}

void AbstractNotationPaintView::zoomIn()
{
    m_inputController->zoomIn();
}

void AbstractNotationPaintView::zoomOut()
{
    m_inputController->zoomOut();
}

void AbstractNotationPaintView::selectOnNavigationActive()
{
    TRACEFUNC;

    if (!notation()) {
        return;
    }

    auto interaction = notation()->interaction();
    if (!interaction->selection()->isNone()) {
        return;
    }

    interaction->select(SelectionTarget::FirstItem);
}

void AbstractNotationPaintView::onCurrentNotationChanged()
{
    TRACEFUNC;

    if (m_notation) {
        onUnloadNotation(m_notation);
    }

    setNotation(projectSession()->currentNotation());

    if (!m_notation) {
        return;
    }

    onLoadNotation(m_notation);
}

void AbstractNotationPaintView::onLoadNotation(INotationPtr)
{
    if (viewport().isValid() && !m_notation->viewState()->isMatrixInited()) {
        initZoomAndPosition();
    }

    if (publishMode()) {
        m_notation->painting()->setViewMode(ViewMode::PAGE);
    } else {
        m_notation->painting()->setViewMode(m_notation->viewState()->viewMode());
    }

    m_notation->notationChanged().onReceive(this, [this](const RectF& updateRect) {
        m_pageCache.clear();
        updateLoopMarkers();
        updateShadowNoteVisibility();
        refreshPlaybackCursors();
        scheduleRedraw(updateRect.isValid() ? fromLogical(updateRect) : RectF());
    });

    // Layout may delete the cached page before notationChanged is sent
    m_notation->elements()->msScore()->changesChannel().onReceive(this, [this](const engraving::ScoreChanges&) {
        m_pageCache.clear();
    });

    onNoteInputStateChanged();
    if (isNoteEnterMode()) {
        emit activeFocusRequested();
    }

    INotationInteractionPtr interaction = notationInteraction();

    interaction->noteInput()->stateChanged().onNotify(this, [this]() {
        onNoteInputStateChanged();
    });

    interaction->noteInput()->noteInputStarted().onReceive(this, [this](bool focusNotation) {
        if (focusNotation) {
            emit activeFocusRequested();
        }
    });

    interaction->selectionChanged().onNotify(this, [this]() {
        updatePlaybackCue();
        scheduleRedraw();
    });

    // With nothing selected, playing from selection starts at the last element clicked, which can change
    // without the selection changing (e.g. clicking a note during playback only seeks)
    interaction->selection()->lastElementHitChanged().onNotify(this, [this]() {
        updatePlaybackCue();
    });

    interaction->showItemRequested().onReceive(this, [this](const INotationInteraction::ShowItemRequest& request) {
        onShowItemRequested(request);
    });

    interaction->textEditingStarted().onNotify(this, [this]() {
        setFlag(ItemAcceptsInputMethod, true);
        setFocus(false); // Remove focus once so that the IME reloads the state
        forceFocusIn();
    });

    interaction->textEditingEnded().onReceive(this, [this](const engraving::TextBase*) {
        setFlag(ItemAcceptsInputMethod, false);
        setFocus(false); // Remove focus once so that the IME reloads the state
        forceFocusIn();
    });

    interaction->dropChanged().onNotify(this, [this]() {
        if (!hasActiveFocus()) {
            forceFocusIn(); // grab keyboard focus after element added from palette
        }
    });

    interaction->shadowNoteChanged().onReceive(this, [this](bool visible) {
        if (m_shadowNoteRect.isValid()) {
            scheduleRedraw(m_shadowNoteRect);
        }

        if (m_previewMeasureRect.isValid()) {
            scheduleRedraw(m_previewMeasureRect);
        }

        if (!visible) {
            m_shadowNoteRect = RectF();
            m_previewMeasureRect = RectF();
            return;
        }

        RectF shadowNoteRect = fromLogical(notationInteraction()->shadowNoteRect());

        if (shadowNoteRect.isValid()) {
            scheduleRedraw(shadowNoteRect);
        }

        m_shadowNoteRect = shadowNoteRect;

        RectF previewMeasureRect = fromLogical(notationInteraction()->previewMeasureRect());

        if (previewMeasureRect.isValid()) {
            scheduleRedraw(previewMeasureRect);
        }

        m_previewMeasureRect = previewMeasureRect;
    });

    updateLoopMarkers();
    // FIXME: only un-/re-subscribe when master notation changes
    notationPlayback()->loopBoundariesChanged().onNotify(this, [this]() {
        updateLoopMarkers();
        updatePlaybackCue();
        scheduleRedraw();
    });

    m_notation->viewModeChanged().onNotify(this, [this]() {
        ensureViewportInsideScrollableArea();
    });

    // FIXME: only un-/re-subscribe when master notation changes
    m_notationAutomationController->init();
    notationAutomation()->automationModeEnabledChanged().onNotify(this, [this]() {
        scheduleRedraw();
        emit automationModeChanged();
    });

    if (isMainView()) {
        connect(this, &QQuickPaintedItem::focusChanged, this, [this](bool focused) {
            if (notation()) {
                notation()->accessibility()->setEnabled(focused);
            }
        });

        notation()->accessibility()->setMapToScreenFunc([this](const RectF& elementRect) {
            if (elementRect.isEmpty()) {
                return RectF(PointF::fromQPointF(mapToGlobal({ 0, 0 })), SizeF(width(), height()));
            }

            auto res = fromLogical(elementRect);
            res = RectF(PointF::fromQPointF(mapToGlobal(res.topLeft().toQPointF())), SizeF(res.width(), res.height()));

            return res;
        });

        notation()->interaction()->setGetViewRectFunc([this]() {
            return viewport();
        });
    }

    async::Async::call(this, [this](){
        forceFocusIn();
    });

    movePlaybackCursor(notationPlayback()->secToTick(playbackState()->playbackPosition()));
    updatePlaybackCue();
    updatePlaybackCursorsVisibility();

    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();
    emit automationModeChanged();
}

void AbstractNotationPaintView::onUnloadNotation(INotationPtr)
{
    m_pageCache.clear();

    m_notation->notationChanged().disconnect(this);
    m_notation->elements()->msScore()->changesChannel().disconnect(this);
    INotationInteractionPtr interaction = m_notation->interaction();
    interaction->noteInput()->stateChanged().disconnect(this);
    interaction->noteInput()->noteInputStarted().disconnect(this);
    interaction->selectionChanged().disconnect(this);
    interaction->selection()->lastElementHitChanged().disconnect(this);
    interaction->showItemRequested().disconnect(this);
    interaction->textEditingStarted().disconnect(this);
    interaction->textEditingEnded().disconnect(this);
    interaction->dropChanged().disconnect(this);
    interaction->shadowNoteChanged().disconnect(this);
    notationPlayback()->loopBoundariesChanged().disconnect(this);
    m_notation->viewModeChanged().disconnect(this);
    notationAutomation()->automationModeEnabledChanged().disconnect(this);

    if (isMainView()) {
        disconnect(this, &QQuickPaintedItem::focusChanged, this, nullptr);
        m_notation->accessibility()->setMapToScreenFunc(nullptr);
        m_notation->interaction()->setGetViewRectFunc(nullptr);
    }
}

void AbstractNotationPaintView::initZoomAndPosition()
{
}

void AbstractNotationPaintView::setMatrix(const Transform& matrix)
{
    if (m_matrix == matrix) {
        return;
    }

    Transform oldMatrix = m_matrix;
    m_matrix = matrix;

    // If `ensureViewportInsideScrollableArea` returns true, it has already
    // notified about matrix changed, so no need to do it again.
    if (ensureViewportInsideScrollableArea()) {
        return;
    }

    onMatrixChanged(oldMatrix, m_matrix, false);
}

void AbstractNotationPaintView::onMatrixChanged(const Transform& oldMatrix, const Transform& newMatrix, bool overrideZoomType)
{
    UNUSED(overrideZoomType);

    Transform oldMatrixInverted = oldMatrix.inverted();

    if (m_shadowNoteRect.isValid()) {
        RectF logicRect = oldMatrixInverted.map(m_shadowNoteRect);
        m_shadowNoteRect = newMatrix.map(logicRect);
    }

    if (m_previewMeasureRect.isValid()) {
        RectF logicRect = oldMatrixInverted.map(m_previewMeasureRect);
        m_previewMeasureRect = newMatrix.map(logicRect);
    }

    if (m_notationAutomationController) {
        m_notationAutomationController->setViewMatrix(newMatrix);
    }

    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit matrixChanged();
    emit viewportChanged();

    // Where the playback cursors appear in the view depends on its transform and size
    updatePlaybackCursorItemsGeometry();
}

void AbstractNotationPaintView::onViewSizeChanged()
{
    TRACEFUNC;

    if (!notation()) {
        return;
    }

    if (viewport().isValid()) {
        if (!notation()->viewState()->isMatrixInited()) {
            initZoomAndPosition();
        } else {
            m_inputController->updateZoomAfterSizeChange();
        }
    }

    ensureViewportInsideScrollableArea();

    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();

    // Where the playback cursors appear in the view depends on its transform and size
    updatePlaybackCursorItemsGeometry();
}

void AbstractNotationPaintView::updateLoopMarkers()
{
    TRACEFUNC;

    const LoopBoundaries& loop = notationPlayback()->loopBoundaries();

    m_loopInMarker->setVisible(loop.enabled);
    m_loopOutMarker->setVisible(loop.enabled);

    if (loop.enabled) {
        m_loopInMarker->updatePosition(loop.loopInTick);
        m_loopOutMarker->updatePosition(loop.loopOutTick);
    }
}

void AbstractNotationPaintView::updateShadowNoteVisibility()
{
    INotationInteractionPtr interaction = notationInteraction();
    const engraving::ShadowNote* shadowNote = interaction ? interaction->shadowNote() : nullptr;
    if (!shadowNote || !shadowNote->visible()) {
        m_shadowNoteRect = RectF();
        m_previewMeasureRect = RectF();
        return;
    }

    if (isNoteEnterMode()) {
        //! NOTE: The following may actually hide the shadow note
        //! if cursorPos is no longer valid...
        const QPointF cursorPos = mapFromGlobal(QCursor::pos());
        interaction->showShadowNoteForPosition(toLogical(cursorPos));
    } else {
        interaction->hideShadowNote();
        m_shadowNoteRect = RectF();
        m_previewMeasureRect = RectF();
        return;
    }
}

bool AbstractNotationPaintView::showPlaybackCursorInNormalMode() const
{
    return notationConfiguration()->showPlaybackCursorInNormalMode();
}

bool AbstractNotationPaintView::showPlaybackCueInNormalMode() const
{
    return notationConfiguration()->showPlaybackCueInNormalMode();
}

NotationViewInputController* AbstractNotationPaintView::inputController() const
{
    return m_inputController.get();
}

INotationPtr AbstractNotationPaintView::notation() const
{
    return m_notation;
}

INotationInteractionPtr AbstractNotationPaintView::notationInteraction() const
{
    return notation() ? notation()->interaction() : nullptr;
}

INotationPlaybackPtr AbstractNotationPaintView::notationPlayback() const
{
    return notation() ? notation()->masterNotation()->playback() : nullptr;
}

QQuickItem* AbstractNotationPaintView::asItem()
{
    return this;
}

INotationNoteInputPtr AbstractNotationPaintView::notationNoteInput() const
{
    return notationInteraction() ? notationInteraction()->noteInput() : nullptr;
}

INotationElementsPtr AbstractNotationPaintView::notationElements() const
{
    return notation() ? notation()->elements() : nullptr;
}

INotationStylePtr AbstractNotationPaintView::notationStyle() const
{
    return notation() ? notation()->style() : nullptr;
}

INotationSelectionPtr AbstractNotationPaintView::notationSelection() const
{
    return notationInteraction() ? notationInteraction()->selection() : nullptr;
}

INotationAutomationPtr AbstractNotationPaintView::notationAutomation() const
{
    return m_notation ? m_notation->masterNotation()->automation() : nullptr;
}

void AbstractNotationPaintView::onNoteInputStateChanged()
{
    TRACEFUNC;

    setAcceptHoverEvents(isNoteEnterMode());
    updateLoopMarkers();
    updateShadowNoteVisibility();
    updatePlaybackCursorsVisibility();

    scheduleRedraw();
}

void AbstractNotationPaintView::onShowItemRequested(const INotationInteraction::ShowItemRequest& request)
{
    IF_ASSERT_FAILED(request.item) {
        return;
    }

    RectF viewRect = viewport();

    RectF showRect = request.showRect;
    RectF itemBoundingRect = request.item->canvasBoundingRect();

    if (viewRect.width() < showRect.width()) {
        showRect.setLeft(itemBoundingRect.x());
        showRect.setWidth(itemBoundingRect.width());
    }

    if (viewRect.height() < showRect.height()) {
        showRect.setTop(itemBoundingRect.y());
        showRect.setHeight(itemBoundingRect.height());
    }

    adjustCanvasPosition(showRect);
}

bool AbstractNotationPaintView::isNoteEnterMode() const
{
    return notationNoteInput() ? notationNoteInput()->isNoteInputMode() : false;
}

void AbstractNotationPaintView::showContextMenu(const ElementType& elementType, const QPointF& pos)
{
    TRACEFUNC;

    if (m_readonly) {
        return;
    }

    QPointF _pos = pos;
    if (_pos.isNull()) {
        _pos = QPointF(width() / 2, height() / 2);
    }

    emit showContextMenuRequested(static_cast<int>(elementType), _pos);
}

void AbstractNotationPaintView::hideContextMenu()
{
    TRACEFUNC;

    if (m_isContextMenuOpen) {
        emit hideContextMenuRequested();
    }
}

void AbstractNotationPaintView::showSearch()
{
    emit showSearchRequested();
}

void AbstractNotationPaintView::showElementPopup(const ElementType& elementType)
{
    TRACEFUNC;

    const PopupModelType modelType = AbstractElementPopupModel::modelTypeFromElement(elementType);
    if (m_currentElementPopupType == modelType) {
        // Don't do anything if a popup of this type is already open...
        return;
    }

    m_currentElementPopupType = modelType;
    emit showElementPopupRequested(modelType);
}

void AbstractNotationPaintView::hideElementPopup(const ElementType& elementType)
{
    TRACEFUNC;

    hideElementPopup(AbstractElementPopupModel::modelTypeFromElement(elementType));
}

void AbstractNotationPaintView::hideElementPopup(PopupModelType modelType)
{
    TRACEFUNC;

    if (m_currentElementPopupType == PopupModelType::TYPE_UNDEFINED) {
        // Popup is already hidden...
        return;
    }

    // Hide the popup if the model type matches the currently open model type, or if no element type was specified
    if (modelType == m_currentElementPopupType || modelType == PopupModelType::TYPE_UNDEFINED) {
        m_currentElementPopupType = PopupModelType::TYPE_UNDEFINED;
        emit hideElementPopupRequested();
    }
}

void AbstractNotationPaintView::toggleElementPopup(const ElementType& elementType)
{
    if (m_currentElementPopupType != PopupModelType::TYPE_UNDEFINED) {
        hideElementPopup(elementType);
        return;
    }

    showElementPopup(elementType);
}

bool AbstractNotationPaintView::elementPopupIsOpen(const ElementType& elementType) const
{
    const PopupModelType modelType = AbstractElementPopupModel::modelTypeFromElement(elementType);
    return m_currentElementPopupType == modelType;
}

void AbstractNotationPaintView::paint(QPainter* qp)
{
    TRACEFUNC;

    RectF rect = RectF::fromQRectF(qp->clipBoundingRect());
    rect = correctDrawRect(rect);

    muse::draw::Painter mup(qp, objectName().toStdString());
    muse::draw::Painter* painter = &mup;

    paintBackground(rect, painter);

    if (!isInited()) {
        return;
    }

    qreal guiScaling = notationContextConfiguration()->guiScaling();
    Transform guiScalingCompensation;
    guiScalingCompensation.scale(guiScaling, guiScaling);

    painter->setWorldTransform(m_matrix * guiScalingCompensation);

    const bool isPrinting = publishMode() || m_inputController->readonly();
    const bool isAutomation = automationMode();
    notation()->painting()->paintView(painter, toLogical(rect), isPrinting, isAutomation);

    const INotationNoteInputPtr noteInput = notationNoteInput();
    if (noteInput->isNoteInputMode() && !publishMode()) {
        if (noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION)
            && !notationConfiguration()->useNoteInputCursorInInputByDuration()) {
            m_ruler->paint(painter, noteInput->state());
        } else {
            m_noteInputCursor->paint(painter);
        }
    }

    m_loopInMarker->paint(painter);
    m_loopOutMarker->paint(painter);

    if (notation()->viewMode() == engraving::LayoutMode::LINE) {
        ContinuousPanel::NotationViewContext nvCtx;
        nvCtx.xOffset = m_matrix.dx();
        nvCtx.yOffset = m_matrix.dy();
        nvCtx.scaling = currentScaling();
        nvCtx.fromLogical = [this](const PointF& pos) -> PointF { return fromLogical(pos); };

        engraving::rendering::PaintOptions opt;
        opt.invertColors = notationConfiguration()->shouldInvertScore();
        m_continuousPanel->paint(*painter, nvCtx, opt);
    }
}

void AbstractNotationPaintView::onNotationSetup()
{
    TRACEFUNC;
    onCurrentNotationChanged();
    onPlayingChanged();

    projectSession()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    notationConfiguration()->foregroundChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });

    notationConfiguration()->notationColorChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        scheduleRedraw();
    }, async::Asyncable::Mode::SetReplace);

    engravingConfiguration()->selectionColorChanged().onReceive(this, [this](voice_idx_t, const muse::draw::Color&) {
        scheduleRedraw();
    });

    engravingConfiguration()->formattingColorChanged().onReceive(this, [this](const Color&) {
        scheduleRedraw();
    });

    engravingConfiguration()->invisibleColorChanged().onReceive(this, [this](const Color&) {
        scheduleRedraw();
    });

    engravingConfiguration()->unlinkedColorChanged().onReceive(this, [this](const Color&) {
        scheduleRedraw();
    });

    engravingConfiguration()->debuggingOptionsChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });

    notationConfiguration()->showPlaybackCueInNormalModeChanged().onReceive(this, [this](bool) {
        updatePlaybackCursorsVisibility();
        emit showPlaybackCueInNormalModeChanged();
    });

    notationConfiguration()->showPlaybackCursorInNormalModeChanged().onReceive(this, [this](bool) {
        updatePlaybackCursorsVisibility();
        emit showPlaybackCursorInNormalModeChanged();
    });

    notationConfiguration()->playbackCueColorChanged().onNotify(this, [this]() {
        if (m_playbackCueItem) {
            m_playbackCueItem->setProperty("color", notationConfiguration()->playbackCueColor());
        }
    });

    notationConfiguration()->playbackCursorColorChanged().onNotify(this, [this]() {
        if (m_playbackCursorItem) {
            m_playbackCursorItem->setProperty("color", notationConfiguration()->playbackCursorColor());
        }
    });
}

void AbstractNotationPaintView::paintBackground(const RectF& rect, muse::draw::Painter* painter)
{
    TRACEFUNC;

    const QPixmap& wallpaper = notationConfiguration()->backgroundWallpaper();

    if (notationConfiguration()->backgroundUseColor() || wallpaper.isNull()) {
        painter->fillRect(rect, notationConfiguration()->backgroundColor());
    } else {
        painter->drawTiledPixmap(rect, wallpaper, rect.topLeft() - PointF(m_matrix.m31(), m_matrix.m32()));
    }
}

PointF AbstractNotationPaintView::canvasCenter() const
{
    TRACEFUNC;
    RectF canvasRect = m_matrix.map(notationContentRect());

    qreal canvasWidth = canvasRect.width();
    qreal canvasHeight = canvasRect.height();

    qreal x = (width() - canvasWidth) / 2;
    qreal y = (height() - canvasHeight) / 2;

    return toLogical(PointF(x, y));
}

std::pair<qreal, qreal> AbstractNotationPaintView::constraintCanvas(qreal dx, qreal dy) const
{
    TRACEFUNC;
    RectF scrollableArea = scrollableAreaRect();
    RectF viewport = this->viewport();

    // horizontal
    {
        qreal newLeft = viewport.left() - dx;
        if (viewport.width() > scrollableArea.width()) {
            newLeft = scrollableArea.center().x() - viewport.width() / 2;
        } else {
            newLeft = qBound(scrollableArea.left(), newLeft, scrollableArea.right() - viewport.width());
        }
        dx = viewport.left() - newLeft;
    }

    // vertical
    {
        qreal newTop = viewport.top() - dy;
        if (viewport.height() > scrollableArea.height()) {
            newTop = scrollableArea.center().y() - viewport.height() / 2;
        } else {
            newTop = qBound(scrollableArea.top(), newTop, scrollableArea.bottom() - viewport.height());
        }
        dy = viewport.top() - newTop;
    }

    return { dx, dy };
}

QVariant AbstractNotationPaintView::matrix() const
{
    return Transform::toQTransform(m_matrix);
}

PointF AbstractNotationPaintView::viewportTopLeft() const
{
    return toLogical(PointF(0.0, 0.0));
}

RectF AbstractNotationPaintView::viewport() const
{
    return toLogical(RectF(0.0, 0.0, width(), height()));
}

QRectF AbstractNotationPaintView::viewport_property() const
{
    return viewport().toQRectF();
}

RectF AbstractNotationPaintView::notationContentRect() const
{
    TRACEFUNC;
    if (!notationElements()) {
        return RectF();
    }

    RectF result;
    for (const Page* page: notationElements()->pages()) {
        result.unite(page->ldata()->bbox().translated(page->pos()));
    }

    return result;
}

RectF AbstractNotationPaintView::scrollableAreaRect() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    qreal overscrollFactor = configuration()->isLimitCanvasScrollArea() ? 0.0 : SCROLL_LIMIT_OFF_OVERSCROLL_FACTOR;

    qreal overscrollX = viewport.width() * overscrollFactor;
    qreal overscrollY = viewport.height() * overscrollFactor;

    return notationContentRect().adjusted(-overscrollX, -overscrollY, overscrollX, overscrollY);
}

qreal AbstractNotationPaintView::horizontalScrollableSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    qreal left = std::min(contentRect.left(), viewport.left());
    qreal right = std::max(contentRect.right(), viewport.right());

    qreal size = 0;
    if ((left < 0) && (right > 0)) {
        size = std::abs(left) + right;
    } else {
        size = std::abs(right) - std::abs(left);
    }

    return size;
}

qreal AbstractNotationPaintView::verticalScrollableSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    qreal top = std::min(contentRect.top(), viewport.top());
    qreal bottom = std::max(contentRect.bottom(), viewport.bottom());

    qreal size = 0;
    if ((top < 0) && (bottom > 0)) {
        size = std::abs(top) + bottom;
    } else {
        size = std::abs(bottom) - std::abs(top);
    }

    return size;
}

qreal AbstractNotationPaintView::horizontalScrollbarSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (viewport.left() < contentRect.left()
        && viewport.right() > contentRect.right()) {
        return 0;
    }

    qreal scrollableWidth = horizontalScrollableSize();
    if (qFuzzyIsNull(scrollableWidth)) {
        return 0;
    }

    return viewport.width() / scrollableWidth;
}

qreal AbstractNotationPaintView::verticalScrollbarSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (viewport.top() < contentRect.top()
        && viewport.bottom() > contentRect.bottom()) {
        return 0;
    }

    qreal scrollableHeight = verticalScrollableSize();
    if (qFuzzyIsNull(scrollableHeight)) {
        return 0;
    }

    return viewport.height() / scrollableHeight;
}

qreal AbstractNotationPaintView::startHorizontalScrollPosition() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.left() - contentRect.left()) / contentRect.width();
}

qreal AbstractNotationPaintView::startVerticalScrollPosition() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.top() - contentRect.top()) / contentRect.height();
}

bool AbstractNotationPaintView::adjustCanvasPosition(const RectF& logicRect, bool adjustVertically)
{
    TRACEFUNC;

    RectF viewRect = viewport();
    PointF pos = viewRect.topLeft();

    // Account for continuous panel if panning in continuous view
    qreal continuousPanelWidth = 0;
    if (notation()->viewMode() == engraving::LayoutMode::LINE) {
        continuousPanelWidth = m_continuousPanel->width();
    }
    viewRect.adjust(continuousPanelWidth, 0, 0, 0);

    double viewArea = viewRect.width() * viewRect.height();
    double logicRectArea = logicRect.width() * logicRect.height();

    if (viewArea < logicRectArea) {
        return false;
    }

    if (viewRect.contains(logicRect)) {
        return false;
    }

    constexpr int BORDER_SPACING_RATIO = 3;

    double _spatium = notationStyle()->styleValue(StyleId::spatium).toDouble();
    qreal border = _spatium * BORDER_SPACING_RATIO;
    qreal _scale = currentScaling();
    if (qFuzzyIsNull(_scale)) {
        _scale = 1;
    }

    PointF oldPos = pos;

    RectF showRect = logicRect;

    if (showRect.left() < viewRect.left()) {
        pos.setX(showRect.left() - border - continuousPanelWidth);
    } else if (showRect.left() > viewRect.right()) {
        pos.setX(showRect.right() - width() / _scale + border);
    } else if (viewRect.width() >= showRect.width() && showRect.right() > viewRect.right()) {
        pos.setX(showRect.left() - border - continuousPanelWidth);
    }

    if (adjustVertically) {
        if (showRect.top() < viewRect.top() && showRect.bottom() < viewRect.bottom()) {
            pos.setY(showRect.top() - border);
        } else if (showRect.top() > viewRect.bottom()) {
            pos.setY(showRect.bottom() - height() / _scale + border);
        } else if (viewRect.height() >= showRect.height() && showRect.bottom() > viewRect.bottom()) {
            pos.setY(showRect.top() - border);
        }
    }

    pos = alignToCurrentPageBorder(showRect, pos);

    if (pos == oldPos) {
        return false;
    }

    return moveCanvasToPosition(pos);
}

bool AbstractNotationPaintView::adjustCanvasPositionSmoothPan(const RectF& cursorRect)
{
    RectF viewRect = viewport();
    PointF pos(cursorRect.x() - (viewRect.width() / 2), viewRect.y());

    if (!viewport().intersects(cursorRect)) {
        pos.setY(cursorRect.y() - (viewRect.height() / 2));
    }

    return moveCanvasToPosition(pos);
}

bool AbstractNotationPaintView::ensureViewportInsideScrollableArea()
{
    TRACEFUNC;

    if (!m_notation) {
        return false;
    }

    // When the viewport is empty, which might happen for example when the view
    // is being loaded or unloaded, the scrollable area is computed without any
    // overscroll allowance, resulting in unexpected clamping of the canvas
    // position to (0, 0).
    if (size().isEmpty()) {
        return false;
    }

    auto [dx, dy] = constraintCanvas(0, 0);
    if (qFuzzyIsNull(dx) && qFuzzyIsNull(dy)) {
        return false;
    }

    Transform oldMatrix = m_matrix;
    m_matrix.translate(dx, dy);
    onMatrixChanged(oldMatrix, m_matrix, false);
    return true;
}

bool AbstractNotationPaintView::moveCanvasToPosition(const PointF& logicPos)
{
    TRACEFUNC;

    PointF viewTopLeft = viewportTopLeft();
    Transform oldMatrix = m_matrix;
    if (doMoveCanvas(viewTopLeft.x() - logicPos.x(), viewTopLeft.y() - logicPos.y())) {
        onMatrixChanged(oldMatrix, m_matrix, false);
        return true;
    }

    return false;
}

bool AbstractNotationPaintView::moveCanvas(qreal dx, qreal dy)
{
    TRACEFUNC;

    Transform oldMatrix = m_matrix;
    bool moved = doMoveCanvas(dx, dy);

    if (moved) {
        onMatrixChanged(oldMatrix, m_matrix, false);

        m_autoScrollEnabled = false;
        m_enableAutoScrollTimer.start(2000);

        hideElementPopup();
    }

    return moved;
}

bool AbstractNotationPaintView::doMoveCanvas(qreal dx, qreal dy)
{
    if (!m_notation) {
        return false;
    }

    if (qFuzzyIsNull(dx) && qFuzzyIsNull(dy)) {
        return false;
    }

    auto [correctedDX, correctedDY] = constraintCanvas(dx, dy);
    if (qFuzzyIsNull(correctedDX) && qFuzzyIsNull(correctedDY)) {
        return false;
    }

    m_matrix.translate(correctedDX, correctedDY);

    return true;
}

void AbstractNotationPaintView::scheduleRedraw(const muse::RectF& rect)
{
    muse::RectF redrawRect = correctDrawRect(rect);

    // Convert the floating-point rectangle to an integer QRect for the update() call,
    // ensuring that we cover the entire area of the original rect.
    int left = floor(redrawRect.left());
    int right = ceil(redrawRect.right());
    int top = floor(redrawRect.top());
    int bottom = ceil(redrawRect.bottom());
    QRect updateRect = QRect(left, top, right - left, bottom - top);

    update(updateRect);
}

RectF AbstractNotationPaintView::correctDrawRect(const RectF& rect) const
{
    if (!rect.isValid() || rect.isNull()) {
        return RectF(0, 0, width(), height());
    }

    // Compensate for floating point precision issues resulting in occasional
    // thin unpainted lines at the edges of the update rectangle.
    return rect.padded(1);
}

void AbstractNotationPaintView::moveCanvasVertical(qreal dy)
{
    moveCanvas(0, dy);
}

void AbstractNotationPaintView::moveCanvasHorizontal(qreal dx)
{
    moveCanvas(dx, 0);
}

qreal AbstractNotationPaintView::currentScaling() const
{
    return m_matrix.m11();
}

void AbstractNotationPaintView::setScaling(qreal scaling, const PointF& pos, bool overrideZoomType)
{
    TRACEFUNC;

    if (!m_notation) {
        return;
    }

    hideElementPopup();

    qreal currentScaling = this->currentScaling();

    IF_ASSERT_FAILED(!qFuzzyIsNull(scaling)) {
        return;
    }

    if (qFuzzyCompare(currentScaling, scaling)) {
        return;
    }

    if (qFuzzyIsNull(currentScaling)) {
        currentScaling = 1;
    }

    qreal deltaScaling = scaling / currentScaling;
    scale(deltaScaling, pos, overrideZoomType);
}

void AbstractNotationPaintView::scale(qreal factor, const PointF& pos, bool overrideZoomType)
{
    TRACEFUNC;

    if (!m_notation) {
        return;
    }

    if (qFuzzyCompare(factor, 1.0)) {
        return;
    }

    PointF pointBeforeScaling = toLogical(pos);

    Transform oldMatrix = m_matrix;
    m_matrix.scale(factor, factor);

    PointF pointAfterScaling = toLogical(pos);

    qreal dx = pointAfterScaling.x() - pointBeforeScaling.x();
    qreal dy = pointAfterScaling.y() - pointBeforeScaling.y();

    doMoveCanvas(dx, dy);

    onMatrixChanged(oldMatrix, m_matrix, overrideZoomType);
}

void AbstractNotationPaintView::pinchToZoom(qreal scaleFactor, const QPointF& pos)
{
    if (isInited()) {
        m_inputController->pinchToZoom(scaleFactor, pos);
    }
}

void AbstractNotationPaintView::wheelEvent(QWheelEvent* event)
{
    TRACEFUNC;
    if (isInited()) {
        m_inputController->wheelEvent(event);
    }
}

void AbstractNotationPaintView::forceFocusIn()
{
    TRACEFUNC;
    setFocus(true);
    emit activeFocusRequested();
    forceActiveFocus();
}

void AbstractNotationPaintView::onContextMenuIsOpenChanged(bool open)
{
    m_isContextMenuOpen = open;
}

void AbstractNotationPaintView::onElementPopupIsOpenChanged(const PopupModelType& popupType)
{
    m_currentElementPopupType = popupType;
}

void AbstractNotationPaintView::mousePressEvent(QMouseEvent* event)
{
    TRACEFUNC;
    forceFocusIn();

    if (isInited()) {
        m_inputController->mousePressEvent(event);
    }
}

void AbstractNotationPaintView::mouseMoveEvent(QMouseEvent* event)
{
    TRACEFUNC;
    if (isInited()) {
        m_inputController->mouseMoveEvent(event);
    }
}

void AbstractNotationPaintView::mouseDoubleClickEvent(QMouseEvent* event)
{
    TRACEFUNC;
    forceFocusIn();

    if (isInited()) {
        m_inputController->mouseDoubleClickEvent(event);
    }
}

void AbstractNotationPaintView::mouseReleaseEvent(QMouseEvent* event)
{
    if (isInited()) {
        m_inputController->mouseReleaseEvent(event);
    }
}

void AbstractNotationPaintView::hoverMoveEvent(QHoverEvent* event)
{
    if (isInited()) {
        m_inputController->hoverMoveEvent(event);
    }
}

void AbstractNotationPaintView::hoverLeaveEvent(QHoverEvent* event)
{
    if (isInited()) {
        m_inputController->hoverLeaveEvent(event);
    }
}

bool AbstractNotationPaintView::shortcutOverride(QKeyEvent* event)
{
    if (isInited()) {
        return m_inputController->shortcutOverrideEvent(event);
    }

    return false;
}

void AbstractNotationPaintView::keyPressEvent(QKeyEvent* event)
{
    if (isInited()) {
        m_inputController->keyPressEvent(event);
    }

    if (event->key() == m_lastAcceptedKey) {
        // required to prevent Qt-Quick from changing focus on tab
        m_lastAcceptedKey = -1;
    }

    return;
}

void AbstractNotationPaintView::keyReleaseEvent(QKeyEvent* event)
{
    if (isInited()) {
        m_inputController->keyReleaseEvent(event);
    }
}

bool AbstractNotationPaintView::event(QEvent* event)
{
    if (!isInited()) {
        return QQuickPaintedItem::event(event);
    }

    QEvent::Type eventType = event->type();
    auto keyEvent = dynamic_cast<QKeyEvent*>(event);

    bool isContextMenuEvent = ((eventType == QEvent::ShortcutOverride && keyEvent->key() == Qt::Key_Menu)
                               || eventType == QEvent::Type::ContextMenu) && hasFocus();

    if (isContextMenuEvent) {
        QContextMenuEvent* contextMenuEvent = dynamic_cast<QContextMenuEvent*>(event);
        if (contextMenuEvent && contextMenuEvent->reason() == QContextMenuEvent::Mouse
            && m_inputController->ignoreNextMouseContextMenuEvent()) {
            return true;
        }
        QPointF pos = contextMenuEvent && !contextMenuEvent->pos().isNull()
                      ? mapFromGlobal(contextMenuEvent->globalPos())
                      : fromLogical(m_inputController->selectionElementPos()).toQPointF();
        showContextMenu(m_inputController->selectionType(), pos);
        return true;
    } else if (eventType == QEvent::Type::ShortcutOverride) {
        bool shouldOverrideShortcut = shortcutOverride(keyEvent);

        if (shouldOverrideShortcut) {
            m_lastAcceptedKey = keyEvent->key();
            keyEvent->accept();
            return true;
        }
    }

    return QQuickPaintedItem::event(event);
}

void AbstractNotationPaintView::inputMethodEvent(QInputMethodEvent* event)
{
    if (isInited()) {
        m_inputController->inputMethodEvent(event);
    }
}

QVariant AbstractNotationPaintView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (isInited() && m_inputController->canHandleInputMethodQuery(query)) {
        return m_inputController->inputMethodQuery(query);
    }

    return QQuickPaintedItem::inputMethodQuery(query);
}

void AbstractNotationPaintView::dragEnterEvent(QDragEnterEvent* event)
{
    if (isInited()) {
        m_inputController->dragEnterEvent(event);
    }
}

void AbstractNotationPaintView::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (isInited()) {
        m_inputController->dragLeaveEvent(event);
    }
}

void AbstractNotationPaintView::dragMoveEvent(QDragMoveEvent* event)
{
    if (isInited()) {
        m_inputController->dragMoveEvent(event);
    }
}

void AbstractNotationPaintView::dropEvent(QDropEvent* event)
{
    if (isInited()) {
        m_inputController->dropEvent(event);
    }
}

void AbstractNotationPaintView::setNotation(INotationPtr notation)
{
    m_notation = notation;

    if (publishMode() && isNoteEnterMode()) {
        notationNoteInput()->endNoteInput(true);
    }

    if (m_loadCalled) {
        m_continuousPanel->setNotation(m_notation);
        m_playbackCue->setNotation(m_notation);
        m_playbackCursor->setNotation(m_notation);
        m_loopInMarker->setNotation(m_notation);
        m_loopOutMarker->setNotation(m_notation);

        // The cursors' QML items are separate from this view's painting, so they must be updated explicitly,
        // e.g. to hide them when the score is closed
        updatePlaybackCursorItemsGeometry();
    }
}

bool AbstractNotationPaintView::readonly() const
{
    return m_readonly;
}

void AbstractNotationPaintView::setReadonly(bool readonly)
{
    if (m_readonly == readonly) {
        return;
    }

    m_readonly = readonly;
    if (m_inputController) {
        m_inputController->setReadonly(m_readonly);
    }

    emit readonlyChanged();
}

void AbstractNotationPaintView::clear()
{
    Transform oldMatrix = m_matrix;
    m_matrix = Transform();
    m_previousHorizontalScrollPosition = 0;
    m_previousVerticalScrollPosition = 0;
    m_shadowNoteRect = RectF();
    m_previewMeasureRect = RectF();
    onMatrixChanged(oldMatrix, m_matrix, false);
}

qreal AbstractNotationPaintView::width() const
{
    return QQuickPaintedItem::width();
}

qreal AbstractNotationPaintView::height() const
{
    return QQuickPaintedItem::height();
}

PointF AbstractNotationPaintView::toLogical(const PointF& point) const
{
    return m_matrix.inverted().map(point);
}

PointF AbstractNotationPaintView::toLogical(const QPointF& point) const
{
    return toLogical(PointF::fromQPointF(point));
}

RectF AbstractNotationPaintView::toLogical(const RectF& rect) const
{
    return m_matrix.inverted().map(rect);
}

PointF AbstractNotationPaintView::fromLogical(const PointF& point) const
{
    return m_matrix.map(point);
}

RectF AbstractNotationPaintView::fromLogical(const RectF& rect) const
{
    return m_matrix.map(rect);
}

bool AbstractNotationPaintView::isInited() const
{
    if (qFuzzyIsNull(width()) || qFuzzyIsNull(height())) {
        return false;
    }

    return notation() != nullptr;
}

void AbstractNotationPaintView::onPlayingChanged()
{
    TRACEFUNC;

    bool isPlaying = playbackState()->isPlaying();

    updatePlaybackCursorsVisibility();

    m_autoScrollEnabled = true;
    m_enableAutoScrollTimer.stop();

    if (isPlaying) {
        m_lastPlaybackPosition = playbackState()->playbackPosition();
        m_lastPlaybackPositionUpdateTimeNs = m_elapsedTimer.nsecsElapsed();
        updatePlaybackCursorInterpolated();
        m_updatePlaybackCursorInterpolatedTimer.start(23); // ~43 FPS (1024 buffer + 44100)
    } else {
        m_updatePlaybackCursorInterpolatedTimer.stop();

        // Snap to the exact position playback stopped at, since the interpolation may have overshot
        if (const INotationPlaybackPtr playback = notationPlayback()) {
            movePlaybackCursor(playback->secToTick(playbackState()->playbackPosition()));
        }

        scheduleRedraw();
    }
}

void AbstractNotationPaintView::updatePlaybackCursorsVisibility()
{
    if (!m_playbackCursor || !m_playbackCue) {
        return;
    }

    const bool isPlaying = playbackState()->isPlaying();
    const bool isNormalMode = !isNoteEnterMode();

    // Outside of playback, both cursors are only ever shown in normal mode (never in any note input mode)
    m_playbackCursor->setIsRenderable(isPlaying || (isNormalMode && showPlaybackCursorInNormalMode()));
    m_playbackCue->setIsRenderable(isNormalMode && showPlaybackCueInNormalMode());

    if (m_playbackCursorItem) {
        m_playbackCursorItem->setVisible(m_playbackCursor->isVisible());
    }

    if (m_playbackCueItem) {
        m_playbackCueItem->setVisible(m_playbackCue->isVisible());
    }

    // Whether the playback cue is drawn as flags depends on both cursors being visible
    updatePlaybackCursorItemsGeometry();
}

void AbstractNotationPaintView::updatePlaybackCue()
{
    movePlaybackCue(playbackState()->playFromSelectionStartTick());
}

void AbstractNotationPaintView::refreshPlaybackCursors()
{
    if (!m_playbackCursor || !m_playbackCue) {
        return;
    }

    // Edits can shift the selection's tick without changing the selection (e.g. measures inserted before it),
    // so the cue's tick must be queried again, not just its rect
    const bool cueChanged = m_playbackCue->refresh(playbackState()->playFromSelectionStartTick());
    const bool cursorChanged = m_playbackCursor->refresh();

    // One update covers both: their layouts depend on each other
    if (cueChanged || cursorChanged) {
        updatePlaybackCursorItemsGeometry();
    }
}

void AbstractNotationPaintView::updatePlaybackCursorInterpolated()
{
    const INotationPlaybackPtr playback = notationPlayback();
    if (!playback) {
        return;
    }

    const qint64 elapsed = m_elapsedTimer.nsecsElapsed();
    const secs_t deltaSecs = (elapsed - m_lastPlaybackPositionUpdateTimeNs) / 1e9;
    const secs_t estimatedSecs = m_lastPlaybackPosition + deltaSecs;
    const midi::tick_t estimatedTick = playback->secToTick(estimatedSecs);

    movePlaybackCursor(estimatedTick);
}

void AbstractNotationPaintView::movePlaybackCue(midi::tick_t newTick)
{
    TRACEFUNC;

    RectF oldCursorRect = m_playbackCue->rect();
    m_playbackCue->move(newTick);

    if (m_playbackCue->rect() != oldCursorRect) {
        onPlaybackCueRectChanged();
    }
}

void AbstractNotationPaintView::movePlaybackCursor(midi::tick_t newTick)
{
    TRACEFUNC;

    RectF oldCursorRect = m_playbackCursor->rect();
    m_playbackCursor->move(newTick);
    const RectF& newCursorRect = m_playbackCursor->rect();

    if (newCursorRect != oldCursorRect) {
        onPlaybackCursorRectChanged();
    }

    if (!playbackState()->isPlaying() || newCursorRect.isNull()) {
        return;
    }

    if (m_isAutomaticallyPanEnabled) {
        if ((notation()->viewMode() == engraving::LayoutMode::LINE) && m_isSmoothPanningEnabled
            && adjustCanvasPositionSmoothPan(newCursorRect)) {
            return;
        }

        if (m_autoScrollEnabled) {
            bool adjustVertically = shouldAdjustCanvasVerticallyDuringPlayback(newCursorRect);
            if (adjustCanvasPosition(newCursorRect, adjustVertically)) {
                return;
            }
        }
    }
}

bool AbstractNotationPaintView::shouldAdjustCanvasVerticallyDuringPlayback(const RectF& cursorRect)
{
    if (!viewport().intersects(cursorRect)) {
        return true;
    }

    const Page* page = pageByPoint(cursorRect.topRight());
    if (!page) {
        return false;
    }

    if (m_pageCache.adjustVerticallyDuringPlayback.has_value()) {
        return m_pageCache.adjustVerticallyDuringPlayback.value();
    }

    int nonEmptySystemCount = 0;
    m_pageCache.adjustVerticallyDuringPlayback = false;

    for (const System* system : page->systems()) {
        if (!system->staves().empty()) {
            nonEmptySystemCount++;
        }

        if (nonEmptySystemCount > 1) {
            m_pageCache.adjustVerticallyDuringPlayback = true;
            break;
        }
    }

    return m_pageCache.adjustVerticallyDuringPlayback.value();
}

// The two cursors' layouts depend on each other (see updatePlaybackCursorItemsGeometry), so a change to either
// one re-lays out both
void AbstractNotationPaintView::onPlaybackCueRectChanged()
{
    updatePlaybackCursorItemsGeometry();
}

void AbstractNotationPaintView::onPlaybackCursorRectChanged()
{
    updatePlaybackCursorItemsGeometry();
}

//! Cursor rect in view (pixel) coordinates, not yet clipped to the view. Empty if the cursor has no position.
QRectF AbstractNotationPaintView::cursorViewRect(const SystemCursor* cursor) const
{
    if (!cursor || cursor->rect().isNull()) {
        return QRectF();
    }

    QRectF cursorRect = fromLogical(cursor->rect()).toQRectF();
    cursorRect.setWidth(std::max(std::round(cursorRect.width()), 1.0)); // makes it move more smoothly

    return cursorRect;
}

static void setItemGeometry(QQuickItem* item, const QRectF& rect)
{
    item->setX(rect.x());
    item->setY(rect.y());
    item->setSize(rect.size());
}

//! Normally the playback cursor and the playback cue are each drawn as a full-height bar. When both are visible
//! and they overlap (which is the usual case after selecting something, since the playback position follows the
//! selection), drawing both bars would either hide one or blend their colors. Instead, the playback cue is drawn as
//! two short "flags" covering the top and bottom ends of the playback cursor's bar, and the playback cursor keeps
//! the middle. Together they take the same space as the playback cursor alone. If an end is outside the view, its
//! flag is pinned to the view's edge.
void AbstractNotationPaintView::updatePlaybackCursorItemsGeometry()
{
    // Each flag's share of the playback cursor's full height, so the flags scale with zoom
    static constexpr qreal FLAG_HEIGHT_RATIO = 0.1; // a percentage

    const QRectF viewRect(0.0, 0.0, width(), height());
    const QRectF cueRect = cursorViewRect(m_playbackCue.get());
    const QRectF headRect = cursorViewRect(m_playbackCursor.get());

    const bool bothVisible = m_playbackCue && m_playbackCue->isVisible()
                             && m_playbackCursor && m_playbackCursor->isVisible();

    // A cursor's rect spans its whole system, so two cursors in the same system have the same top and bottom.
    // Merely intersecting isn't enough: the rects of cursors in neighboring systems can overlap in the margin
    // between the systems.
    static constexpr qreal SAME_Y_TOLERANCE = 0.5; // in view pixels

    const bool sameSystem = (std::abs(cueRect.top() - headRect.top()) < SAME_Y_TOLERANCE)
                            && (std::abs(cueRect.bottom() - headRect.bottom()) < SAME_Y_TOLERANCE);
    const bool sameSpot = sameSystem
                          && (cueRect.left() < headRect.right())
                          && (headRect.left() < cueRect.right());

    // Both must be at least partly on screen, or the flags would mark a cursor that isn't in view
    const bool flagged = bothVisible
                         && sameSpot
                         && headRect.intersects(viewRect)
                         && cueRect.intersects(viewRect);

    if (!flagged) {
        if (m_playbackCueItem) {
            m_playbackCueItem->setProperty("flagged", false);
            setItemGeometry(m_playbackCueItem, cueRect.intersected(viewRect));
        }

        if (m_playbackCursorItem) {
            setItemGeometry(m_playbackCursorItem, headRect.intersected(viewRect));
        }

        return;
    }

    // The flags take the two ends of the playback cursor's bar; the cursor keeps the middle.
    // Clipping first keeps both flags on screen when the cursor runs off the view's edge.
    const QRectF visibleHead = headRect.intersected(viewRect);
    // Sized from the full height, so the flags don't shrink as the cursor scrolls out of view.
    // The cap keeps a mostly off-screen bar in three parts.
    const qreal flagHeight = std::min(headRect.height() * FLAG_HEIGHT_RATIO, visibleHead.height() / 3);

    if (m_playbackCursorItem) {
        setItemGeometry(m_playbackCursorItem, visibleHead.adjusted(0, flagHeight, 0, -flagHeight));
    }

    if (m_playbackCueItem) {
        m_playbackCueItem->setProperty("flagged", true);
        m_playbackCueItem->setProperty("flagHeight", flagHeight);
        setItemGeometry(m_playbackCueItem, visibleHead);
    }
}

const Page* AbstractNotationPaintView::pageByPoint(const PointF& point) const
{
    if (m_pageCache.page) {
        const RectF r = m_pageCache.page->ldata()->bbox().translated(m_pageCache.page->pos());
        if (r.contains(point)) {
            return m_pageCache.page;
        }
    }

    m_pageCache.clear();

    INotationElementsPtr elements = notationElements();
    m_pageCache.page = elements ? elements->pageByPoint(point) : nullptr;

    return m_pageCache.page;
}

PointF AbstractNotationPaintView::alignToCurrentPageBorder(const RectF& showRect, const PointF& pos) const
{
    TRACEFUNC;

    PointF result = pos;
    const Page* page = pageByPoint(showRect.topLeft());
    if (!page) {
        return result;
    }

    RectF viewRect = viewport();

    if (result.x() < page->x() || viewRect.width() >= page->width()) {
        result.setX(page->x());
    } else if (viewRect.width() < page->width() && viewRect.width() + pos.x() > page->width() + page->x()) {
        result.setX((page->width() + page->x()) - viewRect.width());
    }
    if (result.y() < page->y() || viewRect.height() >= page->height()) {
        result.setY(page->y());
    } else if (viewRect.height() < page->height() && viewRect.height() + pos.y() > page->height() + page->y()) {
        result.setY((page->height() + page->y()) - viewRect.height());
    }

    return result;
}

bool AbstractNotationPaintView::publishMode() const
{
    return m_publishMode;
}

void AbstractNotationPaintView::setPublishMode(bool arg)
{
    if (m_publishMode == arg) {
        return;
    }

    m_publishMode = arg;
    if (m_publishMode) {
        setReadonly(true);
    }
    emit publishModeChanged();
}

bool AbstractNotationPaintView::automationMode() const
{
    return notationAutomation() ? notationAutomation()->isAutomationModeEnabled() : false;
}

bool AbstractNotationPaintView::isMainView() const
{
    return m_isMainView;
}

void AbstractNotationPaintView::setIsMainView(bool isMainView)
{
    if (m_isMainView == isMainView) {
        return;
    }

    m_isMainView = isMainView;
    emit isMainViewChanged(m_isMainView);
}

void AbstractNotationPaintView::setPlaybackCursorItem(QQuickItem* cursor)
{
    if (m_playbackCursorItem == cursor) {
        return;
    }

    m_playbackCursorItem = cursor;

    if (m_playbackCursorItem) {
        m_playbackCursorItem->setVisible(m_playbackCursor && m_playbackCursor->isVisible());
        m_playbackCursorItem->setEnabled(false); // ignore mouse & keyboard events
        m_playbackCursorItem->setProperty("color", notationConfiguration()->playbackCursorColor());

        connect(m_playbackCursorItem, &QObject::destroyed, this, [this]() {
            m_playbackCursorItem = nullptr;
        });

        onPlaybackCursorRectChanged();
    }
}

void AbstractNotationPaintView::setPlaybackCueItem(QQuickItem* cursor)
{
    if (m_playbackCueItem == cursor) {
        return;
    }

    m_playbackCueItem = cursor;

    if (m_playbackCueItem) {
        m_playbackCueItem->setVisible(m_playbackCue && m_playbackCue->isVisible());
        m_playbackCueItem->setEnabled(false); // ignore mouse & keyboard events
        m_playbackCueItem->setProperty("color", notationConfiguration()->playbackCueColor());

        connect(m_playbackCueItem, &QObject::destroyed, this, [this]() {
            m_playbackCueItem = nullptr;
        });

        onPlaybackCueRectChanged();
    }
}
