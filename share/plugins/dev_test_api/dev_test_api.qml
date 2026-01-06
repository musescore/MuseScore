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

import QtQuick
import MuseScore 3.0

MuseScore {

    id: root

    title: "Test Api (old)"

    onRun: {
        const score = api.engraving.curScore;
        const measure = score.firstMeasure;
        console.log("measure.type:", measure.type
                    , ", api.engraving.Element.MEASURE:", api.engraving.Element.MEASURE
                    , ", Element:", Element
                    , ", Element.MEASURE:", Element.MEASURE
                    )

        if (measure.type === api.engraving.Element.MEASURE) {
            console.log("this is measure")
        } else {
            console.log("this is not measure")
        }
    }
}
