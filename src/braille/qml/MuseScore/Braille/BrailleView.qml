/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Braille 1.0

StyledFlickable {
    id: root

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "BrailleView"
        enabled: brailleTextArea.enabled && brailleTextArea.visible
        direction: NavigationPanel.Both
    }

    BrailleModel {
        id: brailleModel

        onCurrentItemChanged: {
            if(brailleModel.currentItemPositionStart.valueOf() != -1 &&
                    brailleModel.currentItemPositionEnd.valueOf() != -1) {
                    //brailleInfo.select(brailleModel.currentItemPositionStart.valueOf(), brailleModel.currentItemPositionEnd.valueOf());
                if(brailleTextArea.focus) {
                    brailleTextArea.cursorPosition = brailleModel.currentItemPositionEnd.valueOf();
                }
            }
        }
        onBraillePanelEnabledChanged: {
            root.visible = brailleModel.enabled
        }
        Component.onCompleted: {
            root.visible = brailleModel.enabled
        }
    }

    TextArea.flickable: TextArea {
        id: brailleTextArea
        text: brailleModel.brailleInfo
        wrapMode: Text.AlignLeft

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
            textInputFieldModel.init()
        }

        TextInputFieldModel {
            id: textInputFieldModel
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Tab) {
                //! NOTE: We need to handle Tab key here because https://doc.qt.io/qt-5/qml-qtquick-controls2-textarea.html#tab-focus
                //!       and we don't use qt navigation system
                if (textInputFieldModel.handleShortcut(Qt.Key_Tab, Qt.NoModifier)) {
                    brailleTextArea.focus = false
                    event.accepted = true
                    return
                }
            }

            if (event.key !== Qt.Key_Shift && event.key !== Qt.Key_Alt &&
                    event.key !== Qt.Key_Control) {

                var shortcut = "";

                if(event.modifiers === Qt.ShiftModifier) {
                    shortcut = shortcut === "" ? "Shift" : shortcut += "+Shift";
                }

                if(event.modifiers === Qt.AltModifier) {
                    shortcut = shortcut === "" ? "Alt" : shortcut += "+Alt";
                }
                if(event.modifiers === Qt.ControlModifier) {
                    shortcut = shortcut === "" ? "Ctrl" : shortcut += "+Ctrl";
                }

                if(shortcut !== "") shortcut += "+";

                if(event.key === Qt.Key_Right) {
                    shortcut += "Right"
                } else if(event.key === Qt.Key_Left) {
                    shortcut += "Left"
                } else if(event.key === Qt.Key_Up) {
                    shortcut += "Up"
                } else if(event.key === Qt.Key_Down) {
                    shortcut += "Down"
                } else if(event.key === Qt.Key_PageUp) {
                    shortcut += "PgUp"
                } else if(event.key === Qt.Key_PageDown) {
                    shortcut += "PgDown"
                } else if(event.key === Qt.Key_Home) {
                    shortcut += "Home"
                } else if(event.key === Qt.Key_End) {
                    shortcut += "End"
                }
                if(shortcut !== "Left" && shortcut !== "Right" &&
                        shortcut !== "Up" && shortcut !== "Down") {
                    brailleModel.shorcut = shortcut;
                    event.accepted = true;
                }
            }
        }
    }
}
