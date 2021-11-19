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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

StyledDialogView {
    id: root

    title: qsTrc("appshell", "Getting started")

    contentWidth: 576
    contentHeight: 384

    margins: 24

    FirstLaunchSetupModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    onAboutToClose: function(closeEvent) {
        if (model.canFinish) {
            model.finish()
            return
        }

        let shouldClose = model.askAboutClosingEarly()
        if (!shouldClose) {
            closeEvent.accepted = false
            return
        }

        model.finish()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        spacing: 24

        PageIndicator {
            Layout.alignment: Qt.AlignCenter
            count: model.numberOfPages
            currentIndex: model.currentPageIndex
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: -8
            source: model.currentPage.url
        }

        Item {
            Layout.fillWidth: true
            height: childrenRect.height

            FlatButton {
                anchors.left: parent.left

                text: qsTrc("global", "Back")
                visible: model.canGoBack

                onClicked: {
                    model.currentPageIndex--
                }
            }

            FlatButton {
                anchors.right: parent.right

                text: model.canFinish ? qsTrc("appshell", "Finish")
                                      : model.canSkip ? qsTrc("appshell", "Skip for now")
                                                      : qsTrc("global", "Next")
                accentButton: !model.canSkip

                onClicked: {
                    if (model.canFinish) {
                        model.finish()
                        root.hide()
                        return
                    }

                    model.currentPageIndex++
                }
            }
        }
    }
}
