import QtQuick

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {

    property string search: ""
    property string selectedCategory: ""
    property color backgroundColor: ui.theme.backgroundPrimaryColor

    property int sideMargin: 46

    property NavigationSection navigationSection: null

}
