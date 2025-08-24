/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Braille 1.0

StyledFlickable {
    id: root

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "BrailleView"
        enabled: brailleTextArea.enabled && brailleTextArea.visible
        direction: NavigationPanel.Both
    }

    Component.onCompleted: {
        brailleModel.load()
        root.visible = brailleModel.enabled
    }

    BrailleModel {
        id: brailleModel
        property int keys_pressed: 0
        property string keys_buffer: ""

        onCurrentItemChanged: {
            if(brailleModel.currentItemPositionStart.valueOf() != -1 &&
                    brailleModel.currentItemPositionEnd.valueOf() != -1) {
                    //brailleTextArea.select(brailleModel.currentItemPositionStart.valueOf(), brailleModel.currentItemPositionEnd.valueOf());
                if(brailleTextArea.focus) {
                    brailleTextArea.cursorPosition = brailleModel.currentItemPositionEnd.valueOf();
                }
            }
        }

        onBraillePanelEnabledChanged: {
            root.visible = brailleModel.enabled
        }

        onBrailleModeChanged: {
            switch(brailleModel.mode) {
                case 1: {
                    fakeNavCtrl.accessible.setName("Braille: Normal mode");
                    break;
                }
                case 2: {
                    fakeNavCtrl.accessible.setName("Braille: Note input mode");
                    break;
                }
            }
        }
    }

    TextArea.flickable: TextArea {
        id: brailleTextArea
        color: ui.theme.fontPrimaryColor
        font {
            pixelSize: ui.theme.bodyFont.pixelSize
        }
        text: brailleModel.brailleInfo
        wrapMode: Text.AlignLeft

        property var keyMap: new Map([
            [Qt.Key_0, "0"],
            [Qt.Key_1, "1"],
            [Qt.Key_2, "2"],
            [Qt.Key_3, "3"],
            [Qt.Key_4, "4"],
            [Qt.Key_5, "5"],
            [Qt.Key_6, "6"],
            [Qt.Key_7, "7"],
            [Qt.Key_8, "8"],
            [Qt.Key_9, "9"],
            [Qt.Key_A, "A"],
            [Qt.Key_B, "B"],
            [Qt.Key_C, "C"],
            [Qt.Key_D, "D"],
            [Qt.Key_E, "E"],
            [Qt.Key_F, "F"],
            [Qt.Key_G, "G"],
            [Qt.Key_H, "H"],
            [Qt.Key_I, "I"],
            [Qt.Key_J, "J"],
            [Qt.Key_K, "K"],
            [Qt.Key_L, "L"],
            [Qt.Key_M, "M"],
            [Qt.Key_N, "N"],
            [Qt.Key_O, "O"],
            [Qt.Key_P, "P"],
            [Qt.Key_Q, "Q"],
            [Qt.Key_R, "R"],
            [Qt.Key_S, "S"],
            [Qt.Key_T, "T"],
            [Qt.Key_U, "U"],
            [Qt.Key_V, "V"],
            [Qt.Key_W, "W"],
            [Qt.Key_X, "X"],
            [Qt.Key_Y, "Y"],
            [Qt.Key_Z, "Z"],
            [Qt.Key_Space, "Space"],
            [Qt.Key_Right, "Right"],
            [Qt.Key_Left, "Left"],
            [Qt.Key_Up, "Up"],
            [Qt.Key_Down, "Down"],
            [Qt.Key_PageUp, "PageUp"],
            [Qt.Key_PageDown, "PageDown"],
            [Qt.Key_Home, "Home"],
            [Qt.Key_End, "End"],
            [Qt.Key_Delete, "Delete"],
            [Qt.Key_Escape, "Escape"],
            [Qt.Key_Minus, "Minus"],
            [Qt.Key_Plus, "Plus"],
         ])

        cursorDelegate: Rectangle {
            id: brailleCursor
            visible: brailleTextArea.cursorVisible
            color: ui.theme.isDark && brailleModel.cursorColor == "black" ? ui.theme.fontPrimaryColor : brailleModel.cursorColor
            width: brailleTextArea.cursorRectangle.width

            SequentialAnimation {
                loops: Animation.Infinite
                running: brailleTextArea.cursorVisible

                PropertyAction {
                    target: brailleCursor
                    property: 'visible'
                    value: true
                }

                PauseAnimation {
                    duration: 600
                }

                PropertyAction {
                    target: brailleCursor
                    property: 'visible'
                    value: false
                }

                PauseAnimation {
                    duration: 600
                }

                onStopped: {
                    brailleCursor.visible = false
                }
            }
        }

        NavigationControl {
            id: fakeNavCtrl
            name: "Braille"
            enabled: brailleTextArea.enabled && brailleTextArea.visible

            panel: root.navigationPanel
            order: 1

            accessible.role: MUAccessible.EditableText
            accessible.name: "Braille"
            accessible.visualItem: brailleTextArea
            accessible.text: brailleTextArea.text
            accessible.selectedText: brailleTextArea.selectedText
            accessible.selectionStart: brailleTextArea.selectionStart
            accessible.selectionEnd: brailleTextArea.selectionEnd
            accessible.cursorPosition: brailleTextArea.cursorPosition

            onActiveChanged: {
                if (fakeNavCtrl.active) {
                    brailleTextArea.forceActiveFocus();
                    if(brailleModel.currentItemPositionStart.valueOf() != -1 &&
                        brailleModel.currentItemPositionEnd.valueOf() != -1) {
                            brailleTextArea.cursorPosition = brailleModel.currentItemPositionEnd.valueOf();
                    }
                } else {
                    brailleTextArea.focus = false
                }
            }
        }

        NavigationFocusBorder {
            navigationCtrl: fakeNavCtrl
            drawOutsideParent: false
        }

        onCursorPositionChanged: {
            brailleModel.cursorPosition = brailleTextArea.cursorPosition;
        }

        Component.onCompleted: {
            textInputModel.init()
        }

        TextInputModel {
            id: textInputModel
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Tab) {
                //! NOTE: We need to handle Tab key here because https://doc.qt.io/qt-5/qml-qtquick-controls2-textarea.html#tab-focus
                //!       and we don't use qt navigation system
                if (textInputModel.handleShortcut(Qt.Key_Tab, Qt.NoModifier)) {
                    brailleTextArea.focus = false;
                    event.accepted = true;
                    return;
                }
            }

            if (event.key !== Qt.Key_Shift
                && event.key !== Qt.Key_Alt
                && event.key !== Qt.Key_Control
            ) {

                ++brailleModel.keys_pressed;

                var keys = "";

                if (event.modifiers === Qt.ShiftModifier) {
                    keys = keys === "" ? "Shift" : keys += "+Shift";
                }

                if (event.modifiers === Qt.AltModifier) {
                    keys = keys === "" ? "Alt" : keys += "+Alt";
                }

                if (event.modifiers === Qt.ControlModifier) {
                    keys = keys === "" ? "Ctrl" : keys += "+Ctrl";
                }

                if (keys !== "") {
                    keys += "+";
                }

                if (keyMap.get(event.key) !== "") {
                    keys += keyMap.get(event.key);
                }

                if (keys !== "Left"
                    && keys !== "Right"
                    && keys !== "Up"
                    && keys !== "Down"
                ) {
                    if (brailleModel.keys_buffer !== "") {
                        brailleModel.keys_buffer += "+";
                    }
                    brailleModel.keys_buffer += keys;
                    event.accepted = true;
                }
            }
        }

        Keys.onReleased: function(event) {
            if (event.key !== Qt.Key_Shift
                && event.key !== Qt.Key_Alt
                && event.key !== Qt.Key_Control
            ) {
                brailleModel.keys_pressed--;

                if(brailleModel.keys_pressed <= 0) {
                    brailleModel.keys = brailleModel.keys_buffer;
                    brailleModel.keys_buffer = "";
                    brailleModel.keys_pressed = 0;
                    // set the focus back to braille
                    root.navigationPanel.setActive(true);
                    fakeNavCtrl.setActive(true);
                }
            }
        }
    }
}
