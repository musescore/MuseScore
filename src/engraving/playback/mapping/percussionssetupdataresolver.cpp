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

#include "percussionssetupdataresolver.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

PlaybackSetupData PercussionsSetupDataResolver::doResolve(const Instrument* instrument)
{
    static const std::unordered_map<std::string, mpe::PlaybackSetupData> SETUP_DATA_MAP = {
        { "timpani", { SoundId::Timpani, SoundCategory::Percussions } },
        { "roto-toms", { SoundId::RotoToms, SoundCategory::Percussions } },
        { "tubaphone", { SoundId::Tubaphone, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "soprano-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                      SoundSubCategory::Steel,
                                                                                      SoundSubCategory::Soprano } } },
        { "alto-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                   SoundSubCategory::Steel,
                                                                                   SoundSubCategory::Alto } } },
        { "guitar-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                     SoundSubCategory::Steel } } },
        { "cello-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                    SoundSubCategory::Steel } } },
        { "steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                              SoundSubCategory::Steel } } },
        { "tenor-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                    SoundSubCategory::Steel,
                                                                                    SoundSubCategory::Tenor } } },
        { "bass-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                   SoundSubCategory::Steel,
                                                                                   SoundSubCategory::Bass } } },
        { "glockenspiel", { SoundId::Glockenspiel, SoundCategory::Percussions } },
        { "xylophone", { SoundId::Xylophone, SoundCategory::Percussions } },
        { "xylomarimba", { SoundId::Xylomarimba, SoundCategory::Percussions } },
        { "vibraphone", { SoundId::Vibraphone, SoundCategory::Percussions } },
        { "dulcimer", { SoundId::Dulcimer, SoundCategory::Percussions } },
        { "cimbalom", { SoundId::Cimbalom, SoundCategory::Percussions } },
        { "marimba", { SoundId::Marimba, SoundCategory::Percussions } },
        { "marimba-single", { SoundId::Marimba, SoundCategory::Percussions } },
        { "bass-marimba", { SoundId::Marimba, SoundCategory::Percussions, { SoundSubCategory::Bass } } },
        { "contrabass-marimba", { SoundId::Marimba, SoundCategory::Percussions, { SoundSubCategory::Contra_Bass } } },

        { "crotales", { SoundId::Crotales, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "almglocken", { SoundId::Kalimba, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "tubular-bells", { SoundId::Chimes, SoundCategory::Percussions } },
        { "carillon", { SoundId::Carillon, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "tuned-gongs", { SoundId::Gong, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "opera-gong", { SoundId::Gong, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                       SoundSubCategory::Opera } } },
        { "wind-gong", { SoundId::Gong, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                      SoundSubCategory::Wind } } },
        { "hand-bells", { SoundId::Bell, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                       SoundSubCategory::Hand } } },

        { "orff-soprano-glockenspiel", { SoundId::Glockenspiel, SoundCategory::Percussions, { SoundSubCategory::Soprano,
                                                                                              SoundSubCategory::Orff } } },
        { "orff-alto-glockenspiel", { SoundId::Glockenspiel, SoundCategory::Percussions, { SoundSubCategory::Alto,
                                                                                           SoundSubCategory::Orff } } },
        { "orff-soprano-metallophone", { SoundId::Metallophone, SoundCategory::Percussions, { SoundSubCategory::Soprano,
                                                                                              SoundSubCategory::Orff } } },
        { "orff-soprano-xylophone", { SoundId::Xylophone, SoundCategory::Percussions, { SoundSubCategory::Soprano,
                                                                                        SoundSubCategory::Orff } } },
        { "metallophone", { SoundId::Metallophone, SoundCategory::Percussions, { SoundSubCategory::Orff } } },
        { "orff-alto-metallophone", { SoundId::Metallophone, SoundCategory::Percussions, { SoundSubCategory::Alto,
                                                                                           SoundSubCategory::Orff } } },
        { "orff-alto-xylophone", { SoundId::Xylophone, SoundCategory::Percussions, { SoundSubCategory::Alto,
                                                                                     SoundSubCategory::Orff } } },
        { "orff-bass-metallophone", { SoundId::Metallophone, SoundCategory::Percussions, { SoundSubCategory::Bass,
                                                                                           SoundSubCategory::Orff } } },
        { "orff-bass-xylophone", { SoundId::Xylophone, SoundCategory::Percussions, { SoundSubCategory::Bass,
                                                                                     SoundSubCategory::Orff } } },

        { "flexatone", { SoundId::Flexatone, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "musical-saw", { SoundId::MusicalSaw, SoundCategory::Percussions, { SoundSubCategory::Metal } } },

        { "musical-glasses", { SoundId::MusicalGlasses, SoundCategory::Percussions, { SoundSubCategory::Glass } } },
        { "glass-harmonica", { SoundId::Harmonica, SoundCategory::Percussions, { SoundSubCategory::Glass } } },

        { "alto-kalimba", { SoundId::Kalimba, SoundCategory::Percussions, { SoundSubCategory::Alto } } },
        { "kalimba", { SoundId::Kalimba, SoundCategory::Percussions } },
        { "treble-kalimba", { SoundId::Kalimba, SoundCategory::Percussions, { SoundSubCategory::Treble } } },

        { "automobile-brake-drums", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                   SoundSubCategory::Brake } } },
        { "bongos", { SoundId::Bongos, SoundCategory::Percussions } },
        { "chinese-tom-toms", { SoundId::TomToms, SoundCategory::Percussions, { SoundSubCategory::Chinese } } },
        { "bass-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Bass } } },
        { "snare-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Snare } } },
        { "tom-toms", { SoundId::TomToms, SoundCategory::Percussions } },
        { "congas", { SoundId::Conga, SoundCategory::Percussions } },
        { "xiaogu", { SoundId::Xiaogu, SoundCategory::Percussions } },
        { "bangu", { SoundId::Bangu, SoundCategory::Percussions } },
        { "dagu", { SoundId::Dagu, SoundCategory::Percussions } },
        { "daluo", { SoundId::Daluo, SoundCategory::Percussions } },
        { "xiaoluo", { SoundId::Xiaoluo, SoundCategory::Percussions } },
        { "bo", { SoundId::Bo, SoundCategory::Percussions } },
        { "ban", { SoundId::Ban, SoundCategory::Percussions } },
        { "dabo", { SoundId::Dabo, SoundCategory::Percussions } },
        { "dabo", { SoundId::Dabo, SoundCategory::Percussions } },
        { "naobo", { SoundId::Naobo, SoundCategory::Percussions } },
        { "bangzi", { SoundId::Bangzi, SoundCategory::Percussions } },
        { "djembe", { SoundId::Djembe, SoundCategory::Percussions } },
        { "doumbek", { SoundId::Doumbek, SoundCategory::Percussions } },
        { "cuica", { SoundId::Cuica, SoundCategory::Percussions } },
        { "cajon", { SoundId::Cajon, SoundCategory::Percussions } },
        { "drumset", { SoundId::Drumset, SoundCategory::Percussions } },
        { "drum-kit-4", { SoundId::Drumset, SoundCategory::Percussions, { SoundSubCategory::FourPiece } } },
        { "drum-kit-5", { SoundId::Drumset, SoundCategory::Percussions, { SoundSubCategory::FivePiece } } },
        { "military-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Military } } },
        { "frame-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Frame } } },
        { "piccolo-snare-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Snare,
                                                                               SoundSubCategory::Piccolo } } },
        { "slit-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Slit } } },
        { "tablas", { SoundId::Tablas, SoundCategory::Percussions, { SoundSubCategory::Indian } } },
        { "timbales", { SoundId::Timbales, SoundCategory::Percussions } },

        { "anvil", { SoundId::Anvil, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "bell-plate", { SoundId::Bell, SoundCategory::Percussions, { SoundSubCategory::Plate,
                                                                       SoundSubCategory::Metal } } },
        { "bell-tree", { SoundId::BellTree, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "bells", { SoundId::Bell, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "bowl-gongs", { SoundId::Gong, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                       SoundSubCategory::Bowl } } },
        { "chains", { SoundId::Chain, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "chinese-cymbal", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                             SoundSubCategory::Chinese } } },
        { "china-cymbal", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                           SoundSubCategory::China } } },
        { "cowbell", { SoundId::Bell, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                    SoundSubCategory::Cow } } },
        { "agogo-bells", { SoundId::Bell, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                        SoundSubCategory::Agogo } } },
        { "crash-cymbal", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                           SoundSubCategory::Crash } } },
        { "cymbal", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "finger-cymbals", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                             SoundSubCategory::Finger } } },
        { "hi-hat", { SoundId::HiHat, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "iron-pipes", { SoundId::Pipe, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                       SoundSubCategory::Iron } } },
        { "mark-tree", { SoundId::MarkTree, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "metal-castanets", { SoundId::Castanet, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "metal-wind-chimes", { SoundId::Chimes, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                SoundSubCategory::Wind } } },
        { "ride-cymbal", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                          SoundSubCategory::Ride } } },
        { "sleigh-bells", { SoundId::Bell, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                         SoundSubCategory::Sleigh } } },
        { "splash-cymbal", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                            SoundSubCategory::Splash } } },
        { "tam-tam", { SoundId::TamTam, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "thundersheet", { SoundId::Thundersheet, SoundCategory::Percussions, { SoundSubCategory::Metal } } },
        { "triangle", { SoundId::Triangle, SoundCategory::Percussions, { SoundSubCategory::Metal } } },

        { "castanets", { SoundId::Castanet, SoundCategory::Percussions, { SoundSubCategory::Wooden } } },
        { "claves", { SoundId::Claves, SoundCategory::Percussions, { SoundSubCategory::Wooden } } },
        { "guiro", { SoundId::Guiro, SoundCategory::Percussions, { SoundSubCategory::Wooden } } },
        { "temple-blocks", { SoundId::Block, SoundCategory::Percussions, { SoundSubCategory::Wooden,
                                                                           SoundSubCategory::Temple } } },
        { "log-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Wooden,
                                                                     SoundSubCategory::Log } } },
        { "ocean-drum", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Ocean } } },
        { "wood-blocks", { SoundId::Block, SoundCategory::Percussions, { SoundSubCategory::Wooden } } },
        { "wooden-wind-chimes", { SoundId::Chimes, SoundCategory::Percussions, { SoundSubCategory::Wooden,
                                                                                 SoundSubCategory::Wind } } },
        { "bamboo-wind-chimes", { SoundId::Chimes, SoundCategory::Percussions, { SoundSubCategory::Wooden,
                                                                                 SoundSubCategory::Wind } } },

        { "cabasa", { SoundId::Cabasa, SoundCategory::Percussions } },
        { "maracas", { SoundId::Maraca, SoundCategory::Percussions } },
        { "quijada", { SoundId::Quijada, SoundCategory::Percussions } },
        { "ratchet", { SoundId::Ratchet, SoundCategory::Percussions } },
        { "shaker", { SoundId::Shaker, SoundCategory::Percussions } },
        { "shekere", { SoundId::Shekere, SoundCategory::Percussions } },
        { "sandpaper-blocks", { SoundId::Block, SoundCategory::Percussions, { SoundSubCategory::Sandpaper } } },
        { "glass-wind-chimes", { SoundId::Chimes, SoundCategory::Percussions, { SoundSubCategory::Glass,
                                                                                SoundSubCategory::Wind } } },
        { "shell-wind-chimes", { SoundId::Chimes, SoundCategory::Percussions, { SoundSubCategory::Shell,
                                                                                SoundSubCategory::Wind } } },
        { "percussion", { SoundId::Drumset, SoundCategory::Percussions, { SoundSubCategory::Orchestral } } },
        { "stones", { SoundId::Stones, SoundCategory::Percussions } },
        { "tambourine", { SoundId::Tambourine, SoundCategory::Percussions } },
        { "tubo", { SoundId::Tubo, SoundCategory::Percussions } },
        { "vibraslap", { SoundId::Vibraslap, SoundCategory::Percussions } },
        { "whip", { SoundId::Whip, SoundCategory::Percussions } },
        { "cannon", { SoundId::Cannon, SoundCategory::Percussions } },
        { "bird-call", { SoundId::BirdCall, SoundCategory::Percussions } },

        { "marching-snare", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Marching,
                                                                           SoundSubCategory::Snare } } },
        { "marching-tenor-drums", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Marching,
                                                                                 SoundSubCategory::Snare,
                                                                                 SoundSubCategory::Tenor } } },
        { "marching-show-tenors", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Show_Style,
                                                                                 SoundSubCategory::Snare,
                                                                                 SoundSubCategory::Tenor } } },
        { "marching-bass-drums", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Marching,
                                                                                SoundSubCategory::Snare,
                                                                                SoundSubCategory::Bass } } },
        { "marching-cymbals", { SoundId::Cymbal, SoundCategory::Percussions, { SoundSubCategory::Marching,
                                                                               SoundSubCategory::Metal,
                                                                               SoundSubCategory::Crash } } },

        { "finger-snap", { SoundId::Snap, SoundCategory::Percussions, { SoundSubCategory::Finger } } },
        { "hand-clap", { SoundId::Clap, SoundCategory::Percussions, { SoundSubCategory::Hand } } },
        { "slap", { SoundId::Slap, SoundCategory::Percussions, { SoundSubCategory::Hand } } },
        { "stamp", { SoundId::Stamp, SoundCategory::Percussions, { SoundSubCategory::Foot } } },

        { "taiko", { SoundId::Taiko, SoundCategory::Percussions } },

        { "percussion-synthesizer", { SoundId::Synthesizer, SoundCategory::Percussions, { SoundSubCategory::Electric,
                                                                                          SoundSubCategory::Percussive } } },
    };

    auto search = SETUP_DATA_MAP.find(instrument->id().toStdString());
    if (search == SETUP_DATA_MAP.cend()) {
        if (instrument->useDrumset()) {
            LOGW() << "Unable to resolve setup data for instrument, id: " << instrument->id()
                   << ", family: " << instrument->family() << "; fallback to drumset";

            static const PlaybackSetupData DRUMSET_FALLBACK {
                SoundId::Drumset, SoundCategory::Percussions
            };

            return DRUMSET_FALLBACK;
        }

        static const PlaybackSetupData empty;
        return empty;
    }

    return search->second;
}
