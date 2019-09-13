//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

import QtQuick 2.8
import QtQuick.Controls 2.0

Canvas {
    property bool drawGrid: false
    readonly property real verticalGridWidth: parent.stretchWidth ? (width - (width % parent.cellWidth)) : width
    property real offsetX: 0.
    property real offsetY: 0.

    property color background: mscore.paletteBackground

    onVisibleChanged: {
        if (visible)
            requestPaint();
    }

    onDrawGridChanged: {
        if (visible)
            requestPaint();
    }

    onOffsetXChanged: {
        if (visible && drawGrid)
            requestPaint();
    }

    onOffsetYChanged: {
        if (visible && drawGrid)
            requestPaint();
    }

    function doDrawGrid(ctx) {
        ctx.lineWidth = 1;
        ctx.strokeStyle = "gray";
        ctx.beginPath();

        const offX = offsetX % parent.cellWidth;
        const ncols = Math.ceil((verticalGridWidth - offX) / parent.cellWidth) + (offX ? 1 : 0);
        for (var i = 1; i < ncols; ++i) {
            const x = i * parent.cellWidth - offX;
            ctx.moveTo(x, 0);
            ctx.lineTo(x, height);
        }
        const offY = offsetY % parent.cellHeight;
        const nrows = Math.ceil((height - offY) / parent.cellHeight) + (offY ? 1 : 0);
        for (var i = 1; i < nrows; ++i) {
            const y = i * parent.cellHeight - offY;
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
        }

        ctx.stroke();
    }

    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        ctx.fillStyle = background;
        ctx.fillRect(0, 0, width, height);
        if (drawGrid)
            doDrawGrid(ctx)
    }
}
