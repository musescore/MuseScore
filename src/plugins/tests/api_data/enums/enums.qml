/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

import MuseScore 3.0

MuseScore {
    version: "1.0"
    description: "This plugin tests enums of the API"
    title: "Test enums"
    requiresScore: false
    categoryCode: "tests"

    // Let's use -1 as an initial value, to distinguish between "successfully run" and "not run at all".
    property int errorCount: -1

    function check(v) {
        if (!v) {
            // Will print stack trace
            console.exception("Check failed!")
            ++errorCount
        }
    }

    function countEnumCases(e) {
        let count = 0
        for (let c in e) {
            ++count
        }
        return count
    }

    // Procedure: we assert that the enum is not empty, and then we assert that a random case is not null/undefined
    // (Not a totally random case, because obviously some cases have and should have value 0)
    // For some enums, we do some additional sanity checks
    onRun: {
        errorCount = 0

        console.log("Accidental:", Accidental)
        check(countEnumCases(Accidental))
        check(Accidental.SHARP)

        console.log("Element:", Element)
        check(countEnumCases(Element))
        check(Element.CHORD)

        console.log("Beam:", Beam)
        check(countEnumCases(Beam))
        check(Beam.MID)

        console.log("Placement:", Placement)
        check(countEnumCases(Placement))
        check(Placement.BELOW)

        console.log("Glissando:", Glissando)
        check(countEnumCases(Glissando))
        check(Glissando.WAVY)

        console.log("LayoutBreak:", LayoutBreak)
        check(countEnumCases(LayoutBreak))
        check(LayoutBreak.SECTION)

        console.log("Lyrics:", Lyrics)
        check(countEnumCases(Lyrics))
        check(Lyrics.MIDDLE)

        console.log("Direction:", Direction)
        check(countEnumCases(Direction))
        check(Direction.DOWN)

        console.log("DirectionH:", DirectionH)
        check(countEnumCases(DirectionH))
        check(DirectionH.RIGHT)

        console.log("OrnamentStyle:", OrnamentStyle)
        check(countEnumCases(OrnamentStyle))
        check(OrnamentStyle.BAROQUE)

        console.log("GlissandoStyle:", GlissandoStyle)
        check(countEnumCases(GlissandoStyle))
        check(GlissandoStyle.PORTAMENTO)

        console.log("Tid:", Tid)
        check(countEnumCases(Tid))
        check(Tid.COMPOSER)

        console.log("Align:", Align)
        check(countEnumCases(Align))
        check(Align.CENTER)
        check(Align.CENTER === Align.HCENTER | Align.VCENTER)
        // TODO: failing!
        //check((Align.BOTTOM | Align.RIGHT) & Align.HMASK === Align.RIGHT)
        //check((Align.BOTTOM | Align.RIGHT) & Align.VMASK === Align.BOTTOM)

        console.log("NoteType:", NoteType)
        check(countEnumCases(NoteType))
        check(NoteType.APPOGGIATURA)

        console.log("PlayEventType:", PlayEventType)
        check(countEnumCases(PlayEventType))
        check(PlayEventType.User)

        console.log("NoteHeadType:", NoteHeadType)
        check(countEnumCases(NoteHeadType))
        check(NoteHeadType.HEAD_BREVIS)

        console.log("NoteHeadScheme:", NoteHeadScheme)
        check(countEnumCases(NoteHeadScheme))
        check(NoteHeadScheme.HEAD_SHAPE_NOTE_7_FUNK)

        console.log("NoteHeadGroup:", NoteHeadGroup)
        check(countEnumCases(NoteHeadGroup))
        check(NoteHeadGroup.HEAD_DIAMOND)

        console.log("NoteValueType:", NoteValueType)
        check(countEnumCases(NoteValueType))
        check(NoteValueType.USER_VAL)

        console.log("Segment:", Segment)
        check(countEnumCases(Segment))
        check(Segment.Clef)
        check(Segment.BarLineType
                       === Segment.BeginBarLine | Segment.StartRepeatBarLine | Segment.BarLine | Segment.EndBarLine)

        console.log("Spanner:", Spanner)
        check(countEnumCases(Spanner))
        check(Spanner.CHORD) // This is about spanner anchors

        console.log("SymId:", SymId)
        check(countEnumCases(SymId))
        check(SymId.accSagittal5v13MediumDiesisUp)

        console.log("HarmonyType:", HarmonyType)
        check(countEnumCases(HarmonyType))
        check(HarmonyType.NASHVILLE)
    }
}
