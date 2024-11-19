/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick 2.9

/**
 * @brief Base component for title bars.
 *
 * You need to derive from it to give it a GUI appearance you desire.
 * See TitleBar.qml for an example.
 * Basically you should:
 *  - Show the title
 *  - Show float and close buttons. Bind them to floatButtonVisible, closeButtonEnabled etc.
 *  - emit closeButtonClicked(), floatButtonClicked() as needed
 */
Rectangle {
    id: root

    readonly property QtObject titleBarCpp: parent.titleBarCpp // It's set in the loader
    readonly property string title: titleBarCpp ? titleBarCpp.title : ""
    readonly property bool floatButtonVisible: titleBarCpp && titleBarCpp.floatButtonVisible
    readonly property bool maximizeButtonVisible: titleBarCpp && titleBarCpp.maximizeButtonVisible
    readonly property bool minimizeButtonVisible: titleBarCpp && titleBarCpp.minimizeButtonVisible
    readonly property bool maximizeUsesRestoreIcon: titleBarCpp && titleBarCpp.maximizeUsesRestoreIcon
    readonly property bool closeButtonEnabled: titleBarCpp && titleBarCpp.closeButtonEnabled
    readonly property bool isFocused: titleBarCpp && titleBarCpp.isFocused

    // So the tests can send mouse events programmatically
    readonly property QtObject mouseAreaForTests: titleBarDragMouseArea

    /// The height the title bar should have when visible. Override in your component with another value
    /// Don't set 'hight' directly in the overridden component
    property int heightWhenVisible: 30

    /// Set to true if you're using a custom MouseEventRedirector in your code
    property bool hasCustomMouseEventRedirector: false

    /// @brief Signal emitted by a TitleBar.qml component when the close button is clicked
    signal closeButtonClicked();

    /// @brief Signal emitted by a TitleBar.qml component when the float button is clicked
    signal floatButtonClicked();

    /// @brief Signal emitted by a TitleBar.qml component when the maximize button is clicked
    signal maximizeButtonClicked();

    /// @brief Signal emitted by a TitleBar.qml component when the minimize button is clicked
    signal minimizeButtonClicked();

    visible: titleBarCpp && titleBarCpp.visible
    height: visible ? heightWhenVisible : 0
    implicitHeight: heightWhenVisible

    MouseArea {
        id: titleBarDragMouseArea
        objectName: "titleBarMouseArea" // Name used by tests only
        anchors.fill: parent
        onDoubleClicked: {
            if (titleBarCpp)
                titleBarCpp.onDoubleClicked();
        }
    }

    onTitleBarCppChanged: {
        if (titleBarCpp) {
            if (!root.hasCustomMouseEventRedirector)
                titleBarCpp.redirectMouseEvents(titleBarDragMouseArea)

            // Setting just so the unit-tests can access the buttons
            titleBarCpp.titleBarQmlItem = this;
        }
    }

    onCloseButtonClicked: {
        titleBarCpp.onCloseClicked();
    }

    onFloatButtonClicked: {
        titleBarCpp.onFloatClicked();
    }

    onMaximizeButtonClicked: {
        titleBarCpp.onMaximizeClicked();
    }

    onMinimizeButtonClicked: {
        titleBarCpp.onMinimizeClicked();
    }
}
