/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
import QtQuick 2.9
import QtQuick.Controls 2.9
import QtQuick.Layouts 1.9

import com.kdab.dockwidgets 2.0

MouseArea {
    id: root
    required property int resizeMargin
    required property bool resizeAllowed
    required property QtObject groupCpp
    required property int cursorPosition

    enabled: resizeAllowed
    hoverEnabled: true

    cursorShape: {
        if (!enabled) {
            // Even if disabled the MouseArea changes cursor, as it's different than Item.enabled, so explicitly change cursor if disabled
            return Qt.ArrowCursor;
        }

        var isFixedHeight = groupCpp && groupCpp.isFixedHeight
        var isFixedWidth = groupCpp && groupCpp.isFixedWidth
        if (isFixedHeight && isFixedWidth)
            return Qt.ArrowCursor;

        var noFixed = !isFixedHeight && !isFixedWidth;

        if (noFixed && (cursorPosition === KDDockWidgets.CursorPosition_TopLeft || cursorPosition === KDDockWidgets.CursorPosition_BottomRight)) {
            return Qt.SizeFDiagCursor;
        } else if (noFixed && (cursorPosition === KDDockWidgets.CursorPosition_TopRight || cursorPosition === KDDockWidgets.CursorPosition_BottomLeft)) {
            return Qt.SizeBDiagCursor;
        } else if (!isFixedWidth && (cursorPosition & KDDockWidgets.CursorPosition_Horizontal)) {
            return Qt.SizeHorCursor;
        } else if (!isFixedHeight && (cursorPosition & KDDockWidgets.CursorPosition_Vertical)) {
            return Qt.SizeVerCursor;
        } else {
            return Qt.ArrowCursor;
        }
    }

    onPressed: {
        // install event filter
        groupCpp.startMDIResize()

        // ignore event, so event filter catches press as well
        mouse.accepted = false
    }
}
