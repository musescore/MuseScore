import QtQuick 2.15

import MuseScore.NotationScene 1.0

import "../docksystem"

DockPage {
    id: root

    uniqueName: "Notation"

    central: NotationView {}
}
