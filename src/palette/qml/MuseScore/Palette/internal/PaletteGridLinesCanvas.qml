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
import QtQuick 2.15

Canvas {
    property bool drawGrid: false
    readonly property real verticalGridWidth: parent.stretchWidth ? (width - (width % cellWidth)) : width
    property real offsetX: 0.
    property real offsetY: 0.
    property int cellWidth: 24
    property int cellHeight: 24

    property color gridColor: ui.theme.strokeColor

    onGridColorChanged: {
        if (visible) {
            requestPaint();
        }
    }

    onVisibleChanged: {
        if (visible) {
            requestPaint();
        }
    }

    onEnabledChanged: {
        if (visible) {
            requestPaint();
        }
    }

    onDrawGridChanged: {
        if (visible) {
            requestPaint();
        }
    }

    onOffsetXChanged: {
        if (visible && drawGrid) {
            requestPaint();
        }
    }

    onOffsetYChanged: {
        if (visible && drawGrid) {
            requestPaint();
        }
    }

    onCellWidthChanged: {
        if (visible && drawGrid) {
            requestPaint();
        }
    }

    onCellHeightChanged: {
        if (visible && drawGrid) {
            requestPaint();
        }
    }

    function doDrawGrid(ctx) {
        ctx.lineWidth = 1;
        ctx.strokeStyle = gridColor;
        ctx.beginPath();

        const offX = offsetX % cellWidth;
        const ncols = Math.ceil((verticalGridWidth - offX) / cellWidth) + (offX ? 1 : 0);
        for (var i = 1; i < ncols; ++i) {
            const x = i * cellWidth - offX;
            ctx.moveTo(x, 0);
            ctx.lineTo(x, height);
        }
        const offY = offsetY % cellHeight;
        const nrows = Math.ceil((height - offY) / cellHeight) + (offY ? 1 : 0);
        for (i = 1; i < nrows; ++i) {
            const y = i * cellHeight - offY;
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
        }

        ctx.stroke();
    }

    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        if (drawGrid)
            doDrawGrid(ctx)
    }
}
