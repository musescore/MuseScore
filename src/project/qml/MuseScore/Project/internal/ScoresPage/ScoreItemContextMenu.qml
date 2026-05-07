/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick

import Muse.UiComponents

ContextMenuLoader {
    id: root

    property bool isCreateNew: false
    property bool isNoResultsFound: false
    property bool isCloud: false
    property bool showRemoveFromRecentFiles: false

    signal openRequested()
    signal revealInFileBrowserRequested()
    signal viewOnlineRequested()
    signal removeFromRecentFilesRequested()

    items: {
        if (root.isCreateNew || root.isNoResultsFound) {
            return []
        }

        let result = [
            { id: "open", title: qsTrc("project", "Open") }
        ]

        if (root.isCloud) {
            result.push({ id: "view-online", title: qsTrc("project", "View online") })
        } else {
            result.push({ id: "reveal-in-file-browser", title: qsTrc("project", "Reveal in file browser") })
        }

        if (root.showRemoveFromRecentFiles) {
            result.push({ id: "remove-from-recent-files", title: qsTrc("project", "Remove from recent files list") })
        }

        return result
    }

    onHandleMenuItem: function(itemId) {
        switch (itemId) {
        case "open":
            root.openRequested()
            break
        case "view-online":
            root.viewOnlineRequested()
            break
        case "reveal-in-file-browser":
            root.revealInFileBrowserRequested()
            break
        case "remove-from-recent-files":
            root.removeFromRecentFilesRequested()
            break
        }
    }
}
