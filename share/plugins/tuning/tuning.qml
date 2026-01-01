// Apply a choice of tempraments and tunings.
// Copyright (C) 2018-2019  Bill Hails
// Copyright (C) 2025 XiaoMigros
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents as MU

import MuseScore 3.0

MuseScore {
    version: "4.7"
    title: qsTr("Tuning and temperaments")
    description: qsTr("Apply various temperaments and tunings")
    pluginType: "dialog"
    categoryCode: "playback"
    thumbnailName: "modal_tuning.png"
    id: root
    width: childrenRect.width + 2 * defaultSpacing
    height: childrenRect.height + 2 * defaultSpacing

    /**
     * See http://leware.net/temper/temper.htm and specifically http://leware.net/temper/cents.htm
     *
     * I've taken the liberty of adding the Bach/Lehman temperament http://www.larips.com which was
     * my original motivation for doing this.
     *
     * These values are in cents. One cent is defined as 100th of an equal tempered semitone.
     * Each row is ordered in the cycle of fifths, so C, G, D, A, E, B, F#, C#, G#/Ab, Eb, Bb, F.
     * Values are adjusted for root and 'pure' note before being applied to the score.
     */
    function readDefaults() {
        return [
            { "name": "separatorLine", "displayName": qsTr("Western tuning systems") },
            { "name": "equal",        "offsets": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],                                        "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Equal") },
            { "name": "pythagorean",  "offsets": [0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22],                                 "root": 9,  "pure": 3, "globalOffset": 0, "displayName": qsTr("Pythagorean") },
            { "name": "aaron",        "offsets": [0, -3.5, -7, -10.5, -14, -17.5, -21, -24.5, -28, -31.5, -35, -38.5],        "root": 9,  "pure": 3, "globalOffset": 0, "displayName": qsTr("Aaron") },
            { "name": "silberman",    "offsets": [0, -1.7, -3.3, -5, -6.7, -8.3, -10, -11.7, -13.3, -15, -16.7, -18.3],       "root": 9,  "pure": 3, "globalOffset": 0, "displayName": qsTr("Silberman") },
            { "name": "salinas",      "offsets": [0, -5.3, -10.7, -16, -21.3, -26.7, -32, -37.3, -42.7, -48, -53.3, -58.7],   "root": 9,  "pure": 3, "globalOffset": 0, "displayName": qsTr("Salinas") },
            { "name": "kirnberger",   "offsets": [0, -3.5, -7, -10.5, -14, -12, -10, -10, -8, -6, -4, -2],                    "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Kirnberger") },
            { "name": "vallotti",     "offsets": [0, -2, -4, -6, -8, -10, -8, -6, -4, -2, 0, 2],                              "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Vallotti") },
            { "name": "werkmeister",  "offsets": [0, -4, -8, -12, -10, -8, -12, -10, -8, -6, -4, -2],                         "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Werkmeister") },
            { "name": "marpurg",      "offsets": [0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6],                                        "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Marpurg") },
            { "name": "just",         "offsets": [0, 2, 4, -16, -14, -12, -10, -30, -28, 16, 18, -2],                         "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Just") },
            { "name": "meanSemitone", "offsets": [0, -3.5, -7, -10.5, -14, 3.5, 0, -3.5, -7, -10.5, -14, -17.5],              "root": 6,  "pure": 6, "globalOffset": 0, "displayName": qsTr("Mean semitone") },
            { "name": "grammateus",   "offsets": [0, 2, 4, 6, 8, 10, 12, 2, 4, 6, 8, 10],                                     "root": 11, "pure": 1, "globalOffset": 0, "displayName": qsTr("Grammateus") },
            { "name": "french",       "offsets": [0, -2.5, -5, -7.5, -10, -12.5, -13, -13, -11, -6, -1.5, 2.5],               "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("French") },
            { "name": "french2",      "offsets": [0, -3.5, -7, -10.5, -14, -17.5, -18.2, -19, -17, -10.5, -3.5, 3.5],         "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Tempérament Ordinaire") },
            { "name": "rameau",       "offsets": [0, -3.5, -7, -10.5, -14, -17.5, -15.5, -13.5, -11.5, -2, 7, 3.5],           "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Rameau") },
            { "name": "irrFr17e",     "offsets": [-17, -11, -6, -9, -12, -15, -18, -21, -24, -27, -30, -33],                  "root": 9,  "pure": 3, "globalOffset": 0, "displayName": qsTr("Irr Fr 17e") },
            { "name": "bachLehman",   "offsets": [-5.9, -7.9, -9.8, -11.8, -13.7, -11.8, -9.8, -7.9, -7.9, -7.9, -7.9, -3.9], "root": 0,  "pure": 3, "globalOffset": 0, "displayName": qsTr("Bach/Lehman") },
            // { "name": "separatorLine", "displayName": qsTr("Modal temperaments") },
            { "name": "tuning01", "offsets": [0, -3.5, -6.9, -10.3, -13.7, -17.1, 20.5, 17.1, 13.7, 10.2, 6.8, 3.4],     "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Meantone (1/4) 5 flats") },
            { "name": "tuning02", "offsets": [0, -3.5, -6.9, -10.3, -13.7, -17.1, -20.6, -24, -27.4, 10.2, 6.8, 3.4],    "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Meantone 1/4-comma") },
            { "name": "tuning03", "offsets": [0, -3.5, -6.9, -10.3, -13.7, -17.1, -20.6, -24, -27.4, -30.8, -34.3, 3.4], "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Meantone (1/4) 5 sharps") },
            { "name": "tuning04", "offsets": [0, -2.3, -4.7, -7, -9.3, -11.7, -14, -16.4, -18.7, 7.1, 4.7, 2.4],         "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Meantone 1/5-comma") },
            { "name": "tuning05", "offsets": [0, -1.6, -3.3, -4.9, -6.5, -8.2, -9.8, -11.4, -13, 4.9, 3.2, 1.6],         "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Meantone 1/6-comma") },
            { "name": "tuning06", "offsets": [0, -3.9, -7.8, -11.7, -9.7, -7.8, -11.7, -9.7, -7.8, -5.8, -3.9, -1.9],    "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Werckmeister III") },
            { "name": "tuning07", "offsets": [0, -3.5, -6.9, -10.3, -13.7, -11.8, -9.8, -9.8, -7.9, -5.9, -3.9, -2],     "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Kirnberger III") },
            { "name": "tuning08", "offsets": [0, -2, -3.9, -5.9, -7.9, -9.8, -7.9, -5.9, -3.9, -2, 0, 1.9],              "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Vallotti") },
            { "name": "tuning09", "offsets": [0, -2, -4.1, -6.2, -8.3, -8.1, -8, -6.1, -4.1, -2.1, -6.5, -6.3],          "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Young I") },
            { "name": "tuning10", "offsets": [0, -2.7, -5.5, -8.2, -10.9, -9, -11.7, -9.8, -7.8, -5.9, -3.9, -1.9],      "root": 0, "pure": 3, "globalOffset": 0, "displayName": qsTr("Kellner") },
            { "name": "tuning11", "offsets": [13.2, 10.5, 8.3, 6.6, 5.4, 4.8, 4.6, 5, 5.9, 7.4, 9.3, 11.3],              "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Fernando A. Martin 1/45-comma") },
            { "name": "tuning12", "offsets": [0, 2, -17.6, -15.6, -13.7, -11.7, 31.3, 11.7, 13.7, 15.6, 17.6, -2],       "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("C Cm D♭ Dm E♭ E♭m Em F Fm A♭ Am B♭") },
            { "name": "tuning13", "offsets": [0, 2, -17.6, -15.6, -13.7, -11.7, -31.3, -29.3, -27.4, 15.6, -3.9, -2],    "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("C Cm C♯m D Dm E Em F F♯m A Am B♭") },
            { "name": "tuning14", "offsets": [0, 2, -17.6, -15.6, -13.7, -11.7, -31.3, 11.7, 13.7, 15.6, -3.9, -2],      "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("C Cm D♭ D Dm Em F Fm A♭ Am B♭ B♭m") },
            { "name": "tuning15", "offsets": [0, 2, 3.9, -15.6, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, 17.6, -2],         "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("C Cm D♭ E♭ Em F Fm G Gm A♭ Am Bm") },
            { "name": "tuning16", "offsets": [0, 2, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, 13.7, 15.6, 17.6, 19.6],         "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("C Cm D Dm E♭ Em F♯m G Gm A♭ B♭ Bm") },
            { "name": "tuning17", "offsets": [0, 2, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, -27.4, -25.4, -23.5, -21.5],     "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("C D E♭m E Em F♯ F♯m G G♯m B♭m B Bm") },
            { "name": "tuning18", "offsets": [0, 2, 31.2, -15.6, -13.7, -28.3, -17.5, 38.6, 13.7, 15.6, -31.2, -2],      "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Simple Ratios") },
            { "name": "tuning19", "offsets": [0, 2, 3.9, -15.6, -13.7, -11.7, 17.5, 28.3, 13.7, 15.6, 17.6, -2],         "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Alternate Ratios") },
            { "name": "separatorLine", "displayName": qsTr("Arabic modal systems") },
            { "name": "tuningM01", "offsets": [0, 2, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2],       "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Melodic 1♯ 2♭") },
            { "name": "tuningM02", "offsets": [0, 2, 3.9, -15.6, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, 17.6, -2], "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Harmonic 1♯ 2♭") },
            { "name": "tuningM03", "offsets": [0, 2, 3.9, 5.9, -35.2, -33.2, -9.8, -7.8, -7.8, -25.4, -3.9, -2],  "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Rast, Sikah") },
            { "name": "tuningM04", "offsets": [0, 2, 3.9, 5.9, -35.2, -11.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Suznak, Huzam") },
            { "name": "tuningM05", "offsets": [0, 2, 3.9, -37.1, -35.2, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Nayruz") },
            { "name": "tuningM06", "offsets": [0, 2, 3.9, 5.9, -56.7, -54.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Bayati, Kurd, Huseyni") },
            { "name": "tuningM07", "offsets": [0, 2, 3.9, 5.9, -56.7, -11.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Qarjighar") },
            { "name": "tuningM08", "offsets": [0, 2, 3.9, -15.6, -13.7, -33.2, 9.8, 11.7, 13.7, 43.3, -3.9, -2],  "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Saba, Basta Nikar, Zanjaran") },
            { "name": "tuningM09", "offsets": [0, 2, 3.9, 5.9, -13.7, -33.2, -9.8, -7.8, 13.7, 15.6, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Hijaz, Nikriz") },
            { "name": "tuningM10", "offsets": [0, 2, 3.9, 5.9, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Nawa'athar, Shad Araban") },
            { "name": "tuningM11", "offsets": [0, 2, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, 13.7, 15.6, 17.6, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Shehnaz") },
            { "name": "tuningM12", "offsets": [0, 2, 3.9, 5.9, -13.7, -11.7, -9.8, 11.7, 13.7, -5.9, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Nahawand, Hijaz Kar") },
            { "name": "tuningM13", "offsets": [0, 2, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2],       "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Nahawand, Hijaz Kar Kurd") },
            { "name": "tuningM14", "offsets": [0, 2, 3.9, 5.9, -56.7, -33.2, -31.3, -7.8, 13.7, -5.9, -3.9, -2],  "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Iraq, Yekah, Nawa") },
            { "name": "tuningM15", "offsets": [0, 2, 3.9, 5.9, 7.8, -33.2, -31.3, -29.3, 13.7, -5.9, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Farahnak, Yekah, Nawa") },
            { "name": "tuningM16", "offsets": [0, 2, 3.9, -15.6, -35.2, -11.7, 9.8, 11.7, 13.7, -5.9, -3.9, -2],  "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Jiharkah") },
            { "name": "tuningM17", "offsets": [0, 2, -17.6, -15.6, 7.8, 9.8, 9.8, 11.7, -7.8, -5.9, -3.9, -2],    "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Ajam Ashyran, Shawq Afza") },
            { "name": "tuningM18", "offsets": [0, 2, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, 43.3, 17.6, 19.6],     "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Hisar") },
            { "name": "tuningM19", "offsets": [0, 2, 3.9, 5.9, 7.8, 9.8, -31.3, -29.3, 13.7, -5.9, -3.9, -2],     "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Nishaburek (Rast in D & A)") },
            { "name": "tuningM20", "offsets": [0, 2, 3.9, 5.9, 7.8, -54.7, -31.3, -29.3, 13.7, -5.9, -3.9, -2],   "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Nishaburek (Rast in D, Bayati in A)") },
            { "name": "tuningM21", "offsets": [0, 2, 3.9, -15.6, -13.7, -33.2, 9.8, 11.7, 13.7, -5.9, -3.9, -2],  "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Saba Zamzam") },
            { "name": "tuningM22", "offsets": [0, 2, 3.9, -15.6, -13.7, -33.2, 58.9, 11.7, 13.7, 43.3, -3.9, -2], "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Rakb") },
            { "name": "tuningM23", "offsets": [0, 2, 3.9, 5.9, -35.2, -33.2, -9.8, 39.4, 41.4, -25.4, -3.9, -2],  "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Sikah Baladi") },
            { "name": "tuningM24", "offsets": [0, 2, 3.9, 5.9, -56.7, -33.2, -31.3, -7.8, 13.7, -5.9, -23.5, -2], "root": 0, "pure": 0, "globalOffset": 0, "displayName": qsTr("Iraq (Cadence)") },
            { "name": "separatorLine", "displayName": qsTr("Custom tunings") },
            { "name": "customSlot1",  "offsets": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Custom 1") },
            { "name": "customSlot2",  "offsets": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Custom 2") },
            { "name": "customSlot3",  "offsets": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Custom 3") },
            { "name": "customSlot4",  "offsets": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Custom 4") },
            { "name": "customSlot5",  "offsets": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], "root": 0,  "pure": 0, "globalOffset": 0, "displayName": qsTr("Custom 5") },
        ]
    }

    property var defaultTuning: readDefaults()
    property var tuningModel: readDefaults()

    readonly property int incrementalControlWidth: 60
    readonly property int defaultSpacing: 12

    property alias undoIndex: commandHistory.index
    property int undoLength: 0
    property bool saveIsAvailable: false
    property bool resetIsAvailable: false

    signal refresh()

    readonly property var notesStringModel: [qsTr("C"), qsTr("C♯"), qsTr("D"), qsTr("E♭"), qsTr("E"), qsTr("F"), qsTr("F♯"), qsTr("G"), qsTr("G♯"), qsTr("A"), qsTr("B♭"), qsTr("B")]
    readonly property var fifthsOffsets: [0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5]
    readonly property var chromaticOffsets: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    readonly property var pitchOffsets: chromatic ? chromaticOffsets : fifthsOffsets // convert from C, C#,... to C, G,...
    readonly property int decimals: 1

    property bool chromatic: true
    property int currentTemperament: 1
    property int currentRoot: 0
    property int currentPureTone: 0
    property var currentGlobalOffset: 0.0

    signal refreshTextFields()
    onRefreshTextFields: {
        for (var i in tuningModel[currentTemperament].offsets) {
            finalValueRepeater.itemAt(i).control.currentValue = calculateTuningFromIndex(pitchOffsets[i])
            finalValueRepeater.itemAt(i).control.underlyingValue = tuningModel[currentTemperament].offsets[finalValueRepeater.itemAt(i).control.underlyingIndex]
        }
    }

    onRefresh: {
        saveIsAvailable = formatCurrentValues() != options.data // modifying tuningModel here is intended and necessary
        resetIsAvailable = JSON.stringify(defaultTuning[currentTemperament]) != JSON.stringify(tuningModel[currentTemperament])
        refreshTextFields()
    }

    function calculateTuningFromIndex(index) {
        // index here is modulo pitch
        var rootOffset = tuningModel[currentTemperament].offsets[(fifthsOffsets[index] - currentRoot + 12) % 12]
        var pureToneOffset = defaultTuning[currentTemperament].offsets[(currentPureTone - currentRoot + 12) % 12] - defaultTuning[currentTemperament].offsets[0] // 0 because definitions are aligned to root C / pure C
        return roundValue(rootOffset - pureToneOffset + currentGlobalOffset)
    }

    onRun: {
        tuningModel = restoreSavedValues()
        currentRoot = tuningModel[currentTemperament].root
        currentPureTone = tuningModel[currentTemperament].pure
        currentGlobalOffset = tuningModel[currentTemperament].globalOffset
        root.refresh()
    }

    function applyTemperament() {
        if (curScore.selection.elements.length == 0) {
            curScore.startCmd("Add tuning to score")
            cmd("select-all")
        } else {
            curScore.startCmd("Add tuning to selection")
        }
        var chordList = []
        for (var i in curScore.selection.elements) {
            var el = curScore.selection.elements[i]
            if (el.type == Element.NOTE && el.parent.type == Element.CHORD) {
                var add = true
                for (var j in chordList) {
                    if (chordList[j].is(el.parent)) {
                        add = false
                        break
                    }
                }
                if (add) {
                    chordList.push(el.parent)
                }
            }
        }
        var startPos = curScore.lastMeasure.tick.plus(curScore.lastMeasure.ticks)
        var endPos = fraction(0, 1)
        for (var i in chordList) {
            var chord = chordList[i]

            for (var j in chord.notes) {
                var note = chord.notes[j]
                // Attempt to remove old texts
                // Since annotation always restates all tunings, we don't need to check for selected here
                for (var k in note.elements) {
                    // Ideally: /^-?\d+(\.\d)?$/ - but seems not to work
                    if (note.elements[k].type == Element.TEXT && /-?\d+(\.\d)?/.test(note.elements[k].text)) {
                        note.remove(note.elements[k])
                    }
                }
                // Then apply new tuning
                // list selections could be individual notes
                if (note.selected) {
                    note.tuning = calculateTuningFromIndex(note.pitch % 12)
                }
            }
            if (chord.fraction.lessThan(startPos)) {
                startPos = chord.fraction
            }
            if (chord.fraction.greaterThan(endPos)) {
                endPos = chord.fraction
            }
        }

        // Add new text if needed
        if (annotateBox.checked) {
            curScore.doLayout(startPos, endPos) // needed for real-time layout calculations later
            for (var i in chordList) {
                var chord = chordList[i]
                var vstaff = curScore.staves[chord.vStaffIdx]
                var lines = vstaff.lines(chord.fraction)
                var above = (chord.voice % 2 == 0)
                var lineScale = vstaff.lineDistance(chord.fraction) * vstaff.staffMag(chord.fraction)
                var topline = lineScale * Math.min((chord.bbox.y / lineScale) - 1, -1.5)
                var bottomline = lineScale * (Math.max((chord.bbox.y + chord.bbox.height) / lineScale + 1, lines + 0.5) - 4)
                var tabStaff = vstaff.isTabStaff(chord.fraction)

                // Even for list selections, we add text to every note in the chord (for clarity)
                for (var j in chord.notes) {
                    var note = chord.notes[j]
                    var text = newElement(Element.TEXT) // This adds the text to the note: Better for grace notes and easier to remove
                    text.text = note.tuning.toString()
                    text.fontSize *= curScore.style.value("smallNoteMag")
                                     // * (chord.noteType != NoteType.NORMAL ? curScore.style.value("graceNoteMag") : 1)
                    if (above) {
                        text.placement = Placement.ABOVE
                        text.align = Align.BASELINE
                        text.offset.y = (topline + j * -1.25)
                    } else {
                        text.placement = Placement.BELOW
                        text.align = Align.TOP
                        text.offset.y = (bottomline + (chord.notes.length - 1 - j) * 1.25)
                    }
                    text.offset.y -= (tabStaff ? note.string : 0.5 * note.line) * lineScale
                    note.add(text)
                }
            }
        }

        curScore.endCmd()
        return true
    }

    function error(errorMessage) {
        errorDialog.text = qsTr(errorMessage)
        errorDialog.open()
    }

    function temperamentClicked(temperamentIndex) {
        try {
            if (tuningModel[temperamentIndex].name == "separatorLine" || temperamentIndex == currentTemperament) {
                return
            }
            commandHistory.begin()
            // old settings are already saved by the refresh, no need to resave them here
            undoChangeValue("currentTemperament", temperamentIndex)
            undoChangeValue("currentRoot", tuningModel[temperamentIndex].root)
            undoChangeValue("currentPureTone", tuningModel[temperamentIndex].pure)
            undoChangeValue("currentGlobalOffset", tuningModel[temperamentIndex].globalOffset)
            commandHistory.end()
        } catch (e) {
            error(e.toString() + ",\n temperamentClicked")
        }
    }

    function changeRootNote(rootIndex) {
        try {
            commandHistory.begin()
            undoChangeValue("currentRoot", rootIndex)
            undoChangeValue("currentPureTone", rootIndex)
            commandHistory.end()
        } catch (e) {
            error(e.toString() + ",\n rootNoteClicked")
        }
    }

    function changePureTone(pureIndex) {
        try {
            undoChangeValue("currentPureTone", pureIndex)
        } catch (e) {
            error(e.toString() + ",\n pureToneClicked")
        }
    }

    function changeGlobalOffset(newValue) {
        try {
            undoChangeValue("currentGlobalOffset", newValue)
        } catch (e) {
            error(e.toString() + ",\n changeGlobalOffset")
        }
    }

    function editingFinishedFor(underlyingIndex, newValue, oldValue) {
        try {
            undoChangeSingleOffset(underlyingIndex, newValue)
        } catch (e) {
            error(e.toString() + ",\n editingFinishedFor")
        }
    }

    function resetCurrentPage() {
        try {
            commandHistory.begin()
            undoChangeCurrentOffsets(defaultTuning[currentTemperament].offsets)
            undoChangeValue("currentGlobalOffset", roundValue(defaultTuning[currentTemperament].globalOffset))
            undoChangeValue("currentRoot", roundValue(defaultTuning[currentTemperament].root))
            undoChangeValue("currentPureTone", roundValue(defaultTuning[currentTemperament].pure))
            defaultTuning = readDefaults()
            commandHistory.end()
        } catch (e) {
            error(e.toString() + ",\n resetCurrentPage")
        }
    }

    function roundValue(value) {
        if (typeof value == "string") {
            value = Number.parseFloat(value)
        }
        return Number(value.toFixed(root.decimals))
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: defaultSpacing
        Row {
            spacing: defaultSpacing
            MU.StyledGroupBox {
                title: qsTr("Tuning systems and temperaments")
                width: tuningsFlickable.width + defaultSpacing + tuningsFlickable.visualScrollBarInset - tuningsFlickable.scrollBarThickness
                height: configureRow.height

                MU.StyledListView {
                    id: tuningsFlickable
                    height: configureRowColumn.height
                    width: 300
                    focus: true

                    spacing: 8
                    model: tuningModel

                    delegate: Item {
                        id: tuningItem
                        readonly property bool isSeparatorLine: modelData.name == "separatorLine"
                        anchors.left: parent ? parent.left : undefined
                        anchors.right: parent ? parent.right : undefined
                        anchors.rightMargin: defaultSpacing + tuningsFlickable.visualScrollBarInset
                        height: isSeparatorLine ? tuningLabel.height : radioButton.implicitHeight

                        MU.StyledTextLabel {
                            id: tuningLabel
                            visible: tuningItem.isSeparatorLine
                            text: modelData.displayName
                            font: ui.theme.bodyBoldFont
                            width: parent.width
                            height: implicitHeight + (index > 0) ? 20 : 0
                            verticalAlignment: Text.AlignBottom
                            horizontalAlignment: Text.AlignLeft
                        }

                        MU.RoundedRadioButton {
                            id: radioButton
                            text: modelData.displayName
                            visible: !tuningItem.isSeparatorLine
                            checked: visible && index == currentTemperament
                            width: parent.width
                            // font: ui.theme.bodyFont
                            onToggled: {
                                temperamentClicked(index)
                            }
                        }
                    }
                }
                Rectangle {
                    height: 24
                    anchors.top: tuningsFlickable.top
                    anchors.left: tuningsFlickable.left
                    anchors.right: tuningsFlickable.right
                    anchors.rightMargin: scrollBar.width
                    visible: !tuningsFlickable.atYBeginning
                    gradient: Gradient {
                        GradientStop {position: 0.0; color: ui.theme.backgroundPrimaryColor }
                        GradientStop {position: 1.0; color: "transparent"}
                    }
                }
                Rectangle {
                    height: 24
                    anchors.left: tuningsFlickable.left
                    anchors.right: tuningsFlickable.right
                    anchors.rightMargin: scrollBar.width
                    anchors.bottom: tuningsFlickable.bottom
                    visible: !tuningsFlickable.atYEnd
                    gradient: Gradient {
                        GradientStop {position: 0.0; color: "transparent"}
                        GradientStop {position: 1.0; color: ui.theme.backgroundPrimaryColor}
                    }
                }
            }

            MU.StyledGroupBox {
                id: configureRow
                title: qsTr("Configure tuning")
                ColumnLayout {
                    id: configureRowColumn
                    spacing: defaultSpacing
                    RowLayout {
                        MU.StyledGroupBox {
                            title: qsTr("Root note")
                            GridLayout {
                                columns: 4
                                anchors.margins: defaultSpacing

                                Repeater {
                                    model: pitchOffsets
                                    MU.FlatRadioButton {
                                        text: notesStringModel[modelData]
                                        Layout.preferredWidth: 36
                                        checked: currentRoot == fifthsOffsets[modelData]
                                        onClicked: {
                                            changeRootNote(fifthsOffsets[modelData])
                                        }
                                    }
                                }
                            }
                        }

                        MU.StyledGroupBox {
                            title: qsTr("Pure tone")
                            GridLayout {
                                columns: 4
                                anchors.margins: defaultSpacing

                                Repeater {
                                    model: pitchOffsets
                                    MU.FlatRadioButton {
                                        text: notesStringModel[modelData]
                                        Layout.preferredWidth: 36
                                        checked: currentPureTone == fifthsOffsets[modelData]
                                        onClicked: {
                                            changePureTone(fifthsOffsets[modelData])
                                        }
                                    }
                                }
                            }
                        }
                    }

                    MU.StyledGroupBox {
                        title: qsTr("Pitch offsets")
                        Layout.fillWidth: true
                        GridLayout {
                            columns: 4
                            anchors.margins: defaultSpacing

                            Repeater {
                                model: pitchOffsets
                                id: finalValueRepeater

                                RowLayout {
                                    property alias control: individualOffsetControl
                                    MU.StyledTextLabel {
                                        text: notesStringModel[modelData]
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                        Layout.minimumWidth: 20
                                    }
                                    MU.IncrementalPropertyControl {
                                        Layout.maximumWidth: incrementalControlWidth
                                        id: individualOffsetControl

                                        decimals: root.decimals
                                        step: 0.1
                                        minValue: -99.9
                                        maxValue: 99.9
                                        property var underlyingValue: 0
                                        property int underlyingIndex: (fifthsOffsets[modelData] - currentRoot + 12) % 12

                                        onValueEdited: function(newValue) {
                                            var n = roundValue(newValue)
                                            var c = roundValue(currentValue)
                                            if (n !== c) {
                                                editingFinishedFor(underlyingIndex, roundValue(n - c + underlyingValue), underlyingValue)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        spacing: defaultSpacing
                        MU.StyledTextLabel {
                            text: qsTr("Global tuning offset:")
                            Layout.alignment: Qt.AlignVCenter
                        }
                        MU.IncrementalPropertyControl {
                            id: globalOffsetControl
                            Layout.preferredWidth: incrementalControlWidth

                            currentValue: currentGlobalOffset
                            decimals: root.decimals
                            step: 0.1
                            minValue: -99.9
                            maxValue: 99.9

                            onValueEdited: function(newValue) {
                                var n = roundValue(newValue)
                                var c = roundValue(currentValue)
                                if (n !== c) {
                                    changeGlobalOffset(n)
                                }
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        MU.CheckBox {
                            text: qsTr("Display pitches chromatically")
                            checked: chromatic
                            onClicked: {
                                chromatic = !chromatic
                                root.refreshTextFields()
                            }
                        }
                    }
                    RowLayout {
                        spacing: 8
                        MU.FlatButton {
                            id: saveButton
                            text: qsTranslate("PrefsDialogBase", "Save")
                            enabled: root.saveIsAvailable
                            onClicked: {
                                options.data = formatCurrentValues()
                                root.refresh()
                            }
                        }
                        MU.FlatButton {
                            id: loadButton
                            text: qsTranslate("PrefsDialogBase", "Reset")
                            enabled: root.resetIsAvailable
                            onClicked: {
                                resetCurrentPage()
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        MU.FlatButton {
                            icon: IconCode.UNDO
                            enabled: undoIndex > -1
                            onClicked: {
                                commandHistory.undo()
                            }
                        }
                        MU.FlatButton {
                            icon: IconCode.REDO
                            enabled: undoIndex + 1 < undoLength
                            onClicked: {
                                commandHistory.redo()
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            spacing: 8
            MU.CheckBox {
                Layout.fillWidth: true
                id: annotateBox
                text: qsTr("Annotate tunings in score")
                checked: false
                onClicked: {
                    checked = !checked
                }
            }
            MU.FlatButton {
                text: curScore ? qsTranslate("PrefsDialogBase", "Cancel") : qsTranslate("PrefsDialogBase", "Quit")
                onClicked: {
                    quit()
                }
            }
            MU.FlatButton {
                text: qsTranslate("PrefsDialogBase", "Apply")
                visible: curScore
                accentButton: true
                onClicked: {
                    options.data = formatCurrentValues()
                    applyTemperament()
                    quit()
                }
            }
        }
    }

    MessageDialog {
        id: errorDialog
        title: "Error"
        onAccepted: {
            errorDialog.close()
            quit()
        }
    }

    function undoChangeValue(currentValue, newValue) {
        try {
            if (!root.hasOwnProperty(currentValue)) {
                throw new Error("Tried to change property that doesn't exist!")
            }
            var oldValue = root[currentValue]
            if (oldValue == newValue) {
                return
            }
            commandHistory.add(
                function() {
                    root[currentValue] = oldValue
                },
                function() {
                    root[currentValue] = newValue
                },
                "changed value"
            )
        } catch (e) {
            error(e.toString() + ",\n Tried to change " + currentValue + " to " + newValue)
        }
    }

    function undoChangeCurrentOffsets(tuningOffsets) {
        try {
            var oldSavedOffsets = tuningModel[currentTemperament].offsets
            commandHistory.add(
                function () {
                    tuningModel[currentTemperament].offsets = oldSavedOffsets
                },
                function() {
                    tuningModel[currentTemperament].offsets = tuningOffsets
                },
                "change offsets"
            )
        } catch (e) {
            error(e.toString() + ",\n undoChangeCurrentOffsets")
        }
    }

    function undoChangeSingleOffset(index, value) {
        try {
            if (roundValue(tuningModel[currentTemperament].offsets[index]) == roundValue(value)) {
                return
            }
            var oldValue = tuningModel[currentTemperament].offsets[index]
            commandHistory.add(
                function () {
                    tuningModel[currentTemperament].offsets[index] = oldValue
                },
                function () {
                    tuningModel[currentTemperament].offsets[index] = value
                },
                "edit tuning for note ".concat(notesStringModel[fifthsOffsets[index]])
            )
        } catch (e) {
            error(e.toString() + ",\n Tried to change index " + index + " to " + value)
        }
    }

    function formatCurrentValues() {
        tuningModel[currentTemperament].root = currentRoot
        tuningModel[currentTemperament].pure = currentPureTone
        tuningModel[currentTemperament].globalOffset = roundValue(currentGlobalOffset)
        return JSON.stringify(tuningModel)
    }

    function restoreSavedValues() {
        var newValues = tuningModel
        try {
            var data = JSON.parse(options.data)
            for (var i in data) {
                if (data[i].name == "separatorLine") {
                    continue
                }
                for (var j in newValues) {
                    if (newValues[j].name == data[i].name) {
                        var displayName = newValues[j].displayName
                        newValues[j] = data[i]
                        newValues[j]["displayName"] = displayName
                    }
                }
            }
        } catch (e) {
            // unable to read existing settings
        }
        options.data = JSON.stringify(newValues)
        return newValues
    }

    Settings {
        id: options
        category: "Tuning Plugin"
        property alias chromatic: root.chromatic
        property alias annotate: annotateBox.checked
        property alias currentTemperament: root.currentTemperament
        property var data: ''
    }

    // Command pattern for undo/redo
    QtObject {
        id: commandHistory

        function hCommand(undo_fn, redo_fn, label) {
            this.undo = undo_fn
            this.redo = redo_fn
            this.label = label // for debugging
        }

        property var history: []
        property int index: -1
        property bool transaction: false
        readonly property int maxHistory: 30

        function newHistory(commands) {
            if (index < maxHistory) {
                index++
                history = history.slice(0, index)
            } else {
                history = history.slice(0, index)
            }
            history.push(commands)
            root.undoLength = history.length
        }

        function add(undo, redo, label) {
            var command = new hCommand(undo, redo, label)
            command.redo()
            if (transaction) {
                history[index].push(command)
            } else {
                newHistory([command])
                root.refresh()
            }
        }

        function undo() {
            if (index > -1) {
                history[index].slice().reverse().forEach(
                    function(command) {
                        command.undo()
                    }
                )
                index--
            }
            root.refresh()
        }

        function redo() {
            if ((index + 1) < history.length) {
                index++
                history[index].forEach(
                    function(command) {
                        command.redo()
                    }
                )
            }
            root.refresh()
        }

        function begin() {
            if (transaction) {
                throw new Error("already in transaction")
            }
            newHistory([])
            transaction = true
        }

        function end() {
            if (!transaction) {
                throw new Error("not in transaction")
            }
            transaction = false
            root.refresh()
        }
    }
}
