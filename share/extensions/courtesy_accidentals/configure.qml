//==============================================
//  Cautionary Accidentals v4.0
//  https://github.com/XiaoMigros/Cautionary-Accidentals
//  Copyright (C)2023 XiaoMigros
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//==============================================

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MuseScore 3.0
import Muse.UiComponents 1.0 as MU
import Muse.Ui 1.0
import "assets"
import "assets/examples"
import "assets/defaultsettings.js" as DSettings

MuseScore {
    title: qsTr("Configure Courtesy Accidentals")
    description: qsTr("Choose when to add courtesy accidentals to your scores, and how they look.")
    version: "4.0"
    categoryCode: "composing-arranging-tools"
    thumbnailName: "assets/accidentals.png"
    requiresScore: false
    pluginType: "dialog"
    height: 400
    width: 480
    id: root

    MU.StyledFlickable {
        id: flickable
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footer.top
        focus: true
        contentWidth: contentItem.childrenRect.width + 2 * mainColumn.x
        contentHeight: contentItem.childrenRect.height + 2 * mainColumn.y
        Keys.onUpPressed: scrollBar.decrease()
        Keys.onDownPressed: scrollBar.increase()
        ScrollBar.vertical: MU.StyledScrollBar {id: scrollBar}
        Column {
            id: mainColumn
            spacing: 0
            width: root.width

            MainMenuSection {
                title: qsTr("General Settings")
                isExpanded: true

                SubMenuSection {
                    id: setting0Image
                    title: qsTr("Double accidentals")

                    MU.CheckBox {
                        id: setting0Box
                        anchors.leftMargin: style.regSpace
                        text: qsTr("Use natural flats/sharps when cancelling double accidentals")
                        onClicked: {checked = !checked; updatesetting0Img()}
                        signal setv(bool checked)
                        onSetv: function(value) {checked = value; updatesetting0Img()}
                    }
                }

                SubMenuSection {
                    id: setting6Image
                    title: qsTr("Restating grace note accidentals")

                    Column {
                        spacing: style.minSpace
                        width: parent.width

                        MU.StyledTextLabel {text: qsTr("In same staff:")}

                        AddAccItem {
                            id: setting6aAcc
                            anchors.leftMargin: style.regSpace
                            anchors.rightMargin: style.regSpace
                            width: parent.width - anchors.leftMargin - anchors.rightMargin
                            onClicked: updatesetting6Img()
                        }
                    }

                    Column {
                        spacing: style.minSpace
                        width: parent.width

                        MU.StyledTextLabel {text: qsTr("In different staves of same instrument:")}

                        AddAccItem {
                            id: setting6bAcc
                            anchors.leftMargin: style.regSpace
                            anchors.rightMargin: style.regSpace
                            width: parent.width - anchors.leftMargin - anchors.rightMargin
                        }
                    }
                }

                SubMenuSection {
                    id: setting9aImage
                    title: qsTr("Cancelling in the same measure")

                    CancelModeItem {
                        id: setting9aCancel
                        width: parent.width - anchors.leftMargin
                        onClicked: updatesetting9aImg()
                    }
                }

                SubMenuSection {
                    id: setting9bImage
                    title: qsTr("Cancelling in the next measure")

                    CancelModeItem {
                        id: setting9bCancel
                        width: parent.width - anchors.leftMargin
                        onClicked: updatesetting9bImg()
                    }
                    bottomPadding: isExpanded ? style.regSpace : 0
                }
            }

            MU.SeparatorLine {width: root.width}

            MainMenuSection {
                title: qsTr("Notes in the same staff")

                SubMenuSection {
                    title: qsTr("Notes in the same octave in the next measure")
                    id: setting4aColumn
                    property bool accOn: setting4aAcc.checked

                    AddAccItem {
                        id: setting4aAcc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - anchors.rightMargin
                        onClicked: updatesetting4aImg()
                    }

                    GraceNotesCheckBox {
                        id: setting4a3Box
                        enabled: setting4aColumn.accOn
                        onChanged: updatesetting4aImg()
                    }
                }

                SubMenuSection {
                    title: qsTr("Notes in different octaves in the same measure")
                    id: setting1Column
                    property bool accOn: setting1Acc.checked

                    AddAccItem {
                        id: setting1Acc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - anchors.rightMargin
                        onClicked: updatesetting1Img()
                    }

                    GraceNotesCheckBox {
                        id: setting13Box
                        enabled: setting1Column.accOn
                        onChanged: updatesetting1Img()
                    }

                    DurationModeItem {
                        id: setting1Duration
                        width: parent.width
                        enabled: setting1Column.accOn
                        onClicked: updatesetting1Img()
                    }
                }

                SubMenuSection {
                    title: qsTr("Notes in different octaves in the next measure")
                    id: setting4bColumn
                    property bool accOn: setting4bAcc.checked

                    AddAccItem {
                        id: setting4bAcc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - anchors.rightMargin
                        onClicked: updatesetting4bImg()
                    }

                    GraceNotesCheckBox {
                        id: setting4b3Box
                        enabled: setting4bColumn.accOn
                        onChanged: updatesetting4bImg()
                    }

                    bottomPadding: isExpanded ? style.regSpace : 0
                }
            }

            MU.SeparatorLine {width: root.width}

            MainMenuSection {
                title: qsTr("Notes in different staves of the same instrument")

                SubMenuSection {
                    title: qsTr("Notes in the same octave in the same measure")
                    id: setting2Column
                    property bool accOn: setting2Acc.checked

                    AddAccItem {
                        id: setting2Acc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - anchors.rightMargin
                        onClicked: updatesetting2Img()
                    }

                    GraceNotesCheckBox {
                        id: setting23Box
                        enabled: setting2Column.accOn
                        onChanged: updatesetting2Img()
                    }

                    DurationModeItem {
                        id: setting2Duration
                        width: parent.width
                        enabled: setting2Column.accOn
                        onClicked: updatesetting2Img()
                    }
                }

                SubMenuSection {
                    title: qsTr("Notes in the same octave in the next measure")
                    id: setting5aColumn
                    property bool accOn: setting5aAcc.checked

                    AddAccItem {
                        id: setting5aAcc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - anchors.rightMargin
                        onClicked: updatesetting5aImg()
                    }

                    GraceNotesCheckBox {
                        id: setting5a3Box
                        enabled: setting5aColumn.accOn
                        onChanged: updatesetting5aImg()
                    }

                }

                SubMenuSection {
                    title: qsTr("Notes in different octaves in the same measure")
                    id: setting3Column
                    property bool accOn: setting3Acc.checked

                    AddAccItem {
                        id: setting3Acc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - anchors.rightMargin
                        onClicked: updatesetting3Img()
                    }

                    GraceNotesCheckBox {
                        id: setting33Box
                        enabled: setting3Column.accOn
                        onChanged: updatesetting3Img()
                    }

                    DurationModeItem {
                        id: setting3Duration
                        width: parent.width
                        enabled: setting3Column.accOn
                        onClicked: updatesetting3Img()
                    }
                }

                SubMenuSection {
                    title: qsTr("Notes in different octaves in the next measure")
                    id: setting5bColumn
                    property bool accOn: setting5bAcc.checked

                    AddAccItem {
                        id: setting5bAcc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - anchors.rightMargin
                        onClicked: updatesetting5bImg()
                    }

                    GraceNotesCheckBox {
                        id: setting5b3Box
                        enabled: setting5bColumn.accOn
                        onChanged: updatesetting5bImg()
                    }

                    bottomPadding: isExpanded ? style.regSpace : 0
                }
            }

            MU.SeparatorLine {width: root.width}

            MainMenuSection {
                title: qsTr("Notes after key signature changes")

                SubMenuSection {
                    title: qsTr("Notes after measure key signature changes")
                    id: setting7Column
                    property bool accOn: setting7Acc.checked

                    AddAccItem {
                        id: setting7Acc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - 2 * parent.padding
                        onClicked: updatesetting7Img()
                    }
                    GraceNotesCheckBox {
                        id: setting74Box
                        key: true
                        enabled: setting7Column.accOn
                        onChanged: updatesetting7Img()
                    }
                    OptionalCancelModeItem {
                        id: setting7Cancel
                        enabled: setting7Column.accOn
                        width: parent.width - 2 * parent.padding
                        onClicked: updatesetting7Img()
                    }
                }

                SubMenuSection {
                    title: qsTr("Notes after mid-measure key signature changes")
                    id: setting8Column
                    property bool accOn: setting8Acc.checked

                    AddAccItem {
                        id: setting8Acc
                        anchors.rightMargin: style.regSpace
                        width: parent.width - 2 * parent.padding
                        onClicked: updatesetting8Img()
                    }
                    GraceNotesCheckBox {
                        id: setting84Box
                        key: true
                        enabled: setting8Column.accOn
                        onChanged: updatesetting8Img()
                    }
                    OptionalCancelModeItem {
                        id: setting8Cancel
                        enabled: setting8Column.accOn
                        width: parent.width - 2 * parent.padding
                        onClicked: updatesetting8Img()
                    }
                    bottomPadding: isExpanded ? style.regSpace : 0
                }
            }
        }
    }
    Rectangle {
        height: style.maxSpace
        anchors.top: flickable.top
        anchors.left: flickable.left
        anchors.right: flickable.right
        anchors.rightMargin: scrollBar.width
        visible: !flickable.atYBeginning
        gradient: Gradient {
            GradientStop { position: 0.0; color: ui.theme.backgroundPrimaryColor }
            GradientStop { position: 1.0; color: "transparent"}
        }
    }
    Rectangle {
        height: style.maxSpace
        anchors.left: flickable.left
        anchors.right: flickable.right
        anchors.rightMargin: scrollBar.width
        anchors.bottom: flickable.bottom
        visible: !flickable.atYEnd
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent"}
            GradientStop { position: 1.0; color: ui.theme.backgroundPrimaryColor }
        }
    }
    Item {
        id: footer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: okButton.height + (2 * style.regSpace) + 1
        MU.SeparatorLine {
            anchors.top: parent.top
        }
        MU.FlatButton {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: style.regSpace
            text: qsTr("Reset Settings")
            onClicked: loadSettings(DSettings.read())
        }
        Row {
            id: okButton
            spacing: style.regSpace
            anchors.margins: style.regSpace
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            MU.FlatButton {
                text: qsTr("Cancel")
                onClicked: quit()
            }
            MU.FlatButton {
                text: qsTr("OK")
                accentButton: true
                onClicked: {
                    options.uSettings = JSON.stringify(writeSettings())
                    quit()
                }
            }
        }
    }
    Component.onCompleted: {
        if (JSON.parse(options.uSettings).edited) {
            loadSettings(JSON.parse(options.uSettings))
        } else {
            loadSettings(DSettings.read())
        }
    }

    component AddAccItem: Item {
        id: addAccItem

        height: bracketBox.height //childrenRect.height
        //requires indicated width

        property alias checked: checkBox.checked
        property alias currentValue: bracketBox.currentValue

        signal clicked
        signal setv(bool checked, int value)

        MU.CheckBox {
            id: checkBox
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Add Courtesy Accidentals")
            onClicked: {checked = !checked; addAccItem.clicked()}
            signal setv(bool checked)
            onSetv: function(value) {checked = value; addAccItem.clicked()}
        }
        BracketBox {
            id: bracketBox
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            enabled: checkBox.checked
            onActivated: addAccItem.clicked()
        }
        onSetv: function(checked, value) {
            checkBox.setv(checked)
            bracketBox.setv(value)
        }
    }

    component BracketBox: Row {
        id: bracketBox
        spacing: 6
        signal activated(int index, var value)
        property var currentValue: control.currentValue
        // opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
        signal setv(int index)

        MU.StyledTextLabel {
            id: label
            text: qsTr("Brackets:")
            anchors.verticalCenter: control.verticalCenter
        }

        MU.StyledDropdown {
            id: control
            currentIndex: 0
            model: [
                {text: qsTr("None"),        value: AccidentalBracket.NONE },
                {text: qsTr("Parentheses"), value: AccidentalBracket.PARENTHESIS },
                {text: qsTr("Brackets"),    value: AccidentalBracket.BRACKET },
                {text: qsTr("Brace"),       value: AccidentalBracket.BRACE },
            ]
            onActivated: function(index, value) {
                currentIndex = index
                bracketBox.activated(index, value)
            }
        }
        onSetv: function(index) {
            control.currentIndex = index
            control.activated(index, MU.Utils.getItemValue(control.model, index, control.valueRole, ""))
        }
    }

    component CancelModeItem: Column {
        id: cancelModeItem
        property int value: radioButtonGroup.currentIndex
        spacing: style.regSpace
        // opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
        width: parent.width

        signal clicked
        signal setv(int value)

        MU.RadioButtonGroup {
            id: radioButtonGroup
            orientation: ListView.Vertical
            spacing: style.regSpace
            //leftPadding: style.regSpace
            width: parent.width
            currentIndex: 0

            model: [
                { "text": qsTr("Stop after note is cancelled in original octave"), "value": 0 },
                { "text": qsTr("Always cancel in all octaves"),                    "value": 1 },
            ]

            delegate: MU.RoundedRadioButton {
                text: modelData["text"]
                checked: cancelModeItem.value == index
                onClicked: cancelModeItem.setv(index)
            }
        }
        onSetv: function (nvalue) {
            radioButtonGroup.currentIndex = nvalue
            clicked()
        }
    }

    component DurationModeItem: MU.StyledGroupBox {
        id: durationModeItem
        spacing: style.regSpace
        anchors.topMargin: style.minSpace
        // opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

        property int value: radioButtonGroup.currentIndex

        signal clicked
        signal setv(int nvalue)

        title: qsTr("Add cautionary accidentals to:")

        MU.RadioButtonGroup {
            id: radioButtonGroup
            orientation: ListView.Vertical
            spacing: style.regSpace
            //leftPadding: style.regSpace
            width: parent.width
            currentIndex: 0

            model: [
                { "text": qsTr("All notes after the note with accidental"),                  "value": 0 },
                { "text": qsTr("Notes on the same beat as the note with accidental"),        "value": 1 },
                { "text": qsTr("Notes played any time throughout the note with accidental"), "value": 2 },
            ]

            delegate: MU.RoundedRadioButton {
                text: modelData["text"]
                checked: index == durationModeItem.value
                onClicked: durationModeItem.clicked()
            }
        }
        onSetv: function (nvalue) {
            radioButtonGroup.currentIndex = nvalue
            clicked()
        }
    }

    component DynamicImage: Rectangle {
        property int realWidth: 400
        property int cornerRadius: style.regSpace
        property string source: "assets/accidentals.png" // something
        radius: cornerRadius
        border.color: ui.theme.accentColor
        border.width: 2
        //Layout.preferredWidth: realWidth
        width: parent.width
        height: image.height + cornerRadius
        //anchors.horizontalCenter: parent.horizontalCenter
        Image {
            id: image
            source: parent.source
            width: parent.width - cornerRadius
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit // ensure it fits
            mipmap: true // smoothing
        }
    }

    component GraceNotesCheckBox: MU.CheckBox {
        property bool key: false
        text: key ? qsTr("Add courtesy if note before key change is a grace note")
                  : qsTr("Add courtesy if note with accidental is a grace note")
        signal changed
        onClicked: {checked = !checked; changed()}
        signal setv(bool checked)
        onSetv: function(value) {checked = value; changed()}
    }

    component MainMenuSection: Column {
        id: mainMenuSection
        property alias title: menuButton.title
        property alias isExpanded: menuButton.isExpanded
        default property alias content: column.children

        width: parent.width
        spacing: 0

        MenuButton {
            id: menuButton
        }
        Column {
            id: column
            spacing: 0
            padding: style.regSpace
            topPadding: 0
            bottomPadding: 0
            width: parent.width
            visible: mainMenuSection.isExpanded
        }
    }

    component MenuButton: Item {
        height: toggle.height + 2 * style.regSpace
        width: parent ? parent.width : 0 //fix

        property alias title: toggle.title
        property alias isExpanded: toggle.isExpanded

        MU.ExpandableBlankSection {
            id: toggle
            isExpanded: false
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: style.regSpace
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: toggle.isExpanded = !toggle.isExpanded
        }
    }

    component OptionalCancelModeItem: MU.StyledGroupBox {
        id: optionalCancelModeItem
        // opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

        property alias checked: checkBox.checked   // both values are currently needed separately, but may be combinable in future
        property alias value: cancelModeItem.value

        signal clicked
        signal setv(bool checked, int value)

        label: MU.CheckBox {
            id: checkBox
            enabled: optionalCancelModeItem.enabled
            // opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
            text: qsTr("Add cautionary accidentals to notes in any octave")
            checked: false
            onClicked: {checked = !checked; optionalCancelModeItem.clicked()}
            signal setv(bool checked)
            onSetv: function(value) {checked = value}
        }

        CancelModeItem {
            id: cancelModeItem
            enabled: optionalCancelModeItem.enabled && checkBox.checked
            onClicked: optionalCancelModeItem.clicked()
        }

        onSetv: function(checked, value) {
            checkBox.setv(checked)
            cancelModeItem.setv(value)
        }
    }

    QtObject {
        id: style
        readonly property int maxSpace: 15
        readonly property int regSpace: 10
        readonly property int minSpace: 5
    }

    component StyledFrame: Frame {
        background: Rectangle {color: "transparent"; border.color: ui.theme.strokeColor}
        padding: style.regSpace
        width: parent.width
    }

    component StyledSeparatorLine:  MU.SeparatorLine {
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.topMargin: style.regSpace
        anchors.bottomMargin: style.regSpace
    }

    component SubMenuSection: Column {
        id: subMenuSection
        property alias title: menuButton.title
        property alias isExpanded: menuButton.isExpanded
        default property alias content: column.children

        property alias source: dynamicImage.source

        width: parent.width
        spacing: 0
        anchors.margins: 0

        MenuButton {
            id: menuButton
        }
        StyledFrame {
            visible: subMenuSection.isExpanded
            width: parent.width - 2 * subMenuSection.parent.padding
            padding: style.regSpace
            bottomPadding: 0

            Column {
                spacing: 0
                width: parent.width
                //height: childrenRect.height

                DynamicImage {
                    id: dynamicImage
                }

                Column {
                    id: column
                    spacing: style.regSpace
                    width: parent.width
                    //height: childrenRect.height
                    padding: style.regSpace
                }
            }
        }
    }

    function loadSettings(settingObj) {
        setting0Box.setv(settingObj.setting0.addNaturals)
        //
        setting1Acc.setv(settingObj.setting1.addAccidentals, settingObj.setting1.bracketType)
        setting13Box.setv(settingObj.setting1.parseGraceNotes)
        setting1Duration.setv(settingObj.setting1.durationMode)
        //
        setting2Acc.setv(settingObj.setting2.addAccidentals, settingObj.setting2.bracketType)
        setting23Box.setv(settingObj.setting2.parseGraceNotes)
        setting2Duration.setv(settingObj.setting2.durationMode)
        //
        setting3Acc.setv(settingObj.setting3.addAccidentals, settingObj.setting3.bracketType)
        setting33Box.setv(settingObj.setting3.parseGraceNotes)
        setting3Duration.setv(settingObj.setting3.durationMode)
        //
        setting4aAcc.setv(settingObj.setting4.a.addAccidentals, settingObj.setting4.a.bracketType)
        setting4a3Box.setv(settingObj.setting4.a.parseGraceNotes)
        //
        setting4bAcc.setv(settingObj.setting4.b.addAccidentals, settingObj.setting4.b.bracketType)
        setting4b3Box.setv(settingObj.setting4.b.parseGraceNotes)
        //
        setting5aAcc.setv(settingObj.setting5.a.addAccidentals, settingObj.setting5.a.bracketType)
        setting5a3Box.setv(settingObj.setting5.a.parseGraceNotes)
        //
        setting5bAcc.setv(settingObj.setting5.b.addAccidentals, settingObj.setting5.b.bracketType)
        setting5b3Box.setv(settingObj.setting5.b.parseGraceNotes)
        //
        setting6aAcc.setv(settingObj.setting6.a.addAccidentals, settingObj.setting6.a.bracketType)
        //
        setting6bAcc.setv(settingObj.setting6.b.addAccidentals, settingObj.setting6.b.bracketType)
        //
        setting7Acc.setv(settingObj.setting7.addAccidentals, settingObj.setting7.bracketType)
        setting7Cancel.setv(settingObj.setting7.cancelOctaves, settingObj.setting7.cancelMode ? 1 : 2)
        setting74Box.setv(settingObj.setting7.parseGraceNotes)
        //
        setting8Acc.setv(settingObj.setting8.addAccidentals, settingObj.setting8.bracketType)
        setting8Cancel.setv(settingObj.setting8.cancelOctaves, settingObj.setting8.cancelMode ? 1 : 2)
        setting84Box.setv(settingObj.setting8.parseGraceNotes)
        //
        setting9aCancel.setv(settingObj.setting9.a ? 1 : 2)
        //
        setting9bCancel.setv(settingObj.setting9.a ? 1 : 2)
    }
    function writeSettings() {
        var settingObj = {}
        settingObj.edited = true
        settingObj.setting0 = {
            addNaturals: setting0Box.checked
        }
        settingObj.setting1 = {
            addAccidentals: setting1Acc.checked,
            bracketType: setting1Acc.currentValue,
            parseGraceNotes: setting13Box.checked,
            durationMode: setting1Duration.value
        }
        settingObj.setting2 = {
            addAccidentals: setting1Acc.checked,
            bracketType: setting1Acc.currentValue,
            parseGraceNotes: setting13Box.checked,
            durationMode: setting1Duration.value
        }
        settingObj.setting3 = {
            addAccidentals: setting3Acc.checked,
            bracketType: setting3Acc.currentValue,
            parseGraceNotes: setting33Box.checked,
            durationMode: setting3Duration.value
        }
        settingObj.setting4 = {
            a: {
                addAccidentals: setting4aAcc.checked,
                bracketType: setting4aAcc.currentValue,
                parseGraceNotes: setting4a3Box.checked
            },
            b: {
                addAccidentals: setting4bAcc.checked,
                bracketType: setting4bAcc.currentValue,
                parseGraceNotes: setting4b3Box.checked
            }
        }
        settingObj.setting5 = {
            a: {
                addAccidentals: setting5aAcc.checked,
                bracketType: setting5aAcc.currentValue,
                parseGraceNotes: setting5a3Box.checked
            },
            b: {
                addAccidentals: setting5bAcc.checked,
                bracketType: setting5bAcc.currentValue,
                parseGraceNotes: setting5b3Box.checked
            }
        }
        settingObj.setting6 = {
            a: {
                addAccidentals: setting6aAcc.checked,
                bracketType: setting6aAcc.currentValue
            },
            b: {
                addAccidentals: setting6bAcc.checked,
                bracketType: setting6bAcc.currentValue
            }
        }
        settingObj.setting7 = {
            addAccidentals: setting7Acc.checked,
            bracketType: setting7Acc.currentValue,
            cancelOctaves: setting7Cancel.checked,
            parseGraceNotes: setting74Box.checked,
            cancelMode: setting7Cancel.value == 1
        }
        settingObj.setting8 = {
            addAccidentals: setting8Acc.checked,
            bracketType: setting8Acc.currentValue,
            cancelOctaves: setting8Cancel.checked,
            parseGraceNotes: setting84Box.checked,
            cancelMode: setting8Cancel.value == 1
        }
        settingObj.setting9 = {
            a: setting9aCancel.value == 1,
            b: setting9bCancel.value == 1
        }
        return settingObj
    }
    function updatesetting0Img() {
        setting0Image.source = "assets/examples/setting0/example-" + setting0Box.checked.toString() + ".svg"
    }
    function updatesetting1Img() {
        var imgsource = "assets/examples/setting1/example-"
        if (setting1Acc.checked) {
            imgsource += setting1Acc.currentValue.toString()
            imgsource += setting1Duration.value.toString()
        } else {
            imgsource += "false"
        }
        imgsource += ".svg"
        setting1Column.source = imgsource
    }

    function updatesetting2Img() {
        var imgsource = "assets/examples/setting2/example-"
        if (setting2Acc.checked) {
            imgsource += setting2Acc.currentValue.toString()
            imgsource += setting2Duration.value.toString()
        } else {
            imgsource += "false"
        }
        imgsource += ".svg"
        setting2Column.source = imgsource
    }
    function updatesetting3Img() {
        var imgsource = "assets/examples/setting3/example-"
        if (setting3Acc.checked) {
            imgsource += setting3Acc.currentValue.toString()
            imgsource += setting3Duration.value.toString()
        } else {
            imgsource += "false"
        }
        imgsource += ".svg"
        setting3Column.source = imgsource
    }
    function updatesetting4aImg() {
        var imgsource = "assets/examples/setting4a/example-"
        if (setting4aAcc.checked) {
            imgsource += setting4a3Box.checked ? "1" : "0"
            imgsource += setting4aAcc.currentValue.toString()
        } else {
            imgsource += "false"
        }
        imgsource += ".svg"
        setting4aColumn.source = imgsource
    }
    function updatesetting4bImg() {
        var imgsource = "assets/examples/setting4b/example-"
        if (setting4bAcc.checked) {
            imgsource += setting4b3Box.checked ? "1" : "0"
            imgsource += setting4bAcc.currentValue.toString()
        } else {
            imgsource += "false"
        }
        imgsource += ".svg"
        setting4bColumn.source = imgsource
    }
    function updatesetting5aImg() {
        var imgsource = "assets/examples/setting5a/example-"
        imgsource += setting5aAcc.checked ? (setting5aAcc.currentValue + 2).toString() : "1"
        imgsource += ".svg"
        setting5aColumn.source = imgsource
    }
    function updatesetting5bImg() {
        var imgsource = "assets/examples/setting5b/example-"
        imgsource += setting5bAcc.checked ? (setting5bAcc.currentValue + 2).toString() : "1"
        imgsource += ".svg"
        setting5bColumn.source = imgsource
    }
    function updatesetting6Img() {
        var imgsource = "assets/examples/setting6/example-"
        imgsource += setting6aAcc.checked ? (setting6aAcc.currentValue + 2).toString() : "1"
        imgsource += ".svg"
        setting6Image.source = imgsource
    }
    function updatesetting7Img() {
        var imgsource = "assets/examples/setting7/example-"
        if (setting7Acc.checked) {
            imgsource += setting74Box.checked ? "1" : "0"
            imgsource += setting7Acc.currentValue.toString()
            imgsource += setting7Cancel.checked ? setting7Cancel.value.toString() : "0"
        } else {
            imgsource += "false"
        }
        imgsource += ".svg"
        setting7Column.source = imgsource
    }
    function updatesetting8Img() {
        var imgsource = "assets/examples/setting8/example-"
        if (setting8Acc.checked) {
            imgsource += setting8Acc.currentValue.toString()
            imgsource += setting8Cancel.checked ? setting8Cancel.value.toString() : "0"
        } else {
            imgsource += "false"
        }
        imgsource += ".svg"
        setting8Column.source = imgsource
    }
    function updatesetting9aImg() {
        setting9aImage.source = "assets/examples/setting9a/example-" + setting9aCancel.value.toString() + ".svg"
    }
    function updatesetting9bImg() {
        setting9bImage.source = "assets/examples/setting9b/example-" + setting9bCancel.value.toString() + ".svg"
    }
    Settings {
        id: options
        category: "Courtesy Accidentals Plugin"
        property var uSettings: '{
            "version": "4.0-beta",
            "edited": false
        }'
        //Qt.labs.settings doesn't like working with object types
    }
}
