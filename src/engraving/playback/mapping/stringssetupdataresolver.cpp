/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "stringssetupdataresolver.h"

using namespace mu::engraving;
using namespace mu::mpe;

bool StringsSetupDataResolver::supportsInstrument(const Ms::Instrument* instrument)
{
    static const std::unordered_set<std::string> STRINGS_FAMILY_SET = {
        "harps", "guitars", "bass-guitars", "banjos",
        "ukuleles", "mandolins", "mtn-dulcimers",  "lutes",
        "balalaikas", "bouzoukis", "kotos", "ouds",
        "shamisens", "sitars", "tamburicas", "bandurrias",
        "lauds", "strings", "orchestral-strings", "viols",
        "octobasses", "erhus", "nyckelharpas", "synths"
    };

    return STRINGS_FAMILY_SET.find(instrument->family().toStdString()) != STRINGS_FAMILY_SET.cend();
}

const PlaybackSetupData& StringsSetupDataResolver::doResolve(const Ms::Instrument* instrument)
{
    static std::unordered_map<std::string, mpe::PlaybackSetupData> SETUP_DATA_MAP = {
        { "harp", { SoundId::Harp, SoundCategory::Strings, {} } },
        { "cavaquinho", { SoundId::Cavaquinho, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                         SoundSubCategory::Steel } } },
        { "cavaquinho-tablature", { SoundId::Cavaquinho, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                   SoundSubCategory::Steel } } },
        { "soprano-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                         SoundSubCategory::Nylon,
                                                                         SoundSubCategory::Soprano } } },
        { "alto-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                      SoundSubCategory::Nylon,
                                                                      SoundSubCategory::Alto } } },
        { "baritone-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                          SoundSubCategory::Nylon,
                                                                          SoundSubCategory::Baritone } } },
        { "contra-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                        SoundSubCategory::Nylon,
                                                                        SoundSubCategory::Contra } } },

        { "electric-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "electric-guitar-treble-clef", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "electric-guitar-treble-clef", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },

        { "guitar-steel", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                       SoundSubCategory::Steel } } },
        { "guitar-steel-treble-clef", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                   SoundSubCategory::Steel } } },
        { "guitar-steel-tablature", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                 SoundSubCategory::Steel } } },
        { "pedal-steel-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                             SoundSubCategory::Pedal,
                                                                             SoundSubCategory::Steel } } },

        { "guitar-nylon", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                       SoundSubCategory::Nylon } } },
        { "guitar-nylon-treble-clef", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                   SoundSubCategory::Nylon } } },
        { "guitar-nylon-tablature", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                 SoundSubCategory::Nylon } } },
        { "7-string-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                          SoundSubCategory::Nylon } } },
        { "7-string-guitar-tablature", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                    SoundSubCategory::Nylon } } },
        { "11-string-alto-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                SoundSubCategory::Nylon,
                                                                                SoundSubCategory::Alto } } },
        { "12-string-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                           SoundSubCategory::Steel,
                                                                           SoundSubCategory::TwelveString } } },

        { "5-string-electric-bass-high-c", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                            SoundSubCategory::Tenor } } },
        { "5-string-electric-bass-tab-high-c", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                                SoundSubCategory::Tenor } } },
        { "5-string-electric-bass-tab", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "5-string-electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "6-string-electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "6-string-electric-bass-tab", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "bass-guitar", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "bass-guitar-tablature", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "electric-bass-4-str-tab", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "fretless-electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric } } },
        { "acoustic-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Acoustic } } },

        { "banjo", { SoundId::Banjo, SoundCategory::Strings, {} } },
        { "tenor-banjo", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "banjo-tablature", { SoundId::Banjo, SoundCategory::Strings, {} } },
        { "irish-tenor-banjo", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Irish,
                                                                           SoundSubCategory::Tenor } } },
        { "irish-tenor-banjo-tablature", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Irish,
                                                                                     SoundSubCategory::Tenor } } },

        { "ukulele", { SoundId::Ukulele, SoundCategory::Strings, {} } },
        { "ukulele-4-str-tab", { SoundId::Ukulele, SoundCategory::Strings, {} } },
        { "ukulele-low-g", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "tenor-ukulele", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "baritone-ukulele", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Baritone } } },

        { "mandolin", { SoundId::Mandolin, SoundCategory::Strings, {} } },
        { "mandolin-tablature", { SoundId::Mandolin, SoundCategory::Strings, {} } },
        { "alto-mandola", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Alto } } },
        { "mandola", { SoundId::Mandolin, SoundCategory::Strings, {} } },
        { "tenor-mandola", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "octave-mandolin", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Octave } } },
        { "mandocello", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Octave } } },

        { "mtn-dulcimer-std", { SoundId::MtnDulcimer, SoundCategory::Strings, {} } },
        { "mtn-dulcimer-std-chrom-tab", { SoundId::MtnDulcimer, SoundCategory::Strings, {} } },
        { "mtn-dulcimer-baritone", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Baritone } } },
        { "mtn-dulcimer-bartn-chrom-tab", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Baritone } } },
        { "mtn-dulcimer-bass", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Bass } } },
        { "mtn-dulcimer-bass-chrom-tab", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Bass } } },

        { "lute", { SoundId::Lute, SoundCategory::Strings, {} } },
        { "lute-tablature", { SoundId::Lute, SoundCategory::Strings, {} } },
        { "ren.-tenor-lute-5-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "ren.-tenor-lute-6-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "ren.-tenor-lute-7-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "ren.-tenor-lute-8-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "ren.-tenor-lute-9-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "ren.-tenor-lute-10-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "baroque-lute-13-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Baroque } } },
        { "archlute-14-course", { SoundId::Lute, SoundCategory::Strings, { } } },
        { "bouzouki-3-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Greek } } },
        { "bouzouki-4-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Greek } } },

        { "theorbo-14-course", { SoundId::Theorbo, SoundCategory::Strings, { } } },

        { "balalaika", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima } } },
        { "balalaika-piccolo", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Piccolo } } },
        { "balalaika-prima", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima } } },
        { "balalaika-alto", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                            SoundSubCategory::Alto } } },
        { "balalaika-bass", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                            SoundSubCategory::Bass } } },
        { "balalaika-contrabass", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                                  SoundSubCategory::Contra_Bass } } },
        { "balalaika-secunda", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Secunda } } },

        { "koto", { SoundId::Koto, SoundCategory::Strings, { SoundSubCategory::Japanese } } },
        { "shamisen", { SoundId::Shamisen, SoundCategory::Strings, { SoundSubCategory::Japanese } } },
        { "sitar", { SoundId::Sitar, SoundCategory::Strings, { SoundSubCategory::Indian } } },
        { "oud", { SoundId::Oud, SoundCategory::Strings, { SoundSubCategory::African } } },
        { "prim", { SoundId::Prim, SoundCategory::Strings, { } } },
        { "brac", { SoundId::Brac, SoundCategory::Strings, { } } },
        { "bugarija", { SoundId::Bugarija, SoundCategory::Strings, { } } },
        { "berda", { SoundId::Berda, SoundCategory::Strings, { } } },
        { "celo", { SoundId::Celo, SoundCategory::Strings, { } } },
        { "bandurria", { SoundId::Bandurria, SoundCategory::Strings, { SoundSubCategory::Spanish } } },
        { "bandurria-tablature", { SoundId::Bandurria, SoundCategory::Strings, { SoundSubCategory::Spanish } } },
        { "laud", { SoundId::Laud, SoundCategory::Strings, { SoundSubCategory::Spanish } } },
        { "laud-tablature", { SoundId::Laud, SoundCategory::Strings, { SoundSubCategory::Spanish } } },

        { "strings", { SoundId::StringsGroup, SoundCategory::Strings, { } } },
        { "double-bass", { SoundId::Contrabass, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "contrabass", { SoundId::Contrabass, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "contrabasses", { SoundId::ContrabasseSection, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violin", { SoundId::Violin, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violins", { SoundId::ViolinSection, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "viola", { SoundId::Viola, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violas", { SoundId::ViolaSection, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violoncello", { SoundId::Violoncello, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violoncellos", { SoundId::VioloncelloSection, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },

        { "treble-viol", { SoundId::Viol, SoundCategory::Strings, { } } },
        { "alto-viol", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Alto } } },
        { "pardessus-de-viole", { SoundId::PardessusViol, SoundCategory::Strings, {} } },
        { "tenor-viol", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "baryton", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Baritone } } },
        { "viola-da-gamba", { SoundId::ViolaDaGamba, SoundCategory::Strings, { } } },
        { "viola-da-gamba-tablature", { SoundId::ViolaDaGamba, SoundCategory::Strings, { } } },
        { "violone", { SoundId::Violone, SoundCategory::Strings, { } } },
        { "d-violone", { SoundId::Violone, SoundCategory::Strings, { } } },

        { "octobass", { SoundId::Octobass, SoundCategory::Strings, { } } },
        { "erhu", { SoundId::Erhu, SoundCategory::Strings, { SoundSubCategory::Chinese } } },
        { "nyckelharpa", { SoundId::Nyckelharpa, SoundCategory::Strings, { SoundSubCategory::Swedish } } },

        { "bass-synthesizer", { SoundId::Synthesizer, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                SoundSubCategory::Bass } } },
        { "bowed-synth", { SoundId::Synthesizer, SoundCategory::Strings, { SoundSubCategory::Electric } } },
    };

    auto search = SETUP_DATA_MAP.find(instrument->id().toStdString());
    if (search == SETUP_DATA_MAP.cend()) {
        static PlaybackSetupData empty;
        return empty;
    }

    return search->second;
}
