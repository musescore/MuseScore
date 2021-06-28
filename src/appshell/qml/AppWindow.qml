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

import MuseScore.Ui 1.0
import MuseScore.Shortcuts 1.0
import MuseScore.AppShell 1.0
import MuseScore.Dock 1.0

ApplicationWindow {
    id: root

    width: 1150
    height: 800

    minimumWidth: 1150
    minimumHeight: 600

    visible: true

    MainWindowTitleProvider {
        id: titleProvider
    }

    MainWindowProvider {
        window: root

        //! NOTE These properties of QWindow (of which ApplicationWindow is derived)
        //!      are not available in QML, so we access them via MainWindowProvider
        filePath: titleProvider.filePath
        fileModified: titleProvider.fileModified
    }

    title: titleProvider.title

    ToolTipProvider { }

    //! NOTE Need only create
    Shortcuts { }

    Component.onCompleted: {
        ui.rootItem = root.contentItem
        titleProvider.load()
    }
}
