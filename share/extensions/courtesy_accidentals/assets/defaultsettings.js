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

function read () {
  var settings = {
    version: "4.0-beta",
    setting0: {
      addNaturals: false
    },
    setting1: {
      addAccidentals: true,
      bracketType: AccidentalBracket.NONE,
      parseGraceNotes: true,
      parseOrnaments: true,
      durationMode: 0
    },
    setting2: {
      addAccidentals: true,
      bracketType: AccidentalBracket.NONE,
      parseGraceNotes: true,
      parseOrnaments: true,
      durationMode: 0
    },
    setting3: {
      addAccidentals: true,
      bracketType: AccidentalBracket.NONE,
      parseGraceNotes: false,
      parseOrnaments: false,
      durationMode: 0
    },
    setting4: {
      a: {
        addAccidentals: true,
        bracketType: AccidentalBracket.NONE,
        parseGraceNotes: false,
        parseOrnaments: false
      },
      b: {
        addAccidentals: false,
        bracketType: AccidentalBracket.NONE,
        parseGraceNotes: false,
        parseOrnaments: false
      }
    },
    setting5: {
      a: {
        addAccidentals: true,
        bracketType: AccidentalBracket.NONE,
        parseGraceNotes: false,
        parseOrnaments: false
      },
      b: {
        addAccidentals: false,
        bracketType: AccidentalBracket.NONE,
        parseGraceNotes: false,
        parseOrnaments: false
      }
    },
    setting6: {
      a: {
        addAccidentals: true,
        bracketType: AccidentalBracket.NONE
      },
      b: {
        addAccidentals: false,
        bracketType: AccidentalBracket.NONE
      }
    },
    setting7: {
      addAccidentals: true,
      bracketType: AccidentalBracket.NONE,
      cancelOctaves: false,
      parseGraceNotes: true,
      parseOrnaments: true,
      cancelMode: true
    },
    setting8: {
      addAccidentals: true,
      bracketType: AccidentalBracket.NONE,
      cancelOctaves: true,
      parseGraceNotes: true,
      parseOrnaments: true,
      cancelMode: true
    },
    setting9: {
      a: true,
      b: true
    },
    setting10: {
      a: {
        addAccidentals: true,
        bracketType: AccidentalBracket.NONE
      },
      b: {
        addAccidentals: false,
        bracketType: AccidentalBracket.NONE
      }
    }
  }
  return settings
}
