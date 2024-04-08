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

#include "windssetupdataresolver.h"

using namespace mu::engraving;
using namespace muse::mpe;

PlaybackSetupData WindsSetupDataResolver::doResolve(const Instrument* instrument)
{
    static const std::unordered_map<std::string, PlaybackSetupData> SETUP_DATA_MAP = {
        { "winds", { SoundId::WindsGroup, SoundCategory::Winds } },
        { "eb-piccolo", { SoundId::Piccolo, SoundCategory::Winds } },
        { "db-piccolo", { SoundId::Piccolo, SoundCategory::Winds } },
        { "piccolo", { SoundId::Piccolo, SoundCategory::Winds } },
        { "treble-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Treble } } },
        { "soprano-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "bass-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "contra-alto-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Contra_Alto } } },
        { "contrabass-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },
        { "subcontra-alto-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Sub_Contra_Alto } } },
        { "double-contrabass-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Double_Contra_Bass } } },
        { "hyperbass-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Hyper_Bass } } },
        { "irish-flute", { SoundId::Flute, SoundCategory::Winds, { SoundSubCategory::Irish } } },
        { "flute", { SoundId::Flute, SoundCategory::Winds } },
        { "traverso", { SoundId::Traverso, SoundCategory::Winds, { SoundSubCategory::Baroque } } },
        { "danso", { SoundId::Danso, SoundCategory::Winds } },

        { "a-dizi", { SoundId::Dizi, SoundCategory::Winds, { SoundSubCategory::Chinese } } },
        { "g-dizi", { SoundId::Dizi, SoundCategory::Winds, { SoundSubCategory::Chinese } } },
        { "f-dizi", { SoundId::Dizi, SoundCategory::Winds, { SoundSubCategory::Chinese } } },
        { "e-dizi", { SoundId::Dizi, SoundCategory::Winds, { SoundSubCategory::Chinese } } },
        { "d-dizi", { SoundId::Dizi, SoundCategory::Winds, { SoundSubCategory::Chinese } } },
        { "c-dizi", { SoundId::Dizi, SoundCategory::Winds, { SoundSubCategory::Chinese } } },

        { "shakuhachi", { SoundId::Shakuhachi, SoundCategory::Winds, { SoundSubCategory::Japanese } } },
        { "fife", { SoundId::Fife, SoundCategory::Winds } },

        { "d-tin-whistle", { SoundId::Whistle, SoundCategory::Winds, { SoundSubCategory::Tin } } },
        { "c-tin-whistle", { SoundId::Whistle, SoundCategory::Winds, { SoundSubCategory::Tin } } },
        { "bflat-tin-whistle", { SoundId::Whistle, SoundCategory::Winds, { SoundSubCategory::Tin } } },
        { "slide-whistle", { SoundId::Whistle, SoundCategory::Winds, { SoundSubCategory::Slide } } },

        { "french-flageolet", { SoundId::Flageolet, SoundCategory::Winds } },
        { "english-flageolet", { SoundId::Flageolet, SoundCategory::Winds } },
        { "flageolet", { SoundId::Flageolet, SoundCategory::Winds } },

        { "garklein-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Garklein } } },
        { "sopranino-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "soprano-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "recorder", { SoundId::Recorder, SoundCategory::Winds } },
        { "alto-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "bass-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "tenor-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "greatbass-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Great_Bass } } },
        { "contrabass-recorder", { SoundId::Recorder, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },

        { "g-soprano-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "f-soprano-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "c-soprano-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "bb-soprano-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "g-alto-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "f-alto-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "c-alto-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "bb-alto-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "c-bass-ocarina", { SoundId::Ocarina, SoundCategory::Winds, { SoundSubCategory::Bass } } },

        { "gemshorn", { SoundId::Gemshorn, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "soprano-gemshorn", { SoundId::Gemshorn, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-gemshorn", { SoundId::Gemshorn, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-gemshorn", { SoundId::Gemshorn, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-gemshorn", { SoundId::Gemshorn, SoundCategory::Winds, { SoundSubCategory::Bass } } },

        { "theremin", { SoundId::Theremin, SoundCategory::Winds, { SoundSubCategory::Electric } } },

        { "pan-flute", { SoundId::PanFlute, SoundCategory::Winds } },

        { "quena", { SoundId::Quena, SoundCategory::Winds } },
        { "c-quena", { SoundId::Quena, SoundCategory::Winds } },
        { "g-quena", { SoundId::Quena, SoundCategory::Winds } },
        { "f-quena", { SoundId::Quena, SoundCategory::Winds } },
        { "d-quena", { SoundId::Quena, SoundCategory::Winds } },

        { "piccolo-heckelphone", { SoundId::Heckelphone, SoundCategory::Winds } },
        { "heckelphone", { SoundId::Heckelphone, SoundCategory::Winds } },
        { "piccolo-oboe", { SoundId::Oboe, SoundCategory::Winds } },
        { "baroque-oboe", { SoundId::Oboe, SoundCategory::Winds, { SoundSubCategory::Baroque } } },
        { "oboe", { SoundId::Oboe, SoundCategory::Winds } },
        { "oboe-d'amore", { SoundId::Oboe, SoundCategory::Winds } },
        { "oboe-da-caccia", { SoundId::Oboe, SoundCategory::Winds } },
        { "english-horn", { SoundId::Oboe, SoundCategory::Winds, { SoundSubCategory::English } } },
        { "bass-oboe", { SoundId::Oboe, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "lupophone", { SoundId::Lupophone, SoundCategory::Winds } },

        { "sopranino-shawm", { SoundId::Shawm, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "soprano-shawm", { SoundId::Shawm, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-shawm", { SoundId::Shawm, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-shawm", { SoundId::Shawm, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-shawm", { SoundId::Shawm, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "great-bass-shawm", { SoundId::Shawm, SoundCategory::Winds, { SoundSubCategory::Great_Bass } } },

        { "cromorne", { SoundId::Cromorne, SoundCategory::Winds, { SoundSubCategory::French,
                                                                   SoundSubCategory::Baroque,
                                                                   SoundSubCategory::Reed } } },

        { "crumhorn", { SoundId::Crumhorn, SoundCategory::Winds } },
        { "soprano-crumhorn", { SoundId::Crumhorn, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-crumhorn", { SoundId::Crumhorn, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-crumhorn", { SoundId::Crumhorn, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-crumhorn", { SoundId::Crumhorn, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "greatbass-crumhorn", { SoundId::Crumhorn, SoundCategory::Winds, { SoundSubCategory::Great_Bass } } },

        { "cornamuse", { SoundId::Cornamuse, SoundCategory::Winds } },
        { "soprano-cornamuse", { SoundId::Cornamuse, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-cornamuse", { SoundId::Cornamuse, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-cornamuse", { SoundId::Cornamuse, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-cornamuse", { SoundId::Cornamuse, SoundCategory::Winds, { SoundSubCategory::Bass } } },

        { "soprano-kelhorn", { SoundId::Kelhorn, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-kelhorn", { SoundId::Kelhorn, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-kelhorn", { SoundId::Kelhorn, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-kelhorn", { SoundId::Kelhorn, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "greatbass-kelhorn", { SoundId::Kelhorn, SoundCategory::Winds, { SoundSubCategory::Great_Bass } } },

        { "sopranino-rauschpfeife", { SoundId::Rauschpfeife, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "rauschpfeife", { SoundId::Rauschpfeife, SoundCategory::Winds } },
        { "soprano-rauschpfeife", { SoundId::Rauschpfeife, SoundCategory::Winds, { SoundSubCategory::Soprano } } },

        { "f-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "e-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "d-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "c-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "b-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "bb-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "a-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "g-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Armenian } } },
        { "a-bass-duduk", { SoundId::Duduk, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                    SoundSubCategory::Armenian } } },

        { "shenai", { SoundId::Shenai, SoundCategory::Winds, { SoundSubCategory::Indian } } },

        { "piccolo-clarinet", { SoundId::Clarinet, SoundCategory::Winds } },
        { "soprano-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "eb-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Sopranino,
                                                                      SoundSubCategory::In_E_flat } } },
        { "d-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "c-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "bb-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Soprano,
                                                                      SoundSubCategory::In_B_flat } } },
        { "clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Soprano,
                                                                   SoundSubCategory::In_B_flat } } },
        { "a-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "g-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "basset-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "basset-horn", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "bass-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                        SoundSubCategory::In_B_flat } } },
        { "bb-bass-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                           SoundSubCategory::In_B_flat } } },
        { "bb-bass-clarinet-bass-clef", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                                     SoundSubCategory::In_B_flat } } },
        { "a-bass-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "a-bass-clarinet-bass-clef", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "contra-alto-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Contra_Alto } } },
        { "contrabass-clarinet", { SoundId::Clarinet, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },

        { "sopranino-chalumeau", { SoundId::Chalumeau, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "soprano-chalumeau", { SoundId::Chalumeau, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-chalumeau", { SoundId::Chalumeau, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "chalumeau", { SoundId::Chalumeau, SoundCategory::Winds } },
        { "tenor-chalumeau", { SoundId::Chalumeau, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-chalumeau", { SoundId::Chalumeau, SoundCategory::Winds, { SoundSubCategory::Bass } } },

        { "d-xaphoon", { SoundId::Xaphoon, SoundCategory::Winds } },
        { "xaphoon", { SoundId::Xaphoon, SoundCategory::Winds } },
        { "bb-xaphoon", { SoundId::Xaphoon, SoundCategory::Winds } },
        { "g-xaphoon", { SoundId::Xaphoon, SoundCategory::Winds } },

        { "tarogato", { SoundId::Tarogato, SoundCategory::Winds, { SoundSubCategory::Hungarian,
                                                                   SoundSubCategory::Romanian } } },

        { "octavin", { SoundId::Octavin, SoundCategory::Winds } },

        { "sopranissimo-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Sopranissimo } } },
        { "sopranino-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "aulochrome", { SoundId::Saxophone, SoundCategory::Winds } },
        { "soprano-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "mezzo-soprano-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Mezzo_Soprano } } },
        { "heckelphone-clarinet", { SoundId::HeckelphoneClarinet, SoundCategory::Winds } },
        { "alto-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "melody-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Melody } } },
        { "saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "tenor-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "baritone-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Baritone } } },
        { "bass-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "contrabass-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },
        { "subcontrabass-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Sub_Contra_Bass } } },

        { "bassoon", { SoundId::Bassoon, SoundCategory::Winds } },
        { "contrabassoon", { SoundId::Contrabassoon, SoundCategory::Winds } },
        { "reed-contrabass", { SoundId::Contrabassoon, SoundCategory::Winds, { SoundSubCategory::Reed } } },
        { "dulcian", { SoundId::Dulcian, SoundCategory::Winds } },
        { "rackett", { SoundId::Rackett, SoundCategory::Winds } },

        { "sopranino-sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Sopranino } } },
        { "sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "soprano-sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "baritone-sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Baritone } } },
        { "bass-sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "contrabass-sarrusophone", { SoundId::Sarrusophone, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },

        { "bagpipe", { SoundId::Bagpipe, SoundCategory::Winds } },
        { "accordion", { SoundId::Accordion, SoundCategory::Winds } },
        { "bandoneon", { SoundId::Bandoneon, SoundCategory::Winds } },
        { "concertina", { SoundId::Concertina, SoundCategory::Winds } },

        { "harmonica", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-d10high-g", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-d10f", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-d10d", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-d10c", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-d10a", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-d10g", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-d10low-d", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Diatonic } } },
        { "harmonica-c12c", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Chromatic } } },
        { "harmonica-c12g", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Chromatic } } },
        { "harmonica-c12tenor-c", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Chromatic,
                                                                                SoundSubCategory::Tenor } } },
        { "harmonica-c14c", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Chromatic } } },
        { "harmonica-c16c", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Chromatic } } },
        { "harmonica-chordet", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Huang } } },
        { "bass-harmonica-hohner", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                                 SoundSubCategory::Hohner } } },
        { "bass-harmonica-huang", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                                SoundSubCategory::Huang } } },
        { "bass-harmonica", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Bass } } },

        { "melodica", { SoundId::Melodica, SoundCategory::Winds } },

        { "alto-sheng", { SoundId::Sheng, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-sheng", { SoundId::Sheng, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-sheng", { SoundId::Sheng, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "sheng", { SoundId::Sheng, SoundCategory::Winds } },
        { "soprano-sheng", { SoundId::Sheng, SoundCategory::Winds, { SoundSubCategory::Soprano } } },

        { "brass", { SoundId::BrassGroup, SoundCategory::Winds } },

        { "c-horn-alto", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Alto,
                                                                  SoundSubCategory::In_C } } },
        { "bb-horn-alto", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Alto,
                                                                   SoundSubCategory::In_B_flat } } },
        { "a-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::In_A } } },
        { "ab-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::In_A_flat } } },
        { "g-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::In_G } } },
        { "e-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::In_E } } },
        { "eb-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::In_E_flat } } },
        { "d-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::In_D } } },
        { "horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::French,
                                                           SoundSubCategory::In_F } } },
        { "c-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::In_C } } },
        { "c-horn-bass", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                  SoundSubCategory::In_C } } },
        { "bb-horn-basso", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                    SoundSubCategory::In_B_flat } } },
        { "vienna-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Vienna } } },

        { "bb-wagner-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Wagner } } },
        { "f-wagner-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Wagner } } },
        { "wagner-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Wagner } } },

        { "eb-cornet", { SoundId::Cornet, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "c-cornet", { SoundId::Cornet, SoundCategory::Winds } },
        { "bb-cornet", { SoundId::Cornet, SoundCategory::Winds } },
        { "a-cornet", { SoundId::Cornet, SoundCategory::Winds } },

        { "saxhorn", { SoundId::Saxhorn, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "f-alto-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "eb-alto-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "baritone-horn", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Baritone } } },
        { "baritone-horn-treble", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Baritone,
                                                                           SoundSubCategory::Treble } } },
        { "baritone-horn-central-europe-treble", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::CentralEuropean,
                                                                                          SoundSubCategory::Baritone,
                                                                                          SoundSubCategory::Treble } } },
        { "baritone-horn-central-europe", { SoundId::Horn, SoundCategory::Winds, { SoundSubCategory::Baritone,
                                                                                   SoundSubCategory::CentralEuropean } } },
        { "posthorn", { SoundId::Posthorn, SoundCategory::Winds } },

        { "bb-piccolo-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Piccolo } } },
        { "piccolo-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Piccolo } } },
        { "a-piccolo-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Piccolo } } },
        { "f-trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "e-trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "eb-trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "d-trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "c-trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "bb-trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "a-trumpet", { SoundId::Trumpet, SoundCategory::Winds } },
        { "pocket-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Pocket } } },
        { "slide-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Slide } } },
        { "tenor-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "eb-bass-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "c-bass-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "bass-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "bb-bass-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "f-baroque-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Baroque } } },
        { "eb-baroque-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Baroque } } },
        { "d-baroque-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Baroque } } },
        { "c-baroque-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Baroque } } },
        { "bb-baroque-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Baroque } } },
        { "baroque-trumpet", { SoundId::Trumpet, SoundCategory::Winds, { SoundSubCategory::Baroque } } },

        { "bugle", { SoundId::Bugle, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "soprano-bugle", { SoundId::Bugle, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-bugle", { SoundId::Bugle, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "mellophone", { SoundId::MellophoneBugle, SoundCategory::Winds } },
        { "baritone-bugle", { SoundId::Bugle, SoundCategory::Winds, { SoundSubCategory::Baritone } } },
        { "euphonium-bugle", { SoundId::EuphoniumBugle, SoundCategory::Winds } },
        { "mellophon-bugle", { SoundId::MellophoneBugle, SoundCategory::Winds } },
        { "contrabass-bugle", { SoundId::Bugle, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },

        { "fiscorn", { SoundId::Fiscorn, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "flugelhorn", { SoundId::Flugelhorn, SoundCategory::Winds } },
        { "kuhlohorn", { SoundId::Kuhlohorn, SoundCategory::Winds } },

        { "f-alto-ophicleide", { SoundId::Ophicleide, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "eb-alto-ophicleide", { SoundId::Ophicleide, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "ophicleide", { SoundId::Ophicleide, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "c-bass-ophicleide", { SoundId::Ophicleide, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "bb-bass-ophicleide", { SoundId::Ophicleide, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "eb-contrabass-ophicleide", { SoundId::Ophicleide, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },

        { "cornettino", { SoundId::Cornettino, SoundCategory::Winds } },
        { "cornett", { SoundId::Cornett, SoundCategory::Winds } },
        { "soprano-cornett", { SoundId::Cornett, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-cornett", { SoundId::Cornett, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-cornett", { SoundId::Cornett, SoundCategory::Winds, { SoundSubCategory::Tenor } } },

        { "serpent", { SoundId::Serpent, SoundCategory::Winds } },

        { "soprano-trombone", { SoundId::Trombone, SoundCategory::Winds, { SoundSubCategory::Soprano } } },
        { "alto-trombone", { SoundId::Trombone, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-trombone", { SoundId::Trombone, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "trombone", { SoundId::Trombone, SoundCategory::Winds } },
        { "trombone-treble", { SoundId::Trombone, SoundCategory::Winds, { SoundSubCategory::Treble } } },
        { "contrabass-trombone", { SoundId::Trombone, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },
        { "bass-trombone", { SoundId::Trombone, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "cimbasso", { SoundId::Cimbasso, SoundCategory::Winds } },

        { "alto-sackbut", { SoundId::Sackbut, SoundCategory::Winds, { SoundSubCategory::Alto } } },
        { "tenor-sackbut", { SoundId::Sackbut, SoundCategory::Winds, { SoundSubCategory::Tenor } } },
        { "bass-sackbut", { SoundId::Sackbut, SoundCategory::Winds, { SoundSubCategory::Bass } } },

        { "euphonium", { SoundId::Euphonium, SoundCategory::Winds } },
        { "euphonium-treble", { SoundId::Euphonium, SoundCategory::Winds, { SoundSubCategory::Treble } } },

        { "tuba", { SoundId::Tuba, SoundCategory::Winds } },
        { "f-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "eb-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "bass-eb-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "bass-f-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Bass } } },
        { "eb-tuba-treble", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Treble,
                                                                     SoundSubCategory::Bass } } },
        { "c-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },
        { "bb-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Contra_Bass } } },
        { "bb-tuba-treble", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Treble } } },
        { "subcontrabass-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Sub_Contra_Bass } } },

        { "bb-sousaphone", { SoundId::Sousaphone, SoundCategory::Winds } },
        { "bb-sousaphone-treble", { SoundId::Sousaphone, SoundCategory::Winds, { SoundSubCategory::Treble } } },
        { "sousaphone", { SoundId::Sousaphone, SoundCategory::Winds } },
        { "helicon", { SoundId::Helicon, SoundCategory::Winds } },

        { "conch", { SoundId::Conch, SoundCategory::Winds } },
        { "horagai", { SoundId::Horagai, SoundCategory::Winds, { SoundSubCategory::Japanese } } },

        { "alphorn", { SoundId::Alphorn, SoundCategory::Winds, { SoundSubCategory::Alpine } } },
        { "rag-dung", { SoundId::RagDung, SoundCategory::Winds, { SoundSubCategory::Tibetan } } },
        { "didgeridoo", { SoundId::Didgeridoo, SoundCategory::Winds, { SoundSubCategory::Australian } } },
        { "shofar", { SoundId::Shofar, SoundCategory::Winds } },
        { "vuvuzela", { SoundId::Vuvuzela, SoundCategory::Winds } },
        { "tuned-klaxon-horns", { SoundId::KlaxonHorns, SoundCategory::Winds } },
        { "kazoo", { SoundId::Kazoo, SoundCategory::Winds } },
    };

    auto search = SETUP_DATA_MAP.find(instrument->id().toStdString());
    if (search == SETUP_DATA_MAP.cend()) {
        static PlaybackSetupData empty;
        return empty;
    }

    static const std::unordered_set<SoundId> supportPrimaryAndSecondaryCategories {
        SoundId::Flute,
    };

    if (muse::contains(supportPrimaryAndSecondaryCategories, search->second.soundId())) {
        SoundSubCategory category = instrument->isPrimary() ? SoundSubCategory::Primary : SoundSubCategory::Secondary;
        PlaybackSetupData setupData = search->second;
        setupData.add(category);
        return setupData;
    }

    return search->second;
}
