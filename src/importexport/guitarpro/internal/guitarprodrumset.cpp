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

#include "guitarprodrumset.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/instrument.h"
#include "engraving/types/typesconv.h"

namespace mu::iex::guitarpro::drumset {
//---------------------------------------------------------
//   initGuitarProDrumset
//---------------------------------------------------------

void initGuitarProDrumset()
{
    using namespace mu::engraving;

    if (!gpDrumset) {
        gpDrumset = new Drumset();
        for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
            gpDrumset->drum(i).name.clear();
            gpDrumset->drum(i).notehead = NoteHeadGroup::HEAD_INVALID;
            gpDrumset->drum(i).line     = 0;
            gpDrumset->drum(i).shortcut.clear();
            gpDrumset->drum(i).voice    = 0;
            gpDrumset->drum(i).stemDirection = DirectionV::UP;
            gpDrumset->drum(i).panelRow = -1;
            gpDrumset->drum(i).panelColumn = -1;
        }
        // new drumset determined via guitar pro (third argument specifies position on staff, 10 = C3, 9 = D3, 8 = E3,...)

        gpDrumset->drum(27) = DrumInstrument(TConv::userName(DrumNum(27)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(28) = DrumInstrument(TConv::userName(DrumNum(28)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(29) = DrumInstrument(TConv::userName(DrumNum(29)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);

        gpDrumset->drum(30) = DrumInstrument(TConv::userName(DrumNum(30)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(31) = DrumInstrument(TConv::userName(DrumNum(31)), NoteHeadGroup::HEAD_CROSS, 3, DirectionV::UP);
        gpDrumset->drum(32) = DrumInstrument(TConv::userName(DrumNum(32)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(33) = DrumInstrument(TConv::userName(DrumNum(33)), NoteHeadGroup::HEAD_CROSS, 3, DirectionV::UP);
        gpDrumset->drum(34) = DrumInstrument(TConv::userName(DrumNum(34)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(35) = DrumInstrument(TConv::userName(DrumNum(35)), NoteHeadGroup::HEAD_NORMAL, 8, DirectionV::UP);
        gpDrumset->drum(36) = DrumInstrument(TConv::userName(DrumNum(36)), NoteHeadGroup::HEAD_NORMAL, 7, DirectionV::UP);
        gpDrumset->drum(37) = DrumInstrument(TConv::userName(DrumNum(37)), NoteHeadGroup::HEAD_CROSS, 3, DirectionV::UP);
        gpDrumset->drum(38) = DrumInstrument(TConv::userName(DrumNum(38)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(39) = DrumInstrument(TConv::userName(DrumNum(39)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);

        gpDrumset->drum(40) = DrumInstrument(TConv::userName(DrumNum(40)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(41) = DrumInstrument(TConv::userName(DrumNum(41)), NoteHeadGroup::HEAD_NORMAL, 6, DirectionV::UP);
        gpDrumset->drum(42) = DrumInstrument(TConv::userName(DrumNum(42)), NoteHeadGroup::HEAD_CROSS, -1, DirectionV::UP);
        gpDrumset->drum(43) = DrumInstrument(TConv::userName(DrumNum(43)), NoteHeadGroup::HEAD_NORMAL, 6, DirectionV::UP);
        gpDrumset->drum(44) = DrumInstrument(TConv::userName(DrumNum(44)), NoteHeadGroup::HEAD_CROSS, 9, DirectionV::UP);
        gpDrumset->drum(45) = DrumInstrument(TConv::userName(DrumNum(45)), NoteHeadGroup::HEAD_NORMAL, 5, DirectionV::UP);
        gpDrumset->drum(46) = DrumInstrument(TConv::userName(DrumNum(46)), NoteHeadGroup::HEAD_XCIRCLE, -1, DirectionV::UP);
        gpDrumset->drum(47) = DrumInstrument(TConv::userName(DrumNum(47)), NoteHeadGroup::HEAD_NORMAL, 4, DirectionV::UP);
        gpDrumset->drum(48) = DrumInstrument(TConv::userName(DrumNum(48)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
        gpDrumset->drum(49) = DrumInstrument(TConv::userName(DrumNum(49)), NoteHeadGroup::HEAD_CROSS, -2, DirectionV::UP);

        gpDrumset->drum(50) = DrumInstrument(TConv::userName(DrumNum(50)), NoteHeadGroup::HEAD_NORMAL, 1, DirectionV::UP);
        gpDrumset->drum(51) = DrumInstrument(TConv::userName(DrumNum(51)), NoteHeadGroup::HEAD_CROSS, 0, DirectionV::UP);
        gpDrumset->drum(52) = DrumInstrument(TConv::userName(DrumNum(52)), NoteHeadGroup::HEAD_HEAVY_CROSS_HAT, -3, DirectionV::UP);
        gpDrumset->drum(53) = DrumInstrument(TConv::userName(DrumNum(53)), NoteHeadGroup::HEAD_DIAMOND, 0, DirectionV::UP);
        gpDrumset->drum(54) = DrumInstrument(TConv::userName(DrumNum(54)), NoteHeadGroup::HEAD_CROSS, 2, DirectionV::UP);
        gpDrumset->drum(55) = DrumInstrument(TConv::userName(DrumNum(55)), NoteHeadGroup::HEAD_CROSS, -2, DirectionV::UP);
        gpDrumset->drum(56) = DrumInstrument(TConv::userName(DrumNum(56)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::UP);
        gpDrumset->drum(57) = DrumInstrument(TConv::userName(DrumNum(57)), NoteHeadGroup::HEAD_CROSS, -3, DirectionV::UP);
        gpDrumset->drum(58) = DrumInstrument(TConv::userName(DrumNum(58)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(59) = DrumInstrument(TConv::userName(DrumNum(59)), NoteHeadGroup::HEAD_CROSS, 2, DirectionV::UP);

        gpDrumset->drum(60) = DrumInstrument(TConv::userName(DrumNum(60)), NoteHeadGroup::HEAD_NORMAL, 8, DirectionV::UP);
        gpDrumset->drum(61) = DrumInstrument(TConv::userName(DrumNum(61)), NoteHeadGroup::HEAD_NORMAL, 9, DirectionV::UP);
        gpDrumset->drum(62) = DrumInstrument(TConv::userName(DrumNum(62)), NoteHeadGroup::HEAD_CROSS, 5, DirectionV::UP);
        gpDrumset->drum(63) = DrumInstrument(TConv::userName(DrumNum(63)), NoteHeadGroup::HEAD_CROSS, 4, DirectionV::UP);
        gpDrumset->drum(64) = DrumInstrument(TConv::userName(DrumNum(64)), NoteHeadGroup::HEAD_CROSS, 6, DirectionV::UP);
        gpDrumset->drum(65) = DrumInstrument(TConv::userName(DrumNum(65)), NoteHeadGroup::HEAD_CROSS, 8, DirectionV::UP);
        gpDrumset->drum(66) = DrumInstrument(TConv::userName(DrumNum(66)), NoteHeadGroup::HEAD_CROSS, 9, DirectionV::UP);
        gpDrumset->drum(67) = DrumInstrument(TConv::userName(DrumNum(67)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(68) = DrumInstrument(TConv::userName(DrumNum(68)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(69) = DrumInstrument(TConv::userName(DrumNum(69)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);

        gpDrumset->drum(70) = DrumInstrument(TConv::userName(DrumNum(70)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(71) = DrumInstrument(TConv::userName(DrumNum(71)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(72) = DrumInstrument(TConv::userName(DrumNum(72)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(73) = DrumInstrument(TConv::userName(DrumNum(73)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(74) = DrumInstrument(TConv::userName(DrumNum(74)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(75) = DrumInstrument(TConv::userName(DrumNum(75)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(76) = DrumInstrument(TConv::userName(DrumNum(76)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(77) = DrumInstrument(TConv::userName(DrumNum(77)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(78) = DrumInstrument(TConv::userName(DrumNum(78)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(79) = DrumInstrument(TConv::userName(DrumNum(79)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);

        gpDrumset->drum(80) = DrumInstrument(TConv::userName(DrumNum(80)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(81) = DrumInstrument(TConv::userName(DrumNum(81)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(82) = DrumInstrument(TConv::userName(DrumNum(82)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(83) = DrumInstrument(TConv::userName(DrumNum(83)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(84) = DrumInstrument(TConv::userName(DrumNum(84)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(85) = DrumInstrument(TConv::userName(DrumNum(85)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(86) = DrumInstrument(TConv::userName(DrumNum(86)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);
        gpDrumset->drum(87) = DrumInstrument(TConv::userName(DrumNum(87)), NoteHeadGroup::HEAD_NORMAL, 3, DirectionV::UP);

        gpDrumset->drum(91) = DrumInstrument(TConv::userName(DrumNum(91)), NoteHeadGroup::HEAD_DIAMOND, 3, DirectionV::UP);
        gpDrumset->drum(92) = DrumInstrument(TConv::userName(DrumNum(46)), NoteHeadGroup::HEAD_CROSS, -1, DirectionV::UP);
        gpDrumset->drum(93) = DrumInstrument(TConv::userName(DrumNum(93)), NoteHeadGroup::HEAD_CROSS, 0, DirectionV::UP);

        //Additional clutch presets (midi by default can't play this)
        gpDrumset->drum(99) = DrumInstrument(TConv::userName(DrumNum(99)), NoteHeadGroup::HEAD_TRIANGLE_UP, 1, DirectionV::UP);
        gpDrumset->drum(102)= DrumInstrument(TConv::userName(DrumNum(102)), NoteHeadGroup::HEAD_TRIANGLE_UP, -1, DirectionV::UP);
    }
}

void initGuitarProPercussionSet(const GpDrumSet& ds)
{
    using namespace mu::engraving;
    switch (ds.idx) {
    case DrumSetType::DRUMS:
        break;
    case DrumSetType::AGOGO:
        if (!gpAgogoSet) {
            gpAgogoSet = new Drumset;
            gpAgogoSet->drum(67) = DrumInstrument(TConv::userName(DrumNum(67)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
            gpAgogoSet->drum(68) = DrumInstrument(TConv::userName(DrumNum(68)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
        }
        break;
    case DrumSetType::HAND_CLAP:
        if (!gpHandClapSet) {
            gpHandClapSet = new Drumset;
            gpHandClapSet->drum(39) = DrumInstrument(TConv::userName(DrumNum(39)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::TAMBOURINE:
        if (!gpTambourineSet) {
            gpTambourineSet = new Drumset;
            gpTambourineSet->drum(54) = DrumInstrument(TConv::userName(DrumNum(54)), NoteHeadGroup::HEAD_TRIANGLE_UP, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::COWBELL:
        if (!gpCowbellSet) {
            gpCowbellSet = new Drumset;
            gpCowbellSet->drum(56) = DrumInstrument(TConv::userName(DrumNum(56)), NoteHeadGroup::HEAD_TRIANGLE_UP, 2, DirectionV::DOWN);
            gpCowbellSet->drum(99) = DrumInstrument(TConv::userName(DrumNum(99)), NoteHeadGroup::HEAD_TRIANGLE_UP, 4, DirectionV::UP);
            gpCowbellSet->drum(102) = DrumInstrument(TConv::userName(DrumNum(102)), NoteHeadGroup::HEAD_TRIANGLE_UP, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::VIBRASLAP:
        if (!gpVibraslapSet) {
            gpVibraslapSet = new Drumset;
            gpVibraslapSet->drum(58) = DrumInstrument(TConv::userName(DrumNum(58)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::BONGOS:
        if (!gpBongosSet) {
            gpBongosSet = new Drumset;
            gpBongosSet->drum(60) = DrumInstrument(TConv::userName(DrumNum(60)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
            gpBongosSet->drum(61) = DrumInstrument(TConv::userName(DrumNum(61)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::CONGAS:
        if (!gpCongasSet) {
            gpCongasSet = new Drumset;
            gpCongasSet->drum(62) = DrumInstrument(TConv::userName(DrumNum(62)), NoteHeadGroup::HEAD_CROSS, 0, DirectionV::DOWN);
            gpCongasSet->drum(63) = DrumInstrument(TConv::userName(DrumNum(63)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
            gpCongasSet->drum(64) = DrumInstrument(TConv::userName(DrumNum(64)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
        }
        break;
    case DrumSetType::TIMBALE:
        if (!gpTimbalesSet) {
            gpTimbalesSet = new Drumset;
            gpTimbalesSet->drum(65) = DrumInstrument(TConv::userName(DrumNum(65)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
            gpTimbalesSet->drum(66) = DrumInstrument(TConv::userName(DrumNum(66)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
        }
        break;
    case DrumSetType::CABASA:
        if (!gpCabasaSet) {
            gpCabasaSet = new Drumset;
            gpCabasaSet->drum(69) = DrumInstrument(TConv::userName(DrumNum(69)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::MARACAS:
        if (!gpMaracasSet) {
            gpMaracasSet = new Drumset;
            gpMaracasSet->drum(70) = DrumInstrument(TConv::userName(DrumNum(70)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::WHISTLE:
        if (!gpWhistleSet) {
            gpWhistleSet = new Drumset;
            gpWhistleSet->drum(71) = DrumInstrument(TConv::userName(DrumNum(71)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
            gpWhistleSet->drum(72) = DrumInstrument(TConv::userName(DrumNum(72)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
        }
        break;
    case DrumSetType::GUIRO:
        if (!gpGuiroSet) {
            gpGuiroSet = new Drumset;
            gpGuiroSet->drum(73) = DrumInstrument(TConv::userName(DrumNum(73)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
            gpGuiroSet->drum(74) = DrumInstrument(TConv::userName(DrumNum(74)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::CLAVES:
        if (!gpClavesSet) {
            gpClavesSet = new Drumset;
            gpClavesSet->drum(75) = DrumInstrument(TConv::userName(DrumNum(75)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::WOODBLOCK:
        if (!gpWoodblockSet) {
            gpWoodblockSet = new Drumset;
            gpWoodblockSet->drum(76) = DrumInstrument(TConv::userName(DrumNum(76)), NoteHeadGroup::HEAD_NORMAL, 2, DirectionV::UP);
            gpWoodblockSet->drum(77) = DrumInstrument(TConv::userName(DrumNum(77)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::CUICA:
        if (!gpCuicaSet) {
            gpCuicaSet = new Drumset;
            gpCuicaSet->drum(78) = DrumInstrument(TConv::userName(DrumNum(78)), NoteHeadGroup::HEAD_CROSS, 0, DirectionV::DOWN);
            gpCuicaSet->drum(79) = DrumInstrument(TConv::userName(DrumNum(79)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::TRIANGLE:
        if (!gpTriangleSet) {
            gpTriangleSet = new Drumset;
            gpTriangleSet->drum(80) = DrumInstrument(TConv::userName(DrumNum(80)), NoteHeadGroup::HEAD_CROSS, 0, DirectionV::DOWN);
            gpTriangleSet->drum(81) = DrumInstrument(TConv::userName(DrumNum(81)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::SHAKER:
        if (!gpShakerSet) {
            gpShakerSet = new Drumset;
            gpShakerSet->drum(82) = DrumInstrument(TConv::userName(DrumNum(82)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::JINGLE_BELL:
        if (!gpJingleBellSet) {
            gpJingleBellSet = new Drumset;
            gpJingleBellSet->drum(83) = DrumInstrument(TConv::userName(DrumNum(83)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::BELL_TREE:
        if (!gpBellTreeSet) {
            gpBellTreeSet = new Drumset;
            gpBellTreeSet->drum(84) = DrumInstrument(TConv::userName(DrumNum(84)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::CASTANETS:
        if (!gpCastanetsSet) {
            gpCastanetsSet = new Drumset;
            gpCastanetsSet->drum(85) = DrumInstrument(TConv::userName(DrumNum(85)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
        }
        break;
    case DrumSetType::SURDO:
        if (!gpSurdoSet) {
            gpSurdoSet = new Drumset;
            gpSurdoSet->drum(86) = DrumInstrument(TConv::userName(DrumNum(86)), NoteHeadGroup::HEAD_NORMAL, 0, DirectionV::DOWN);
            gpSurdoSet->drum(87) = DrumInstrument(TConv::userName(DrumNum(87)), NoteHeadGroup::HEAD_CROSS, 0, DirectionV::DOWN);
        }
        break;
    }
}

void setInstrumentDrumset(mu::engraving::Instrument* instrument, const GpDrumSet& ds)
{
    switch (ds.idx) {
    case DrumSetType::DRUMS:
        instrument->setDrumset(gpDrumset);
        break;
    case DrumSetType::AGOGO:
        instrument->setDrumset(gpAgogoSet);
        break;
    case DrumSetType::HAND_CLAP:
        instrument->setDrumset(gpHandClapSet);
        break;
    case DrumSetType::TAMBOURINE:
        instrument->setDrumset(gpTambourineSet);
        break;
    case DrumSetType::COWBELL:
        instrument->setDrumset(gpCowbellSet);
        break;
    case DrumSetType::VIBRASLAP:
        instrument->setDrumset(gpVibraslapSet);
        break;
    case DrumSetType::BONGOS:
        instrument->setDrumset(gpBongosSet);
        break;
    case DrumSetType::CONGAS:
        instrument->setDrumset(gpCongasSet);
        break;
    case DrumSetType::TIMBALE:
        instrument->setDrumset(gpTimbalesSet);
        break;
    case DrumSetType::CABASA:
        instrument->setDrumset(gpCabasaSet);
        break;
    case DrumSetType::MARACAS:
        instrument->setDrumset(gpMaracasSet);
        break;
    case DrumSetType::WHISTLE:
        instrument->setDrumset(gpWhistleSet);
        break;
    case DrumSetType::GUIRO:
        instrument->setDrumset(gpGuiroSet);
        break;
    case DrumSetType::CLAVES:
        instrument->setDrumset(gpClavesSet);
        break;
    case DrumSetType::WOODBLOCK:
        instrument->setDrumset(gpWoodblockSet);
        break;
    case DrumSetType::CUICA:
        instrument->setDrumset(gpCuicaSet);
        break;
    case DrumSetType::TRIANGLE:
        instrument->setDrumset(gpTriangleSet);
        break;
    case DrumSetType::SHAKER:
        instrument->setDrumset(gpShakerSet);
        break;
    case DrumSetType::JINGLE_BELL:
        instrument->setDrumset(gpJingleBellSet);
        break;
    case DrumSetType::BELL_TREE:
        instrument->setDrumset(gpBellTreeSet);
        break;
    case DrumSetType::CASTANETS:
        instrument->setDrumset(gpCastanetsSet);
        break;
    case DrumSetType::SURDO:
        instrument->setDrumset(gpSurdoSet);
        break;
    }
}
} // namespace mu::iex::guitarpro::drumset
