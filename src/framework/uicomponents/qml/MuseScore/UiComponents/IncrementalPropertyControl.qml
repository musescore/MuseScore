import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property alias iconModeEnum: _iconModeEnum
    property int iconMode: !iconImage.isEmpty ? iconModeEnum.left : iconModeEnum.hidden
    property int iconBackgroundSize: 20
    property alias icon: iconImage.iconCode

    property alias isIndeterminate: textInputField.isIndeterminate
    property alias currentValue: textInputField.currentText
    property real step: 0.5
    property int decimals: 2
    property real maxValue: 999
    property real minValue: -999
    property alias validator: textInputField.validator
    property alias measureUnitsSymbol: textInputField.measureUnitsSymbol

    readonly property int spacing: 8

    signal valueEdited(var newValue)

    implicitHeight: 30
    implicitWidth: parent.width

    function increment() {
        var value = root.isIndeterminate ? 0.0 : currentValue
        var newValue = value + step

        if (newValue > root.maxValue)
            return

        root.valueEdited(+newValue.toFixed(decimals))
    }

    function decrement() {
        var value = root.isIndeterminate ? 0.0 : currentValue
        var newValue = value - step

        if (newValue < root.minValue)
            return

        root.valueEdited(+newValue.toFixed(decimals))
    }

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Up:
            increment()
            break
        case Qt.Key_Down:
            decrement()
            break
        }
    }

    QtObject {
        id: _iconModeEnum

        readonly property int left: 1
        readonly property int right: 2
        readonly property int hidden: 3
    }

    Rectangle {
        id: iconBackground

        anchors.verticalCenter: parent.verticalCenter

        height: root.iconBackgroundSize
        width: root.iconBackgroundSize

        color: ui.theme.buttonColor
        opacity: root.enabled ? ui.theme.buttonOpacityNormal : ui.theme.itemOpacityDisabled

        visible: !iconImage.isEmpty

        StyledIconLabel {
            id: iconImage

            anchors.fill: parent
        }
    }

    TextInputField {
        id: textInputField

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        DoubleInputValidator {
            id: doubleInputValidator
            top: maxValue
            bottom: minValue
            decimal: decimals
        }

        IntInputValidator {
            id: intInputValidator
            top: maxValue
            bottom: minValue
        }

        validator: decimals > 0 ? doubleInputValidator : intInputValidator

        ValueAdjustControl {
            id: valueAdjustControl

            anchors.verticalCenter: textInputField.verticalCenter
            anchors.right: textInputField.right

            icon: IconCode.SMALL_ARROW_DOWN

            onIncreaseButtonClicked: increment()

            onDecreaseButtonClicked: decrement()
        }

        onCurrentTextEdited: {
            var newVal = parseFloat(newTextValue)

            if (isNaN(newVal)) {
                newVal = 0
            }

            root.valueEdited(+newVal.toFixed(decimals))
        }
    }

    states: [
        State {
            name: "ICON_ALIGN_LEFT"
            when: root.iconMode === iconModeEnum.left

            AnchorChanges { target: iconBackground; anchors.left: root.left }

            PropertyChanges { target: iconBackground; visible: true }

            AnchorChanges { target: textInputField; anchors.left: iconBackground.right }

            PropertyChanges { target: textInputField; anchors.leftMargin: spacing
                                                          width: root.width - iconBackground.width - root.spacing }
        },

        State {
            name: "ICON_ALIGN_RIGHT"
            when: root.iconMode === iconModeEnum.right

            AnchorChanges { target: textInputField; anchors.left: root.left }

            PropertyChanges { target: textInputField; width: root.width - iconBackground.width - root.spacing }

            AnchorChanges { target: iconBackground; anchors.left: textInputField.right }

            PropertyChanges { target: iconBackground; anchors.leftMargin: spacing
                                                      visible: true }
        },

        State {
            name: "ICON_MODE_HIDDEN"
            when: root.iconMode === iconModeEnum.hidden

            AnchorChanges { target: textInputField; anchors.left: root.left }

            PropertyChanges { target: textInputField; width: root.width }

            PropertyChanges { target: iconBackground; visible: false }
        }
    ]
}
