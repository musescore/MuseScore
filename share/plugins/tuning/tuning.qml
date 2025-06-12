// Apply a choice of tempraments and tunings.
// Copyright (C) 2018-2019  Bill Hails
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Muse.UiComponents 
import Muse.Ui
import MuseScore 3.0
import FileIO 

MuseScore {
    id: window
    version: "4"
    title: "Temperaments"
    description: "Apply various temperaments and tunings"
    pluginType: "dialog"
    categoryCode: "playback"
    thumbnailName: "modal_tuning.png"
    
    width: 800
    height: 640    

    /**
     * See http://leware.net/temper/temper.htm and specifically http://leware.net/temper/cents.htm
     *
     * I've taken the liberty of adding the Bach/Lehman temperament http://www.larips.com which was
     * my original motivation for doing this.
     *
     * These values are in cents. One cent is defined as 100th of an equal tempered semitone.
     * Each row is ordered in the cycle of fifths, so C, G, D, A, E, B, F#, C#, G#/Ab, Eb, Bb, F;
     * and the values are offsets from the equal tempered value.
     *
     * However for tunings who's default root/pure note is not C, the values are pre-rotated and adjusted accordingly. 
     */
    
    property var defaultWesternTemperaments: [
        {
            'name': "Equal",
            'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Pythagorean",
            'offsets': [-6, -4, -2, 0, 2, 4, -18, -16, -14, -12, -10, -8],
            'root': 9,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Aaron",
            'offsets': [10.5, 7, 3.5, 0, -3.5, -7, 31.5, 28, 24.5, 21, 17.5, 14],
            'root': 9,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Silberman",
            'offsets': [5, 3.3, 1.7, 0, -1.7, -3.3, 15, 13.3, 11.7, 10, 8.3, 6.7],
            'root': 9,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Salinas",
            'offsets': [16, 10.7, 5.3, 0, -5.3, -10.7, 48, 42.7, 37.3, 32, 26.7, 21.3],
            'root': 9,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Kirnberger",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, -12.0, -10.0, -10.0, -8.0, -6.0, -4.0, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Vallotti",
            'offsets': [0.0, -2.0, -4.0, -6.0, -8.0, -10.0, -8.0, -6.0, -4.0, -2.0, 0.0, 2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Werkmeister",
            'offsets': [0.0, -4.0, -8.0, -12.0, -10.0, -8.0, -12.0, -10.0, -8.0, -6.0, -4.0, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Marpurg",
            'offsets': [0.0, 2.0, 4.0, 6.0, 0.0, 2.0, 4.0, 6.0, 0.0, 2.0, 4.0, 6.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Just",
            'offsets': [0.0, 2.0, 4.0, -16.0, -14.0, -12.0, -10.0, -30.0, -28.0, 16.0, 18.0, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Mean Semitone",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, 3.5, 0.0, -3.5, -7.0, -10.5, -14.0, -17.5],
            'root': 6,
            'pure': 6,
            'tweak': 0
        },
        {
            'name': "Grammateus",
            'offsets': [-2, 0, 2, 4, 6, -4, -2, 0, 2, 4, -6, -4],
            'root': 11,
            'pure': 1,
            'tweak': 0
        },
        {
            'name': "French",
            'offsets': [0.0, -2.5, -5.0, -7.5, -10.0, -12.5, -13.0, -13.0, -11.0, -6.0, -1.5, 2.5],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "French 2",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, -17.5, -18.2, -19.0, -17.0, -10.5, -3.5, 3.5],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Rameau",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, -17.5, -15.5, -13.5, -11.5, -2.0, 7.0, 3.5],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Irr Fr 17e",
            'offsets': [9, 6, 3, 0, -3, -6, 10, 16, 21, 18, 15, 12],
            'root': 9,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Bach Lehman",
            'offsets': [5.9, 3.9, 2, 0, -1.9, 0, 2, 3.9, 3.9, 3.9, 3.9, 7.9],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Meantone (1/4) 5 flats",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, 30.8, 27.4, 24.0, 20.5, 17.1, 13.7],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Meantone 1/4-comma",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, -10.3, -13.7, -17.1, 20.5, 17.1, 13.7],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Meantone (1/4) 5 sharps",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, -10.3, -13.7, -17.1, -20.5, -24.0, 13.7],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Meantone 1/5-comma",
            'offsets': [7.0, 4.7, 2.3, 0.0, -2.3, -4.7, -7.0, -9.4, -11.7, 14.1, 11.7, 9.4],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Meantone 1/6-comma",
            'offsets': [4.9, 3.3, 1.6, 0.0, -1.6, -3.3, -4.9, -6.5, -8.1, 9.8, 8.1, 6.5],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Werckmeister III",
            'offsets': [11.7, 7.8, 3.9, 0.0, 2.0, 3.9, 0.0, 2.0, 3.9, 5.9, 7.8, 9.8],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Kirnberger III",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -1.5, 0.5, 0.5, 2.4, 4.4, 6.4, 8.3],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Vallotti",
            'offsets': [5.9, 3.9, 2.0, 0.0, -2.0, -3.9, -2.0, 0.0, 2.0, 3.9, 5.9, 7.8],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Young I",
            'offsets': [6.2, 4.2, 2.1, 0.0, -2.1, -1.9, -1.8, 0.1, 2.1, 4.1, -0.3, -0.1],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Kellner",
            'offsets': [8.2, 5.5, 2.7, 0.0, -2.7, -0.8, -3.5, -1.6, 0.4, 2.3, 4.3, 6.3],
            'root': 0,
            'pure': 3,
            'tweak': 0
        },
        {
            'name': "Fernando A. Martin 1/45-comma",
            'offsets': [6.6, 3.9, 1.7, 0.0, -1.2, -1.8, -2.0, -1.6, -0.7, 0.8, 2.7, 4.7],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "C Cm Db Dm Eb Ebm Em F Fm Ab Am Bb",
            'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, 31.3, 11.7, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "C Cm C#m D Dm E Em F F#m A Am Bb",
            'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, -31.3, -29.3, -27.4, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "C Cm Db D Dm Em F Fm Ab Am Bb Bbm",
            'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, -31.3, 11.7, 13.7, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "C Cm Db Eb Em F Fm G Gm Ab Am Bm",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "C Cm D Dm Eb Em F#m G Gm Ab Bb Bm",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, 13.7, 15.6, 17.6, 19.6],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "C D Ebm E Em F# F#m G G#m Bbm B Bm",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, -27.4, -25.4, -23.5, -21.5],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Simple Ratios",
            'offsets': [0.0, 2.0, 31.2, -15.6, -13.7, -28.3, -17.5, 38.6, 13.7, 15.6, -31.2, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Alternate Ratios",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, 17.5, 28.3, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        }
    ]

    property var defaultMiddleEasternTemperaments: [
        {
            'name': "Melodic 1# 2b",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Harmonic 1# 2b",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Rast, Sikah",
            'offsets': [0.0, 2.0, 3.9, 5.9, -35.2, -33.2, -9.8, -7.8, -7.8, -25.4, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Suznak, Huzam",
            'offsets': [0.0, 2.0, 3.9, 5.9, -35.2, -11.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Nayruz",
            'offsets': [0.0, 2.0, 3.9, -37.1, -35.2, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Bayati, Kurd, Huseyni",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -54.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Qarjighar",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -11.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Saba, Basta Nikar, Zanjaran",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -33.2, 9.8, 11.7, 13.7, 43.3, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Hijaz, Nikriz",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -33.2, -9.8, -7.8, 13.7, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Nawa'athar, Shad Araban",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Shehnaz",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Nahawand, Hijaz Kar",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, 11.7, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "hawand, Hijaz Kar Kurd",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Iraq, Yekah, Nawa",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -33.2, -31.3, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Farahnak, Yekah, Nawa",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, -33.2, -31.3, -29.3, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Jiharkah",
            'offsets': [0.0, 2.0, 3.9, -15.6, -35.2, -11.7, 9.8, 11.7, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Ajam Ashyran, Shawq Afza",
            'offsets': [0.0, 2.0, -17.6, -15.6, 7.8, 9.8, 9.8, 11.7, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Hisar",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, 43.3, 17.6, 19.6],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Nishaburek (Rast in D & A)",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -31.3, -29.3, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Nishaburek (Rast in D, Bayati in A)",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, -54.7, -31.3, -29.3, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Saba Zamzam",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -33.2, 9.8, 11.7, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Rakb",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -33.2, 58.9, 11.7, 13.7, 43.3, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Ikah Baladi",
            'offsets': [0.0, 2.0, 3.9, 5.9, -35.2, -33.2, -9.8, 39.4, 41.4, -25.4, -3.9, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        },
        {
            'name': "Iraq (Cadence)",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -33.2, -31.3, -7.8, 13.7, -5.9, -23.5, -2.0],
            'root': 0,
            'pure': 0,
            'tweak': 0
        }
    ]    

    property var defaultTemperaments: [defaultWesternTemperaments, defaultMiddleEasternTemperaments]
    
    property var temperaments: [[], []] 
        
    property var userTemperaments: [[], []]

    property var currentDefaultTemperament: defaultWesternTemperaments[currentTemperamentIndex]
    property var currentTemperament: temperaments[0][0] 

    property var currentTemperamentIndex: 0
    property var currentRoot: 0
    property var currentPureTone: 0
    property var currentTweak: 0.0

    property var currentListView: tabBar.currentIndex == 0 ? westernListView : middleEasternListView
    property var currentTab: tabBar.currentIndex    

    onRun: {
        var fileData = fileIO.exists() ? JSON.parse(fileIO.read()) : [JSON.parse(JSON.stringify(defaultTemperaments)), 0, 0] // [temperaments, lastOpenTab, lastOpenTemperamentIndex] 
        // reload last session temperament         
        temperaments = fileData[0]          
        tabBar.currentIndex = fileData[1]
        currentTemperamentIndex = fileData[2]
                
        userTemperaments = fileIOUserTemp.exists() ? JSON.parse(fileIOUserTemp.read()) : [[],[]]        

        currentListView.positionViewAtIndex(currentTemperamentIndex, currentListView.Center)
        currentListView.itemAtIndex(currentTemperamentIndex).checked = true                
        currentListView.itemAtIndex(currentTemperamentIndex).clicked()        
    }

    function applyTemperament() {
        var selection = new scoreSelection()
        curScore.startCmd()
        selection.map(filterNotes, reTune(getFinalTuning()))
        if (annotateValue.checked == true) {
            selection.map(filterNotes, annotate)
        }
        curScore.endCmd()
        return true
    }

    function filterNotes(element) {
        return element.type == Element.CHORD
    }

    function annotate(chord, cursor) {
        function addText(noteIndex, placement) {
            var note = chord.notes[noteIndex]
            var text = newElement(Element.STAFF_TEXT);
            text.text = '' + note.tuning
            text.autoplace = true
            text.fontSize = 7 // smaller
            text.placement = placement
            cursor.add(text)
        }

        if (cursor.voice == 0 || cursor.voice == 2) {
            for (var index = 0; index < chord.notes.length; index++) {
                addText(index, Placement.ABOVE)
            }
        } else {
            for (var index = chord.notes.length - 1; index >= 0; index--) {
                addText(index, Placement.BELOW)
            }
        }
    }

    function reTune(tuning) {
        return function(chord, cursor) {
            for (var i = 0; i < chord.notes.length; i++) {
                var note = chord.notes[i]
                note.tuning = tuning(note.pitch)
            }
        }
    }

    function scoreSelection() {
        const SCORE_START = 0
        const SELECTION_START = 1
        const SELECTION_END = 2
        var fullScore
        var startStaff
        var endStaff
        var endTick
        var inRange
        var rewind
        var cursor = curScore.newCursor()
        cursor.rewind(SELECTION_START)
        if (cursor.segment) {
            startStaff = cursor.staffIdx
            cursor.rewind(SELECTION_END)
            endStaff = cursor.staffIdx;
            endTick = 0 // unused
            if (cursor.tick === 0) {
               endTick = curScore.lastSegment.tick + 1;
            } else {
               endTick = cursor.tick;
            }
            inRange = function() {
                return cursor.segment && cursor.tick < endTick
            }
            rewind = function (voice, staff) {
                // no idea why, but if there is a selection then
                // we need to rewind the cursor *before* setting
                // the voice and staff index.
                cursor.rewind(SELECTION_START)
                cursor.voice = voice
                cursor.staffIdx = staff
            }
        } else {
            startStaff = 0
            endStaff  = curScore.nstaves - 1
            inRange = function () {
                return cursor.segment
            }
            rewind = function (voice, staff) {
                // no idea why, but if there's no selection then
                // we need to rewind the cursor *after* setting
                // the voice and staff index.
                cursor.voice = voice
                cursor.staffIdx = staff
                cursor.rewind(SCORE_START)
            }
        }

        this.map = function(filter, process) {
            for (var staff = startStaff; staff <= endStaff; staff++) {
                for (var voice = 0; voice < 4; voice++) {
                    rewind(voice, staff)
                    while (inRange()) {
                        if (cursor.element && filter(cursor.element)) {
                            process(cursor.element, cursor)
                        }
                        cursor.next()
                    }
                }
            }
        }
    }    

    function getTuning(temp=currentTemperament) {
        var dRoot = currentRoot - temp.root
        var rotatedOffsets = temp.offsets.slice((12-dRoot)%12, 12).concat(temp.offsets.slice(0, (12-dRoot)%12)) 
        var pureToneOffset = rotatedOffsets[currentPureTone]                    
        var newOffsets = rotatedOffsets.map(x => x - pureToneOffset + currentTweak)
        return newOffsets
    }

    function getFinalTuning() {
        return function(pitch) {
            pitch = pitch % 12
            return parseFloat(finalOffsets.itemAt(pitch*7 % 12).children[1].currentText )             
        }
    }
    
    function recalculate(tuning=getTuning()) {        
        for (var i=0; i<12; i++) {
            finalOffsets.itemAt(i).children[1].currentText = tuning[i].toFixed(1)
        }           
    }    

    function temperamentClicked(temperament) {
        currentRoot = temperament.root            
        currentPureTone = temperament.pure         
        currentTweak = temperament.tweak

        rootNotes.itemAt(currentRoot).checked=true 
        pureTones.itemAt(currentPureTone).checked=true
        tweakValue.currentValue = currentTweak
        
        recalculate() 
        checkIfEdited()     
    }

    function rootNoteClicked(note) {        
        currentRoot = note        
        currentPureTone = note
        currentTweak = 0.0

        pureTones.itemAt(currentPureTone).checked=true
        tweakValue.currentValue = currentTweak
        
        recalculate() 
        updateCurrentTemperament()    
    }

    function pureToneClicked(note) {        
        currentPureTone = note
        currentTweak = 0.0
        tweakValue.currentValue = currentTweak
        recalculate()  
        updateCurrentTemperament()    
    }

    function tweaked() {  
        currentTweak = tweakValue.currentValue         
        recalculate() 
        updateCurrentTemperament()      
    }    

    function updateCurrentTemperament() {
        currentTemperament.root = currentRoot
        currentTemperament.pure = currentPureTone 
        currentTemperament.tweak = currentTweak       
        for (var i=0; i<12; i++) {
             currentTemperament.offsets[i] = parseFloat(finalOffsets.itemAt(i).children[1].currentText)
        }
        checkIfEdited()
    }
    
    function checkIfEdited() {  
        if (currentDefaultTemperament.root  !== currentTemperament.root ||
            currentDefaultTemperament.pure  !== currentTemperament.pure ||
            currentDefaultTemperament.tweak !== currentTemperament.tweak) {
            resetButton.enabled = true 
            saveButton.enabled = true
            return
        } 
        else {            
            for (var i=0; i<12; i++) {
                if (currentDefaultTemperament.offsets[i] !== currentTemperament.offsets[i]) {
                    resetButton.enabled = true
                    saveButton.enabled = true
                    return
                }                
            }
            resetButton.enabled = false
            saveButton.enabled = false
        }
    }

    StyledTabBar {
        id:tabBar
        width: 400               
        height: 40
        x: 30
        y: 10
        
        currentIndex: 0 

        StyledTabButton {
            id: westernTab 
            text: "Western temperaments" 
        }
        StyledTabButton {
            id: middleEasternTab
            text: "Middle Eastern temperaments" 
        }
    }    

    Column {
        anchors.left: window.left
        anchors.top: window.top
        anchors.leftMargin:20
        anchors.topMargin: 60
        spacing: 12
        RowLayout {
            width: parent.width
            FlatButton {
                id: addButton
                text: "Add"  
                Layout.fillWidth: true               
                onClicked: addDialog.open() 

                StyledPopupView {        
                    id: addDialog                                                                  
                    contentWidth:  addButton.width //c1.childrenRect.width
                    contentHeight: c1.childrenRect.height 
                    onOpened: customTempName.focus = true
                    Column {
                        width: parent.width 
                        id: c1                            
                        spacing: 12
                        TextInputField {                                            
                            id: customTempName                            
                            hint: "Temperament name"  
                            focus: true 
                            onTextEdited: function (newText) { currentText = newText } 
                            onAccepted: {                                
                                addTemperament()
                                addDialog.close()
                                saveCustomTemperaments()                                
                                saveUserTemperaments()
                                customTempName.currentText = ""
                            }                                  
                        }
                        FlatButton {
                            width: parent.width 
                            text: "Ok"
                            accentButton: true
                            onClicked: {                   
                                addTemperament()
                                addDialog.close()
                                saveCustomTemperaments()                                
                                saveUserTemperaments()
                                customTempName.currentText = ""                               
                            }   
                        } 
                    }  
                }
            }
            
            FlatButton {
                id: removeButton
                icon: IconCode.DELETE_TANK              
                enabled: temperaments[currentTab].indexOf(currentTemperament) <  defaultTemperaments[currentTab].length ? false : true
                                                 
                onClicked: {
                    removeTemperament()
                    saveCustomTemperaments()
                    saveUserTemperaments()
                }
            }                    
        }
        GroupBox {                            
            width:260
            height: 500
            ButtonGroup { id: tempGroup }   

            StyledListView {
                id: westernListView
                width: parent.width
                height: parent.height-10                      
                visible: westernTab.checked  
                model: temperaments[0]

                delegate: PageTabButton {
                    width: 280
                    height: 30
                    ButtonGroup.group: tempGroup
                    orientation: Qt.Horizontal
                    leftPadding: 5                    
                    title: modelData.name                    
                    onClicked: {
                        currentTemperamentIndex = index
                        currentTemperament = temperaments[0][index]
                        if (index < defaultWesternTemperaments.length) {
                            currentDefaultTemperament = defaultWesternTemperaments[index]
                        }
                        else {   
                            currentDefaultTemperament = userTemperaments[currentTab][index - defaultWesternTemperaments.length]
                        }
                        temperamentClicked(currentTemperament)
                    }                    
                }        
            }

            StyledListView {
                id: middleEasternListView
                width: parent.width
                height: parent.height-10                   
                visible: middleEasternTab.checked 
                model: temperaments[1]

                delegate: PageTabButton {
                    width: 280
                    height: 30
                    ButtonGroup.group: tempGroup
                    orientation: Qt.Horizontal
                    leftPadding: 5
                    title: modelData.name
                    onClicked: {
                        currentTemperamentIndex = index
                        currentTemperament = temperaments[1][index]
                        if (index < defaultMiddleEasternTemperaments.length) {
                            currentDefaultTemperament = defaultMiddleEasternTemperaments[index]
                        }
                        else {   
                            currentDefaultTemperament = userTemperaments[currentTab][index - defaultMiddleEasternTemperaments.length]
                        }   
                        temperamentClicked(currentTemperament)
                    }
                }
            }
        }
    }

    ColumnLayout {
        id: rightColumn
        anchors.top: parent.top
        anchors.topMargin: 60
        anchors.margins: 20
        spacing: 12
        x: 310
        GroupBox {            
            ColumnLayout {
                spacing: 10
                GroupBox {
                    title: "Root note"
                    GridLayout {
                        columns: 6
                        anchors.margins: 10
                        ButtonGroup { id: rootNoteGroup }
                        Repeater {
                            id: rootNotes
                            model: ["C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "Eb", "Bb", "F"]
                            delegate: FlatRadioButton {
                                ButtonGroup.group: rootNoteGroup
                                text: modelData
                                checked: index==0? true : false
                                onClicked: { rootNoteClicked(index) }
                            }
                        }
                    }
                }

                GroupBox {
                    title: "Pure tone"
                    GridLayout {
                        columns: 6
                        anchors.margins: 10
                        ButtonGroup { id: pureToneGroup }
                        Repeater {
                            id: pureTones
                            model: ["C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "Eb", "Bb", "F"]
                            delegate: FlatRadioButton {
                                ButtonGroup.group: pureToneGroup
                                text: modelData
                                checked: index==0? true : false
                                onClicked: { pureToneClicked(index) }
                            }
                        }
                    }
                }

                GroupBox {
                    title: "Pure tone offset"                    
                    IncrementalPropertyControl {
                        id: tweakValue
                        width: 80
                        maxValue: 99.9                           
                        minValue: -99.9
                        step: 0.1
                        decimals: 1 
                        currentValue: 0.0

                        onValueEdited:  function (newValue) {
                            currentValue = newValue                            
                            tweaked() 
                        }

                        StyledTextLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 30
                            text: "ℂ"                                
                        }  
                    }                      
                }

                GroupBox {
                    title: "Final offsets"
                    GridLayout {
                        columns: 6
                        anchors.margins: 0
                        Repeater {
                            id: finalOffsets
                            model: ["C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "Eb", "Bb", "F"]
                            delegate: Row {
                                StyledTextLabel {
                                    width:20
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData
                                }
                                TextInputField{
                                    width: 50   
                                    currentText: "0.0"                                    
                                    validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }                                                                    
                                    onTextEditingFinished: function (newText) {
                                                                if ( newText != "" ) {
                                                                    currentText = newText
                                                                }
                                                                else { 
                                                                    currentText = "0.0"
                                                                } 
                                                                updateCurrentTemperament()
                                                                recalculate()
                                                                updateCurrentTemperament()
                                                                checkIfEdited()                                                               
                                                            }                                    
                                }
                            }
                        }
                    }
                }
            }
        }

        CheckBox {
            anchors.topMargin: 12
            id: annotateValue
            text: qsTr("Show tuning offset values above notes")
            checked: false
            onClicked: checked = !checked 
        }        
    }

    FlatButton {
        id: resetButton
        icon: IconCode.UNDO
        anchors.top: rightColumn.top
        anchors.right: rightColumn.right
        anchors.margins: 12 
        onClicked: {
            currentTemperament.root  = currentDefaultTemperament.root
            currentTemperament.pure  = currentDefaultTemperament.pure
            currentTemperament.tweak = currentDefaultTemperament.tweak

            for (var i=0; i<12; i++) {
                currentTemperament.offsets[i] = currentDefaultTemperament.offsets[i]
            }   
            temperamentClicked(currentTemperament)
            updateCurrentTemperament()
        }
    }

    FlatButton {
        id: saveButton
        icon: IconCode.SAVE
        anchors.top: buttonBox.top
        anchors.right: buttonBox.left
        anchors.rightMargin: 12 
        visible: temperaments[currentTab].indexOf(currentTemperament) <  defaultTemperaments[currentTab].length ? false : true
                         
        onClicked: {
            var userTempIndex = currentTemperamentIndex - defaultTemperaments[currentTab].length            
            userTemperaments[currentTab][userTempIndex] = JSON.parse(JSON.stringify(currentTemperament))
              
            saveUserTemperaments()
            currentListView.itemAtIndex(currentTemperamentIndex).clicked() 
        }
    }

    ButtonBox {
        id: buttonBox                
        anchors.bottom: window.bottom
        anchors.right: window.right
        anchors.margins: 20
        buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Apply ]                   

        onStandardButtonClicked: function(buttonId) {
            if (buttonId === ButtonBoxModel.Cancel) {                
                quit()                            
            } 
            else if (buttonId === ButtonBoxModel.Apply) {
                if (applyTemperament()) { 
                    saveCustomTemperaments()                   
                    quit()                                
                }
            }
        }
    }    

    FileIO {
        id: fileIO
        source: fileIO.homePath()+"/Documents/MuseScore4/Plugins/tuningPluginData.json" //Qt.resolvedUrl("tuningPluginData.json").toString().replace("file:///", ""); 
    }   
    
    FileIO {
        id: fileIOUserTemp
        source: fileIO.homePath()+"/Documents/MuseScore4/Plugins/tuningPluginData2.json"     
    }

    function addTemperament() {
        var entry= {
                        "name": customTempName.currentText,
                        "offsets": [],
                        "root": currentRoot,
                        "pure": currentPureTone, 
                        "tweak": currentTweak                       
                    } 
        for (var i=0; i<12; i++) {
            entry.offsets.push( parseFloat(finalOffsets.itemAt(i).children[1].currentText) )
        }
        
        temperaments[currentTab].push(entry) //adds new entry and updates buttons 
        currentListView.model = temperaments[currentTab]
        userTemperaments[currentTab].push(JSON.parse(JSON.stringify(entry)))
        currentListView.positionViewAtEnd()
        currentListView.itemAtIndex(temperaments[currentTab].length-1).checked = true
        currentListView.itemAtIndex(temperaments[currentTab].length-1).clicked()
    }

    function removeTemperament() {                   
        var userTempIndex = currentTemperamentIndex - defaultTemperaments[currentTab].length  
        userTemperaments[currentTab] = userTemperaments[currentTab].filter((x,i) => i !== userTempIndex)  
                                    
        temperaments[currentTab] = temperaments[currentTab].filter((x,i) => i !== currentTemperamentIndex)                
        currentListView.model = temperaments[currentTab]
        currentListView.positionViewAtEnd()
        currentListView.itemAtIndex(temperaments[currentTab].length-1).checked = true
        currentListView.itemAtIndex(temperaments[currentTab].length-1).clicked()                
    }

    function saveCustomTemperaments() {
        fileIO.write(
            JSON.stringify([temperaments, currentTab, currentTemperamentIndex])
        ) 
    }    

    function saveUserTemperaments() {
        fileIOUserTemp.write(
            JSON.stringify(userTemperaments)
        )
    }
}

