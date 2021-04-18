import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundSecondaryColor

    Flickable {
        id: flickableWrapper

        anchors.fill: parent
        anchors.margins: 12

        contentHeight: contentColumn.implicitHeight

        Column {
            id: contentColumn

            width: parent.width

            spacing: 24

            Repeater {
                width: parent.width

                model: [
                    { textRole: "StyledComboBox", componentRole: comboboxSample },
                    { textRole: "StyledPopup", componentRole: popupSample },
                    { textRole: "StyledPopupView", componentRole: styledPopupViewComponent },
                    { textRole: "StyledMenu", componentRole: styledMenuComponent },
                    { textRole: "CheckBox", componentRole: checkBoxSample },
                    { textRole: "ColorPicker", componentRole: colorPickerSample },
                    { textRole: "ExpandableBlank", componentRole: expandableBlankSample },
                    { textRole: "FlatButton", componentRole: flatButtonSample },
                    { textRole: "ProgressButton", componentRole: progressButtonSample },
                    { textRole: "RadioButtonGroup + FlatRadioButton", componentRole: flatRadioButtonSample },
                    { textRole: "RoundedRadioButton", componentRole: roundedRadioButtonSample },
                    { textRole: "IncrementalPropertyControl (Hidden icon, Icon left, Icon right)", componentRole: incrementalPropertyControlSample },
                    { textRole: "FlatToggleButton", componentRole: flatToggleButtonSample },
                    { textRole: "RoundedRectangle (which allows to round the particular corners)", componentRole: roundedRectangleSample },
                    { textRole: "TextInputField", componentRole: textInputFieldSample },
                    { textRole: "SearchField", componentRole: searchFieldSample },
                    { textRole: "FilePicker", componentRole: filePickerSample },
                    { textRole: "TabPanel", componentRole: tabPanelSample },
                    { textRole: "GradientTabButton", componentRole: gradientTabButtonsSample },
                    { textRole: "GridView", componentRole: gridViewVertical },
                    { textRole: "StyledSlider", componentRole: slidersSample },
                    { textRole: "NumberInputField", componentRole: numberInputFieldSample },
                    { textRole: "TimeInputField", componentRole: timeInputFieldSample },
                    { textRole: "ValueList", componentRole: valueListSample }
                ]

                delegate: Column {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 8

                    spacing: 24

                    StyledTextLabel {
                        text: modelData["textRole"]
                    }

                    Loader {
                        sourceComponent: modelData["componentRole"]
                    }

                    SeparatorLine { anchors.margins: -18}
                }
            }
        }
    }

    Component {
        id: comboboxSample

        StyledComboBox {

            property var currValue

            width: 400

            textRoleName: "text"
            valueRoleName: "value"

            model: [
                { text: "Option 1", value: 1 },
                { text: "Option 2", value: 2 },
                { text: "Option 3", value: 3 },
                { text: "Option 4", value: 4 },
                { text: "Option 5", value: 5 },
                { text: "Option 6", value: 6 },
                { text: "Option 7", value: 7 },
                { text: "Option 8", value: 8 },
                { text: "Option 9", value: 9 },
                { text: "Option 10", value: 10 },
                { text: "Option 11", value: 11 }
            ]

            currentIndex: indexOfValue(currValue)

            onValueChanged: {
                currValue = value
            }
        }
    }

    Component {
        id: popupSample

        Row {
            spacing: 12

            FlatButton {
                id: popupDownButton

                text: "Show Popup downward"

                onClicked: {
                    if (popupDown.opened) {
                        popupDown.close()
                    } else {
                        popupDown.open()
                    }
                }
            }

            StyledPopup {
                id: popupDown

                width: 200
                height: 200

                anchorItem: popupDownButton

                StyledTextLabel {
                    text: "Hello, World!"

                    anchors.centerIn: parent
                }
            }

            FlatButton {
                id: popupUpButton

                text: "Show Popup upward"

                onClicked: {
                    if (popupUp.opened) {
                        popupUp.close()
                    } else {
                        popupUp.open()
                    }
                }
            }

            StyledPopup {
                id: popupUp

                width: 200
                height: 200

                anchorItem: popupUpButton

                opensUpward: true

                StyledTextLabel {
                    text: "Hello, World!"

                    anchors.centerIn: parent
                }
            }
        }
    }

    Component {
        id: styledPopupViewComponent

        Row {
            spacing: 12

            FlatButton {
                text: "Show PopupView downward"

                onClicked: {
                    popupViewDown.toggleOpened()
                }

                StyledPopupView {
                    id: popupViewDown

                    Column {
                        spacing: 12

                        CheckBox {
                            text: "Some checkbox"
                        }

                        FlatButton {
                            text: "Some button"
                        }

                        FlatButton {
                            text: "Accent button 1"
                            accentButton: true
                        }

                        FlatButton {
                            text: "Accent button 2"
                            accentButton: true
                        }
                    }
                }
            }

            FlatButton {
                text: "Show PopupView upward"

                onClicked: {
                    popupViewUp.toggleOpened()
                }

                StyledPopupView {
                    id: popupViewUp
                    opensUpward: true

                    Column {
                        spacing: 12

                        CheckBox {
                            text: "Some checkbox"
                        }

                        FlatButton {
                            text: "Some button"
                        }

                        FlatButton {
                            text: "Accent button 1"
                            accentButton: true
                        }

                        FlatButton {
                            text: "Accent button 2"
                            accentButton: true
                        }
                    }
                }
            }
        }
    }

    Component {
        id: styledMenuComponent

        Row {
            spacing: 12

            FlatButton {
                text: "Show Menu"

                onClicked: {
                    menu.toggleOpened()
                }

                StyledMenu {
                    id: menu

                    model: {
                        var _subitems = [
                                    {code: "2", icon: IconCode.PAGE, title: "first action", enabled: true},
                                    {code: "3", icon: IconCode.PAGE, title: "with subitems", enabled: true, subitems: [
                                            {code: "4", title: "first action", enabled: true, selectable: true},
                                            {code: "5", title: "second action", enabled: true, selectable: true, selected: true},
                                            {code: "6", title: "third action", enabled: true, selectable: true},
                                            {},
                                            {code: "7", title: "clear"}
                                        ]}
                                ]

                        var items = [
                                    {code: "0", icon: IconCode.PAGE, title: "enabled action", enabled: true},
                                    {code: "1", icon: IconCode.AMBITUS, title: "with subitems", enabled: true, shortcut: "Ctrl+A", subitems: _subitems },
                                    {},
                                    {code: "5", title: "with shortcut", enabled: true, shortcut: "Ctrl+Shift+G"},
                                    {code: "6", icon: IconCode.PAGE, title: "disabled action", enabled: false},
                                    {code: "7", icon: IconCode.CLEF_BASS, title: "checkable action", enabled: true, checkable: true, checked: true}
                                ]

                        return items
                    }

                    onHandleAction: {
                        console.log("selected " + actionCode + " index " + actionIndex)
                        menu.close()
                    }
                }
            }
        }
    }

    Component {
        id: checkBoxSample

        CheckBox {
            width: 200

            text: "Option"
            onClicked: {
                checked = !checked
            }
        }
    }

    Component {
        id: colorPickerSample

        ColorPicker {
            width: 200

            color: "black"

            onNewColorSelected: {
                color = newColor
            }
        }
    }

    Component {
        id: expandableBlankSample

        ExpandableBlank {
            title: isExpanded ? "Collapse me" : "Expand me"

            width: 200

            contentItemComponent: Rectangle {
                implicitHeight: 50
                width: 200

                color: "gray"

                StyledTextLabel {
                    anchors.fill: parent

                    text: "Some content"
                }
            }
        }
    }

    Component {
        id: flatButtonSample

        Column {
            spacing: 8

            width: 200

            FlatButton {
                icon: IconCode.SAVE

                text: "Text with icon"
            }

            FlatButton {
                icon: IconCode.SAVE

                text: "Text with icon"

                orientation: Qt.Horizontal
            }

            FlatButton {
                text: "Just text"
            }

            FlatButton {
                icon: IconCode.SAVE
            }

            FlatButton {
                icon: IconCode.SAVE

                accentButton: true

                text: "Accent button"
            }

            FlatButton {
                icon: IconCode.SAVE

                normalStateColor: "transparent"

                text: "Transparent button"
            }

        }
    }

    Component {
        id: progressButtonSample

        ProgressButton {
            Timer {
                id: timer

                interval: 1000
                repeat: true

                property int progress: 0

                onTriggered: {
                    progress += 10
                    parent.setProgress(progress + "/100", false, progress, 100)

                    if (progress === 100) {
                        progress = 0
                        stop()
                    }
                }
            }

            text: "Start processing"

            onClicked: {
                timer.start()
            }
        }
    }

    Component {
        id: flatRadioButtonSample

        Column {
            spacing: 8

            width: 200

            RadioButtonGroup {
                id: iconButtonList

                property var currentValue: -1

                height: 30

                model: [
                    { iconRole: IconCode.AUTO, valueRole: 0 },
                    { iconRole: IconCode.ARROW_DOWN, valueRole: 1 },
                    { iconRole: IconCode.ARROW_UP, valueRole: 2 }
                ]

                delegate: FlatRadioButton {
                    ButtonGroup.group: iconButtonList.radioButtonGroup

                    checked: iconButtonList.currentValue === modelData["valueRole"]

                    onToggled: {
                        iconButtonList.currentValue = modelData["valueRole"]
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }

            RadioButtonGroup {
                id: textButtonList

                property var currentValue: -1

                height: 30

                model: [
                    { textRole: "Text 1", valueRole: 0 },
                    { textRole: "Text 2", valueRole: 1 },
                    { textRole: "Text 3", valueRole: 2 }
                ]

                delegate: FlatRadioButton {
                    ButtonGroup.group: textButtonList.radioButtonGroup

                    checked: textButtonList.currentValue === modelData["valueRole"]

                    onToggled: {
                        textButtonList.currentValue = modelData["valueRole"]
                    }

                    StyledTextLabel {
                        text: modelData["textRole"]
                    }
                }
            }
        }
    }

    Component {
        id: roundedRadioButtonSample

        Row {
            spacing: 12

            RoundedRadioButton {
                text: "Option 1"
            }

            RoundedRadioButton {
                text: "Option 2"
            }

            RoundedRadioButton {
                text: "Option 3"
            }
        }
    }

    Component {
        id: incrementalPropertyControlSample

        Column {
            spacing: 8

            width: 200

            IncrementalPropertyControl {
                iconMode: iconModeEnum.hidden

                currentValue: 0

                maxValue: 999
                minValue: 0
                step: 0.5

                onValueEdited: {
                    currentValue = newValue
                }
            }

            IncrementalPropertyControl {
                icon: IconCode.AUDIO
                iconMode: iconModeEnum.right

                currentValue: 0

                maxValue: 999
                minValue: 0
                step: 0.5

                onValueEdited: {
                    currentValue = newValue
                }
            }

            IncrementalPropertyControl {
                icon: IconCode.AUDIO
                iconMode: iconModeEnum.left

                currentValue: 0

                maxValue: 999
                minValue: 0
                step: 0.5

                onValueEdited: {
                    currentValue = newValue
                }
            }
        }
    }

    Component {
        id: flatToggleButtonSample

        FlatToggleButton {
            id: lockButton

            height: 20
            width: 20

            icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

            onToggled: {
                checked = !checked
            }
        }
    }

    Component {
        id: roundedRectangleSample

        GridLayout {
            columnSpacing: 12
            rowSpacing: 12
            columns: 2

            width: 400

            RoundedRectangle {
                Layout.fillWidth: true
                height: 80

                topLeftRadius: 40

                color: "gray"
                border.color: "red"
                border.width: 3
            }

            RoundedRectangle {
                Layout.fillWidth: true
                height: 80

                topRightRadius: 30

                color: "gray"
                border.color: "yellow"
                border.width: 10
            }

            RoundedRectangle {
                Layout.fillWidth: true
                height: 80

                bottomLeftRadius: 20

                color: "gray"
                border.color: "green"
                border.width: 8
            }

            RoundedRectangle {
                Layout.fillWidth: true
                height: 80

                bottomRightRadius: 10

                color: "gray"
                border.color: "blue"
                border.width: 5
            }
        }
    }

    Component {
        id: textInputFieldSample

        TextInputField {
            height: 40
            width: 200
        }
    }

    Component {
        id: searchFieldSample

        SearchField {}
    }

    Component {
        id: filePickerSample

        FilePicker {
            width: 220

            path: "/some/test/path/foo.txt"
        }
    }

    Component {
        id: tabPanelSample

        TabPanel {
            id: tabPanel

            height: 40
            width: 200

            Tab {
                id: firstTab

                title: "Tab 1"

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: 4

                    height: 40
                    width: tabPanel.width

                    color: "blue"
                }
            }

            Tab {
                id: secondTab

                title: "Tab 2"

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: 4

                    height: 40
                    width: tabPanel.width

                    color: "gray"
                }
            }

            Tab {
                id: thirdTab

                title: "Tab 3"

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: 4

                    height: 40
                    width: tabPanel.width

                    color: "black"
                }
            }
        }
    }

    Component {
        id: gradientTabButtonsSample

        Row {
            spacing: 30

            Column {
                GradientTabButton {
                    title: "Tab 1"

                    orientation: Qt.Horizontal

                    width: 200

                    iconComponent: StyledIconLabel {
                        iconCode: IconCode.SAVE
                    }

                    checked: true
                }

                GradientTabButton {
                    title: "Tab 2"

                    width: 200

                    orientation: Qt.Horizontal
                }

                GradientTabButton {
                    title: "Tab 3"

                    width: 200

                    orientation: Qt.Horizontal
                }
            }

            Column {
                GradientTabButton {
                    title: "Tab 1"

                    width: 200

                    iconComponent: StyledIconLabel {
                        iconCode: IconCode.SAVE
                    }

                    checked: true
                }

                GradientTabButton {
                    title: "Tab 2"

                    width: 200
                }

                GradientTabButton {
                    title: "Tab 3"

                    width: 200
                }
            }
        }
    }

    Component {
        id: gridViewVertical

        Item {

            height: 320
            width: 500

            GridViewSectional {
                id: gridView

                height: parent.height
                width: parent.width

                orientation: Qt.Vertical

                cellWidth: 100
                cellHeight: 36

                sectionWidth: 204
                sectionHeight: 36
                sectionRole: "sectionRole"

                columns: 2

                model: ListModel {
                    ListElement {
                        name: "Text 1"
                        sectionRole: "Section 0"
                    }
                    ListElement {
                        name: "Text 2"
                        sectionRole: "Section 0"
                    }
                    ListElement {
                        name: "Text 5"
                        sectionRole: "Section 0"
                    }
                    ListElement {
                        name: "Text 3"
                        sectionRole: "Section 1"
                    }
                    ListElement {
                        name: "Text 4"
                        sectionRole: "Section 1"
                    }
                }

                sectionDelegate: Rectangle {
                    color: ui.theme.strokeColor

                    property var itemData: Boolean(itemModel) ? itemModel : null

                    width: gridView.sectionWidth
                    height: gridView.sectionHeight

                    StyledTextLabel {
                        anchors.centerIn: parent
                        text: Boolean(itemData) ? itemData : ""
                    }
                }

                itemDelegate: FlatButton {
                    property var item: Boolean(itemModel) ? itemModel : null

                    width: gridView.cellWidth
                    height: gridView.cellHeight

                    text: Boolean(item) ? item.name : ""
                }
            }
        }
    }

    Component {
        id: slidersSample

        Column {
            spacing: 12

            StyledSlider {
                value: 0.25
            }

            StyledSlider {
                orientation: Qt.Vertical
                value: 0.75
            }
        }
    }

    Component {
        id: numberInputFieldSample

        NumberInputField {
            maxValue: 999
            value: 33
        }
    }

    Component {
        id: timeInputFieldSample

        TimeInputField {
            Component.onCompleted: {
                time = new Date(2021, 1, 3, 1, 23, 44, 3)
            }
        }
    }

    Component {
        id: valueListSample

        ValueList {

            width: 560
            height: 226

            keyRoleName: "name"
            keyTitle: "Name"
            valueRoleName: "age"
            valueTitle: "Age"

            model: ListModel {
                ListElement {
                    name: "Alex"
                    age: 12
                    valueType: "Int"
                    min: 1
                    max: 15
                }
                ListElement {
                    name: "Tony"
                    age: 15
                    valueType: "Int"
                }
                ListElement {
                    name: "Fred"
                    age: 10
                    valueType: "Int"
                }
                ListElement {
                    name: "Emma"
                    age: 5
                    valueType: "Int"
                }
                ListElement {
                    name: "Anna"
                    age: 11
                    valueType: "Int"
                }
            }
        }
    }
}
