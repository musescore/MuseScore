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

import Muse.Ui
import Muse.UiComponents

CheckBox {
    id: root

    property string termsOfUseUrl: ""

    readonly property string termsOfUseText: qsTrc("project/convert", "Terms of Use")

    //: %1 is the "Terms of Use" link
    readonly property string agreementText: qsTrc("project/convert",
                                                  "I agree to the %1 and confirm my use doesn’t infringe anyone’s copyright or other rights.")

    text: root.agreementText.arg("<a href=\"" + root.termsOfUseUrl + "\">" + root.termsOfUseText + "</a>")

    navigation.accessible.name: root.agreementText.arg(root.termsOfUseText)

    onClicked: {
        root.checked = !root.checked
    }
}
