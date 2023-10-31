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

#include "bendgridcanvas.h"

#include <cmath>

#include <QPainterPath>

#include "log.h"

using namespace mu::inspector;
using namespace mu::ui;

static constexpr int GRIP_RADIUS = 6;
static constexpr int GRIP_CENTER_RADIUS = GRIP_RADIUS - 2;
static constexpr int GRIP_SELECTED_RADIUS = GRIP_RADIUS + 2;
static constexpr int GRIP_FOCUS_RADIUS = GRIP_SELECTED_RADIUS + 2;

BendGridCanvas::BendGridCanvas(QQuickItem* parent)
    : uicomponents::QuickPaintedView(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    uiConfig()->currentThemeChanged().onNotify(this, [this]() {
        update();
    });

    uiConfig()->fontChanged().onNotify(this, [this]() {
        update();
    });
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

bool BendGridCanvas::shouldShowNegativeRows() const
{
    return m_showNegativeRows;
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

void BendGridCanvas::setShouldShowNegativeRows(bool shouldShowNegativeRows)
{
    if (m_showNegativeRows == shouldShowNegativeRows) {
        return;
    }

    m_showNegativeRows = shouldShowNegativeRows;
    emit shouldShowNegativeRowsChanged(m_showNegativeRows);
}

void BendGridCanvas::setPointList(QVariant points)
{
    CurvePoints newPointList = curvePointsFromQVariant(points);

    if (m_points == newPointList) {
        return;
    }

    m_points = newPointList;

    update();
    emit pointListChanged(points);
}

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void BendGridCanvas::paint(QPainter* painter)
{
    if (!(m_rows && m_columns)) {
        LOGD("GridCanvas::paintEvent: number of columns or rows set to 0.\nColumns: %i, Rows: %i", m_rows,
             m_columns);
        return;
    }

    QRectF frameRect = this->frameRect();

    drawBackground(painter, frameRect);
    drawCurve(painter, frameRect);
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void BendGridCanvas::mousePressEvent(QMouseEvent* event)
{
    if (!(m_rows && m_columns)) {
        LOGD("GridCanvas::mousePressEvent: number of columns or rows set to 0.\nColumns: %i, Rows: %i", m_rows,
             m_columns);
        return;
    }

    QRectF frameRect = this->frameRect();
    std::pair<int, int> coord = this->frameCoord(frameRect, event->pos().x(), event->pos().y());
    CurvePoint point = this->point(frameRect, coord.first, coord.second);

    m_currentPointIndex = this->pointIndex(point);
}

void BendGridCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_currentPointIndex.has_value()) {
        return;
    }

    QRectF frameRect = this->frameRect();
    std::pair<int, qreal> coord = this->frameCoord(frameRect, event->pos().x(), event->pos().y());

    CurvePoint point = this->point(frameRect, coord.first, coord.second);

    int index = m_currentPointIndex.value();

    bool canMoveHorizontally = m_points[index].canMove(CurvePoint::MoveDirection::Horizontal);
    bool canMoveVertically = m_points[index].canMove(CurvePoint::MoveDirection::Vertical);

    if (!canMoveHorizontally && !canMoveVertically) {
        return;
    }

    if (canMoveVertically) {
        m_points[index].pitch = point.pitch;

        bool isDashed = m_points[index].endDashed;
        bool isNextDashed = (index + 1 < m_points.size()) && m_points[index + 1].endDashed;

        if (isDashed) {
            m_points[index - 1].pitch = point.pitch;
        }

        if (isNextDashed) {
            m_points[index + 1].pitch = point.pitch;
        }
    }

    if (canMoveHorizontally) {
        bool canMove = false;
        bool moveToLeft = m_points[index].time > point.time;
        if (moveToLeft) {
            canMove = (index - 1 >= 0) && m_points[index - 1].time < point.time;
        } else {
            canMove = (index + 1 < m_points.size()) && m_points[index + 1].time > point.time;
        }

        if (canMove) {
            m_points[index].time = point.time;
        }
    }

    update();
    emit canvasChanged();
}

void BendGridCanvas::mouseReleaseEvent(QMouseEvent*)
{
    m_currentPointIndex = std::nullopt;
}

void BendGridCanvas::hoverEnterEvent(QHoverEvent*)
{
    m_hoverPointIndex = std::nullopt;
}

void BendGridCanvas::hoverMoveEvent(QHoverEvent* event)
{
    auto oldPointIndex = m_hoverPointIndex;

    QRectF frameRect = this->frameRect();
    std::pair<int, int> coord = this->frameCoord(frameRect, event->pos().x(), event->pos().y());
    CurvePoint point = this->point(frameRect, coord.first, coord.second);

    m_hoverPointIndex = this->pointIndex(point);

    if (oldPointIndex != m_hoverPointIndex) {
        update();
    }
}

void BendGridCanvas::hoverLeaveEvent(QHoverEvent*)
{
    m_hoverPointIndex = std::nullopt;
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

std::pair<int, int> BendGridCanvas::frameCoord(const QRectF& frameRect, int x, int y) const
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

    return { x - frameRect.left(), y - frameRect.top() };
}

void BendGridCanvas::drawBackground(QPainter* painter, const QRectF& frameRect)
{
    const qreal rowHeight = this->rowHeight(frameRect);
    const qreal columnWidth = this->columnWidth(frameRect);

    const ThemeInfo& currentTheme = uiConfig()->currentTheme();
    QColor primaryLinesColor(currentTheme.codeKey == DARK_THEME_CODE ? Qt::white : Qt::black);
    QColor secondaryLinesColor(Qt::gray);

    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor backgroundColor(currentTheme.values[BACKGROUND_PRIMARY_COLOR].toString());
    painter->fillRect(QRect(0, 0, width(), height()), backgroundColor);

    QPen pen = painter->pen();
    pen.setWidth(1);

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

    for (int i = 1; i < m_rows - 1; ++i) {
        int ypos = frameRect.top() + i * rowHeight;

        bool isPrimary = !(i % m_primaryRowsInterval);

        // lighter middle lines
        pen.setColor(isPrimary ? primaryLinesColor : secondaryLinesColor);
        if (m_showNegativeRows) {
            pen.setWidth(i == (m_rows - 1) / 2 ? 3 : 1);
        }
        painter->setPen(pen);
        painter->drawLine(frameRect.left(), ypos, frameRect.right(), ypos);

        int interval = (m_primaryRowsInterval - 1) - i / m_primaryRowsInterval;
        bool negative = false;

        if (m_showNegativeRows) {
            int curveRowMiddleIndex = m_rows / 2;
            negative = i > curveRowMiddleIndex;

            if (negative) {
                interval = -(i - curveRowMiddleIndex) / m_primaryRowsInterval;
            } else {
                interval = (curveRowMiddleIndex - i) / m_primaryRowsInterval;
            }
        }

        bool isHalf = !((i + (i - lastPrimaryRowIndex)) % m_primaryRowsInterval) && !isPrimary;

        if (isPrimary) {
            lastPrimaryRowIndex = i;
        }

        if (!isPrimary && !isHalf) {
            continue;
        }

        if (!m_showNegativeRows && isHalf) {
            --interval;
        }

        pen.setColor(primaryLinesColor);
        painter->setPen(pen);

        QString intervalStr = QString::number(interval);
        if (interval == 0 && isHalf) {
            intervalStr = negative ? "-" : "";
        }

        QString text = QString("%1%2")
                       .arg(intervalStr)
                       .arg(isHalf ? "½" : "");

        QRect textRect(0, ypos - stringHeight / 2, frameRect.left(), stringHeight);

        painter->drawText(textRect, Qt::AlignCenter, text);
    }

    // draw a frame
    QPainterPath path;
    path.addRoundedRect(frameRect, 3, 3);

    pen.setColor(primaryLinesColor);
    pen.setWidth(1);
    pen.setStyle(Qt::PenStyle::SolidLine);
    painter->setPen(pen);

    painter->fillPath(path, Qt::transparent);
    painter->drawPath(path);
}

void BendGridCanvas::drawCurve(QPainter* painter, const QRectF& frameRect)
{
    const qreal rowHeight = this->rowHeight(frameRect);
    const ThemeInfo& currentTheme = uiConfig()->currentTheme();
    QColor backgroundColor(currentTheme.values[BACKGROUND_PRIMARY_COLOR].toString());

    // this lambda takes as input a pitch value, and determines where what are its x and y coordinates
    auto getPosition = [this, rowHeight, &frameRect](const CurvePoint& v) -> QPointF {
        const qreal x = round(qreal(v.time) * (frameRect.width() / CurvePoint::MAX_TIME)) + frameRect.left();
        qreal y = 0;
        if (m_showNegativeRows) {                    // get the middle pos and add the top margin and half of the rows
            y = frameRect.top() + rowHeight * (m_rows - 1) * .5;
        } else {                    // from the bottom
            y = frameRect.bottom();
        }
        // add the offset
        y -=  (qreal(v.pitch) / (100 * (m_rows / m_primaryRowsInterval)) * (m_rows - 1))
             * rowHeight;
        return QPointF(x, y);
    };

    QPointF lastPoint(0, 0);
    QPen pen = painter->pen();
    pen.setWidth(3);

    QColor color(currentTheme.values[ACCENT_COLOR].toString());
    pen.setColor(color);
    painter->setPen(pen);

    // draw line between points
    for (const CurvePoint& v : m_points) {
        QPointF currentPoint = getPosition(v);

        QPainterPath path;
        path.moveTo(lastPoint);

        // draw line only if there is a point before the current one
        if (lastPoint.x()) {
            QPointF c(currentPoint.x(), lastPoint.y());
            path.quadTo(c, currentPoint);

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
    hoverColor.setAlpha(200);
    QBrush hoverBrush(hoverColor, Qt::SolidPattern);

    painter->setPen(Qt::NoPen);

    for (int i = 0; i < m_points.size(); ++i) {
        const CurvePoint& v = m_points[i];
        if (!v.canMove()) {
            continue;
        }

        QPointF pos = getPosition(v);

        bool isNotActiveButton = (!m_hoverPointIndex.has_value() || m_hoverPointIndex.value() != i)
                                 && (!m_currentPointIndex.has_value() || m_currentPointIndex.value() != i);

        if (isNotActiveButton) { // normal
            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);

            painter->setBrush(backgroundBrush);
            painter->drawEllipse(pos, GRIP_CENTER_RADIUS, GRIP_CENTER_RADIUS);
        } else if (m_hoverPointIndex.has_value() && m_currentPointIndex.has_value()) { // focused
            QColor fontPrimaryColor(currentTheme.values[FONT_PRIMARY_COLOR].toString());
            QBrush fontPrimaryBrush(fontPrimaryColor, Qt::SolidPattern);
            painter->setBrush(fontPrimaryBrush);
            painter->drawEllipse(pos, GRIP_FOCUS_RADIUS, GRIP_FOCUS_RADIUS);

            painter->setBrush(backgroundBrush);
            painter->drawEllipse(pos, GRIP_SELECTED_RADIUS, GRIP_SELECTED_RADIUS);

            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);
        } else if (m_hoverPointIndex.has_value() && m_hoverPointIndex.value() == i) { // hover
            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);

            painter->setBrush(hoverBrush);
            painter->drawEllipse(pos, GRIP_CENTER_RADIUS, GRIP_CENTER_RADIUS);
        } else if (m_currentPointIndex.has_value() && m_currentPointIndex.value() == i) { // selected
            painter->setBrush(backgroundBrush);
            painter->drawEllipse(pos, GRIP_SELECTED_RADIUS, GRIP_SELECTED_RADIUS);

            painter->setBrush(activeBrush);
            painter->drawEllipse(pos, GRIP_RADIUS, GRIP_RADIUS);
        }
    }
}

std::optional<int> BendGridCanvas::pointIndex(const CurvePoint& pitch, bool movable) const
{
    const int numberOfPoints = m_points.size();

    for (int i = 0; i < numberOfPoints; ++i) {
        const CurvePoint& point = m_points[i];
        if (movable != point.canMove()) {
            continue;
        }

        if (std::pow((pitch.time - point.time), 2)
            + std::pow((pitch.pitch - point.pitch), 2) < std::pow(GRIP_CENTER_RADIUS, 2)) {
            return i;
        }
    }

    return std::nullopt;
}

CurvePoint BendGridCanvas::point(const QRectF& frameRect, int frameX, int frameY) const
{
    CurvePoint point;
    point.time = qreal(frameX) / (frameRect.width() / CurvePoint::MAX_TIME);

    const qreal rowHeight = this->rowHeight(frameRect);
    int row = m_rows - 1 - round(qreal(frameY) / rowHeight);
    if (m_showNegativeRows) {
        int half = (m_rows - 1) / 2;
        if (row > half) {
            row -= half;
        } else {
            row += -half;
        }
    }

    point.pitch = row * 100 / m_primaryRowsInterval;

    return point;
}
