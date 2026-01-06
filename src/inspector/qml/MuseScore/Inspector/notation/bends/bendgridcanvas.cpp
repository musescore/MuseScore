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

#include "bendgridcanvas.h"

#include <cmath>

#include <QPainterPath>

#include "engraving/dom/utils.h"

#include "translation.h"
#include "log.h"

using namespace mu::inspector;
using namespace muse::ui;
using namespace muse::accessibility;

static constexpr int GRIP_RADIUS = 6;
static constexpr int GRIP_CENTER_RADIUS = GRIP_RADIUS - 2;
static constexpr int GRIP_SELECTED_RADIUS = GRIP_RADIUS + 2;
static constexpr int GRIP_FOCUS_RADIUS = GRIP_SELECTED_RADIUS + 2;

static constexpr int GRID_LINE_WIDTH = 1;
static constexpr int CURVE_LINE_WIDTH = 3;

static constexpr int INVALID_INDEX = -1;

static QPointF constrainToGrid(const QRectF& frameRectWithoutBorders, const QPointF& point)
{
    QPointF result = point;
    if (!frameRectWithoutBorders.contains(result)) {
        if (result.x() < frameRectWithoutBorders.left()) {
            result.setX(frameRectWithoutBorders.left());
        } else if (result.x() > frameRectWithoutBorders.right()) {
            result.setX(frameRectWithoutBorders.right());
        }

        if (result.y() < frameRectWithoutBorders.top()) {
            result.setY(frameRectWithoutBorders.top());
        } else if (result.y() > frameRectWithoutBorders.bottom()) {
            result.setY(frameRectWithoutBorders.bottom());
        }
    }

    return result;
}

BendGridCanvas::BendGridCanvas(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    setKeepMouseGrab(true);
}

BendGridCanvas::~BendGridCanvas()
{
    muse::DeleteAll(m_pointsAccessibleItems);
}

void BendGridCanvas::classBegin()
{
    QuickPaintedView::classBegin();

    uiConfig()->currentThemeChanged().onNotify(this, [this]() {
        update();
    });

    uiConfig()->fontChanged().onNotify(this, [this]() {
        update();
    });

    connect(this, &BendGridCanvas::enabledChanged, [this](){
        update();
    });

    qApp->installEventFilter(this);
}

QVariant BendGridCanvas::pointList() const
{
    return curvePointsToQVariant(m_points);
}

int BendGridCanvas::rowCount() const
{
    return m_rows;
}

int BendGridCanvas::columnCount() const
{
    return m_columns;
}

int BendGridCanvas::rowSpacing() const
{
    return m_primaryRowsInterval;
}

int BendGridCanvas::columnSpacing() const
{
    return m_primaryColumnsInterval;
}

int BendGridCanvas::topLineValue() const
{
    return m_topLineValue;
}

bool BendGridCanvas::showHalfs() const
{
    return m_showHalfs;
}

bool BendGridCanvas::straightLines() const
{
    return m_straightLines;
}

bool BendGridCanvas::focusOnFirstPoint()
{
    if (m_focusedPointIndex.has_value()) {
        return true;
    }

    int firstPointIndex = INVALID_INDEX;
    for (int i = 0; i < m_points.size(); ++i) {
        if (m_points[i].canMove()) {
            firstPointIndex = i;
            break;
        }
    }

    if (!isPointIndexValid(firstPointIndex)) {
        return false;
    }

    setFocusedPointIndex(firstPointIndex);

    update();

    return true;
}

bool BendGridCanvas::resetFocus()
{
    if (!m_focusedPointIndex.has_value()) {
        return false;
    }

    setFocusedPointIndex(INVALID_INDEX);

    update();

    return true;
}

bool BendGridCanvas::moveFocusedPointToLeft()
{
    if (!m_focusedPointIndex.has_value()) {
        return false;
    }

    int index = m_focusedPointIndex.value();
    CurvePoint focusedPoint = m_points.at(index);

    int newTime = focusedPoint.time - 1;
    if (newTime < 0) {
        newTime = 0;
    }

    focusedPoint.time = newTime;

    if (movePoint(index, focusedPoint)) {
        update();
        emit canvasChanged();
    }

    return true;
}

bool BendGridCanvas::moveFocusedPointToRight()
{
    if (!m_focusedPointIndex.has_value()) {
        return false;
    }

    int index = m_focusedPointIndex.value();
    CurvePoint focusedPoint = m_points.at(index);

    int newTime = focusedPoint.time + 1;
    if (newTime > CurvePoint::MAX_TIME) {
        newTime = CurvePoint::MAX_TIME;
    }

    if (newTime == focusedPoint.time) {
        return true;
    }

    focusedPoint.time = newTime;

    if (movePoint(index, focusedPoint)) {
        update();
        emit canvasChanged();
    }

    return true;
}

bool BendGridCanvas::moveFocusedPointToUp()
{
    if (!m_focusedPointIndex.has_value()) {
        return false;
    }

    int index = m_focusedPointIndex.value();
    CurvePoint focusedPoint = m_points.at(index);

    QRectF frameRect = this->frameRect();
    QPointF focusedPointCoord = pointCoord(frameRect, focusedPoint);

    int rowHeight = round(this->rowHeight(frameRect));
    focusedPointCoord.setY(focusedPointCoord.y() - rowHeight);

    if (focusedPointCoord.y() < frameRect.top()) {
        focusedPointCoord.setY(frameRect.top());
    }

    CurvePoint newPoint = this->point(frameRect, focusedPointCoord.x(), focusedPointCoord.y());
    focusedPoint.pitch = newPoint.pitch;

    if (movePoint(index, focusedPoint)) {
        update();
        emit canvasChanged();
    }

    return true;
}

bool BendGridCanvas::moveFocusedPointToDown()
{
    if (!m_focusedPointIndex.has_value()) {
        return false;
    }

    int index = m_focusedPointIndex.value();
    CurvePoint focusedPoint = m_points.at(index);

    QRectF frameRect = this->frameRect();
    QPointF focusedPointCoord = pointCoord(frameRect, focusedPoint);

    int rowHeight = round(this->rowHeight(frameRect));
    focusedPointCoord.setY(focusedPointCoord.y() + rowHeight);

    if (focusedPointCoord.y() > frameRect.bottom()) {
        focusedPointCoord.setY(frameRect.bottom());
    }

    CurvePoint newPoint = this->point(frameRect, focusedPointCoord.x(), focusedPointCoord.y());
    focusedPoint.pitch = newPoint.pitch;

    if (movePoint(index, focusedPoint)) {
        update();
        emit canvasChanged();
    }

    return true;
}

void BendGridCanvas::setRowCount(int rowCount)
{
    if (m_rows == rowCount) {
        return;
    }

    m_rows = rowCount;
    emit rowCountChanged(m_rows);
}

void BendGridCanvas::setColumnCount(int columnCount)
{
    if (m_columns == columnCount) {
        return;
    }

    m_columns = columnCount;
    emit columnCountChanged(m_columns);
}

void BendGridCanvas::setRowSpacing(int rowSpacing)
{
    if (m_primaryRowsInterval == rowSpacing) {
        return;
    }

    m_primaryRowsInterval = rowSpacing;
    emit rowSpacingChanged(m_primaryRowsInterval);
}

void BendGridCanvas::setColumnSpacing(int columnSpacing)
{
    if (m_primaryColumnsInterval == columnSpacing) {
        return;
    }

    m_primaryColumnsInterval = columnSpacing;
    emit columnSpacingChanged(m_primaryColumnsInterval);
}

void BendGridCanvas::setTopLineValue(int topLineValue)
{
    if (m_topLineValue == topLineValue) {
        return;
    }

    m_topLineValue = topLineValue;
    emit topLineValueChanged(m_topLineValue);
}

void BendGridCanvas::setShowHalfs(bool showHalfs)
{
    if (m_showHalfs == showHalfs) {
        return;
    }

    m_showHalfs = showHalfs;
    emit showHalfsChanged(m_showHalfs);
}

void BendGridCanvas::setStraightLines(bool straightLines)
{
    if (m_straightLines == straightLines) {
        return;
    }

    m_straightLines = straightLines;
    emit straightLinesChanged(m_straightLines);
}

void BendGridCanvas::setPointList(QVariant points)
{
    CurvePoints newPointList = curvePointsFromQVariant(points);

    if (m_points == newPointList) {
        return;
    }

    m_points = newPointList;

    m_pointsAccessibleItems.clear();
    for (const CurvePoint& point : m_points) {
        muse::ui::AccessibleItem* item = new muse::ui::AccessibleItem(this);
        item->setName(pointAccessibleName(point));
        item->setAccessibleParent(m_accessibleParent);
        item->setRole(MUAccessible::Role::Information);
        item->componentComplete();

        m_pointsAccessibleItems << item;
    }

    update();
    emit pointListChanged(points);
}

void BendGridCanvas::paint(QPainter* painter)
{
    if (!(m_rows && m_columns)) {
        LOGD("GridCanvas::paintEvent: number of columns or rows set to 0.\nColumns: %i, Rows: %i", m_rows,
             m_columns);
        return;
    }

    QRectF frameRect = this->frameRect();

    drawBackground(painter, frameRect);

    if (isEnabled()) {
        drawCurve(painter, frameRect);
    }
}

void BendGridCanvas::mousePressEvent(QMouseEvent* event)
{
    if (!(m_rows && m_columns)) {
        LOGD("GridCanvas::mousePressEvent: number of columns or rows set to 0.\nColumns: %i, Rows: %i", m_rows,
             m_columns);
        return;
    }

    QRectF frameRect = this->frameRect();
    QPointF coord = this->frameCoord(frameRect, event->pos().x(), event->pos().y());
    CurvePoint point = this->point(frameRect, coord.x(), coord.y());

    m_currentPointIndex = this->pointIndex(point);
    m_canvasWasChanged = false;

    update();
}

void BendGridCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_currentPointIndex.has_value()) {
        return;
    }

    QRectF frameRect = this->frameRect();
    QPointF coord = this->frameCoord(frameRect, event->pos().x(), event->pos().y());

    CurvePoint point = this->point(frameRect, coord.x(), coord.y());

    if (movePoint(m_currentPointIndex.value(), point)) {
        m_canvasWasChanged = true;
        update();
    }
}

void BendGridCanvas::mouseReleaseEvent(QMouseEvent*)
{
    m_currentPointIndex = std::nullopt;

    if (m_canvasWasChanged) {
        emit canvasChanged();
    }

    m_canvasWasChanged = false;
}

void BendGridCanvas::hoverEnterEvent(QHoverEvent*)
{
    m_hoverPointIndex = std::nullopt;
}

void BendGridCanvas::hoverMoveEvent(QHoverEvent* event)
{
    auto oldPointIndex = m_hoverPointIndex;

    QRectF frameRect = this->frameRect();
    QPointF pos = event->position();
    QPointF coord = this->frameCoord(frameRect, pos.x(), pos.y());
    CurvePoint point = this->point(frameRect, coord.x(), coord.y());

    m_hoverPointIndex = this->pointIndex(point);

    if (oldPointIndex != m_hoverPointIndex) {
        update();
    }
}

void BendGridCanvas::hoverLeaveEvent(QHoverEvent*)
{
    m_hoverPointIndex = std::nullopt;
}

bool BendGridCanvas::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Type::ShortcutOverride) {
        return shortcutOverride(dynamic_cast<QKeyEvent*>(event));
    }

    return QQuickPaintedItem::eventFilter(watched, event);
}

bool BendGridCanvas::shortcutOverride(QKeyEvent* event)
{
    if (!m_focusedPointIndex.has_value()) {
        return false;
    }

    if (!(event->modifiers() & Qt::KeyboardModifier::AltModifier)) {
        return false;
    }

    int index = m_focusedPointIndex.value();
    switch (event->key()) {
    case Qt::Key_Left:
        index--;

        if (!isPointIndexValid(index)) {
            return false;
        }

        if (!m_points.at(index).canMove()) {
            return false;
        }
        break;
    case Qt::Key_Right:
        index++;

        if (!isPointIndexValid(index)) {
            return false;
        }

        if (!m_points.at(index).canMove()) {
            return false;
        }
        break;
    default:
        return false;
    }

    setFocusedPointIndex(index);

    update();

    event->accept();
    return true;
}

QRectF BendGridCanvas::frameRect() const
{
    // not qreal here, even though elsewhere yes,
    // because width and height return a number of pixels,
    // hence integers.
    const int w = width();
    const int h = height();

    // let half a column of margin around
    const qreal margin = 12.0;
    const qreal leftPos = margin * 3.0;   // also left margin
    const qreal topPos = margin;      // also top margin
    const qreal rightPos = qreal(w) - margin;   // right end position of graph
    const qreal bottomPos = qreal(h) - margin;   // bottom end position of graph

    return QRectF(QPointF(leftPos, topPos), QPointF(rightPos, bottomPos));
}

qreal BendGridCanvas::columnWidth(const QRectF& frameRect) const
{
    return frameRect.width() / (m_columns - 1);
}

qreal BendGridCanvas::rowHeight(const QRectF& frameRect) const
{
    return frameRect.height() / (m_rows - 1);
}

QPointF BendGridCanvas::frameCoord(const QRectF& frameRect, double x, double y) const
{
    // restrict to clickable area
    if (x > frameRect.right()) {
        x = frameRect.right();
    } else if (x < frameRect.left()) {
        x = frameRect.left();
    }
    if (y > frameRect.bottom()) {
        y = frameRect.bottom();
    } else if (y < frameRect.top()) {
        y = frameRect.top();
    }

    return QPointF(x, y);
}

void BendGridCanvas::drawBackground(QPainter* painter, const QRectF& frameRect)
{
    const qreal rowHeight = this->rowHeight(frameRect);
    const qreal columnWidth = this->columnWidth(frameRect);

    const ThemeInfo& currentTheme = uiConfig()->currentTheme();
    QColor primaryLinesColor(isEnabled() ? (currentTheme.codeKey == DARK_THEME_CODE ? Qt::white : Qt::black) : Qt::gray);
    QColor secondaryLinesColor(Qt::gray);

    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor backgroundColor(currentTheme.values[BACKGROUND_PRIMARY_COLOR].toString());
    painter->fillRect(QRect(0, 0, width(), height()), backgroundColor);

    QPen pen = painter->pen();
    pen.setWidth(GRID_LINE_WIDTH);

    // draw vertical lines
    for (int i = 1; i < m_columns - 1; ++i) {
        qreal xpos = frameRect.left() + i * columnWidth;
        // lighter middle lines
        pen.setColor(i % m_primaryColumnsInterval ? secondaryLinesColor : primaryLinesColor);
        painter->setPen(pen);
        painter->drawLine(xpos, frameRect.top(), xpos, frameRect.bottom());
    }

    // draw horizontal lines
    QFont font;
    font.setFamily(QString::fromStdString(uiConfig()->fontFamily()));
    font.setPixelSize(uiConfig()->fontSize());
    int stringHeight = QFontMetrics(font).height();
    painter->setFont(font);

    int lastPrimaryRowIndex = 0;

    int interval = m_topLineValue;

    for (int i = 1; i < m_rows - 1; ++i) {
        int ypos = frameRect.top() + i * rowHeight;

        bool isPrimary = !(i % m_primaryRowsInterval);

        // lighter middle lines
        pen.setColor(isPrimary ? primaryLinesColor : secondaryLinesColor);
        painter->setPen(pen);
        painter->drawLine(frameRect.left(), ypos, frameRect.right(), ypos);

        bool negative = false;

        bool isHalf = !((i + (i - lastPrimaryRowIndex)) % m_primaryRowsInterval) && !isPrimary;

        if (isPrimary) {
            lastPrimaryRowIndex = i;
        }

        if (!isPrimary && !isHalf) {
            continue;
        }

        if (isHalf) {
            --interval;
            if (!m_showHalfs) {
                continue;
            }
        }

        pen.setColor(primaryLinesColor);
        painter->setPen(pen);

        QString intervalStr = QString::number(interval);
        if (interval == 0 && isHalf) {
            intervalStr = negative ? "-" : "";
        }

        QString text = QString("%1%2")
                       .arg(intervalStr)
                       .arg(isHalf ? "\u00BD" : "");

        QRect textRect(0, ypos - stringHeight / 2, frameRect.left(), stringHeight);

        painter->drawText(textRect, Qt::AlignCenter, text);
    }

    // draw a frame
    QPainterPath path;
    path.addRoundedRect(frameRect, 3, 3);

    pen.setColor(primaryLinesColor);
    pen.setWidth(GRID_LINE_WIDTH);
    pen.setStyle(Qt::PenStyle::SolidLine);
    painter->setPen(pen);

    painter->fillPath(path, Qt::transparent);
    painter->drawPath(path);
}

void BendGridCanvas::drawCurve(QPainter* painter, const QRectF& frameRect)
{
    const ThemeInfo& currentTheme = uiConfig()->currentTheme();
    QColor backgroundColor(currentTheme.values[BACKGROUND_PRIMARY_COLOR].toString());

    QPointF lastPoint(0, 0);
    QPen pen = painter->pen();
    pen.setWidth(CURVE_LINE_WIDTH);

    QColor color(currentTheme.values[ACCENT_COLOR].toString());
    pen.setColor(color);
    painter->setPen(pen);

    QRectF frameRectWithoutBorders = frameRect - QMargins(GRID_LINE_WIDTH, GRID_LINE_WIDTH, GRID_LINE_WIDTH, GRID_LINE_WIDTH);

    // draw line between points
    for (const CurvePoint& v : m_points) {
        QPointF currentPoint = constrainToGrid(frameRectWithoutBorders, pointCoord(frameRect, v));

        QPainterPath path;
        path.moveTo(lastPoint);

        // draw line only if there is a point before the current one
        if (lastPoint.x()) {
            QPointF point = constrainToGrid(frameRectWithoutBorders, QPointF(currentPoint.x(), lastPoint.y()));

            if (m_straightLines) {
                path.lineTo(currentPoint);
            } else {
                path.quadTo(point, currentPoint);
            }

            if (v.endDashed) {
                pen.setColor(backgroundColor);
                painter->strokePath(path, pen);
            }

            pen.setColor(color);
            pen.setStyle(v.endDashed ? Qt::PenStyle::DashLine : Qt::PenStyle::SolidLine);
            painter->strokePath(path, pen);

            pen.setStyle(Qt::PenStyle::SolidLine);
        }

        lastPoint = currentPoint;
    }

    // draw points
    QBrush backgroundBrush(backgroundColor, Qt::SolidPattern);
    QBrush activeBrush(color, Qt::SolidPattern);

    QColor hoverColor(color);
    hoverColor.setAlpha(150);
    QBrush hoverBrush(hoverColor, Qt::SolidPattern);

    painter->setPen(Qt::NoPen);

    for (int i = 0; i < m_points.size(); ++i) {
        const CurvePoint& point = m_points.at(i);
        if (!point.canMove()) {
            continue;
        }

        QPointF pos = pointCoord(frameRect, point);

        bool isNotActiveButton = (!m_hoverPointIndex.has_value() || m_hoverPointIndex.value() != i)
                                 && (!m_currentPointIndex.has_value() || m_currentPointIndex.value() != i)
                                 && (!m_focusedPointIndex.has_value() || m_focusedPointIndex.value() != i);

        if (isNotActiveButton) { // normal
            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);

            painter->setBrush(backgroundBrush);
            painter->drawEllipse(pos, GRIP_CENTER_RADIUS, GRIP_CENTER_RADIUS);
        } else if (m_focusedPointIndex.has_value() && m_focusedPointIndex.value() == i) { // focused
            QColor fontPrimaryColor(currentTheme.values[FONT_PRIMARY_COLOR].toString());
            QBrush fontPrimaryBrush(fontPrimaryColor, Qt::SolidPattern);
            painter->setBrush(fontPrimaryBrush);
            painter->drawEllipse(pos, GRIP_FOCUS_RADIUS, GRIP_FOCUS_RADIUS);

            painter->setBrush(backgroundBrush);
            painter->drawEllipse(pos, GRIP_SELECTED_RADIUS, GRIP_SELECTED_RADIUS);

            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);
        } else if (m_currentPointIndex.has_value() && m_currentPointIndex.value() == i) { // selected
            painter->setBrush(backgroundBrush);
            painter->drawEllipse(pos, GRIP_SELECTED_RADIUS, GRIP_SELECTED_RADIUS);

            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);
        } else if (m_hoverPointIndex.has_value() && m_hoverPointIndex.value() == i) { // hover
            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);

            painter->setBrush(backgroundBrush);
            painter->drawEllipse(pos, GRIP_CENTER_RADIUS, GRIP_CENTER_RADIUS);

            painter->setBrush(hoverBrush);
            painter->drawEllipse(pos, GRIP_CENTER_RADIUS, GRIP_CENTER_RADIUS);
        }
    }
}

bool BendGridCanvas::isPointIndexValid(int index) const
{
    return index >= 0 && index < m_points.size();
}

std::optional<int> BendGridCanvas::pointIndex(const CurvePoint& point, bool movable) const
{
    const int numberOfPoints = m_points.size();

    for (int i = 0; i < numberOfPoints; ++i) {
        const CurvePoint& _point = m_points.at(i);
        if (movable != _point.canMove()) {
            continue;
        }

        if (std::pow((point.time - _point.time), 2)
            + std::pow((point.pitch - _point.pitch), 2) < std::pow(GRIP_CENTER_RADIUS, 2)) {
            return i;
        }
    }

    return std::nullopt;
}

CurvePoint BendGridCanvas::point(const QRectF& frameRect, int frameX, int frameY) const
{
    CurvePoint point;
    point.time = qreal(frameX - frameRect.left()) / (frameRect.width() / CurvePoint::MAX_TIME);

    const qreal rowHeight = this->rowHeight(frameRect);
    int rowFromTop = round(qreal(frameY - frameRect.top()) / rowHeight);

    point.pitch = 100 * m_topLineValue - 100 * rowFromTop / m_primaryRowsInterval;

    return point;
}

QPointF BendGridCanvas::pointCoord(const QRectF& frameRect, const CurvePoint& point) const
{
    const qreal rowHeight = this->rowHeight(frameRect);

    const qreal x = round(qreal(point.time) * (frameRect.width() / CurvePoint::MAX_TIME)) + frameRect.left();

    qreal y0 = frameRect.top() + m_topLineValue * m_primaryRowsInterval * rowHeight;
    qreal yPitch = (qreal(point.pitch) / 100) * rowHeight * m_primaryRowsInterval;

    const qreal y = y0 - yPitch;

    return QPointF(x, y);
}

QString BendGridCanvas::pointAccessibleName(const CurvePoint& point)
{
    int fulls = point.pitch / 100;
    int quarts = (point.pitch % 100) / 25;

    QString pointName = m_needVoicePointName ? point.name : "";
    QString string = mu::engraving::bendAmountToString(fulls, quarts).toQString();

    return (!pointName.isEmpty() ? pointName + "; " : "")
           + muse::qtrc("inspector", "Time: %2, value: %3").arg(QString::number(point.time), string);
}

void BendGridCanvas::updatePointAccessibleName(int index)
{
    if (!isPointIndexValid(index)) {
        return;
    }

    muse::ui::AccessibleItem* accItem = m_pointsAccessibleItems[index];
    if (accItem) {
        accItem->setName(pointAccessibleName(m_points.at(index)));
        accItem->accessiblePropertyChanged().send(IAccessible::Property::Name, muse::Val());
    }

    m_needVoicePointName = false;
}

bool BendGridCanvas::movePoint(int pointIndex, const CurvePoint& toPoint)
{
    if (!isPointIndexValid(pointIndex)) {
        return false;
    }

    bool moved = false;

    CurvePoint& currentPoint = m_points[pointIndex];

    if (currentPoint == toPoint) {
        return moved;
    }

    bool canMoveHorizontally = currentPoint.canMove(CurvePoint::MoveDirection::Horizontal);
    bool canMoveVertically = currentPoint.canMove(CurvePoint::MoveDirection::Vertical);

    if (!canMoveHorizontally && !canMoveVertically) {
        return moved;
    }

    if (canMoveVertically) {
        bool canMove = true;

        if (currentPoint.limitMoveVerticallyByNearestPoints) {
            bool moveToTop = currentPoint.pitch < toPoint.pitch;
            if (pointIndex - 1 >= 0) {
                const CurvePoint& leftPoint = m_points.at(pointIndex - 1);
                bool isLeftValid = moveToTop ? leftPoint.pitch >= currentPoint.pitch : leftPoint.pitch <= currentPoint.pitch;
                if (isLeftValid) {
                    canMove = leftPoint.generated || (moveToTop ? leftPoint.pitch > toPoint.pitch : leftPoint.pitch < toPoint.pitch);
                }
            }

            if (!canMove) {
                return moved;
            }

            if (pointIndex + 1 < m_points.size()) {
                const CurvePoint& rightPoint = m_points.at(pointIndex + 1);
                bool isRightValid = moveToTop ? rightPoint.pitch >= currentPoint.pitch : rightPoint.pitch <= currentPoint.pitch;
                if (isRightValid) {
                    canMove = rightPoint.generated || (moveToTop ? rightPoint.pitch > toPoint.pitch : rightPoint.pitch < toPoint.pitch);
                }
            }
        }

        if (canMove) {
            currentPoint.pitch = toPoint.pitch;

            bool isDashed = currentPoint.endDashed;
            bool isNextDashed = (pointIndex + 1 < m_points.size()) && m_points.at(pointIndex + 1).endDashed;

            if (isDashed) {
                m_points[pointIndex - 1].pitch = toPoint.pitch;
            }

            if (isNextDashed) {
                m_points[pointIndex + 1].pitch = toPoint.pitch;
            }

            moved = true;
        }
    }

    if (canMoveHorizontally) {
        bool canMove = true;

        bool moveToLeft = currentPoint.time > toPoint.time;
        if (moveToLeft) {
            if (pointIndex - 1 >= 0) {
                const CurvePoint& leftPoint = m_points.at(pointIndex - 1);
                canMove = leftPoint.generated || leftPoint.time < toPoint.time;
            }
        } else {
            if (pointIndex + 1 < m_points.size()) {
                const CurvePoint& rightPoint = m_points.at(pointIndex + 1);
                canMove = rightPoint.generated || rightPoint.time > toPoint.time;
            }
        }

        if (canMove) {
            currentPoint.time = toPoint.time;
            moved = true;
        }
    }

    if (moved) {
        updatePointAccessibleName(pointIndex);
    }

    return moved;
}

void BendGridCanvas::setFocusedPointIndex(int index)
{
    if (m_focusedPointIndex.has_value()) {
        m_pointsAccessibleItems[m_focusedPointIndex.value()]->setState(muse::ui::AccessibleItem::State::Focused, false);
    }

    bool isIndexValid = isPointIndexValid(index);
    m_focusedPointIndex = isIndexValid ? std::make_optional(index) : std::nullopt;

    if (!isIndexValid) {
        return;
    }

    m_needVoicePointName = true;
    updatePointAccessibleName(index);

    m_pointsAccessibleItems[index]->setState(muse::ui::AccessibleItem::State::Focused, true);
}

muse::ui::AccessibleItem* BendGridCanvas::accessibleParent() const
{
    return m_accessibleParent;
}

void BendGridCanvas::setAccessibleParent(muse::ui::AccessibleItem* parent)
{
    if (m_accessibleParent == parent) {
        return;
    }

    m_accessibleParent = parent;

    for (muse::ui::AccessibleItem* item : m_pointsAccessibleItems) {
        item->setAccessibleParent(m_accessibleParent);
    }

    emit accessibleParentChanged();
}
