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

#ifndef MU_IMPORTEXPORT_GUITARPRODRUMSET_H
#define MU_IMPORTEXPORT_GUITARPRODRUMSET_H

#include <unordered_map>
#include <string_view>

namespace mu::engraving {
class Drumset;
class Instrument;
}

namespace mu::iex::guitarpro::drumset {
enum class DrumSetType : uint16_t {
    DRUMS = 0, AGOGO, HAND_CLAP, TAMBOURINE,
    COWBELL, VIBRASLAP, BONGOS, CONGAS,
    TIMBALE, CABASA, MARACAS, WHISTLE,
    GUIRO, CLAVES, WOODBLOCK, CUICA,
    TRIANGLE, SHAKER, JINGLE_BELL, BELL_TREE,
    CASTANETS, SURDO
};

struct GpDrumSet {
    std::string_view name;
    DrumSetType idx;
    uint8_t numLines;
};

static const std::unordered_map<std::string_view, GpDrumSet> PERC_STAFF_LINES_FROM_INSTRUMENT = {
    { "Agogo", { "Agogo", DrumSetType::AGOGO, 2 } },
    { "Hand Clap", { "Hand Clap", DrumSetType::HAND_CLAP, 1 } },
    { "Tambourine", { "Tambourine", DrumSetType::TAMBOURINE, 1 } },
    { "Cowbell", { "Cowbell", DrumSetType::COWBELL, 3 } },
    { "Vibraslap", { "Vibraslap", DrumSetType::VIBRASLAP, 1 } },
    { "Bongos", { "Bongos", DrumSetType::BONGOS, 2 } },
    { "Congas", { "Congas", DrumSetType::CONGAS, 2 } },
    { "Timbale", { "Timbale", DrumSetType::TIMBALE, 2 } },
    { "Cabasa", { "Cabasa", DrumSetType::CABASA, 1 } },
    { "Maracas", { "Maracas", DrumSetType::MARACAS, 2 } },
    { "Whistle", { "Whistle", DrumSetType::WHISTLE, 2 } },
    { "Guiro", { "Guiro", DrumSetType::GUIRO, 2 } },
    { "Claves", { "Claves", DrumSetType::CLAVES, 1 } },
    { "Woodblock", { "Woodblock", DrumSetType::WOODBLOCK, 2 } },
    { "Cuica", { "Cuica", DrumSetType::CUICA, 1 } },
    { "Triangle", { "Triangle", DrumSetType::TRIANGLE, 1 } },
    { "Shaker", { "Shaker", DrumSetType::SHAKER, 1 } },
    { "Jingle Bell", { "Jingle Bell", DrumSetType::JINGLE_BELL, 1 } },
    { "Bell Tree", { "Bell Tree", DrumSetType::BELL_TREE, 1 } },
    { "Castanets", { "Castanets", DrumSetType::CASTANETS, 1 } },
    { "Surdo",  { "Surdo", DrumSetType::SURDO, 1 } },
};

inline mu::engraving::Drumset* gpDrumset = nullptr;
inline mu::engraving::Drumset* gpAgogoSet = nullptr;
inline mu::engraving::Drumset* gpHandClapSet = nullptr;
inline mu::engraving::Drumset* gpTambourineSet = nullptr;
inline mu::engraving::Drumset* gpCowbellSet = nullptr;
inline mu::engraving::Drumset* gpVibraslapSet = nullptr;
inline mu::engraving::Drumset* gpBongosSet = nullptr;
inline mu::engraving::Drumset* gpCongasSet = nullptr;
inline mu::engraving::Drumset* gpTimbalesSet = nullptr;
inline mu::engraving::Drumset* gpCabasaSet = nullptr;
inline mu::engraving::Drumset* gpMaracasSet = nullptr;
inline mu::engraving::Drumset* gpWhistleSet = nullptr;
inline mu::engraving::Drumset* gpGuiroSet = nullptr;
inline mu::engraving::Drumset* gpClavesSet = nullptr;
inline mu::engraving::Drumset* gpWoodblockSet = nullptr;
inline mu::engraving::Drumset* gpCuicaSet = nullptr;
inline mu::engraving::Drumset* gpTriangleSet = nullptr;
inline mu::engraving::Drumset* gpShakerSet = nullptr;
inline mu::engraving::Drumset* gpJingleBellSet = nullptr;
inline mu::engraving::Drumset* gpBellTreeSet = nullptr;
inline mu::engraving::Drumset* gpCastanetsSet = nullptr;
inline mu::engraving::Drumset* gpSurdoSet = nullptr;

void initGuitarProDrumset();
void initGuitarProPercussionSet(const GpDrumSet& ds);
void setInstrumentDrumset(mu::engraving::Instrument* instrument, const GpDrumSet& ds);
} // namespace mu::iex::guitarpro::drumset
#endif
