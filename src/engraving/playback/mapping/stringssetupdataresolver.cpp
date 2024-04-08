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

#include "stringssetupdataresolver.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

PlaybackSetupData StringsSetupDataResolver::doResolve(const Instrument* instrument)
{
    static const std::unordered_map<std::string, mpe::PlaybackSetupData> SETUP_DATA_MAP = {
        { "harp", { SoundId::Harp, SoundCategory::Strings, { mpe::SoundSubCategory::Plucked } } },
        { "cavaquinho", { SoundId::Cavaquinho, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                         SoundSubCategory::Steel,
                                                                         SoundSubCategory::Plucked } } },
        { "cavaquinho-tablature", { SoundId::Cavaquinho, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                   SoundSubCategory::Steel,
                                                                                   SoundSubCategory::Plucked } } },
        { "soprano-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                         SoundSubCategory::Nylon,
                                                                         SoundSubCategory::Soprano,
                                                                         SoundSubCategory::Plucked } } },
        { "alto-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                      SoundSubCategory::Nylon,
                                                                      SoundSubCategory::Alto,
                                                                      SoundSubCategory::Plucked } } },
        { "baritone-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                          SoundSubCategory::Nylon,
                                                                          SoundSubCategory::Baritone,
                                                                          SoundSubCategory::Plucked } } },
        { "contra-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                        SoundSubCategory::Nylon,
                                                                        SoundSubCategory::Contra,
                                                                        SoundSubCategory::Plucked } } },

        { "electric-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                          SoundSubCategory::Plucked } } },
        { "electric-guitar-treble-clef", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                      SoundSubCategory::Plucked } } },
        { "electric-guitar-tablature", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                    SoundSubCategory::Plucked } } },

        { "guitar-steel", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                       SoundSubCategory::Steel,
                                                                       SoundSubCategory::Plucked } } },
        { "guitar-steel-treble-clef", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                   SoundSubCategory::Steel,
                                                                                   SoundSubCategory::Plucked } } },
        { "guitar-steel-tablature", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                 SoundSubCategory::Steel,
                                                                                 SoundSubCategory::Plucked } } },
        { "pedal-steel-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                             SoundSubCategory::Pedal,
                                                                             SoundSubCategory::Steel,
                                                                             SoundSubCategory::Plucked } } },

        { "guitar-nylon", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                       SoundSubCategory::Nylon,
                                                                       SoundSubCategory::Plucked } } },
        { "guitar-nylon-treble-clef", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                   SoundSubCategory::Nylon,
                                                                                   SoundSubCategory::Plucked } } },
        { "guitar-nylon-tablature", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                 SoundSubCategory::Nylon,
                                                                                 SoundSubCategory::Plucked } } },
        { "7-string-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                          SoundSubCategory::Nylon,
                                                                          SoundSubCategory::Plucked } } },
        { "7-string-guitar-tablature", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                    SoundSubCategory::Nylon,
                                                                                    SoundSubCategory::Plucked } } },
        { "11-string-alto-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                                SoundSubCategory::Nylon,
                                                                                SoundSubCategory::Alto,
                                                                                SoundSubCategory::Plucked } } },
        { "12-string-guitar", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                           SoundSubCategory::Steel,
                                                                           SoundSubCategory::TwelveString,
                                                                           SoundSubCategory::Plucked } } },

        { "5-string-electric-bass-high-c", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                            SoundSubCategory::Tenor,
                                                                                            SoundSubCategory::Plucked } } },
        { "5-string-electric-bass-tab-high-c", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                                SoundSubCategory::Tenor,
                                                                                                SoundSubCategory::Plucked } } },
        { "5-string-electric-bass-tab", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                         SoundSubCategory::Plucked } } },
        { "5-string-electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                     SoundSubCategory::Plucked } } },
        { "6-string-electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                     SoundSubCategory::Plucked } } },
        { "6-string-electric-bass-tab", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                         SoundSubCategory::Plucked } } },
        { "bass-guitar", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                          SoundSubCategory::Plucked } } },
        { "bass-guitar-tablature", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                    SoundSubCategory::Plucked } } },
        { "electric-bass-4-str-tab", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                      SoundSubCategory::Plucked } } },
        { "electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                            SoundSubCategory::Plucked } } },
        { "fretless-electric-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                     SoundSubCategory::Fretless,
                                                                                     SoundSubCategory::Plucked } } },
        { "acoustic-bass", { SoundId::BassGuitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                            SoundSubCategory::Plucked } } },

        { "banjo", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "tenor-banjo", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                     SoundSubCategory::Plucked } } },
        { "banjo-tablature", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "irish-tenor-banjo", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Irish,
                                                                           SoundSubCategory::Tenor,
                                                                           SoundSubCategory::Plucked } } },
        { "irish-tenor-banjo-tablature", { SoundId::Banjo, SoundCategory::Strings, { SoundSubCategory::Irish,
                                                                                     SoundSubCategory::Tenor,
                                                                                     SoundSubCategory::Plucked } } },

        { "ukulele", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "ukulele-4-str-tab", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "ukulele-low-g", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                         SoundSubCategory::Plucked } } },
        { "tenor-ukulele", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                         SoundSubCategory::Plucked } } },
        { "baritone-ukulele", { SoundId::Ukulele, SoundCategory::Strings, { SoundSubCategory::Baritone,
                                                                            SoundSubCategory::Plucked } } },

        { "mandolin", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "mandolin-tablature", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "alto-mandola", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Alto,
                                                                         SoundSubCategory::Plucked } } },
        { "mandola", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "tenor-mandola", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                          SoundSubCategory::Plucked } } },
        { "octave-mandolin", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Octave,
                                                                            SoundSubCategory::Plucked } } },
        { "mandocello", { SoundId::Mandolin, SoundCategory::Strings, { SoundSubCategory::Octave,
                                                                       SoundSubCategory::Plucked } } },

        { "mtn-dulcimer-std", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "mtn-dulcimer-std-chrom-tab", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "mtn-dulcimer-baritone", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Baritone,
                                                                                     SoundSubCategory::Plucked, } } },
        { "mtn-dulcimer-bartn-chrom-tab", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Baritone,
                                                                                            SoundSubCategory::Plucked } } },
        { "mtn-dulcimer-bass", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Bass,
                                                                                 SoundSubCategory::Plucked } } },
        { "mtn-dulcimer-bass-chrom-tab", { SoundId::MtnDulcimer, SoundCategory::Strings, { SoundSubCategory::Bass,
                                                                                           SoundSubCategory::Plucked } } },

        { "lute", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "lute-tablature", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "ren.-tenor-lute-5-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                                 SoundSubCategory::Plucked } } },
        { "ren.-tenor-lute-6-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                                 SoundSubCategory::Plucked } } },
        { "ren.-tenor-lute-7-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                                 SoundSubCategory::Plucked } } },
        { "ren.-tenor-lute-8-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                                 SoundSubCategory::Plucked } } },
        { "ren.-tenor-lute-9-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                                 SoundSubCategory::Plucked, } } },
        { "ren.-tenor-lute-10-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Tenor,
                                                                                  SoundSubCategory::Plucked } } },
        { "baroque-lute-13-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Baroque,
                                                                               SoundSubCategory::Plucked } } },
        { "archlute-14-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "bouzouki-3-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Greek,
                                                                          SoundSubCategory::Plucked } } },
        { "bouzouki-4-course", { SoundId::Lute, SoundCategory::Strings, { SoundSubCategory::Greek,
                                                                          SoundSubCategory::Plucked } } },

        { "theorbo-14-course", { SoundId::Theorbo, SoundCategory::Strings, { SoundSubCategory::Plucked } } },

        { "balalaika", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                       SoundSubCategory::Plucked } } },
        { "balalaika-piccolo", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Piccolo,
                                                                               SoundSubCategory::Plucked } } },
        { "balalaika-prima", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                             SoundSubCategory::Plucked } } },
        { "balalaika-alto", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                            SoundSubCategory::Alto,
                                                                            SoundSubCategory::Plucked } } },
        { "balalaika-bass", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                            SoundSubCategory::Bass,
                                                                            SoundSubCategory::Plucked } } },
        { "balalaika-contrabass", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Prima,
                                                                                  SoundSubCategory::Contra_Bass,
                                                                                  SoundSubCategory::Plucked } } },
        { "balalaika-secunda", { SoundId::Balalaika, SoundCategory::Strings, { SoundSubCategory::Secunda,
                                                                               SoundSubCategory::Plucked } } },

        { "koto", { SoundId::Koto, SoundCategory::Strings, { SoundSubCategory::Japanese,
                                                             SoundSubCategory::Plucked } } },
        { "shamisen", { SoundId::Shamisen, SoundCategory::Strings, { SoundSubCategory::Japanese,
                                                                     SoundSubCategory::Plucked } } },
        { "sitar", { SoundId::Sitar, SoundCategory::Strings, { SoundSubCategory::Indian,
                                                               SoundSubCategory::Plucked } } },
        { "oud", { SoundId::Oud, SoundCategory::Strings, { SoundSubCategory::African,
                                                           SoundSubCategory::Plucked } } },
        { "prim", { SoundId::Prim, SoundCategory::Strings, { SoundSubCategory::Plucked, } } },
        { "brac", { SoundId::Brac, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "bugarija", { SoundId::Bugarija, SoundCategory::Strings, { mpe::SoundSubCategory::Plucked } } },
        { "berda", { SoundId::Berda, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "celo", { SoundId::Celo, SoundCategory::Strings, { SoundSubCategory::Plucked } } },
        { "bandurria", { SoundId::Bandurria, SoundCategory::Strings, { SoundSubCategory::Spanish,
                                                                       SoundSubCategory::Plucked } } },
        { "bandurria-tablature", { SoundId::Bandurria, SoundCategory::Strings, { SoundSubCategory::Spanish,
                                                                                 SoundSubCategory::Plucked } } },
        { "laud", { SoundId::Laud, SoundCategory::Strings, { SoundSubCategory::Spanish,
                                                             SoundSubCategory::Plucked } } },
        { "laud-tablature", { SoundId::Laud, SoundCategory::Strings, { SoundSubCategory::Spanish,
                                                                       SoundSubCategory::Plucked } } },

        { "strings", { SoundId::StringsGroup, SoundCategory::Strings } },
        { "double-bass", { SoundId::Contrabass, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "contrabass", { SoundId::Contrabass, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "contrabasses", { SoundId::Contrabass, SoundCategory::Strings, { SoundSubCategory::Orchestral,
                                                                           SoundSubCategory::Section } } },
        { "violin", { SoundId::Violin, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violins", { SoundId::Violin, SoundCategory::Strings, { SoundSubCategory::Orchestral,
                                                                  SoundSubCategory::Section } } },
        { "viola", { SoundId::Viola, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violas", { SoundId::Viola, SoundCategory::Strings, { SoundSubCategory::Orchestral,
                                                                SoundSubCategory::Section } } },
        { "violoncello", { SoundId::Violoncello, SoundCategory::Strings, { SoundSubCategory::Orchestral } } },
        { "violoncellos", { SoundId::Violoncello, SoundCategory::Strings, { SoundSubCategory::Orchestral,
                                                                            SoundSubCategory::Section } } },

        { "treble-viol", { SoundId::Viol, SoundCategory::Strings } },
        { "alto-viol", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Alto } } },
        { "pardessus-de-viole", { SoundId::PardessusViol, SoundCategory::Strings } },
        { "tenor-viol", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Tenor } } },
        { "baryton", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Baritone } } },
        { "viola-da-gamba", { SoundId::ViolaDaGamba, SoundCategory::Strings } },
        { "viola-da-gamba-tablature", { SoundId::ViolaDaGamba, SoundCategory::Strings } },
        { "violone", { SoundId::Violone, SoundCategory::Strings } },
        { "d-violone", { SoundId::Violone, SoundCategory::Strings } },

        { "octobass", { SoundId::Octobass, SoundCategory::Strings } },
        { "erhu", { SoundId::Erhu, SoundCategory::Strings, { SoundSubCategory::Chinese } } },
        { "nyckelharpa", { SoundId::Nyckelharpa, SoundCategory::Strings, { SoundSubCategory::Swedish } } },

        { "bass-synthesizer", { SoundId::Synthesizer, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                                SoundSubCategory::Bass,
                                                                                SoundSubCategory::Plucked } } },
        { "bowed-synth", { SoundId::Synthesizer, SoundCategory::Strings, { SoundSubCategory::Electric,
                                                                           SoundSubCategory::Bowed } } },
    };

    auto search = SETUP_DATA_MAP.find(instrument->id().toStdString());
    if (search == SETUP_DATA_MAP.cend()) {
        static PlaybackSetupData empty;
        return empty;
    }

    static const std::unordered_set<SoundId> supportPrimaryAndSecondaryCategories {
        SoundId::Violin
    };

    if (muse::contains(supportPrimaryAndSecondaryCategories, search->second.soundId())) {
        SoundSubCategory category = instrument->isPrimary() ? SoundSubCategory::Primary : SoundSubCategory::Secondary;
        PlaybackSetupData setupData = search->second;
        setupData.add(category);
        return setupData;
    }

    return search->second;
}
