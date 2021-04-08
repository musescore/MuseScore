import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.AppShell 1.0

import com.kdab.dockwidgets 1.0 as KDDW

ApplicationWindow {
    id: root

    width: 800
    height: 600

    KDDW.MainWindowLayout {
        anchors.fill: parent

        uniqueName: "mainWindowLayout"

        KDDW.DockWidget {
            id: dock1

            uniqueName: "dock1"

            Rectangle {
                color: "red"
                opacity: 0.5
            }
        }

        KDDW.DockWidget {
            id: dock2

            uniqueName: "dock2"

            Rectangle {
                color: "green"
                opacity: 0.5
            }
        }

        Component.onCompleted: {
            addDockWidget(dock2, KDDW.KDDockWidgets.Location_OnLeft)
            addDockWidget(dock1, KDDW.KDDockWidgets.Location_OnLeft)
        }
    }
}
