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

    enum AccBracketType { //todo: add enum from api and use it
		NONE: 0,
		ROUND: 1,
		SQUARE: 2,
	}
    enum AccDurationMode {
		NOT_BEFORE: 0,
		INSTANTANEOUS: 1,
		DURING: 2,
	}

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
        "bracketType": 0,
        "parseGraceNotes": true,   //  Include grace notes in calculations and adding
        "parseOrnaments": true,
        "durationMode": 0
    }

    // Notes in same measure in different staves (of same instrument)
    property var setting2: {
        "addAccidentals": true,
        "bracketType": 0,
        "parseGraceNotes": true,
        "parseOrnaments": true,
        "durationMode": 0
    }

    // Notes in same measure, different octave, different staves
    property var setting3: {
        "addAccidentals": true,
        "bracketType": 0,
        "parseGraceNotes": false,
        "parseOrnaments": false,
        "durationMode": 0
    }

    // Notes in different measures
    property var setting4: {
        //  same octave
        "a": {
            "addAccidentals": true,
            "bracketType": 0,
            "parseGraceNotes": true,
			"parseOrnaments": true,
        },
        //  different octaves
        "b": {
            "addAccidentals": true,
            "bracketType": 0,
            "parseGraceNotes": false,
			"parseOrnaments": false,
        }
    }

    // Notes in different measures & different staves
    property var setting5: {
        "a": {
            "addAccidentals": true,
            "bracketType": 0,
            "parseGraceNotes": false,
			"parseOrnaments": false,
        },
        "b": {
            "addAccidentals": false,
            "bracketType": 0,
            "parseGraceNotes": false,
	        "parseOrnaments": false,
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
        "addAccidentals": true,
        "bracketType": 0,
        "cancelOctaves": false,  //  Cancel in different octaves
        "parseGraceNotes": true,
		"parseOrnaments": true,
        "cancelMode": true       //  Excessive cancelling mode (see setting9)
    }

    // Notes over a key change (mid bar)
    property var setting8: {
        "addAccidentals": true,
        "bracketType": 0,
        "cancelOctaves": false,
        "parseGraceNotes": true,
        "parseOrnaments": true,
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

    // (same) Notes after ornaments that add accidental
    property var setting10: {
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

    onRun: Accidentals.runPlugin("add")

    Settings {
        id: options
        category: "Courtesy Accidentals Plugin"
        property var uSettings
    }
}
