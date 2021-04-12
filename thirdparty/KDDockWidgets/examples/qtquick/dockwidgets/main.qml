/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick 2.6
import QtQuick.Controls 2.12
import com.kdab.dockwidgets 1.0 as KDDW

ApplicationWindow {
    visible: true
    width: 1000
    height: 800

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")

            Action {
                text: qsTr("Save layout")
                onTriggered: {
                    layoutSaver.saveToFile("mySavedLayout.json");
                }
            }

            Action {
                text: qsTr("Restore layout")
                onTriggered: {
                    layoutSaver.restoreFromFile("mySavedLayout.json");
                }
            }

            Action {
                text: qsTr("Toggle widget #4")
                onTriggered: {
                    toggleDockWidget(dock4);
                }
            }

            Action {
                text: qsTr("Toggle widget #5")
                onTriggered: {
                    toggleDockWidget(dock5);
                }
            }

            Action {
                text: qsTr("Close All")
                onTriggered: {
                   _kddwDockRegistry.clear();
                }
            }

            MenuSeparator { }
            Action { text: qsTr("&Quit")
                onTriggered: {
                    Qt.quit();
                }
            }
        }
    }

    KDDW.MainWindowLayout {
        anchors.fill: parent

        // Each main layout needs a unique id
        uniqueName: "MyWindowName-1"

        Repeater {
            model: 3
            KDDW.DockWidget {
                uniqueName: "fromRepeater-" + index
                source: ":/Another.qml"
            }
        }

        KDDW.DockWidget {
            id: dock4
            uniqueName: "dock4" // Each dock widget needs a unique id
            source: ":/Another.qml"
        }

        KDDW.DockWidget {
            id: dock5
            uniqueName: "dock5"
            Rectangle {
                id: guest
                color: "pink"
            }
        }

        Component.onCompleted: {
            // Add dock4 to the Bottom location
            addDockWidget(dock4, KDDW.KDDockWidgets.Location_OnBottom);

            // Add dock5 to the left of dock4
            addDockWidget(dock5, KDDW.KDDockWidgets.Location_OnRight, dock4);
        }
    }

    KDDW.LayoutSaver {
        id: layoutSaver
    }

    function toggleDockWidget(dw) {
        if (dw.dockWidget.visible) {
            dw.dockWidget.close();
        } else {
            dw.dockWidget.show();
        }
    }
}
