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
#include "drawingpathcollector.h"

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QQuickWindow>
#include <QCoreApplication>
#include <QPainter>
#include <QPainterPath>
#include <QCache>
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <vector>
#include "draw/svgrenderer.h"
#include <QMimeData>
#include <QSaveFile>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSvgRenderer>
#include "project/inotationproject.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "engraving/dom/svgdrawing.h"
#include "engraving/dom/score.h"
#include "engraving/dom/page.h"

#include "async/async.h"

#include "actions/actiontypes.h"
#include "engraving/dom/shadownote.h"
#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::draw;
using namespace muse::actions;

static constexpr qreal SCROLL_LIMIT_OFF_OVERSCROLL_FACTOR = 0.75;

static void compensateFloatPart(RectF& rect)
{
    rect.adjust(-1, -1, 1, 1);
}

QString AbstractNotationPaintView::insertSvgDrawing(const QString& document, const QString& mode, const QRectF& bounds)
{
    auto n = notation();
    if (!n) {
        return QStringLiteral("Abra uma partitura.");
    }
    // The native overlay stores geometry in score canvas coordinates.
    const RectF logical(bounds.x(), bounds.y(), bounds.width(), bounds.height());
    const QString result = engraving::SvgDrawing::insert(n->elements()->msScore(), document, mode, bounds, logical, true, true, false);
    scheduleRedraw();
    return result;
}

QString AbstractNotationPaintView::nativeDrawingDocument() const
{
    const auto n = notation();
    if (n) {
        engraving::SvgDrawing::setEditing(n->elements()->msScore(), m_drawingMode);
    }
    return n ? engraving::SvgDrawing::nativeDocument(n->elements()->msScore()) : QString();
}

QString AbstractNotationPaintView::syncNativeDrawingDocument(const QString& document)
{
    const auto n = notation();
    if (!n) {
        return QCoreApplication::translate("drawing-tools", "Open a score.");
    }
    auto score = n->elements()->msScore();
    const QString error = engraving::SvgDrawing::syncNative(score, document);
    engraving::SvgDrawing::setEditing(score, m_drawingMode);
    scheduleRedraw();
    return error;
}

QVariantList AbstractNotationPaintView::nativeDrawingFragments() const
{
    const auto n = notation();
    return n ? engraving::SvgDrawing::nativeFragments(n->elements()->msScore()) : QVariantList();
}

void AbstractNotationPaintView::drawingUndo(bool redo)
{
    dispatcher()->dispatch(redo ? "action://notation/redo" : "action://notation/undo");
}

QString AbstractNotationPaintView::drawingSvgPaths(const QString& svg, const QVariantList& matrix) const
{
    if (matrix.size() != 6) {
        return {};
    }
    for (const auto& value : matrix) {
        if (!std::isfinite(value.toDouble())) {
            return {};
        }
    }
    QSvgRenderer renderer(svg.toUtf8());
    if (!renderer.isValid()) {
        return {};
    }
    mu::notationscene::drawing::PathDevice device(QTransform(matrix[0].toDouble(), matrix[1].toDouble(),
                                                             matrix[2].toDouble(), matrix[3].toDouble(), matrix[4].toDouble(),
                                                             matrix[5].toDouble()));
    QPainter painter(&device);
    renderer.render(&painter, QRectF(0, 0, 1024, 1024));
    painter.end();
    if (!device.engine.supported) {
        return {};
    }
    return QString::fromUtf8(QJsonDocument(device.engine.paths).toJson(QJsonDocument::Compact));
}

void AbstractNotationPaintView::openDrawingCharacters()
{
    dispatcher()->dispatch("show-drawing-characters");
}

QVariantList AbstractNotationPaintView::previewNativeDrawingFragments(const QString& document) const
{
    const auto n = notation();
    return n ? engraving::SvgDrawing::nativeFragments(n->elements()->msScore(), document) : QVariantList();
}

static const QImage* drawingAlphaMask(const QString& svg)
{
    // Shared by point selection and region erasing; transforms do not invalidate it.
    static QCache<QString, QImage> masks(8);
    if (const QImage* cached = masks.object(svg)) {
        return cached;
    }
    constexpr int size = 1024;
    auto image = new QImage(size, size, QImage::Format_ARGB32_Premultiplied);
    image->fill(Qt::transparent);
    QPainter qp(image);
    qp.setRenderHint(QPainter::Antialiasing);
    muse::draw::Painter painter(&qp, "drawing-hit-mask");
    muse::draw::SvgRenderer renderer(muse::ByteArray::fromQByteArray(svg.toUtf8()));
    renderer.render(&painter, RectF(0, 0, size, size));
    qp.end();
    masks.insert(svg, image);
    return image;
}

bool AbstractNotationPaintView::drawingSvgContains(const QString& svg, const QVariantList& matrix,
                                                   const QPointF& point, qreal tolerance) const
{
    if (matrix.size() != 6 || svg.size() > 16 * 1024 * 1024) {
        return false;
    }
    const QTransform transform(matrix[0].toDouble(), matrix[1].toDouble(), matrix[2].toDouble(),
                               matrix[3].toDouble(), matrix[4].toDouble(), matrix[5].toDouble());
    bool invertible = false;
    const QTransform inverse = transform.inverted(&invertible);
    if (!invertible) {
        return false;
    }
    const QPointF local = inverse.map(point);
    const QPointF reachX = inverse.map(point + QPointF(tolerance, 0)) - local;
    const QPointF reachY = inverse.map(point + QPointF(0, tolerance)) - local;
    const double reach = std::max(std::hypot(reachX.x(), reachX.y()), std::hypot(reachY.x(), reachY.y()));
    if (local.x() < -reach || local.y() < -reach || local.x() > 1 + reach || local.y() > 1 + reach) {
        return false;
    }
    const QImage* mask = drawingAlphaMask(svg);
    constexpr int size = 1024;
    const int x = std::lround(local.x() * (size - 1)), y = std::lround(local.y() * (size - 1));
    const int radius = std::clamp(int(std::ceil(reach * size)), 1, 12);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= radius * radius && x + dx >= 0 && x + dx < size && y + dy >= 0 && y + dy < size
                && qAlpha(mask->pixel(x + dx, y + dy)) > 8) {
                return true;
            }
        }
    }
    return false;
}

bool AbstractNotationPaintView::drawingSvgIntersects(const QString& svg, const QVariantList& matrix,
                                                     const QVariantList& polygon) const
{
    if (matrix.size() != 6 || svg.size() > 16 * 1024 * 1024 || polygon.size() < 3 || polygon.size() > 4096) {
        return false;
    }
    for (const auto& value : matrix) {
        if (!std::isfinite(value.toDouble())) {
            return false;
        }
    }
    const QTransform transform(matrix[0].toDouble(), matrix[1].toDouble(), matrix[2].toDouble(),
                               matrix[3].toDouble(), matrix[4].toDouble(), matrix[5].toDouble());
    bool invertible = false;
    const auto inverse = transform.inverted(&invertible);
    if (!invertible) {
        return false;
    }
    constexpr int size = 1024;
    QPolygonF local;
    for (const auto& value : polygon) {
        const auto point = value.toMap();
        const QPointF mapped = inverse.map(QPointF(point.value("x").toDouble(), point.value("y").toDouble())) * size;
        if (!std::isfinite(mapped.x()) || !std::isfinite(mapped.y())) {
            return false;
        }
        local.append(mapped);
    }
    const auto bounds = local.boundingRect().intersected(QRectF(0, 0, size, size));
    if (bounds.isEmpty()) {
        return false;
    }
    const auto mask = drawingAlphaMask(svg);
    // Scan only the polygon's filled intervals. Whitespace inside a text frame
    // must not count as ink, and a large eraser must not require many QML calls.
    std::vector<double> crossings;
    crossings.reserve(local.size());
    const int top = std::clamp(int(std::floor(bounds.top())), 0, size - 1);
    const int bottom = std::clamp(int(std::ceil(bounds.bottom())), 0, size - 1);
    for (int y = top; y <= bottom; ++y) {
        crossings.clear();
        const double scan = y + 0.5;
        for (int i = 0, previous = local.size() - 1; i < local.size(); previous = i++) {
            const auto a = local[previous], b = local[i];
            if ((a.y() <= scan && b.y() > scan) || (b.y() <= scan && a.y() > scan)) {
                crossings.push_back(a.x() + (scan - a.y()) * (b.x() - a.x()) / (b.y() - a.y()));
            }
        }
        std::sort(crossings.begin(), crossings.end());
        const auto row = reinterpret_cast<const QRgb*>(mask->constScanLine(y));
        for (size_t i = 0; i + 1 < crossings.size(); i += 2) {
            if (crossings[i + 1] < 0 || crossings[i] >= size) {
                continue;
            }
            const int left = int(std::max(0.0, std::ceil(crossings[i] - 0.5)));
            const int right = int(std::min(double(size - 1), std::floor(crossings[i + 1] - 0.5)));
            for (int x = left; x <= right; ++x) {
                if (qAlpha(row[x]) > 8) {
                    return true;
                }
            }
        }
    }
    return false;
}

QString AbstractNotationPaintView::saveSvgDrawing(const QString& svg, const QString& document, const QUrl& file) const
{
    if (!file.isLocalFile()) {
        return QStringLiteral("Escolha um arquivo local.");
    }
    const QString path = file.toLocalFile();
    QSaveFile out(path);
    const QByteArray svgBytes = svg.toUtf8();
    if (!out.open(QIODevice::WriteOnly) || out.write(svgBytes) != svgBytes.size() || !out.commit()) {
        return QStringLiteral("Não foi possível salvar o SVG.");
    }
    Q_UNUSED(document);
    return QCoreApplication::translate("drawing-tools", "SVG saved.");
}

QString AbstractNotationPaintView::loadSvgDrawing(const QUrl& file) const
{
    if (!file.isLocalFile()) {
        return {};
    }
    QFile input(file.toLocalFile());
    if (!input.open(QIODevice::ReadOnly) || input.size() > 16 * 1024 * 1024) {
        return {};
    }
    return QString::fromUtf8(input.readAll());
}

QString AbstractNotationPaintView::selectedSvgDrawing() const
{
    auto n = notation();
    if (!n) {
        return {};
    }
    auto score = n->elements()->msScore();
    QJsonObject source = QJsonDocument::fromJson(engraving::SvgDrawing::selectedDocument(score).toUtf8()).object();
    QJsonArray shapes = source.value("shapes").toArray();
    QJsonArray anchors;
    const QVariantList editingPoints = engraving::SvgDrawing::editingPoints(score);
    for (const QVariant& v : editingPoints) {
        auto point = v.toMap();
        const int si = point.value("shape").toInt(), ci = point.value("command").toInt(), field = point.value("field").toInt();
        if (si < 0 || si >= shapes.size()) {
            continue;
        }
        auto shape = shapes.at(si).toObject();
        auto commands = shape.value("commands").toArray();
        if (ci < 0 || ci >= commands.size()) {
            continue;
        }
        auto command = commands.at(ci).toArray();
        if (field < 1 || field + 1 >= command.size()) {
            continue;
        }
        const PointF view(point.value("x").toDouble(), point.value("y").toDouble());
        command[field] = view.x();
        command[field + 1] = view.y();
        commands[ci] = command;
        shape["commands"] = commands;
        shapes[si] = shape;
        point["scale"] = point.value("spatium").toDouble();
        if (!point.value("key").toString().isEmpty()) {
            const auto selected = score->selection().element();
            point["key"] = QStringLiteral("%1:%2").arg(selected ? selected->staffIdx() : 0).arg(point.value("key").toString());
        }
        // Preserve explicit editor vertices; internal spline controls stay hidden.
        point["editable"] = false;
        for (const auto& av : source.value("anchors").toArray()) {
            const auto a = av.toObject();
            if (a.value("shape").toInt() == si && a.value("command").toInt() == ci
                && a.value("field").toInt() == field && a.value("editable").toBool()) {
                point["editable"] = true;
                break;
            }
        }
        if (!point.value("key").toString().isEmpty()) {
            anchors.append(QJsonObject::fromVariantMap(point));
        }
    }
    if (!editingPoints.isEmpty()) {
        for (int si = 0; si < shapes.size(); ++si) {
            auto shape = shapes.at(si).toObject();
            if (shape.value("kind").toString() == "pencil") {
                QJsonArray points;
                for (const auto& cv : shape.value("commands").toArray()) {
                    const auto c = cv.toArray();
                    if (c.size() >= 3) {
                        points.append(QJsonObject { { "x", c.at(c.size() - 2) }, { "y", c.at(c.size() - 1) } });
                    }
                }
                shape["rawPoints"] = points;
                shapes[si] = shape;
            }
        }
        source["anchors"] = anchors;
        source["shapes"] = shapes;
    }
    return source.isEmpty() ? QString() : QString::fromUtf8(QJsonDocument(source).toJson(QJsonDocument::Compact));
}

QRectF AbstractNotationPaintView::drawingPopupBounds(QQuickItem* item) const
{
    if (!item) {
        return QRectF();
    }
    const QPointF origin=item->mapToGlobal(QPointF());
    QScreen* screen=QGuiApplication::screenAt(item->mapToGlobal(QPointF(item->width() / 2, item->height() / 2)).toPoint());
    if (!screen) {
        screen=QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return QRectF();
    }
    QRectF bounds=screen->availableGeometry();
    // A narrow floating dock is a reference point, not a clipping rectangle.
    // In an ordinary editor window, keep the popup inside its visible area.
    if (auto window=item->window(); window && window->width() >= 520 && window->height() >= 320) {
        const QRectF windowBounds(window->mapToGlobal(QPoint()), window->size());
        const QRectF visible=bounds.intersected(windowBounds);
        if (visible.width() >= 240 && visible.height() >= 160) {
            bounds=visible;
        }
    }
    return bounds.translated(-origin);
}

QVariantList AbstractNotationPaintView::drawingAnchorPoints(int subdivision) const
{
    auto n = notation();
    QVariantList points = n ? engraving::SvgDrawing::anchors(n->elements()->msScore(), subdivision) : QVariantList();
    for (QVariant& point : points) {
        QVariantMap p = point.toMap();
        p["scale"] = p.value("spatium").toDouble();
        point = p;
    }
    return points;
}

QVariantMap AbstractNotationPaintView::drawingViewTransform() const
{
    const RectF visible = viewport();
    double right = std::max(1.0, visible.right()), bottom = std::max(1.0, visible.bottom());
    if (notation()) {
        for (const auto page : notation()->elements()->msScore()->pages()) {
            const auto box = page->canvasBoundingRect();
            right = std::max(right, box.right());
            bottom = std::max(bottom, box.bottom());
        }
    }
    return { { "scale", currentScaling() }, { "x", m_matrix.dx() }, { "y", m_matrix.dy() },
        { "width", right }, { "height", bottom }, { "left", visible.x() }, { "top", visible.y() },
        { "visibleWidth", visible.width() }, { "visibleHeight", visible.height() } };
}

void AbstractNotationPaintView::setDrawingMode(bool active)
{
    if (m_drawingMode == active) {
        return;
    }
    if (active && notationInteraction()) {
        notationInteraction()->noteInput()->endNoteInput();
    }
    m_drawingMode = active;
    if (notation()) {
        engraving::SvgDrawing::setEditing(notation()->elements()->msScore(), active);
        scheduleRedraw();
    }
}

AbstractNotationPaintView::AbstractNotationPaintView(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsDrops, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &AbstractNotationPaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &AbstractNotationPaintView::onViewSizeChanged);
    connect(this, &AbstractNotationPaintView::viewportChanged, this, &AbstractNotationPaintView::drawingViewportChanged);

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
}

AbstractNotationPaintView::~AbstractNotationPaintView()
{
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
    m_playbackCursor = std::make_unique<PlaybackCursor>(iocContext());
    m_playbackCursor->setVisible(false);
    m_noteInputCursor = std::make_unique<NoteInputCursor>(iocContext(), notationConfiguration()->thinNoteInputCursor());
    m_ruler = std::make_unique<NotationRuler>(iocContext());

    m_loopInMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopIn, iocContext());
    m_loopOutMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopOut, iocContext());

    m_continuousPanel = std::make_unique<ContinuousPanel>(iocContext());

    //! NOTE For diagnostic tools
    if (!dispatcher()->isReg(this)) {
        dispatcher()->reg(this, "diagnostic-notationview-redraw", [this]() {
            scheduleRedraw();
        });
        dispatcher()->reg(this, "drawing-insert-character", [this](const ActionData& args) {
            if (!m_drawingMode || !isMainView() || args.count() < 2) {
                return;
            }
            emit drawingCharacterReady(QString::fromStdString(args.arg<std::string>(0)),
                                       QString::fromStdString(args.arg<std::string>(1)));
        });
    }

    m_inputController->setReadonly(m_readonly);
    m_inputController->init();
    dispatcher()->preDispatch().onReceive(this, [this](const ActionCode& code) {
        if (code.rfind("file-save", 0) == 0 || code == "file-close" || code == "quit"
            || code == "file-export" || code == "parts" || code == "print") {
            emit drawingCommitRequested();
        }
        if (m_drawingMode && (code.rfind("note-input", 0) == 0 || code.rfind("pad-", 0) == 0
                              || code == "toggle-insert-mode")) {
            emit drawingModeExitRequested();
        }
    });

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

    interaction->selectFirstElement(false);
}

bool AbstractNotationPaintView::canReceiveAction(const ActionCode& actionCode) const
{
    if (actionCode == "diagnostic-notationview-redraw") {
        return true;
    }

    return hasFocus();
}

void AbstractNotationPaintView::onCurrentNotationChanged()
{
    TRACEFUNC;

    if (m_notation) {
        onUnloadNotation(m_notation);
    }

    setNotation(globalContext()->currentNotation());

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

    m_notation->notationChanged().onNotify(this, [this]() {
        updateLoopMarkers();
        updateShadowNoteVisibility();
        scheduleRedraw();
        emit drawingLayoutChanged();
        emit drawingViewportChanged();
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
        emit drawingModeExitRequested();
        if (focusNotation) {
            emit activeFocusRequested();
        }
    });

    interaction->selectionChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });

    interaction->showItemRequested().onReceive(this, [this](const INotationInteraction::ShowItemRequest& request) {
        onShowItemRequested(request);
    });

    interaction->textEditingStarted().onNotify(this, [this]() {
        emit drawingModeExitRequested();
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

            if (!visible) {
                m_shadowNoteRect = RectF();
                return;
            }
        }

        RectF shadowNoteRect = fromLogical(notationInteraction()->shadowNoteRect());

        if (shadowNoteRect.isValid()) {
            compensateFloatPart(shadowNoteRect);
            scheduleRedraw(shadowNoteRect);
        }

        m_shadowNoteRect = shadowNoteRect;
    });

    updateLoopMarkers();
    // FIXME: only un-/re-subscribe when master notation changes
    notationPlayback()->loopBoundariesChanged().onNotify(this, [this]() {
        updateLoopMarkers();
    });

    m_notation->viewModeChanged().onNotify(this, [this]() {
        ensureViewportInsideScrollableArea();
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

    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();
    emit drawingLayoutChanged();
}

void AbstractNotationPaintView::onUnloadNotation(INotationPtr)
{
    m_notation->notationChanged().disconnect(this);
    INotationInteractionPtr interaction = m_notation->interaction();
    interaction->noteInput()->stateChanged().disconnect(this);
    interaction->noteInput()->noteInputStarted().disconnect(this);
    interaction->selectionChanged().disconnect(this);
    interaction->showItemRequested().disconnect(this);
    interaction->textEditingStarted().disconnect(this);
    interaction->textEditingEnded().disconnect(this);
    interaction->dropChanged().disconnect(this);
    interaction->shadowNoteChanged().disconnect(this);
    notationPlayback()->loopBoundariesChanged().disconnect(this);
    m_notation->viewModeChanged().disconnect(this);

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

    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit matrixChanged();
    emit viewportChanged();

    onPlaybackCursorRectChanged();
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

    onPlaybackCursorRectChanged();
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

    scheduleRedraw();
}

void AbstractNotationPaintView::updateShadowNoteVisibility()
{
    INotationInteractionPtr interaction = notationInteraction();
    const engraving::ShadowNote* shadowNote = interaction ? interaction->shadowNote() : nullptr;
    if (!shadowNote || !shadowNote->visible()) {
        m_shadowNoteRect = RectF();
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
        return;
    }
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

void AbstractNotationPaintView::onNoteInputStateChanged()
{
    TRACEFUNC;

    setAcceptHoverEvents(isNoteEnterMode());
    updateLoopMarkers();
    updateShadowNoteVisibility();
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
    static const bool profileNavigation = qEnvironmentVariableIsSet("MUSEDRAW_PROFILE_NAVIGATION");
    QElapsedTimer navigationTimer;
    if (profileNavigation) {
        navigationTimer.start();
    }

    RectF rect = RectF::fromQRectF(qp->clipBoundingRect());
    rect = correctDrawRect(rect);

    muse::draw::Painter mup(qp, objectName().toStdString());
    muse::draw::Painter* painter = &mup;

    paintBackground(rect, painter);

    if (!isInited()) {
        return;
    }

    qreal guiScaling = notationConfiguration()->guiScaling();
    Transform guiScalingCompensation;
    guiScalingCompensation.scale(guiScaling, guiScaling);

    painter->setWorldTransform(m_matrix * guiScalingCompensation);

    bool isPrinting = publishMode() || m_inputController->readonly();
    notation()->painting()->paintView(painter, toLogical(rect), isPrinting);
    if (profileNavigation) {
        static QElapsedTimer lastReport;
        const auto elapsed = navigationTimer.elapsed();
        if (elapsed >= 32 && (!lastReport.isValid() || lastReport.elapsed() >= 1000)) {
            LOGI() << "[MuseDraw navigation] scorePaintMs=" << elapsed
                   << " drawingMode=" << m_drawingMode << " printing=" << isPrinting;
            lastReport.start();
        }
    }

    const ui::UiContext& uiCtx = uiContextResolver()->currentUiContext();
    const bool isOnNotationPage = uiCtx == ui::UiCtxProjectOpened || uiCtx == ui::UiCtxProjectFocused;

    const INotationNoteInputPtr noteInput = notationNoteInput();
    if (noteInput->isNoteInputMode() && isOnNotationPage) {
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

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onPlayingChanged();
    });

    playbackController()->currentPlaybackPositionChanged().onReceive(this, [this](audio::secs_t, midi::tick_t tick) {
        movePlaybackCursor(tick);
    });

    notationConfiguration()->foregroundChanged().onNotify(this, [this]() {
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
    QRect qrect = correctDrawRect(rect).toQRect();
    update(qrect);
}

RectF AbstractNotationPaintView::correctDrawRect(const RectF& rect) const
{
    if (!rect.isValid() || rect.isNull()) {
        return RectF(0, 0, width(), height());
    }

    return rect;
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
    if (m_notation == notation) {
        return;
    }
    emit drawingScoreAboutToChange();
    if (m_notation) {
        // Images hidden by the editor belong to the old tab. Restore their
        // normal painting state before loading another score/individual part.
        engraving::SvgDrawing::setEditing(m_notation->elements()->msScore(), false);
    }
    m_notation = notation;
    if (m_notation) {
        engraving::SvgDrawing::setEditing(m_notation->elements()->msScore(), m_drawingMode);
    }
    emit drawingScoreChanged();

    if (m_loadCalled) {
        m_continuousPanel->setNotation(m_notation);
        m_playbackCursor->setNotation(m_notation);
        m_loopInMarker->setNotation(m_notation);
        m_loopOutMarker->setNotation(m_notation);
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

    if (!notationPlayback()) {
        return;
    }

    bool isPlaying = playbackController()->isPlaying();
    m_playbackCursor->setVisible(isPlaying);

    if (m_playbackCursorItem) {
        m_playbackCursorItem->setVisible(isPlaying);
    }

    m_autoScrollEnabled = true;
    m_enableAutoScrollTimer.stop();

    if (isPlaying) {
        audio::secs_t pos = globalContext()->playbackState()->playbackPosition();
        muse::midi::tick_t tick = notationPlayback()->secToTick(pos);
        movePlaybackCursor(tick);
    } else {
        scheduleRedraw();
    }
}

void AbstractNotationPaintView::movePlaybackCursor(muse::midi::tick_t tick)
{
    TRACEFUNC;

    if (!notationPlayback()) {
        return;
    }

    RectF oldCursorRect = m_playbackCursor->rect();
    m_playbackCursor->move(tick);
    const RectF& newCursorRect = m_playbackCursor->rect();

    if (newCursorRect != oldCursorRect) {
        onPlaybackCursorRectChanged();
    }

    if (!m_playbackCursor->visible() || newCursorRect.isNull()) {
        return;
    }

    if (m_isAutomaticallyPanEnabled) {
        if ((notation()->viewMode() == engraving::LayoutMode::LINE) && m_isSmoothPanningEnabled
            && adjustCanvasPositionSmoothPan(newCursorRect)) {
            return;
        }

        if (m_autoScrollEnabled) {
            bool adjustVertically = needAdjustCanvasVerticallyWhilePlayback(newCursorRect);
            if (adjustCanvasPosition(newCursorRect, adjustVertically)) {
                return;
            }
        }
    }
}

bool AbstractNotationPaintView::needAdjustCanvasVerticallyWhilePlayback(const RectF& cursorRect)
{
    if (!viewport().intersects(cursorRect)) {
        return true;
    }

    const Page* page = pageByPoint(cursorRect.topRight());
    if (!page) {
        return false;
    }

    int nonEmptySystemCount = 0;

    for (const System* system : page->systems()) {
        if (!system->staves().empty()) {
            nonEmptySystemCount++;
        }
    }

    return nonEmptySystemCount > 1;
}

void AbstractNotationPaintView::onPlaybackCursorRectChanged()
{
    if (!m_playbackCursor || !m_playbackCursorItem) {
        return;
    }

    QRectF cursorRect = fromLogical(m_playbackCursor->rect()).toQRectF();
    cursorRect.setWidth(std::max(std::round(cursorRect.width()), 1.0)); // makes it move more smoothly

    QRectF viewRect(0.0, 0.0, width(), height());
    QRectF newRect = cursorRect.intersected(viewRect);

    m_playbackCursorItem->setX(newRect.x());
    m_playbackCursorItem->setY(newRect.y());
    m_playbackCursorItem->setSize(newRect.size());
}

const Page* AbstractNotationPaintView::pageByPoint(const PointF& point) const
{
    INotationElementsPtr elements = notationElements();
    return elements ? elements->pageByPoint(point) : nullptr;
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
    emit publishModeChanged();
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
        m_playbackCursorItem->setVisible(playbackController()->isPlaying());
        m_playbackCursorItem->setEnabled(false); // ignore mouse & keyboard events
        m_playbackCursorItem->setProperty("color", notationConfiguration()->playbackCursorColor());

        connect(m_playbackCursorItem, &QObject::destroyed, this, [this]() {
            m_playbackCursorItem = nullptr;
        });
    }
}
