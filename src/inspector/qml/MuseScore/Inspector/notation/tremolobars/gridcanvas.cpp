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

#include "gridcanvas.h"

#include <cmath>

#include <QGuiApplication>
#include <QPalette>

#include "draw/types/pen.h"

#include "log.h"

using namespace mu::inspector;
using namespace muse::ui;
using namespace mu::engraving;

GridCanvas::GridCanvas(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

QVariant GridCanvas::pointList() const
{
    return pitchValuesToQVariant(m_points);
}

int GridCanvas::rowCount() const
{
    return m_rows;
}

int GridCanvas::columnCount() const
{
    return m_columns;
}

int GridCanvas::rowSpacing() const
{
    return m_primaryRowsInterval;
}

int GridCanvas::columnSpacing() const
{
    return m_primaryColumnsInterval;
}

bool GridCanvas::shouldShowNegativeRows() const
{
    return m_showNegativeRows;
}

void GridCanvas::setRowCount(int rowCount)
{
    if (m_rows == rowCount) {
        return;
    }

    m_rows = rowCount;
    emit rowCountChanged(m_rows);
}

void GridCanvas::setColumnCount(int columnCount)
{
    if (m_columns == columnCount) {
        return;
    }

    m_columns = columnCount;
    emit columnCountChanged(m_columns);
}

void GridCanvas::setRowSpacing(int rowSpacing)
{
    if (m_primaryRowsInterval == rowSpacing) {
        return;
    }

    m_primaryRowsInterval = rowSpacing;
    emit rowSpacingChanged(m_primaryRowsInterval);
}

void GridCanvas::setColumnSpacing(int columnSpacing)
{
    if (m_primaryColumnsInterval == columnSpacing) {
        return;
    }

    m_primaryColumnsInterval = columnSpacing;
    emit columnSpacingChanged(m_primaryColumnsInterval);
}

void GridCanvas::setShouldShowNegativeRows(bool shouldShowNegativeRows)
{
    if (m_showNegativeRows == shouldShowNegativeRows) {
        return;
    }

    m_showNegativeRows = shouldShowNegativeRows;
    emit shouldShowNegativeRowsChanged(m_showNegativeRows);
}

void GridCanvas::setPointList(QVariant points)
{
    PitchValues newPointList = pitchValuesFromQVariant(points);

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

void GridCanvas::paint(QPainter* painter)
{
    if (!(m_rows && m_columns)) {
        LOGD("GridCanvas::paintEvent: number of columns or rows set to 0.\nColumns: %i, Rows: %i", m_rows,
             m_columns);
        return;
    }
    // not qreal here, even though elsewhere yes,
    // because width and height return a number of pixels,
    // hence integers.
    const int w = width();
    const int h = height();

    const qreal columnWidth = qreal(w) / m_columns;
    const qreal rowHeight = qreal(h) / m_rows;

    // let half a column of margin around
    const qreal leftPos = columnWidth * .5;   // also left margin
    const qreal topPos = rowHeight * .5;      // also top margin
    const qreal rightPos = w - leftPos;   // right end position of graph
    const qreal bottomPos = h - topPos;   // bottom end position of graph

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->fillRect(childrenRect(), QGuiApplication::palette().color(QPalette::Window).lighter());
    QPen pen = painter->pen();
    pen.setWidth(1);

    QColor primaryLinesColor(uiConfig()->currentTheme().codeKey == DARK_THEME_CODE ? Qt::white : Qt::black);
    QColor secondaryLinesColor(Qt::gray);
    // draw vertical lines
    for (int i = 0; i < m_columns; ++i) {
        qreal xpos = leftPos + i * columnWidth;
        // lighter middle lines
        pen.setColor(i % m_primaryColumnsInterval ? secondaryLinesColor : primaryLinesColor);
        painter->setPen(pen);
        painter->drawLine(xpos, topPos, xpos, bottomPos);
    }

    // draw horizontal lines
    for (int i = 0; i < m_rows; ++i) {
        int ypos = topPos + i * rowHeight;
        // lighter middle lines
        pen.setColor(i % m_primaryRowsInterval ? secondaryLinesColor : primaryLinesColor);
        if (m_showNegativeRows) {
            pen.setWidth(i == (m_rows - 1) / 2 ? 3 : 1);
        }
        painter->setPen(pen);
        painter->drawLine(leftPos, ypos, rightPos, ypos);
    }

    // this lambda takes as input a pitch value, and determines where what are its x and y coordinates
    auto getPosition = [this, columnWidth, rowHeight, leftPos, topPos, bottomPos](const mu::engraving::PitchValue& v) -> QPointF {
        const qreal x = round((qreal(v.time) / 60) * (m_columns - 1)) * columnWidth + leftPos;
        qreal y = 0;
        if (m_showNegativeRows) {                    // get the middle pos and add the top margin and half of the rows
            y = topPos + rowHeight * (m_rows - 1) * .5;
        } else {                    // from the bottom
            y = bottomPos;
        }
        // add the offset
        y -=  round((qreal(v.pitch) / (100 * (m_rows / m_primaryRowsInterval))) * (m_rows - 1))
             * rowHeight;
        return QPointF(x, y);
    };

    static constexpr int GRIP_HALF_RADIUS = 5;
    QPointF lastPoint(0, 0);
    pen = painter->pen();
    pen.setWidth(3);
    pen.setColor(Qt::red);   // not theme dependant
    painter->setPen(pen);
    // draw line between points
    for (const mu::engraving::PitchValue& v : m_points) {
        QPointF currentPoint = getPosition(v);
        // draw line only if there is a point before the current one
        if (lastPoint.x()) {
            painter->drawLine(lastPoint, currentPoint);
        }
        lastPoint = currentPoint;
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor::fromRgb(32, 116, 189));   // Musescore blue
    // draw points
    for (const mu::engraving::PitchValue& v : m_points) {
        painter->drawEllipse(getPosition(v), GRIP_HALF_RADIUS, GRIP_HALF_RADIUS);
    }
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void GridCanvas::mousePressEvent(QMouseEvent* ev)
{
    if (!(m_rows && m_columns)) {
        LOGD("GridCanvas::mousePressEvent: number of columns or rows set to 0.\nColumns: %i, Rows: %i", m_rows,
             m_columns);
        return;
    }
    const qreal columnWidth = qreal(width()) / m_columns;
    const qreal rowHeight = qreal(height()) / m_rows;

    // Half a column/row of margin around
    const QPointF pos = ev->position();
    const double x = pos.x() - columnWidth * .5;
    const double y = pos.y() - rowHeight * .5;

    int column = round(x / columnWidth);
    int row = round(y / rowHeight);

    // restrict to clickable area
    if (column >= m_columns) {
        column = m_columns - 1;
    } else if (column < 0) {
        column = 0;
    }
    if (row >= m_rows) {
        row = m_rows - 1;
    } else if (row < 0) {
        row = 0;
    }

    // invert y.
    if (m_showNegativeRows) {
        row = (m_rows - 1) / 2 - row;
    } else {
        row = (m_rows - 1) - row;
    }

    const int time = column * 60 / (m_columns - 1);
    const int pitch = row * 100 / m_primaryRowsInterval;

    const size_t numberOfPoints = m_points.size();
    bool found = false;
    for (size_t i = 0; i < numberOfPoints; ++i) {
        if (round(qreal(m_points[i].time) / 60 * (m_columns - 1)) > column) {
            m_points.insert(m_points.begin() + i, mu::engraving::PitchValue(time, pitch, false));
            found = true;
            break;
        }
        if (round(qreal(m_points[i].time) / 60 * (m_columns - 1)) == column) {
            if (round(qreal(m_points[i].pitch) / (100 * (m_rows / m_primaryRowsInterval)) * (m_rows - 1)) == row
                && i > 0 && i < (numberOfPoints - 1)) {
                m_points.erase(m_points.begin() + i);
            } else {
                m_points[i].pitch = pitch;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        m_points.push_back(mu::engraving::PitchValue(time, pitch, false));
    }

    update();
    emit canvasChanged();
}
