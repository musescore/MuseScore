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

import QtQuick 2.0
import MuseScore 3.0
import "assets/defaultsettings.js" as DSettings
import "assets/accidentals.js" as Accidentals

MuseScore {
    title: qsTr("Add Courtesy Accidentals")
    version: "4.0"
    description: qsTr("This plugin adds courtesy accidentals to the score.")
    categoryCode: "composing-arranging-tools"
    thumbnailName: "assets/accidentals.png"
    requiresScore: true

    //  TODO:
    //  Add option to restate accidentals coming from chromatic runs

    //===============================================================================
    //  Settings
    //===============================================================================

    // Cancel single accidentals when preceded by double
    property var setting0: {
        "addNaturals": false
    }

    // Notes in same measure at different octave
    property var setting1: {
        "addAccidentals": true,    //  If to cancel, bracket type
        "bracketType": 0,          //  0 = no brackets, 1 = round, 2 = square
        "parseGraceNotes": true,   //  Include grace notes in calculations and adding
        "durationMode": 0          //  How to parse durations (0": not before, 1": instantaneous, 2": during)
    }

    // Notes in same measure in different staves (of same instrument)
    property var setting2: {
        "addAccidentals": true,
        "bracketType": 0,
        "parseGraceNotes": true,
        "durationMode": 0
    }

    // Notes in same measure, different octave, different staves
    property var setting3: {
        "addAccidentals": true,
        "bracketType": 0,
        "parseGraceNotes": false,
        "durationMode": 0
    }

    // Notes in different measures
    property var setting4: {
        //  same octave
        "a": {
            "addAccidentals": true,
            "bracketType": 0,
            "parseGraceNotes": true
        },
        //  different octaves
        "b": {
            "addAccidentals": true,
            "bracketType": 0,
            "parseGraceNotes": false
        }
    }

    // Notes in different measures & different staves
    property var setting5: {
        "a": {
            "addAccidentals": true,
            "bracketType": 0,
            "parseGraceNotes": false
        },
        "b": {
            "addAccidentals": false,
            "bracketType": 0,
            "parseGraceNotes": false
        }
    }

    // (same) Notes after grace notes that add accidental
    property var setting6: {
        "a": {
            "addAccidentals": true,
            "bracketType": 0
        },
        // add to notes in different staves
        "b": {
            "addAccidentals": true,
            "bracketType": 0
        }
    }

    // Notes over a key change (new bar)
    property var setting7: {
        "addAccidentals": true,    //  If to run
        "bracketType": 0,          //  Bracket type
        "cancelOctaves": false,    //  Cancel in different octaves
        "parseGraceNotes": true,   //  Include grace notes in calculation and adding
        "cancelMode": true         //  Excessive cancelling mode (see setting9)
    }

    // Notes over a key change (mid bar)
    property var setting8: {
        "addAccidentals": true,
        "bracketType": 0,
        "cancelOctaves": false,
        "parseGraceNotes": true,
        "cancelMode": true
    }

    // How to handle excessive cancelling
    property var setting9: {
        "a": true,  // In same measure
        "b": true   // In different measures
    }
    //option true: add accidentals as needed until cancelled in original octave
    //   notes of same tick get cancelled
    //   cancelling has to happen in original staff
    //option false: continue to add accidentals as needed if not previously cancelled in same octave

    onRun: Accidentals.runPlugin("add")

    Settings {
        id: options
        category: "Courtesy Accidentals Plugin"
        property var uSettings
    }
}
