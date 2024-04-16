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

#include "keyboardssetupdataresolver.h"

#include <unordered_set>

using namespace mu::engraving;
using namespace muse::mpe;

PlaybackSetupData KeyboardsSetupDataResolver::doResolve(const Instrument* instrument)
{
    static const std::unordered_map<std::string, PlaybackSetupData> SETUP_DATA_MAP = {
        { "celesta", { SoundId::Celesta, SoundCategory::Keyboards } },
        { "clavichord", { SoundId::Clavichord, SoundCategory::Keyboards, { SoundSubCategory::Baroque } } },
        { "clavinet", { SoundId::Clavichord, SoundCategory::Keyboards, { SoundSubCategory::Electric } } },
        { "ondes-martenot", { SoundId::OndesMartenot, SoundCategory::Keyboards, { SoundSubCategory::Electric } } },
        { "harpsichord", { SoundId::Harpsichord, SoundCategory::Keyboards } },
        { "virginal", { SoundId::Virginal, SoundCategory::Keyboards, { SoundSubCategory::Baroque } } },
        { "electric-piano", { SoundId::Piano, SoundCategory::Keyboards, { SoundSubCategory::Electric } } },
        { "grand-piano", { SoundId::Piano, SoundCategory::Keyboards, { SoundSubCategory::Grand } } },
        { "honky-tonk-piano", { SoundId::Piano, SoundCategory::Keyboards, { SoundSubCategory::HonkyTonk } } },
        { "piano", { SoundId::Piano, SoundCategory::Keyboards } },
        { "toy-piano", { SoundId::Piano, SoundCategory::Keyboards, { SoundSubCategory::Toy } } },
        { "upright-piano", { SoundId::Piano, SoundCategory::Keyboards, { SoundSubCategory::Upright } } },

        { "hammond-organ", { SoundId::Organ, SoundCategory::Keyboards, { SoundSubCategory::Hammond } } },
        { "organ", { SoundId::Organ, SoundCategory::Keyboards } },
        { "percussive-organ", { SoundId::Organ, SoundCategory::Keyboards, { SoundSubCategory::Percussive } } },
        { "pipe-organ", { SoundId::Organ, SoundCategory::Keyboards, { SoundSubCategory::Piped } } },
        { "rotary-organ", { SoundId::Organ, SoundCategory::Keyboards, { SoundSubCategory::Rotary } } },
        { "harmonium", { SoundId::Organ, SoundCategory::Keyboards, { SoundSubCategory::Reed } } },
        { "reed-organ", { SoundId::Organ, SoundCategory::Keyboards, { SoundSubCategory::Reed } } },

        { "brass-synthesizer", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                                   SoundSubCategory::Brass } } },
        { "string-synthesizer", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                                    SoundSubCategory::String } } },
        { "mallet-synthesizer", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric } } },
        { "atmosphere-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                                  SoundSubCategory::FX_Atmosphere } } },
        { "brightness-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                                  SoundSubCategory::FX_Brightness } } },
        { "crystal-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                               SoundSubCategory::FX_Crystal } } },
        { "echoes-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                              SoundSubCategory::FX_Echoes } } },
        { "goblins-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                               SoundSubCategory::FX_Goblins } } },
        { "rain-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric, SoundSubCategory::FX_Rain } } },
        { "effect-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric, SoundSubCategory::FX_Atmosphere,
                                                                              SoundSubCategory::FX_Brightness, SoundSubCategory::FX_Crystal,
                                                                              SoundSubCategory::FX_Echoes, SoundSubCategory::FX_Goblins,
                                                                              SoundSubCategory::FX_Rain } } },
        { "sine-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                            SoundSubCategory::Sine_Wave } } },
        { "square-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                              SoundSubCategory::Square_Wave } } },
        { "saw-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                           SoundSubCategory::Sawtooth_Wave } } },
        { "new-age-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                               SoundSubCategory::NewAge } } },
        { "pad-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                           SoundSubCategory::Pad } } },
        { "warm-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                            SoundSubCategory::Warm } } },
        { "poly-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                            SoundSubCategory::Polysynth } } },
        { "choir-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                             SoundSubCategory::Choir } } },
        { "metallic-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                                SoundSubCategory::Metallic } } },
        { "halo-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                            SoundSubCategory::Halo } } },
        { "sweep-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                             SoundSubCategory::Sweep } } },
        { "soundtrack-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                                  SoundSubCategory::FX_SoundTrack } } },
        { "sci-fi-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                              SoundSubCategory::FX_SciFi } } },
    };

    auto search = SETUP_DATA_MAP.find(instrument->id().toStdString());
    if (search == SETUP_DATA_MAP.cend()) {
        static PlaybackSetupData empty;
        return empty;
    }

    return search->second;
}
