/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "finaletypesconv.h"

#include <vector>
#include <string_view>
#include <unordered_map>

#include "internal/text/finaletextconv.h"
#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/note.h"
#include "engraving/dom/noteval.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/utils.h"

#include "importfinalelogger.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

int midiNoteFromPercussionNoteType(const String& instrument, musx::dom::PercussionNoteTypeId noteTypeId)
{
    const std::string instrumentKey = instrument.toStdString();

    static const std::unordered_map<std::string, std::unordered_map<int, int>> instrumentToMidi = {
        { "agogo-bells", {
            { 30, 67 }, // agogo-bells: High Agog -> High Agogo (score 17)
            { 31, 68 }, // agogo-bells: Low Agog -> Low Agogo (score 17)
        }},
        { "anvil", {
            { 216, 68 }, // anvil: Anvil -> Anvil (score 100)
        }},
        { "automobile-brake-drums", {
            { 218, 68 }, // automobile-brake-drums: Brake Drum -> Brake Drum (score 100)
        }},
        { "bamboo-wind-chimes", {
            { 460, 69 }, // bamboo-wind-chimes: Bamboo Wind Chimes -> Wind Chimes (score 22)
        }},
        { "bass-drum", {
            { 13, 36 }, // bass-drum: Concert Bass Drum -> Bass Drum (score 22)
        }},
        { "bell-plate", {
            { 161, 81 }, // bell-plate: Bell Plate -> Bell Plate (score 100)
        }},
        { "bell-tree", {
            { 473, 84 }, // bell-tree: Glissando Down -> Guiro Down Up (score 10)
        }},
        { "bird-call", {
            { 183, 50 }, // bird-call: Bird Call -> Crash Cymbals Whale Call (score 10)
        }},
        { "bongos", {
            { 32, 60 }, // bongos: High Bongo -> High Bongo (score 105)
            { 33, 61 }, // bongos: Low Bongo -> Low Bongo (score 105)
        }},
        { "bowl-gongs", {
            { 354, 81 }, // bowl-gongs: Bowl Gong -> Bowl (score 12)
        }},
        { "cabasa", {
            { 34, 69 }, // cabasa: Cabasa -> Cabasa (score 105)
        }},
        { "cannon", {
            { 219, 60 }, // cannon: Cannon -> Cannon (score 100)
        }},
        { "castanets", {
            { 362, 85 }, // castanets: Castanets -> Castanets (score 105)
        }},
        { "china-cymbal", {
            { 17, 52 }, // china-cymbal: China Cymbal -> China Cymbal (score 105)
        }},
        { "chinese-tom-toms", {
            { 24, 61 }, // chinese-tom-toms: Chinese Tom-toms -> High Tom (score 10)
        }},
        { "claves", {
            { 35, 75 }, // claves: Claves -> Claves (score 105)
        }},
        { "congas", {
            { 36, 63 }, // congas: High Conga -> Conga (score 17)
            { 332, 62 }, // congas: Mute High Conga -> High Agogo Mute (score 20)
        }},
        { "cowbell", {
            { 6, 56 }, // cowbell: Cowbell -> Cowbell (score 105)
        }},
        { "crash-cymbal", {
            { 88, 57 }, // crash-cymbal: Suspended Cymbal -> Suspended Cymbal Roll (score 22)
        }},
        { "cuica", {
            { 39, 78 }, // cuica: Mute Cuica -> Cuica High (score 15)
            { 40, 79 }, // cuica: Open Cuica -> Cuica Low (score 15)
        }},
        { "cymbal", {
            { 164, 57 }, // cymbal: Hand Cymbals -> Crash Cymbals Crash (score 10)
        }},
        { "djembe", {
            { 9, 63 }, // djembe: Open -> Triangle Open (score 12)
            { 12, 62 }, // djembe: Slap -> Vibra Slap (score 12)
            { 13, 64 }, // djembe: Bass -> Bass Drum (score 12)
        }},
        { "doumbek", {
            { 9, 63 }, // doumbek: Open -> Triangle Open (score 12)
            { 12, 62 }, // doumbek: Slap -> Vibra Slap (score 12)
            { 13, 64 }, // doumbek: Bass -> Bass Drum (score 12)
        }},
        { "drum-kit-4", {
            { 1, 38 }, // drum-kit-4: Snare -> Snare Drum (score 17)
            { 3, 37 }, // drum-kit-4: Cross-stick -> Snare Cross Stick (score 27)
            { 4, 49 }, // drum-kit-4: Crash Cymbal -> Crash Cymbal (score 105)
            { 13, 36 }, // drum-kit-4: Bass Drum -> Bass Drum (score 100)
            { 14, 46 }, // drum-kit-4: Open Hi-Hat -> Hi-Hat Open (score 35)
            { 15, 42 }, // drum-kit-4: Closed Hi-Hat -> Hi-Hat Closed (score 35)
            { 16, 44 }, // drum-kit-4: Pedal Hi-Hat -> Hi-Hat Foot (score 25)
            { 18, 51 }, // drum-kit-4: Ride Cymbal -> Ride Cymbal (score 105)
            { 20, 53 }, // drum-kit-4: Ride Bell -> Ride Bell (score 105)
            { 24, 50 }, // drum-kit-4: Tom -> High Tom (score 17)
            { 29, 41 }, // drum-kit-4: Floor Tom -> Floor Tom 2 (score 27)
        }},
        { "drum-kit-5", {
            { 1, 38 }, // drum-kit-5: Snare -> Snare Drum (score 17)
            { 3, 37 }, // drum-kit-5: Cross-stick -> Snare Cross Stick (score 27)
            { 4, 49 }, // drum-kit-5: Crash Cymbal -> Crash Cymbal (score 105)
            { 5, 57 }, // drum-kit-5: Crash Cymbal 2 -> Crash Cymbal 2 (score 105)
            { 13, 36 }, // drum-kit-5: Bass Drum -> Bass Drum (score 100)
            { 14, 46 }, // drum-kit-5: Open Hi-Hat -> Hi-Hat Open (score 35)
            { 15, 42 }, // drum-kit-5: Closed Hi-Hat -> Hi-Hat Closed (score 35)
            { 16, 44 }, // drum-kit-5: Pedal Hi-Hat -> Hi-Hat Foot (score 25)
            { 17, 52 }, // drum-kit-5: China Cymbal -> China Cymbal (score 105)
            { 18, 51 }, // drum-kit-5: Ride Cymbal -> Ride Cymbal (score 105)
            { 20, 53 }, // drum-kit-5: Ride Bell -> Ride Bell (score 105)
            { 21, 55 }, // drum-kit-5: Splash Cymbal -> Splash Cymbal (score 105)
            { 24, 50 }, // drum-kit-5: High Tom -> High Tom (score 105)
            { 27, 47 }, // drum-kit-5: Low Tom -> Low Tom (score 100)
            { 29, 41 }, // drum-kit-5: Floor Tom -> Floor Tom 2 (score 27)
        }},
        { "drumset", {
            { 1, 38 }, // drumset: Acoustic Snare -> Snare Drum (score 15)
            { 3, 37 }, // drumset: Side Stick -> Snare Cross Stick (score 15)
            { 4, 49 }, // drumset: Crash Cymbal 1 -> Crash Cymbal (score 27)
            { 5, 57 }, // drumset: Crash Cymbal 2 -> Crash Cymbal 2 (score 105)
            { 6, 56 }, // drumset: Cowbell -> Cowbell (score 105)
            { 7, 54 }, // drumset: Tambourine -> Tambourine (score 105)
            { 13, 35 }, // drumset: Bass Drum 2 -> Bass Drum (score 27)
            { 14, 46 }, // drumset: Open Hi-Hat -> Hi-Hat Open (score 35)
            { 15, 42 }, // drumset: Closed Hi-Hat -> Hi-Hat Closed (score 35)
            { 16, 44 }, // drumset: Pedal Hi-Hat -> Hi-Hat Foot (score 25)
            { 17, 52 }, // drumset: China Cymbal -> China Cymbal (score 105)
            { 18, 51 }, // drumset: Ride Cymbal 1 -> Ride Cymbal (score 27)
            { 19, 59 }, // drumset: Ride Cymbal 2 -> Ride Cymbal 2 (score 105)
            { 20, 53 }, // drumset: Ride Bell -> Ride Bell (score 105)
            { 21, 55 }, // drumset: Splash Cymbal -> Splash Cymbal (score 105)
            { 23, 40 }, // drumset: Electric Snare -> Electric Snare Drum (score 27)
            { 24, 50 }, // drumset: High Tom -> High Tom (score 105)
            { 25, 48 }, // drumset: Hi-Mid Tom -> High-Mid Tom (score 25)
            { 26, 47 }, // drumset: Low-Mid Tom -> Low-Mid Tom (score 105)
            { 27, 45 }, // drumset: Low Tom -> Low Tom (score 105)
            { 28, 43 }, // drumset: High Floor Tom -> Floor Tom 1 (score 25)
            { 29, 41 }, // drumset: Low Floor Tom -> Floor Tom 2 (score 25)
        }},
        { "finger-cymbals", {
            { 206, 81 }, // finger-cymbals: Finger Cymbals -> Finger Cymbals (score 100)
        }},
        { "finger-snap", {
            { 234, 75 }, // finger-snap: Finger Snap -> Finger Snaps (score 12)
        }},
        { "frame-drum", {
            { 392, 45 }, // frame-drum: Frame Drum -> Frame Drum (score 100)
        }},
        { "glass-wind-chimes", {
            { 460, 84 }, // glass-wind-chimes: Glass Wind Chimes -> Wind Chimes (score 22)
        }},
        { "guiro", {
            { 41, 74 }, // guiro: Long Giro -> Guiro Long (score 15)
            { 42, 73 }, // guiro: Short Giro -> Guiro Short (score 15)
        }},
        { "hand-clap", {
            { 22, 39 }, // hand-clap: Hand Clap -> Hand Clap (score 105)
        }},
        { "hi-hat", {
            { 14, 46 }, // hi-hat: Open Hi-Hat -> Hi-Hat Open (score 35)
            { 15, 42 }, // hi-hat: Closed Hi-Hat -> Hi-Hat Closed (score 35)
            { 16, 44 }, // hi-hat: Pedal Hi-Hat -> Hi-Hat Foot (score 25)
        }},
        { "log-drum", {
            { 10, 62 }, // log-drum: High Sound -> High Woodblock (score 10)
            { 25, 61 }, // log-drum: High-Mid Sound -> High-Mid Tom (score 20)
            { 26, 59 }, // log-drum: Low-Mid Sound -> Low-Mid Tom (score 20)
            { 522, 58 }, // log-drum: Low Sound -> Flexatone Low (score 10)
            { 523, 60 }, // log-drum: Mid Sound -> Flexatone Mid (score 10)
        }},
        { "maracas", {
            { 43, 70 }, // maracas: Maracas -> Maracas (score 105)
        }},
        { "marching-bass-drums", {
            { 2, 37 }, // marching-bass-drums: Drum 5 -> Kick Drum (score 10)
            { 78, 39 }, // marching-bass-drums: Drum 5 Rim -> Bass Drum Rim (score 20)
            { 82, 90 }, // marching-bass-drums: Unison -> Bass Drum Unison Hits (score 12)
        }},
        { "marching-bassline", {
            { 2, 61 }, // marching-bassline: Hit Drum 10 -> Kick Drum (score 10)
            { 22, 35 }, // marching-bassline: Hand Clap -> Hand Clap (score 100)
            { 78, 73 }, // marching-bassline: Rim Drum 10 -> Bass Drum Rim (score 20)
            { 82, 28 }, // marching-bassline: Dut (Unison) -> Bass Drum Unison Hits (score 10)
            { 175, 84 }, // marching-bassline: Sticks In -> Crash Cymbals Scratch In (score 10)
            { 461, 33 }, // marching-bassline: Metronome -> Metronome Click (score 12)
        }},
        { "marching-cymballine", {
            { 4, 38 }, // marching-cymballine: Orchestra Crash 3 -> Crash Cymbal (score 10)
            { 5, 62 }, // marching-cymballine: Orchestra Crash 2 -> Crash Cymbal 2 (score 20)
            { 121, 47 }, // marching-cymballine: Sizzle Cymbal 3 -> Sizzle Cymbal (score 22)
            { 189, 45 }, // marching-cymballine: Crunch Cymbal 3 -> Cymbal Section Crunch Choke (score 20)
            { 190, 50 }, // marching-cymballine: Ding Cymbal 3 -> Cymbal Section Ding (score 20)
            { 191, 51 }, // marching-cymballine: Zing Cymbal 3 -> Cymbal Section Fast Zing (score 20)
            { 201, 43 }, // marching-cymballine: Tap Cymbal 3 -> Cymbal Section Tap Choke (score 20)
        }},
        { "marching-cymbals", {
            { 4, 72 }, // marching-cymbals: Full Crash -> Crash Cymbal (score 10)
            { 14, 76 }, // marching-cymbals: Hi-Hat -> Hi-Hat Open (score 22)
            { 49, 93 }, // marching-cymbals: Roll -> Cowbell Roll (score 12)
            { 70, 84 }, // marching-cymbals: Bell Tap -> Bell Tree (score 15)
            { 121, 77 }, // marching-cymbals: Sizzle -> Sizzle Cymbal (score 12)
            { 171, 91 }, // marching-cymbals: Zing -> Crash Cymbals Fast Zing (score 12)
            { 179, 83 }, // marching-cymbals: Tap-Choke -> Crash Cymbals Tap Choke (score 22)
            { 187, 79 }, // marching-cymbals: Crash-Choke -> Cymbal Section Crash Choke Fat (score 22)
        }},
        { "marching-snare", {
            { 1, 50 }, // marching-snare: Battery Snare -> Snare Drum (score 10)
            { 14, 74 }, // marching-snare: Open Hi-Hat -> Hi-Hat Open (score 30)
            { 15, 76 }, // marching-snare: Closed Hi-Hat -> Hi-Hat Closed (score 30)
            { 18, 72 }, // marching-snare: Ride Cymbal 1 -> Ride Cymbal (score 22)
            { 78, 53 }, // marching-snare: Rim Click -> Bass Drum Rim (score 10)
            { 105, 57 }, // marching-snare: Stick Shot -> Suspended Cymbal Fat Choke w/ Stick (score 10)
            { 239, 48 }, // marching-snare: Buzz -> Snare Buzz Roll (score 12)
            { 251, 52 }, // marching-snare: Rim Shot -> Snare Rim Shot (score 22)
            { 256, 55 }, // marching-snare: Stick Click -> Stick Click (score 100)
        }},
        { "marching-snareline", {
            { 4, 40 }, // marching-snareline: Crash Cymbal -> Crash Cymbal (score 100)
            { 6, 32 }, // marching-snareline: Cowbell -> Cowbell (score 100)
            { 15, 36 }, // marching-snareline: Hi-Hat (closed) -> Hi-Hat Closed (score 100)
            { 18, 38 }, // marching-snareline: Ride Cymbal -> Ride Cymbal (score 100)
            { 22, 23 }, // marching-snareline: Hand Clap -> Hand Clap (score 100)
            { 31, 29 }, // marching-snareline: Agogo Low -> Low Agogo (score 20)
            { 68, 31 }, // marching-snareline: Jam Block -> High Jam Block (score 22)
            { 78, 63 }, // marching-snareline: Rim -> Bass Drum Rim (score 12)
            { 82, 28 }, // marching-snareline: Dut Unison -> Bass Drum Unison Hits (score 10)
            { 105, 65 }, // marching-snareline: Stick Shot -> Suspended Cymbal Fat Choke w/ Stick (score 10)
            { 175, 77 }, // marching-snareline: Sticks In -> Crash Cymbals Scratch In (score 10)
            { 240, 64 }, // marching-snareline: Ping Shot -> Snare Cross Shot (score 10)
            { 251, 61 }, // marching-snareline: Rim Shot -> Snare Rim Shot (score 22)
            { 256, 76 }, // marching-snareline: Stick Click -> Stick Click (score 100)
            { 273, 72 }, // marching-snareline: Rods -> Snare Section Rods (score 12)
            { 461, 21 }, // marching-snareline: Metronome -> Metronome Click (score 12)
            { 524, 30 }, // marching-snareline: Agogo Hi -> Flexatone Hi (score 10)
        }},
        { "marching-tenor-drums", {
            { 2, 36 }, // marching-tenor-drums: Drum 4 -> Kick Drum (score 15)
            { 5, 96 }, // marching-tenor-drums: Spock 2 -> Crash Cymbal 2 (score 10)
            { 23, 40 }, // marching-tenor-drums: Drum 4 Shell -> Electric Snare Drum (score 15)
            { 28, 84 }, // marching-tenor-drums: Spock 1 -> Floor Tom 1 (score 10)
            { 78, 37 }, // marching-tenor-drums: Drum 4 Rim -> Bass Drum Rim (score 20)
            { 256, 43 }, // marching-tenor-drums: Stick Click -> Stick Click (score 100)
            { 325, 38 }, // marching-tenor-drums: Drum 4 Buzz -> Spock Drum Buzz Roll (score 20)
            { 328, 85 }, // marching-tenor-drums: Spock 1 Rim -> Spock Drum Rim Shot (score 20)
        }},
        { "marching-tenorline", {
            { 2, 60 }, // marching-tenorline: Hit Drum 4 -> Kick Drum (score 10)
            { 5, 64 }, // marching-tenorline: Hit Spock 2 -> Crash Cymbal 2 (score 10)
            { 6, 32 }, // marching-tenorline: Cowbell -> Cowbell (score 100)
            { 22, 23 }, // marching-tenorline: Hand Clap -> Hand Clap (score 100)
            { 28, 65 }, // marching-tenorline: Hit Spock 1 -> Floor Tom 1 (score 10)
            { 31, 29 }, // marching-tenorline: Agogo Low -> Low Agogo (score 20)
            { 68, 31 }, // marching-tenorline: Jam Block -> High Jam Block (score 22)
            { 78, 78 }, // marching-tenorline: Rim Drum 4 -> Bass Drum Rim (score 20)
            { 82, 28 }, // marching-tenorline: Dut (unison) -> Bass Drum Unison Hits (score 10)
            { 90, 45 }, // marching-tenorline: Mallet Click -> Suspended Cymbal w/ Mallet (Crash) (score 10)
            { 117, 44 }, // marching-tenorline: Stand Hit -> Hi-Hat Stand (score 10)
            { 175, 46 }, // marching-tenorline: Sticks In -> Crash Cymbals Scratch In (score 10)
            { 328, 72 }, // marching-tenorline: Rim Shot Drum 4 -> Spock Drum Rim Shot (score 30)
            { 461, 21 }, // marching-tenorline: Metronome -> Metronome Click (score 12)
            { 524, 30 }, // marching-tenorline: Agogo Hi -> Flexatone Hi (score 10)
        }},
        { "mark-tree", {
            { 473, 84 }, // mark-tree: Glissando Down -> Guiro Down Up (score 10)
        }},
        { "metal-castanets", {
            { 362, 67 }, // metal-castanets: Metal Castanets -> Castanets (score 12)
        }},
        { "metal-wind-chimes", {
            { 460, 84 }, // metal-wind-chimes: Metal Wind Chimes -> Wind Chimes (score 22)
        }},
        { "military-drum", {
            { 1, 38 }, // military-drum: Snare -> Snare Drum (score 17)
            { 3, 37 }, // military-drum: Side Stick -> Snare Cross Stick (score 15)
        }},
        { "ocean-drum", {
            { 506, 48 }, // ocean-drum: Ocean Sound -> Agogo Press Sound (score 10)
        }},
        { "okedo-daiko", {
            { 428, 36 }, // okedo-daiko: Okedo-Daiko -> Okedo (score 12)
        }},
        { "opera-gong", {
            { 135, 52 }, // opera-gong: Opera Gong -> Large Gong (score 10)
        }},
        { "percussion", {
            { 1, 38 }, // percussion: Snare -> Snare Drum (score 17)
            { 6, 56 }, // percussion: Cowbell -> Cowbell (score 105)
            { 7, 54 }, // percussion: Tambourine -> Tambourine (score 105)
            { 8, 80 }, // percussion: Mute Triangle -> Triangle Mute (score 25)
            { 9, 81 }, // percussion: Triangle -> Triangle Open (score 17)
            { 10, 76 }, // percussion: Hi Woodblock -> High Woodblock (score 15)
            { 11, 77 }, // percussion: Lo Woodblock -> Low Woodblock (score 15)
            { 12, 58 }, // percussion: Vibraslap -> Vibra Slap (score 105)
            { 13, 36 }, // percussion: Bass Drum -> Bass Drum (score 100)
            { 14, 29 }, // percussion: Open Hi-Hat -> Hi-Hat Open (score 30)
            { 15, 27 }, // percussion: Closed Hi-Hat -> Hi-Hat Closed (score 30)
            { 18, 30 }, // percussion: Ride Cymbal -> Ride Cymbal (score 100)
            { 21, 55 }, // percussion: Splash Cymbal -> Splash Cymbal (score 105)
            { 32, 60 }, // percussion: Hi Bongo -> High Bongo (score 15)
            { 33, 61 }, // percussion: Lo Bongo -> Low Bongo (score 15)
            { 34, 69 }, // percussion: Cabasa -> Cabasa (score 105)
            { 35, 75 }, // percussion: Claves -> Claves (score 105)
            { 36, 63 }, // percussion: Hi Conga -> Conga (score 17)
            { 37, 62 }, // percussion: Mute Hi Conga -> Conga Dead Stroke (score 15)
            { 39, 78 }, // percussion: Mute Cuica -> Cuica High (score 15)
            { 40, 79 }, // percussion: Open Cuica -> Cuica Low (score 15)
            { 41, 74 }, // percussion: Long Giro -> Guiro Long (score 15)
            { 42, 73 }, // percussion: Short Giro -> Guiro Short (score 15)
            { 43, 70 }, // percussion: Maracas -> Maracas (score 105)
            { 44, 65 }, // percussion: Hi Timbale -> High Timbale (score 15)
            { 45, 66 }, // percussion: Lo Timbale -> Low Timbale (score 15)
            { 46, 71 }, // percussion: Long Whistle -> Whistle Long (score 20)
            { 48, 86 }, // percussion: Mute Surdo -> Cowbell Mute (score 10)
            { 59, 82 }, // percussion: Shaker -> Shaker (score 105)
            { 74, 83 }, // percussion: Sleigh Bells -> Sleigh Bells (score 105)
            { 88, 57 }, // percussion: Suspended Cymbal -> Suspended Cymbal Roll (score 22)
            { 164, 59 }, // percussion: Hand Cymbals -> Crash Cymbals Crash (score 10)
            { 225, 72 }, // percussion: Short Whistle -> Police Whistle Short (score 20)
            { 249, 37 }, // percussion: Snare Rim -> Snare Rim (score 100)
            { 256, 31 }, // percussion: Stick Click -> Stick Click (score 105)
            { 362, 85 }, // percussion: Castanets -> Castanets (score 105)
            { 511, 84 }, // percussion: Mark Tree -> Mark Tree (score 100)
            { 524, 67 }, // percussion: Hi Agog -> Flexatone Hi (score 10)
        }},
        { "percussion-synthesizer", {
            { 1, 38 }, // percussion-synthesizer: Acoustic Snare -> Snare Drum (score 15)
            { 3, 37 }, // percussion-synthesizer: Side Stick -> Snare Cross Stick (score 15)
            { 4, 49 }, // percussion-synthesizer: Crash Cymbal 1 -> Crash Cymbal (score 27)
            { 5, 57 }, // percussion-synthesizer: Crash Cymbal 2 -> Crash Cymbal 2 (score 105)
            { 6, 56 }, // percussion-synthesizer: Cowbell -> Cowbell (score 105)
            { 7, 54 }, // percussion-synthesizer: Tambourine -> Tambourine (score 105)
            { 8, 80 }, // percussion-synthesizer: Mute Triangle -> Triangle Mute (score 25)
            { 9, 81 }, // percussion-synthesizer: Open Triangle -> Triangle Open (score 25)
            { 10, 76 }, // percussion-synthesizer: High Woodblock -> High Woodblock (score 105)
            { 11, 77 }, // percussion-synthesizer: Low Woodblock -> Low Woodblock (score 105)
            { 12, 58 }, // percussion-synthesizer: Vibraslap -> Vibra Slap (score 105)
            { 13, 35 }, // percussion-synthesizer: Acoustic Bass Drum -> Bass Drum (score 27)
            { 14, 46 }, // percussion-synthesizer: Open Hi-hat -> Hi-Hat Open (score 35)
            { 15, 42 }, // percussion-synthesizer: Closed Hi-hat -> Hi-Hat Closed (score 35)
            { 16, 44 }, // percussion-synthesizer: Pedal Hi-hat -> Hi-Hat Foot (score 25)
            { 17, 52 }, // percussion-synthesizer: China Cymbal -> China Cymbal (score 105)
            { 18, 51 }, // percussion-synthesizer: Ride Cymbal 1 -> Ride Cymbal (score 27)
            { 19, 59 }, // percussion-synthesizer: Ride Cymbal 2 -> Ride Cymbal 2 (score 105)
            { 20, 53 }, // percussion-synthesizer: Ride Bell -> Ride Bell (score 105)
            { 21, 55 }, // percussion-synthesizer: Splash Cymbal -> Splash Cymbal (score 105)
            { 22, 39 }, // percussion-synthesizer: Hand Clap -> Hand Clap (score 105)
            { 23, 40 }, // percussion-synthesizer: Electric Snare -> Electric Snare Drum (score 27)
            { 24, 50 }, // percussion-synthesizer: High Tom -> High Tom (score 105)
            { 25, 48 }, // percussion-synthesizer: Hi-Mid Tom -> High-Mid Tom (score 25)
            { 26, 47 }, // percussion-synthesizer: Low-Mid Tom -> Low-Mid Tom (score 105)
            { 27, 45 }, // percussion-synthesizer: Low Tom -> Low Tom (score 105)
            { 28, 43 }, // percussion-synthesizer: High Floor Tom -> Floor Tom 1 (score 25)
            { 29, 41 }, // percussion-synthesizer: Low Floor Tom -> Floor Tom 2 (score 25)
            { 30, 67 }, // percussion-synthesizer: High Agog -> High Agogo (score 17)
            { 31, 68 }, // percussion-synthesizer: Low Agog -> Low Agogo (score 17)
            { 32, 60 }, // percussion-synthesizer: High Bongo -> High Bongo (score 105)
            { 33, 61 }, // percussion-synthesizer: Low Bongo -> Low Bongo (score 105)
            { 34, 69 }, // percussion-synthesizer: Cabasa -> Cabasa (score 105)
            { 35, 75 }, // percussion-synthesizer: Claves -> Claves (score 105)
            { 36, 63 }, // percussion-synthesizer: Open High Conga -> Conga (score 17)
            { 39, 78 }, // percussion-synthesizer: Mute Cuica -> Cuica High (score 15)
            { 40, 79 }, // percussion-synthesizer: Open Cuica -> Cuica Low (score 15)
            { 41, 74 }, // percussion-synthesizer: Long Giro -> Guiro Long (score 15)
            { 42, 73 }, // percussion-synthesizer: Short Giro -> Guiro Short (score 15)
            { 43, 70 }, // percussion-synthesizer: Maracas -> Maracas (score 105)
            { 44, 65 }, // percussion-synthesizer: High Timbale -> High Timbale (score 105)
            { 45, 66 }, // percussion-synthesizer: Low Timbale -> Low Timbale (score 105)
            { 46, 72 }, // percussion-synthesizer: Long Whistle -> Whistle Long (score 25)
            { 47, 71 }, // percussion-synthesizer: Short Whistle -> Whistle Short (score 25)
            { 48, 86 }, // percussion-synthesizer: Mute Surdo -> Cowbell Mute (score 10)
            { 59, 82 }, // percussion-synthesizer: Shaker -> Shaker (score 105)
            { 70, 84 }, // percussion-synthesizer: Belltree -> Bell Tree (score 105)
            { 261, 31 }, // percussion-synthesizer: Sticks -> Snare Section Cross Sticks (score 12)
            { 332, 62 }, // percussion-synthesizer: Mute High Conga -> High Agogo Mute (score 20)
            { 362, 85 }, // percussion-synthesizer: Castanets -> Castanets (score 105)
            { 461, 33 }, // percussion-synthesizer: Metronome Click -> Metronome Click (score 100)
            { 462, 34 }, // percussion-synthesizer: Metronome Bell -> Metronome Bell (score 105)
            { 463, 29 }, // percussion-synthesizer: Scratch Push -> Scratch Push (score 105)
            { 464, 30 }, // percussion-synthesizer: Scratch Pull -> Scratch Pull (score 105)
        }},
        { "piccolo-snare-drum", {
            { 3, 37 }, // piccolo-snare-drum: Side Stick -> Snare Cross Stick (score 15)
            { 23, 40 }, // piccolo-snare-drum: Snare -> Electric Snare Drum (score 17)
        }},
        { "ratchet", {
            { 71, 73 }, // ratchet: Ratchet -> Ratchet (score 100)
        }},
        { "ride-cymbal", {
            { 18, 51 }, // ride-cymbal: Ride Cymbal -> Ride Cymbal (score 105)
            { 20, 53 }, // ride-cymbal: Ride Bell -> Ride Bell (score 105)
        }},
        { "shaker", {
            { 59, 82 }, // shaker: Shaker -> Shaker (score 105)
        }},
        { "shekere", {
            { 65, 82 }, // shekere: Shake -> Egg Shaker Multi Shake (score 12)
        }},
        { "shell-wind-chimes", {
            { 460, 69 }, // shell-wind-chimes: Shell Wind Chimes -> Wind Chimes (score 22)
        }},
        { "shime-daiko", {
            { 426, 63 }, // shime-daiko: Shime-Daiko -> Shime (score 12)
        }},
        { "slap", {
            { 12, 28 }, // slap: Slap -> Vibra Slap (score 12)
        }},
        { "sleigh-bells", {
            { 74, 83 }, // sleigh-bells: Sleigh Bell -> Sleigh Bells (score 17)
        }},
        { "slit-drum", {
            { 2, 77 }, // slit-drum: Slit Drum -> Kick Drum (score 10)
        }},
        { "snare-drum", {
            { 1, 38 }, // snare-drum: Snare -> Snare Drum (score 17)
            { 3, 37 }, // snare-drum: Side Stick -> Snare Cross Stick (score 15)
        }},
        { "splash-cymbal", {
            { 21, 55 }, // splash-cymbal: Splash Cymbal -> Splash Cymbal (score 105)
        }},
        { "tablas", {
            { 418, 62 }, // tablas: High Tabla -> Tabla High (score 20)
            { 419, 64 }, // tablas: Low Tabla -> Tabla Low (score 20)
        }},
        { "taiko", {
            { 48, 86 }, // taiko: Taiko Mute -> Cowbell Mute (score 10)
        }},
        { "tam-tam", {
            { 149, 52 }, // tam-tam: Tam-tam -> Tam-Tam (score 100)
        }},
        { "tambourine", {
            { 7, 54 }, // tambourine: Tambourine -> Tambourine (score 105)
        }},
        { "temple-blocks", {
            { 459, 58 }, // temple-blocks: Low Temple Block -> Temple Block Low (score 30)
            { 518, 59 }, // temple-blocks: Low-Mid Temple Block -> Temple Block Low-Mid (score 40)
            { 520, 61 }, // temple-blocks: High-Mid Temple Block -> Temple Block High-Mid (score 40)
        }},
        { "timbales", {
            { 44, 65 }, // timbales: High Timbale -> High Timbale (score 105)
            { 45, 66 }, // timbales: Low Timbale -> Low Timbale (score 105)
        }},
        { "tom-toms", {
            { 26, 47 }, // tom-toms: Tom 3 -> Low-Mid Tom (score 15)
            { 27, 45 }, // tom-toms: Tom 4 -> Low Tom (score 15)
            { 28, 50 }, // tom-toms: Tom 1 -> Floor Tom 1 (score 22)
            { 29, 48 }, // tom-toms: Tom 2 -> Floor Tom 2 (score 22)
        }},
        { "triangle", {
            { 8, 80 }, // triangle: Mute Triangle -> Triangle Mute (score 25)
            { 9, 81 }, // triangle: Open Triangle -> Triangle Open (score 25)
        }},
        { "tubo", {
            { 36, 63 }, // tubo: High Conga -> Conga (score 17)
            { 332, 62 }, // tubo: Mute High Conga -> High Agogo Mute (score 20)
        }},
        { "vibraslap", {
            { 12, 58 }, // vibraslap: Vibraslap -> Vibra Slap (score 105)
        }},
        { "whip", {
            { 76, 52 }, // whip: Whip -> Whip (score 100)
        }},
        { "wind-gong", {
            { 148, 52 }, // wind-gong: Wind Gong -> Wind Gong (score 100)
        }},
        { "wood-blocks", {
            { 10, 76 }, // wood-blocks: High Wood Block -> High Woodblock (score 105)
            { 11, 77 }, // wood-blocks: Low Wood Block -> Low Woodblock (score 105)
        }},
        { "wooden-wind-chimes", {
            { 460, 69 }, // wooden-wind-chimes: Wooden Wind Chimes -> Wind Chimes (score 22)
        }},
    };


    // Fallback generic mappings (independent of instrument id)
    static const std::unordered_map<int, int> noteFallback = {
        { 1, 38 }, // snare-drum: Snare -> Snare Drum (score 17)
        { 2, 36 }, // marching-tenor-drums: Drum 4 -> Kick Drum (score 15)
        { 3, 37 }, // drum-kit-4: Cross-stick -> Snare Cross Stick (score 27)
        { 4, 49 }, // drum-kit-4: Crash Cymbal -> Crash Cymbal (score 105)
        { 5, 57 }, // drumset: Crash Cymbal 2 -> Crash Cymbal 2 (score 105)
        { 6, 56 }, // drumset: Cowbell -> Cowbell (score 105)
        { 7, 54 }, // drumset: Tambourine -> Tambourine (score 105)
        { 8, 80 }, // triangle: Mute Triangle -> Triangle Mute (score 25)
        { 9, 81 }, // triangle: Open Triangle -> Triangle Open (score 25)
        { 10, 76 }, // wood-blocks: High Wood Block -> High Woodblock (score 105)
        { 11, 77 }, // wood-blocks: Low Wood Block -> Low Woodblock (score 105)
        { 12, 58 }, // vibraslap: Vibraslap -> Vibra Slap (score 105)
        { 13, 36 }, // drum-kit-4: Bass Drum -> Bass Drum (score 100)
        { 14, 46 }, // drumset: Open Hi-Hat -> Hi-Hat Open (score 35)
        { 15, 36 }, // marching-snareline: Hi-Hat (closed) -> Hi-Hat Closed (score 100)
        { 16, 44 }, // drumset: Pedal Hi-Hat -> Hi-Hat Foot (score 25)
        { 17, 52 }, // drumset: China Cymbal -> China Cymbal (score 105)
        { 18, 51 }, // drum-kit-4: Ride Cymbal -> Ride Cymbal (score 105)
        { 19, 59 }, // drumset: Ride Cymbal 2 -> Ride Cymbal 2 (score 105)
        { 20, 53 }, // drumset: Ride Bell -> Ride Bell (score 105)
        { 21, 55 }, // drumset: Splash Cymbal -> Splash Cymbal (score 105)
        { 22, 39 }, // percussion-synthesizer: Hand Clap -> Hand Clap (score 105)
        { 23, 40 }, // drumset: Electric Snare -> Electric Snare Drum (score 27)
        { 24, 50 }, // drumset: High Tom -> High Tom (score 105)
        { 25, 48 }, // drumset: Hi-Mid Tom -> High-Mid Tom (score 25)
        { 26, 47 }, // drumset: Low-Mid Tom -> Low-Mid Tom (score 105)
        { 27, 45 }, // drumset: Low Tom -> Low Tom (score 105)
        { 28, 43 }, // drumset: High Floor Tom -> Floor Tom 1 (score 25)
        { 29, 41 }, // drum-kit-4: Floor Tom -> Floor Tom 2 (score 27)
        { 30, 67 }, // agogo-bells: High Agog -> High Agogo (score 17)
        { 31, 29 }, // marching-snareline: Agogo Low -> Low Agogo (score 20)
        { 32, 60 }, // bongos: High Bongo -> High Bongo (score 105)
        { 33, 61 }, // bongos: Low Bongo -> Low Bongo (score 105)
        { 34, 69 }, // cabasa: Cabasa -> Cabasa (score 105)
        { 35, 75 }, // claves: Claves -> Claves (score 105)
        { 36, 63 }, // congas: High Conga -> Conga (score 17)
        { 37, 62 }, // percussion: Mute Hi Conga -> Conga Dead Stroke (score 15)
        { 39, 78 }, // cuica: Mute Cuica -> Cuica High (score 15)
        { 40, 79 }, // cuica: Open Cuica -> Cuica Low (score 15)
        { 41, 74 }, // guiro: Long Giro -> Guiro Long (score 15)
        { 42, 73 }, // guiro: Short Giro -> Guiro Short (score 15)
        { 43, 70 }, // maracas: Maracas -> Maracas (score 105)
        { 44, 65 }, // timbales: High Timbale -> High Timbale (score 105)
        { 45, 66 }, // timbales: Low Timbale -> Low Timbale (score 105)
        { 46, 72 }, // percussion-synthesizer: Long Whistle -> Whistle Long (score 25)
        { 47, 71 }, // percussion-synthesizer: Short Whistle -> Whistle Short (score 25)
        { 48, 86 }, // taiko: Taiko Mute -> Cowbell Mute (score 10)
        { 49, 93 }, // marching-cymbals: Roll -> Cowbell Roll (score 12)
        { 59, 82 }, // shaker: Shaker -> Shaker (score 105)
        { 65, 82 }, // shekere: Shake -> Egg Shaker Multi Shake (score 12)
        { 68, 31 }, // marching-snareline: Jam Block -> High Jam Block (score 22)
        { 70, 84 }, // percussion-synthesizer: Belltree -> Bell Tree (score 105)
        { 71, 73 }, // ratchet: Ratchet -> Ratchet (score 100)
        { 74, 83 }, // percussion: Sleigh Bells -> Sleigh Bells (score 105)
        { 76, 52 }, // whip: Whip -> Whip (score 100)
        { 78, 37 }, // marching-tenor-drums: Drum 4 Rim -> Bass Drum Rim (score 20)
        { 82, 90 }, // marching-bass-drums: Unison -> Bass Drum Unison Hits (score 12)
        { 88, 57 }, // crash-cymbal: Suspended Cymbal -> Suspended Cymbal Roll (score 22)
        { 90, 45 }, // marching-tenorline: Mallet Click -> Suspended Cymbal w/ Mallet (Crash) (score 10)
        { 105, 57 }, // marching-snare: Stick Shot -> Suspended Cymbal Fat Choke w/ Stick (score 10)
        { 117, 44 }, // marching-tenorline: Stand Hit -> Hi-Hat Stand (score 10)
        { 121, 47 }, // marching-cymballine: Sizzle Cymbal 3 -> Sizzle Cymbal (score 22)
        { 135, 52 }, // opera-gong: Opera Gong -> Large Gong (score 10)
        { 148, 52 }, // wind-gong: Wind Gong -> Wind Gong (score 100)
        { 149, 52 }, // tam-tam: Tam-tam -> Tam-Tam (score 100)
        { 161, 81 }, // bell-plate: Bell Plate -> Bell Plate (score 100)
        { 164, 57 }, // cymbal: Hand Cymbals -> Crash Cymbals Crash (score 10)
        { 171, 91 }, // marching-cymbals: Zing -> Crash Cymbals Fast Zing (score 12)
        { 175, 77 }, // marching-snareline: Sticks In -> Crash Cymbals Scratch In (score 10)
        { 179, 83 }, // marching-cymbals: Tap-Choke -> Crash Cymbals Tap Choke (score 22)
        { 183, 50 }, // bird-call: Bird Call -> Crash Cymbals Whale Call (score 10)
        { 187, 79 }, // marching-cymbals: Crash-Choke -> Cymbal Section Crash Choke Fat (score 22)
        { 189, 45 }, // marching-cymballine: Crunch Cymbal 3 -> Cymbal Section Crunch Choke (score 20)
        { 190, 50 }, // marching-cymballine: Ding Cymbal 3 -> Cymbal Section Ding (score 20)
        { 191, 51 }, // marching-cymballine: Zing Cymbal 3 -> Cymbal Section Fast Zing (score 20)
        { 201, 43 }, // marching-cymballine: Tap Cymbal 3 -> Cymbal Section Tap Choke (score 20)
        { 206, 81 }, // finger-cymbals: Finger Cymbals -> Finger Cymbals (score 100)
        { 216, 68 }, // anvil: Anvil -> Anvil (score 100)
        { 218, 68 }, // automobile-brake-drums: Brake Drum -> Brake Drum (score 100)
        { 219, 60 }, // cannon: Cannon -> Cannon (score 100)
        { 225, 72 }, // percussion: Short Whistle -> Police Whistle Short (score 20)
        { 234, 75 }, // finger-snap: Finger Snap -> Finger Snaps (score 12)
        { 239, 48 }, // marching-snare: Buzz -> Snare Buzz Roll (score 12)
        { 240, 64 }, // marching-snareline: Ping Shot -> Snare Cross Shot (score 10)
        { 249, 37 }, // percussion: Snare Rim -> Snare Rim (score 100)
        { 251, 52 }, // marching-snare: Rim Shot -> Snare Rim Shot (score 22)
        { 256, 31 }, // percussion: Stick Click -> Stick Click (score 105)
        { 261, 31 }, // percussion-synthesizer: Sticks -> Snare Section Cross Sticks (score 12)
        { 273, 72 }, // marching-snareline: Rods -> Snare Section Rods (score 12)
        { 325, 38 }, // marching-tenor-drums: Drum 4 Buzz -> Spock Drum Buzz Roll (score 20)
        { 328, 72 }, // marching-tenorline: Rim Shot Drum 4 -> Spock Drum Rim Shot (score 30)
        { 332, 62 }, // congas: Mute High Conga -> High Agogo Mute (score 20)
        { 354, 81 }, // bowl-gongs: Bowl Gong -> Bowl (score 12)
        { 362, 85 }, // castanets: Castanets -> Castanets (score 105)
        { 392, 45 }, // frame-drum: Frame Drum -> Frame Drum (score 100)
        { 418, 62 }, // tablas: High Tabla -> Tabla High (score 20)
        { 419, 64 }, // tablas: Low Tabla -> Tabla Low (score 20)
        { 426, 63 }, // shime-daiko: Shime-Daiko -> Shime (score 12)
        { 428, 36 }, // okedo-daiko: Okedo-Daiko -> Okedo (score 12)
        { 459, 58 }, // temple-blocks: Low Temple Block -> Temple Block Low (score 30)
        { 460, 84 }, // metal-wind-chimes: Metal Wind Chimes -> Wind Chimes (score 22)
        { 461, 33 }, // percussion-synthesizer: Metronome Click -> Metronome Click (score 100)
        { 462, 34 }, // percussion-synthesizer: Metronome Bell -> Metronome Bell (score 105)
        { 463, 29 }, // percussion-synthesizer: Scratch Push -> Scratch Push (score 105)
        { 464, 30 }, // percussion-synthesizer: Scratch Pull -> Scratch Pull (score 105)
        { 473, 84 }, // bell-tree: Glissando Down -> Guiro Down Up (score 10)
        { 506, 48 }, // ocean-drum: Ocean Sound -> Agogo Press Sound (score 10)
        { 511, 84 }, // percussion: Mark Tree -> Mark Tree (score 100)
        { 518, 59 }, // temple-blocks: Low-Mid Temple Block -> Temple Block Low-Mid (score 40)
        { 520, 61 }, // temple-blocks: High-Mid Temple Block -> Temple Block High-Mid (score 40)
        { 522, 58 }, // log-drum: Low Sound -> Flexatone Low (score 10)
        { 523, 60 }, // log-drum: Mid Sound -> Flexatone Mid (score 10)
        { 524, 67 }, // percussion: Hi Agog -> Flexatone Hi (score 10)
    };



    if (!instrumentKey.empty()) {
        if (const auto instIt = instrumentToMidi.find(instrumentKey); instIt != instrumentToMidi.end()) {
            if (const auto noteIt = instIt->second.find(noteTypeId); noteIt != instIt->second.end()) {
                return noteIt->second;
            }
        }
    }

    if (const auto fallbackIt = noteFallback.find(noteTypeId); fallbackIt != noteFallback.end()) {
        return fallbackIt->second;
    }

    const auto& noteType = percussion::getPercussionNoteTypeFromId(noteTypeId);
    return pitchIsValid(noteType.generalMidi) ? noteType.generalMidi : INVALID_PITCH;
}

ID createPartId(int partNumber)
{
    return "P" + std::to_string(partNumber);
}

ID createStaffId(musx::dom::StaffCmper staffId)
{
    return std::to_string(staffId);
}

int createFinaleVoiceId(musx::dom::LayerIndex layerIndex, bool forV2)
{
    return (layerIndex * 2 + int(forV2));
}

DurationType noteTypeToDurationType(musx::dom::NoteType noteType)
{
    static const std::unordered_map<musx::dom::NoteType, DurationType> noteTypeTable = {
        { musx::dom::NoteType::Maxima,     DurationType::V_INVALID },
        { musx::dom::NoteType::Longa,      DurationType::V_LONG },
        { musx::dom::NoteType::Breve,      DurationType::V_BREVE },
        { musx::dom::NoteType::Whole,      DurationType::V_WHOLE },
        { musx::dom::NoteType::Half,       DurationType::V_HALF },
        { musx::dom::NoteType::Quarter,    DurationType::V_QUARTER },
        { musx::dom::NoteType::Eighth,     DurationType::V_EIGHTH },
        { musx::dom::NoteType::Note16th,   DurationType::V_16TH },
        { musx::dom::NoteType::Note32nd,   DurationType::V_32ND },
        { musx::dom::NoteType::Note64th,   DurationType::V_64TH },
        { musx::dom::NoteType::Note128th,  DurationType::V_128TH },
        { musx::dom::NoteType::Note256th,  DurationType::V_256TH },
        { musx::dom::NoteType::Note512th,  DurationType::V_512TH },
        { musx::dom::NoteType::Note1024th, DurationType::V_1024TH },
        { musx::dom::NoteType::Note2048th, DurationType::V_INVALID },
        { musx::dom::NoteType::Note4096th, DurationType::V_INVALID },
    };
    return muse::value(noteTypeTable, noteType, DurationType::V_INVALID);
}

TDuration musxDurationInfoToDuration(std::pair<musx::dom::NoteType, unsigned> noteInfo)
{
    TDuration d = noteTypeToDurationType(noteInfo.first);
    int ndots = static_cast<int>(noteInfo.second);
    if (d.isValid() && ndots <= MAX_DOTS) {
        d.setDots(ndots);
        return d;
    }
    return TDuration(DurationType::V_INVALID);
}

engraving::NoteType durationTypeToNoteType(DurationType type, bool after)
{
    if (int(type) < int(DurationType::V_EIGHTH)) {
        return after ? engraving::NoteType::GRACE4 : engraving::NoteType::GRACE8_AFTER;
    }
    if (int(type) >= int(DurationType::V_32ND)) {
        return after ? engraving::NoteType::GRACE32_AFTER : engraving::NoteType::GRACE32;
    }
    if (type == DurationType::V_16TH) {
        return after ? engraving::NoteType::GRACE16_AFTER : engraving::NoteType::GRACE16;
    }
    return after ? engraving::NoteType::GRACE8_AFTER : engraving::NoteType::APPOGGIATURA;
}

String instrTemplateIdfromUuid(std::string uuid)
{
    // keep in sync with 'id' property of https://docs.google.com/spreadsheets/d/1SwqZb8lq5rfv5regPSA10drWjUAoi65EuMoYtG-4k5s/edit
    // todo: Add (sensible) defaults: woodwinds-end
    // todo: Detect midi program
    static const std::unordered_map<std::string_view, String> uuidTable = {
        // General
        { uuid::BlankStaff,                u"piano" }, // 'sensible' different default
        { uuid::GrandStaff,                u"piano" }, //
        { uuid::Unknown,                   u"piano" }, //

        // Strings
        { uuid::Violin,                    u"violin" },
        { uuid::Viola,                     u"viola" },
        { uuid::Cello,                     u"violoncello" },
        { uuid::DoubleBass,                u"contrabass" },
        { uuid::ViolinSection,             u"violins" },
        { uuid::ViolaSection,              u"violas" },
        { uuid::CelloSection,              u"violoncellos" },
        { uuid::VioloncelloSection,        u"violoncellos" },
        { uuid::DoubleBassSection,         u"contrabasses" },
        { uuid::ContrabassSection,         u"contrabasses" },
        { uuid::StringEnsemble,            u"strings" },
        { uuid::ViolaDAmore,               u"violoncello" }, //
        { uuid::Ajaeng,                    u"erhu" }, //
        { uuid::Arpeggione,                u"viola-da-gamba" }, //
        { uuid::Baryton,                   u"baryton" },
        { uuid::ByzantineLyra,             u"violin" }, //
        { uuid::CretanLyra,                u"violin" }, //
        { uuid::Crwth,                     u"violoncello" }, //
        { uuid::Dahu,                      u"erhu" }, //
        { uuid::Dangao,                    u"violin" }, //
        { uuid::Dihu,                      u"erhu" }, //
        { uuid::Erhu,                      u"erhu" },
        { uuid::Erxian,                    u"erhu" }, //
        { uuid::Fiddle,                    u"violin" }, //
        { uuid::Gaohu,                     u"erhu" }, //
        { uuid::Gehu,                      u"violoncello" }, //
        { uuid::Haegeum,                   u"erhu" }, //
        { uuid::HardangerFiddle,           u"violin" }, //
        { uuid::HurdyGurdy,                u"violin" }, //
        { uuid::Igil,                      u"violin" }, //
        { uuid::Kamancha,                  u"violin" }, //
        { uuid::Kokyu,                     u"violin" }, //
        { uuid::Kora,                      u"lute" }, //
        { uuid::LaruAn,                    u"violoncello" }, //
        { uuid::Leiqin,                    u"erhu" }, //
        { uuid::Lirone,                    u"viola-da-gamba" }, //
        { uuid::MorinKhuur,                u"violin" }, //
        { uuid::Nyckelharpa,               u"nyckelharpa" },
        { uuid::Octobass,                  u"octobass" },
        { uuid::Rebab,                     u"violin" }, //
        { uuid::Rebec,                     u"viola-da-gamba" }, //
        { uuid::Sarangi,                   u"violin" }, //
        { uuid::SarangiDrone,              u"violin" }, //
        { uuid::StrohViolin,               u"violin" }, //
        { uuid::Trombamarina,              u"violoncello" }, //
        { uuid::Vielle,                    u"viola" }, //
        { uuid::Viol,                      u"viola-da-gamba" }, //
        { uuid::ViolaDaGamba,              u"viola-da-gamba" },
        { uuid::ViolinoPiccolo,            u"violin" }, //
        { uuid::VioloncelloPiccolo,        u"violoncello" }, //
        { uuid::Violotta,                  u"violoncello" }, //
        { uuid::Zhonghu,                   u"erhu" }, //

        // Keyboards
        { uuid::Piano,                     u"piano" },
        { uuid::PianoNoName,               u"piano" },
        { uuid::Harpsichord,               u"harpsichord" },
        { uuid::Organ,                     u"organ" },
        { uuid::Organ2Staff,               u"organ" },
        { uuid::Celesta,                   u"celesta" },
        { uuid::Accordion,                 u"accordion" },
        { uuid::Melodica,                  u"melodica" },
        { uuid::ElectricPiano,             u"electric-piano" },
        { uuid::Clavinet,                  u"clavinet" },
        { uuid::SynthPad,                  u"pad-synth" },
        { uuid::SynthLead,                 u"saw-synth" }, //
        { uuid::SynthBrass,                u"brass-synthesizer" },
        { uuid::SynthSoundtrack,           u"soundtrack-synth" },
        { uuid::SoundFX,                   u"piano" }, //
        { uuid::Harmonium,                 u"harmonium" },
        { uuid::OndesMartenot,             u"ondes-martenot" },
        { uuid::Theremin,                  u"theremin" },
        { uuid::Virginal,                  u"virginal" },
        { uuid::Clavichord,                u"clavichord" },

        // Voices
        { uuid::SopranoVoice,              u"soprano" }, // todo, account for u"soprano-c-clef", same for alt-baritone and mezzo-soprano
        { uuid::AltoVoice,                 u"alto" },
        { uuid::TenorVoice,                u"tenor" },
        { uuid::BaritoneVoice,             u"baritone" },
        { uuid::BassVoice,                 u"bass" },
        { uuid::Vocals,                    u"voice" }, //
        { uuid::Voice,                     u"voice" },
        { uuid::VoiceNoName,               u"voice" },
        { uuid::MezzoSopranoVoice,         u"mezzo-soprano" },
        { uuid::ContraltoVoice,            u"contralto" },
        { uuid::CountertenorVoice,         u"countertenor" },
        { uuid::BassBaritoneVoice,         u"bass" }, //
        { uuid::ChoirAahs,                 u"voice" }, //
        { uuid::ChoirOohs,                 u"voice" }, //
        { uuid::Yodel,                     u"voice" }, //
        { uuid::Beatbox,                   u"voice" }, //
        { uuid::Kazoo,                     u"kazoo" },
        { uuid::Talkbox,                   u"voice" },
        { uuid::VocalPercussion,           u"voice" }, //

        // Woodwinds
        { uuid::Piccolo,                   u"piccolo" },
        { uuid::Flute,                     u"flute" },
        { uuid::AltoFlute,                 u"alto-flute" },
        { uuid::Oboe,                      u"oboe" },
        { uuid::OboeDAmore,                u"oboe-d'amore" },
        { uuid::EnglishHorn,               u"english-horn" },
        { uuid::ClarinetBFlat,             u"bb-clarinet" },
        { uuid::ClarinetA,                 u"a-clarinet" },
        { uuid::ClarinetEFlat,             u"eb-clarinet" },
        { uuid::AltoClarinet,              u"alto-clarinet" },
        { uuid::ContraltoClarinet,         u"contra-alto-clarinet" },
        { uuid::BassClarinet,              u"bass-clarinet" },
        { uuid::ContrabassClarinet,        u"contrabass-clarinet" },
        { uuid::Bassoon,                   u"bassoon" },
        { uuid::Contrabassoon,             u"contrabassoon" },
        { uuid::WindSection,               u"winds" },
        { uuid::SopranoSax,                u"soprano-saxophone" },
        { uuid::AltoSax,                   u"alto-saxophone" },
        { uuid::TenorSax,                  u"tenor-saxophone" },
        { uuid::BaritoneSax,               u"baritone-saxophone" },
        { uuid::SopranoRecorder,           u"soprano-recorder" },
        { uuid::SopraninoRecorder,         u"sopranino-recorder" },
        { uuid::AltoRecorder,              u"alto-recorder" },
        { uuid::TenorRecorder,             u"tenor-recorder" },
        { uuid::BassRecorder,              u"bass-recorder" },
        { uuid::DescantRecorder,           u"soprano-recorder" }, //
        { uuid::Ocarina,                   u"ocarina" },
        { uuid::PennyWhistle,              u"c-tin-whistle" }, //
        { uuid::PennyWhistleD,             u"d-tin-whistle" }, //
        { uuid::PennyWhistleG,             u"c-tin-whistle" }, //
        { uuid::LowIrishWhistle,           u"c-tin-whistle" }, //
        { uuid::TinWhistleBFlat,           u"bflat-tin-whistle" },
        { uuid::Harmonica,                 u"harmonica" },
        { uuid::BassHarmonica,             u"bass-harmonica" },
        { uuid::Concertina,                u"concertina" },
        { uuid::Bandoneon,                 u"bandoneon" },
        { uuid::HornF_WWQuintet,           u"horn" }, //
        { uuid::Bagpipes,                  u"bagpipe" },
        { uuid::UilleannPipes,             u"bagpipe" }, //
        { uuid::GaidaPipes,                u"bagpipe" }, //
        { uuid::ContraAltoFlute,           u"contra-alto-flute" },
        { uuid::BassFlute,                 u"bass-flute" },
        { uuid::ContrabassFlute,           u"contrabass-flute" },
        { uuid::DoubleContrabassFlute,     u"double-contrabass-flute" },
        { uuid::HyperbassFlute,            u"hyperbass-flute" },
        { uuid::PanPipes,                  u"pan-flute" },
        { uuid::Fife,                      u"fife" },
        { uuid::BottleBlow,                u"flute" }, //
        { uuid::Jug,                       u"flute" }, //
        { uuid::PiccoloOboe,               u"piccolo-oboe" },
        { uuid::PiccoloHeckelphone,        u"piccolo-heckelphone" },
        { uuid::Heckelphone,               u"heckelphone" },
        { uuid::BassOboe,                  u"bass-oboe" },
        { uuid::BassetClarinet,            u"basset-clarinet" },
        { uuid::BassetHorn,                u"basset-horn" },
        { uuid::Hornpipe,                  u"english-horn" }, //
        { uuid::PiccoloClarinet,           u"piccolo-clarinet" },
        { uuid::Saxonette,                 u"c-clarinet" }, //
        { uuid::SopraninoSax,              u"sopranino-saxophone" },
        { uuid::MezzoSopranoSax,           u"mezzo-soprano-saxophone" },
        { uuid::Sopranino,                 u"sopranino-saxophone" }, //
        { uuid::CMelodySax,                u"melody-saxophone" },
        { uuid::Aulochrome,                u"aulochrome" },
        { uuid::Xaphoon,                   u"xaphoon" },
        { uuid::BassSax,                   u"bass-saxophone" },
        { uuid::ContrabassSax,             u"contrabass-saxophone" },
        { uuid::SubContrabassSax,          u"subcontrabass-saxophone" },
        { uuid::Tubax,                     u"subcontrabass-saxophone" },
        { uuid::Bansuri,                   u"flute" }, //
        { uuid::Danso,                     u"danso" },
        { uuid::Dizi,                      u"e-dizi" },
        { uuid::DilliKaval,                u"flute" }, //
        { uuid::Diple,                     u"flute" }, //
        { uuid::DoubleFlute,               u"flute" }, //
        { uuid::Dvojnice,                  u"flute" }, //
        { uuid::DvojniceDrone,             u"flute" }, //
        { uuid::Flageolet,                 u"flageolet" },
        { uuid::Fujara,                    u"contrabass-flute" }, //
        { uuid::Gemshorn,                  u"gemshorn" },
        { uuid::Hocchiku,                  u"shakuhachi" }, //
        { uuid::Hun,                       u"flute" }, //
        { uuid::IrishFlute,                u"irish-flute" },
        { uuid::Kaval,                     u"flute" }, //
        { uuid::Khlui,                     u"flute" }, //
        { uuid::KnotweedFlute,             u"flute" }, //
        { uuid::KoncovkaAltoFlute,         u"alto-flute" }, //
        { uuid::Koudi,                     u"flute" }, //
        { uuid::Ney,                       u"flute" }, //
        { uuid::Nohkan,                    u"flute" }, //
        { uuid::NoseFlute,                 u"flute" }, //
        { uuid::Palendag,                  u"flute" }, //
        { uuid::Quena,                     u"quena" },
        { uuid::Ryuteki,                   u"flute" }, //
        { uuid::Shakuhachi,                u"shakuhachi" },
        { uuid::ShepherdsPipe,             u"flute" },
        { uuid::Shinobue,                  u"flute" },
        { uuid::ShivaWhistle,              u"flute" },
        { uuid::Shvi,                      u"flute" },
        { uuid::Suling,                    u"flute" },
        { uuid::Tarka,                     u"flute" },
        { uuid::TenorOvertoneFlute,        u"flute" },
        { uuid::Tumpong,                   u"flute" },
        { uuid::Venu,                      u"flute" },
        { uuid::Xiao,                      u"flute" },
        { uuid::Xun,                       u"flute" },
        { uuid::Albogue,                   u"flute" },
        { uuid::Alboka,                    u"flute" },
        { uuid::AltoCrumhorn,              u"alto-crumhorn" },
        { uuid::Arghul,                    u"flute" },
        { uuid::Bawu,                      u"flute" },
        { uuid::Chalumeau,                 u"chalumeau" },
        { uuid::ClarinetteDAmour,          u"flute" },
        { uuid::Cornamuse,                 u"cornamuse" },
        { uuid::Diplica,                   u"flute" },
        { uuid::DoubleClarinet,            u"flute" },
        { uuid::HeckelClarina,             u"flute" },
        { uuid::HeckelphoneClarinet,       u"heckelphone-clarinet" },
        { uuid::Hirtenschalmei,            u"flute" },
        { uuid::Launeddas,                 u"flute" },
        { uuid::Maqrunah,                  u"flute" },
        { uuid::Mijwiz,                    u"flute" },
        { uuid::Octavin,                   u"octavin" },
        { uuid::Pibgorn,                   u"flute" },
        { uuid::Rauschpfeife,              u"rauschpfeife" },
        { uuid::Sipsi,                     u"flute" },
        { uuid::ModernTarogato,            u"flute" },
        { uuid::TenorCrumhorn,             u"tenor-crumhorn" },
        { uuid::Zhaleika,                  u"flute" },
        { uuid::Algaita,                   u"flute" },
        { uuid::Bifora,                    u"flute" },
        { uuid::Bombarde,                  u"flute" },
        { uuid::Cromorne,                  u"cromorne" },
        { uuid::Duduk,                     u"duduk" },
        { uuid::Dulcian,                   u"dulcian" },
        { uuid::Dulzaina,                  u"flute" },
        { uuid::Guan,                      u"flute" },
        { uuid::Guanzi,                    u"flute" },
        { uuid::Hichiriki,                 u"flute" },
        { uuid::Hne,                       u"flute" },
        { uuid::JogiBaja,                  u"flute" },
        { uuid::KenBau,                    u"flute" },
        { uuid::Mizmar,                    u"flute" },
        { uuid::Nadaswaram,                u"flute" },
        { uuid::OboeDaCaccia,              u"oboe-da-caccia" },
        { uuid::Pi,                        u"flute" },
        { uuid::Piri,                      u"flute" },
        { uuid::PungiSnakeCharmer,         u"flute" },
        { uuid::Rackett,                   u"rackett" },
        { uuid::ReedContrabass,            u"reed-contrabass" },
        { uuid::Rhaita,                    u"flute" },
        { uuid::Rothphone,                 u"flute" },
        { uuid::Sarrusophone,              u"sarrusophone" },
        { uuid::Shawm,                     u"flute" },
        { uuid::Shehnai,                   u"shenai" },
        { uuid::Sopila,                    u"flute" },
        { uuid::Sorna,                     u"flute" },
        { uuid::Sralai,                    u"flute" },
        { uuid::Suona,                     u"flute" },
        { uuid::Surnay,                    u"flute" },
        { uuid::Taepyeongso,               u"flute" },
        { uuid::AncientTarogato,           u"flute" },
        { uuid::TrompetaChina,             u"flute" },
        { uuid::Zurla,                     u"flute" },
        { uuid::Zurna,                     u"flute" },
        { uuid::KhaenMouthOrgan,           u"flute" },
        { uuid::Hulusi,                    u"flute" },
        { uuid::Sheng,                     u"sheng" },

        // Brass
        { uuid::TrumpetBFlat,              u"bb-trumpet" },
        { uuid::TrumpetC,                  u"c-trumpet" },
        { uuid::TrumpetD,                  u"d-trumpet" },
        { uuid::Cornet,                    u"bb-cornet" },
        { uuid::Flugelhorn,                u"flugelhorn" },
        { uuid::Mellophone,                u"mellophone" },
        { uuid::HornF,                     u"horn" },
        { uuid::Trombone,                  u"trombone" },
        { uuid::BassTrombone,              u"bass-trombone" },
        { uuid::Euphonium,                 u"euphonium" },
        { uuid::BaritoneBC,                u"baritone-horn" },
        { uuid::BaritoneTC,                u"baritone-horn-treble" },
        { uuid::Tuba,                      u"tuba" },
        { uuid::BassTuba,                  u"tuba" },
        { uuid::Sousaphone,                u"sousaphone" },
        { uuid::BrassSection,              u"brass" },
        { uuid::PiccoloTrumpetA,           u"a-piccolo-trumpet" },
        { uuid::Bugle,                     u"bugle" },
        { uuid::CornetEFlat,               u"eb-cornet" },
        { uuid::HornEFlat,                 u"eb-horn" },
        { uuid::AltoTrombone,              u"alto-trombone" },
        { uuid::TenorTrombone,             u"tenor-trombone" },
        { uuid::ContrabassTrombone,        u"contrabass-trombone" },
        { uuid::Alphorn,                   u"alphorn" },
        { uuid::AltoHorn,                  u"eb-alto-horn" },
        { uuid::Didgeridoo,                u"didgeridoo" },
        { uuid::PostHorn,                  u"posthorn" },
        { uuid::ViennaHorn,                u"vienna-horn" },
        { uuid::WagnerTuba,                u"wagner-tuba" },
        { uuid::BaroqueTrumpet,            u"baroque-trumpet" },
        { uuid::BassTrumpet,               u"bass-trumpet" },
        { uuid::Cornetto,                  u"cornett" },
        { uuid::Fiscorn,                   u"fiscorn" },
        { uuid::Kuhlohorn,                 u"kuhlohorn" },
        { uuid::PocketTrumpet,             u"pocket-trumpet" },
        { uuid::Saxhorn,                   u"saxhorn" },
        { uuid::SlideTrumpet,              u"slide-trumpet" },
        { uuid::Cimbasso,                  u"cimbasso" },
        { uuid::DoubleBellEuphonium,       u"euphonium" },
        { uuid::Sackbut,                   u"tenor-sackbut" },
        { uuid::Helicon,                   u"helicon" },
        { uuid::Ophicleide,                u"ophicleide" },
        { uuid::Serpent,                   u"serpent" },
        { uuid::SubContrabassTuba,         u"subcontrabass-tuba" },
        { uuid::ConchShell,                u"conch" },
        { uuid::Horagai,                   u"horagai" },
        { uuid::Shofar,                    u"shofar" },
        { uuid::Vuvuzela,                  u"vuvuzela" },

        // Plucked Strings
        { uuid::Harp,                      u"harp" },
        { uuid::TroubadorHarp,             u"guitar-steel" },
        { uuid::Guitar,                    u"guitar-steel" },
        { uuid::Guitar8vb,                 u"guitar-steel" },
        { uuid::AcousticGuitar,            u"guitar-steel" },
        { uuid::ClassicalGuitar,           u"guitar-nylon" },
        { uuid::ElectricGuitar,            u"electric-guitar" },
        { uuid::SteelGuitar,               u"pedal-steel-guitar" },
        { uuid::Banjo,                     u"banjo" },
        { uuid::TenorBanjo,                u"tenor-banjo" },
        { uuid::AcousticBass,              u"acoustic-bass" },
        { uuid::BassGuitar,                u"bass-guitar" },
        { uuid::ElectricBass,              u"electric-bass" },
        { uuid::FretlessBass,              u"fretless-electric-bass" },
        { uuid::StringBass,                u"double-bass" },
        { uuid::Mandolin,                  u"mandolin" },
        { uuid::Dulcimer,                  u"dulcimer" },
        { uuid::HammeredDulcimer,          u"guitar-steel" },
        { uuid::Dulcimer8vb,               u"guitar-steel" },
        { uuid::Autoharp,                  u"guitar-steel" },
        { uuid::Lute,                      u"lute" },
        { uuid::Ukulele,                   u"ukulele" },
        { uuid::TenorUkulele,              u"tenor-ukulele" },
        { uuid::Sitar,                     u"sitar" },
        { uuid::Zither,                    u"guitar-steel" },
        { uuid::Archlute,                  u"archlute-14-course" },
        { uuid::Baglama,                   u"guitar-steel" },
        { uuid::Balalaika,                 u"balalaika" },
        { uuid::Bandura,                   u"guitar-steel" },
        { uuid::Banjolele,                 u"guitar-steel" },
        { uuid::Barbat,                    u"guitar-steel" },
        { uuid::Begena,                    u"guitar-steel" },
        { uuid::Biwa,                      u"guitar-steel" },
        { uuid::Bolon,                     u"guitar-steel" },
        { uuid::Bordonua,                  u"guitar-steel" },
        { uuid::Bouzouki,                  u"bouzouki-3-course" },
        { uuid::BulgarianTambura,          u"guitar-steel" },
        { uuid::ChapmanStick,              u"guitar-steel" },
        { uuid::Charango,                  u"guitar-steel" },
        { uuid::ChitarraBattente,          u"guitar-steel" },
        { uuid::ChaozhouGuzheng,           u"guitar-steel" },
        { uuid::Cimbalom,                  u"cimbalom" },
        { uuid::Cittern,                   u"guitar-steel" },
        { uuid::Cuatro,                    u"guitar-steel" },
        { uuid::DanBau,                    u"guitar-steel" },
        { uuid::DanNguyet,                 u"guitar-steel" },
        { uuid::DanTamThapLuc,             u"guitar-steel" },
        { uuid::DanTranh,                  u"guitar-steel" },
        { uuid::DanTyBa,                   u"guitar-steel" },
        { uuid::DiddleyBow,                u"guitar-steel" },
        { uuid::Dobro,                     u"guitar-steel" },
        { uuid::Domra,                     u"guitar-steel" },
        { uuid::Dutar,                     u"guitar-steel" },
        { uuid::Duxianqin,                 u"guitar-steel" },
        { uuid::Ektara1,                   u"guitar-steel" },
        { uuid::FlamencoGuitar,            u"guitar-steel" },
        { uuid::Geomungo,                  u"guitar-steel" },
        { uuid::Ektara2,                   u"guitar-steel" },
        { uuid::Gottuvadhyam,              u"guitar-steel" },
        { uuid::GuitarraQuintaHuapanguera, u"guitar-steel" },
        { uuid::Guitarron,                 u"guitar-steel" },
        { uuid::Guitjo,                    u"guitar-steel" },
        { uuid::GuitjoDoubleNeck,          u"guitar-steel" },
        { uuid::Guqin,                     u"guitar-steel" },
        { uuid::Guzheng,                   u"guitar-steel" },
        { uuid::HarpGuitar,                u"guitar-steel" },
        { uuid::IrishBouzouki,             u"guitar-steel" },
        { uuid::JaranaHuasteca,            u"guitar-steel" },
        { uuid::JaranaJarocho,             u"guitar-steel" },
        { uuid::JaranaMosquito,            u"guitar-steel" },
        { uuid::JaranaSegunda,             u"guitar-steel" },
        { uuid::JaranaTercera,             u"guitar-steel" },
        { uuid::Kabosy,                    u"guitar-steel" },
        { uuid::Kantele,                   u"guitar-steel" },
        { uuid::Kayagum,                   u"guitar-steel" },
        { uuid::Khim,                      u"guitar-steel" },
        { uuid::Kobza,                     u"guitar-steel" },
        { uuid::Komuz,                     u"guitar-steel" },
        { uuid::Koto,                      u"koto" },
        { uuid::Kutiyapi,                  u"guitar-steel" },
        { uuid::Langeleik,                 u"guitar-steel" },
        { uuid::Lyre,                      u"guitar-steel" },
        { uuid::MandoBass,                 u"guitar-steel" },
        { uuid::MandoCello,                u"mandocello" },
        { uuid::Mandola,                   u"mandola" },
        { uuid::Mandora,                   u"guitar-steel" },
        { uuid::Mandore,                   u"guitar-steel" },
        { uuid::Mangbetu,                  u"guitar-steel" },
        { uuid::Marovany,                  u"guitar-steel" },
        { uuid::MohanVeena,                u"guitar-steel" },
        { uuid::MoodSwinger,               u"guitar-steel" },
        { uuid::MusicalBow,                u"guitar-steel" },
        { uuid::Ngoni,                     u"guitar-steel" },
        { uuid::OctaveMandolin,            u"octave-mandolin" },
        { uuid::Oud,                       u"oud" },
        { uuid::Pipa,                      u"guitar-steel" },
        { uuid::PortugueseGuitar,          u"guitar-steel" },
        { uuid::Psaltery,                  u"guitar-steel" },
        { uuid::RequintoGuitar,            u"guitar-steel" },
        { uuid::Ruan,                      u"guitar-steel" },
        { uuid::RudraVeena,                u"guitar-steel" },
        { uuid::Sallaneh,                  u"guitar-steel" },
        { uuid::Sanshin,                   u"guitar-steel" },
        { uuid::Santoor,                   u"guitar-steel" },
        { uuid::Sanxian,                   u"guitar-steel" },
        { uuid::Sarod,                     u"guitar-steel" },
        { uuid::Saung,                     u"guitar-steel" },
        { uuid::Saz,                       u"guitar-steel" },
        { uuid::Se,                        u"guitar-steel" },
        { uuid::Setar,                     u"guitar-steel" },
        { uuid::Shamisen,                  u"shamisen" },
        { uuid::Tambura,                   u"guitar-steel" },
        { uuid::TarPlucked,                u"guitar-steel" },
        { uuid::Theorbo,                   u"theorbo-14-course" },
        { uuid::Timple,                    u"guitar-steel" },
        { uuid::Tres,                      u"guitar-steel" },
        { uuid::Tsymbaly,                  u"guitar-steel" },
        { uuid::Valiha,                    u"guitar-steel" },
        { uuid::Veena,                     u"guitar-steel" },
        { uuid::VichitraVeena,             u"guitar-steel" },
        { uuid::VihuelaMexico,             u"guitar-steel" },
        { uuid::VihuelaSpain,              u"guitar-steel" },
        { uuid::WashtubBass,               u"guitar-steel" },
        { uuid::Whamola,                   u"guitar-steel" },
        { uuid::Xalam,                     u"guitar-steel" },
        { uuid::Yangqin,                   u"guitar-steel" },
        { uuid::Yazheng,                   u"guitar-steel" },
        { uuid::Yueqin,                    u"guitar-steel" },

        // Tablature
        { uuid::TabGuitar,                 u"guitar-steel-tablature" },
        { uuid::TabGuitarNoName,           u"guitar-steel-tablature" },
        { uuid::TabGuitarStems,            u"guitar-steel-tablature" },
        { uuid::TabGuitarD,                u"guitar-steel-tablature" },
        { uuid::TabGuitarDADGAD,           u"guitar-steel-tablature" },
        { uuid::TabGuitarDoubled,          u"guitar-steel-tablature" },
        { uuid::TabGuitarDropD,            u"guitar-steel-tablature" },
        { uuid::TabGuitarG,                u"guitar-steel-tablature" },
        { uuid::TabGuitar7String,          u"7-string-guitar-tablature" },
        { uuid::TabBanjoG,                 u"banjo-tablature" },
        { uuid::TabTenorBanjo,             u"irish-tenor-banjo-tablature" },
        { uuid::TabBanjoC,                 u"banjo-tablature" },
        { uuid::TabBanjoD,                 u"banjo-tablature" },
        { uuid::TabBanjoDoubleC,           u"banjo-tablature" },
        { uuid::TabBanjoGModal,            u"banjo-tablature" },
        { uuid::TabBanjoPlectrum,          u"banjo-tablature" },
        { uuid::TabBassGuitar4,            u"bass-guitar-tablature" },
        { uuid::TabBassGuitar5,            u"bass-guitar-tablature" },
        { uuid::TabBassGuitar6,            u"bass-guitar-tablature" },
        { uuid::TabDulcimerDAA,            u"mtn-dulcimer-std-chrom-tab" },
        { uuid::TabDulcimerDAAUnison,      u"mtn-dulcimer-std-chrom-tab" },
        { uuid::TabDulcimerDAD,            u"mtn-dulcimer-std-chrom-tab" },
        { uuid::TabGamba,                  u"viola-da-gamba-tablature" },
        { uuid::TabLuteItalian,            u"lute-tablature" },
        { uuid::TabLuteLetters,            u"lute-tablature" },
        { uuid::TabMandolin,               u"mandolin-tablature" },
        { uuid::TabRequinto,               u"guitar-nylon-tablature" },
        { uuid::TabSitarShankar,           u"sitar" },
        { uuid::TabSitarKhan,              u"sitar" },
        { uuid::TabUkulele,                u"ukulele-4-str-tab" },
        { uuid::TabVihuela,                u"guitar-steel-tablature" },

        // Pitched Percussion
        { uuid::Timpani,                   u"timpani" },
        { uuid::Mallets,                   u"piano" },
        { uuid::Bells,                     u"piano" },
        { uuid::Chimes,                    u"tubular-bells" },
        { uuid::Crotales,                  u"crotales" },
        { uuid::Glockenspiel,              u"glockenspiel" },
        { uuid::SopranoGlockenspiel,       u"orff-soprano-glockenspiel" },
        { uuid::AltoGlockenspiel,          u"orff-alto-glockenspiel" },
        { uuid::Marimba,                   u"marimba" },
        { uuid::BassMarimba,               u"bass-marimba" },
        { uuid::MarimbaSingleStaff,        u"marimba-single" },
        { uuid::TubularBells,              u"tubular-bells" },
        { uuid::Vibraphone,                u"vibraphone" },
        { uuid::Xylophone,                 u"xylophone" },
        { uuid::SopranoXylophone,          u"orff-soprano-xylophone" },
        { uuid::AltoXylophone,             u"orff-alto-xylophone" },
        { uuid::BassXylophone,             u"orff-bass-xylophone" },
        { uuid::Xylorimba,                 u"xylomarimba" },
        { uuid::BellLyre,                  u"glockenspiel" },
        { uuid::Boomwhackers,              u"piano" },
        { uuid::ChromanotesInstruments,    u"piano" },
        { uuid::Carillon,                  u"carillon" },
        { uuid::CrystalGlasses,            u"musical-glasses" },
        { uuid::FlexatonePitched,          u"flexatone" },
        { uuid::GlassHarmonica,            u"glass-harmonica" },
        { uuid::GlassMarimba,              u"piano" },
        { uuid::Handbells,                 u"hand-bells" },
        { uuid::HandbellsTClef,            u"piano" },
        { uuid::HandbellsBClef,            u"piano" },
        { uuid::HangTClef,                 u"piano" },
        { uuid::JawHarp,                   u"piano" },
        { uuid::Kalimba,                   u"kalimba" },
        { uuid::SopranoMetallophone,       u"orff-soprano-metallophone" },
        { uuid::AltoMetallophone,          u"orff-alto-metallophone" },
        { uuid::BassMetallophone,          u"orff-bass-metallophone" },
        { uuid::MusicalSaw,                u"musical-saw" },
        { uuid::SlideWhistle,              u"slide-whistle" },
        { uuid::SteelDrumsTClef,           u"soprano-steel-drums" },
        { uuid::SteelDrumsBClef,           u"bass-steel-drums" },
        { uuid::BonangGamelan,             u"piano" },
        { uuid::GansaGamelan,              u"piano" },
        { uuid::GenderGamelan,             u"piano" },
        { uuid::GiyingGamelan,             u"piano" },
        { uuid::KantilGamelan,             u"piano" },
        { uuid::PelogPanerusGamelan,       u"piano" },
        { uuid::PemadeGamelan,             u"piano" },
        { uuid::PenyacahGamelan,           u"piano" },
        { uuid::SaronBarungGamelan,        u"piano" },
        { uuid::SaronDemongGamelan,        u"piano" },
        { uuid::SaronPanerusGamelan,       u"piano" },
        { uuid::SlendroPanerusGamelan,     u"piano" },
        { uuid::SlenthemGamelan,           u"piano" },
        { uuid::Almglocken,                u"almglocken" },
        { uuid::Angklung,                  u"piano" },
        { uuid::ArrayMbira,                u"piano" },
        { uuid::Balafon,                   u"piano" },
        { uuid::Balaphon,                  u"piano" },
        { uuid::Bianqing,                  u"piano" },
        { uuid::Bianzhong,                 u"piano" },
        { uuid::Fangxiang,                 u"piano" },
        { uuid::GandinganAKayo,            u"piano" },
        { uuid::Gyil,                      u"piano" },
        { uuid::Kubing,                    u"piano" },
        { uuid::Kulintang,                 u"piano" },
        { uuid::KulintangAKayo,            u"piano" },
        { uuid::KulintangATiniok,          u"piano" },
        { uuid::Lamellaphone,              u"piano" },
        { uuid::Likembe,                   u"piano" },
        { uuid::Luntang,                   u"piano" },
        { uuid::Mbira,                     u"piano" },
        { uuid::Murchang,                  u"piano" },
        { uuid::RanatEklek,                u"piano" },
        { uuid::RanatThumLek,              u"piano" },
        { uuid::Sanza,                     u"piano" },
        { uuid::TaikoDrums,                u"taiko" },
        { uuid::TempleBells,               u"piano" },
        { uuid::TibetanBells,              u"piano" },
        { uuid::TibetanSingingBowls,       u"piano" },

        // Drums
        { uuid::SnareDrum,                 u"snare-drum" },
        { uuid::BassDrum,                  u"bass-drum" },
        { uuid::DrumSet,                   u"drumset" },
        { uuid::TenorDrum,                 u"tenor-drum" },
        { uuid::QuadToms,                  u"tom-toms" },
        { uuid::QuintToms,                 u"tom-toms" },
        { uuid::RotoToms,                  u"roto-toms" },
        { uuid::TenorLine,                 u"marching-tenorline" },
        { uuid::SnareLine,                 u"marching-snareline" },
        { uuid::BassDrums5Line,            u"marching-bassline" },
        { uuid::Djembe,                    u"djembe" },
        { uuid::BongoDrums,                u"bongos" },
        { uuid::CongaDrums,                u"congas" },
        { uuid::LogDrum,                   u"log-drum" },
        { uuid::Tablas,                    u"tablas" },
        { uuid::Timbales,                  u"timbales" },
        { uuid::AfricanLogDrum,            u"log-drum" },
        { uuid::Apentemma,                 u"snare-drum" },
        { uuid::ArabianFrameDrum,          u"snare-drum" },
        { uuid::Ashiko,                    u"snare-drum" },
        { uuid::Atabaque,                  u"snare-drum" },
        { uuid::Bata,                      u"snare-drum" },
        { uuid::Bendir,                    u"snare-drum" },
        { uuid::Bodhran,                   u"snare-drum" },
        { uuid::Bombo,                     u"snare-drum" },
        { uuid::Bougarabou,                u"snare-drum" },
        { uuid::BuffaloDrum,               u"snare-drum" },
        { uuid::Chenda,                    u"snare-drum" },
        { uuid::Chudaiko,                  u"snare-drum" },
        { uuid::Dabakan,                   u"snare-drum" },
        { uuid::Daibyosi,                  u"snare-drum" },
        { uuid::Damroo,                    u"snare-drum" },
        { uuid::Darabuka,                  u"snare-drum" },
        { uuid::DatangulionDrum,           u"snare-drum" },
        { uuid::Dhol,                      u"snare-drum" },
        { uuid::Dholak,                    u"snare-drum" },
        { uuid::Dollu,                     u"snare-drum" },
        { uuid::Dondo,                     u"snare-drum" },
        { uuid::Doundounba,                u"snare-drum" },
        { uuid::Duff,                      u"snare-drum" },
        { uuid::Dumbek,                    u"doumbek" },
        { uuid::EweDrumKagan,              u"snare-drum" },
        { uuid::EweDrumKpanlogo1Large,     u"snare-drum" },
        { uuid::EweDrumKpanlogo2Medium,    u"snare-drum" },
        { uuid::EweDrumKpanlogo3Combo,     u"snare-drum" },
        { uuid::EweDrumSogo,               u"snare-drum" },
        { uuid::Fontomfrom,                u"snare-drum" },
        { uuid::Geduk,                     u"snare-drum" },
        { uuid::HandDrum,                  u"snare-drum" },
        { uuid::Hiradaiko,                 u"snare-drum" },
        { uuid::Igihumurizo,               u"snare-drum" },
        { uuid::Ingoma,                    u"snare-drum" },
        { uuid::Inyahura,                  u"snare-drum" },
        { uuid::Janggu,                    u"janggu" },
        { uuid::Kakko,                     u"kakko" },
        { uuid::Kanjira,                   u"snare-drum" },
        { uuid::KendangGamelan,            u"snare-drum" },
        { uuid::Kenkeni,                   u"snare-drum" },
        { uuid::Khol,                      u"snare-drum" },
        { uuid::Kodaiko,                   u"snare-drum" },
        { uuid::Kudum,                     u"snare-drum" },
        { uuid::LambegDrum,                u"snare-drum" },
        { uuid::Madal,                     u"snare-drum" },
        { uuid::Maddale,                   u"snare-drum" },
        { uuid::MoroccoDrum,               u"snare-drum" },
        { uuid::Mridangam,                 u"snare-drum" },
        { uuid::Naal,                      u"snare-drum" },
        { uuid::NagaDodaiko,               u"snare-drum" },
        { uuid::Nagara,                    u"snare-drum" },
        { uuid::Naqara,                    u"snare-drum" },
        { uuid::NativeLogDrum,             u"log-drum" },
        { uuid::NigerianLogDrum,           u"log-drum" },
        { uuid::Odaiko,                    u"snare-drum" },
        { uuid::Okawa,                     u"snare-drum" },
        { uuid::OkedoDodaiko,              u"okedo-daiko" },
        { uuid::PahuHula,                  u"snare-drum" },
        { uuid::Pakhavaj,                  u"snare-drum" },
        { uuid::Pandero,                   u"snare-drum" },
        { uuid::PowwowDrum,                u"snare-drum" },
        { uuid::PuebloDrum,                u"snare-drum" },
        { uuid::Repinique,                 u"snare-drum" },
        { uuid::Sabar,                     u"snare-drum" },
        { uuid::Sakara,                    u"snare-drum" },
        { uuid::Sampho,                    u"snare-drum" },
        { uuid::Sangban,                   u"snare-drum" },
        { uuid::ShimeDaiko,                u"shime-daiko" },
        { uuid::Surdo,                     u"snare-drum" },
        { uuid::TalkingDrum,               u"snare-drum" },
        { uuid::Tama,                      u"snare-drum" },
        { uuid::Tamborita,                 u"snare-drum" },
        { uuid::Tamte,                     u"snare-drum" },
        { uuid::Tantan,                    u"snare-drum" },
        { uuid::Tangku,                    u"snare-drum" },
        { uuid::Taphon,                    u"snare-drum" },
        { uuid::TarDrum,                   u"snare-drum" },
        { uuid::Tasha,                     u"snare-drum" },
        { uuid::Thavil,                    u"snare-drum" },
        { uuid::Tombak,                    u"snare-drum" },
        { uuid::Tumbak,                    u"snare-drum" },
        { uuid::Tsuzumi,                   u"o-tsuzumi" },
        { uuid::UchiwaDaiko,               u"snare-drum" },
        { uuid::Udaku,                     u"snare-drum" },
        { uuid::Zarb,                      u"snare-drum" },

        // Percussion
        { uuid::PercussionGeneral,         u"percussion" },
        { uuid::PercAccessories,           u"percussion" },
        { uuid::WindChimes,                u"percussion" },
        { uuid::ChimeTree,                 u"percussion" },
        { uuid::BellTree,                  u"bell-tree" },
        { uuid::JingleBells,               u"percussion" },
        { uuid::Tambourine,                u"tambourine" },
        { uuid::Triangle,                  u"triangle" },
        { uuid::Cymbals,                   u"cymbal" },
        { uuid::FingerCymbals,             u"finger-cymbals" },
        { uuid::CrashCymbal,               u"crash-cymbal" },
        { uuid::HiHatCymbal,               u"hi-hat" },
        { uuid::RideCymbal,                u"ride-cymbal" },
        { uuid::SplashCymbal,              u"splash-cymbal" },
        { uuid::TamTam,                    u"tam-tam" },
        { uuid::Gong,                      u"tam-tam" },
        { uuid::AgogoBells,                u"agogo-bells" },
        { uuid::AirHorn,                   u"percussion" },
        { uuid::BrakeDrum,                 u"automobile-brake-drums" },
        { uuid::Cabasa,                    u"cabasa" },
        { uuid::Cajon,                     u"cajon" },
        { uuid::Castanets,                 u"castanets" },
        { uuid::Clap,                      u"percussion" },
        { uuid::Clapper,                   u"percussion" },
        { uuid::Claves,                    u"claves" },
        { uuid::Cowbell,                   u"cowbell" },
        { uuid::Cuica,                     u"cuica" },
        { uuid::Guiro,                     u"guiro" },
        { uuid::Maracas,                   u"maracas" },
        { uuid::PoliceWhistle,             u"percussion" },
        { uuid::Rainstick,                 u"percussion" },
        { uuid::Ratchet,                   u"ratchet" },
        { uuid::Rattle,                    u"percussion" },
        { uuid::SandBlock,                 u"percussion" },
        { uuid::Shakers,                   u"shaker" },
        { uuid::Spoons,                    u"percussion" },
        { uuid::TempleBlocks,              u"temple-blocks" },
        { uuid::Vibraslap,                 u"vibraslap" },
        { uuid::Washboard,                 u"percussion" },
        { uuid::Whip,                      u"whip" },
        { uuid::WindMachine,               u"percussion" },
        { uuid::WoodBlocks,                u"wood-blocks" },
        { uuid::CengCengGamelan,           u"percussion" },
        { uuid::GongAgengGamelan,          u"percussion" },
        { uuid::KempulGamelan,             u"percussion" },
        { uuid::KempyangGamelan,           u"percussion" },
        { uuid::KenongGamelan,             u"percussion" },
        { uuid::KetukGamelan,              u"percussion" },
        { uuid::ReyongGamelan,             u"percussion" },
        { uuid::Adodo,                     u"percussion" },
        { uuid::AeolianHarp,               u"percussion" },
        { uuid::Afoxe,                     u"percussion" },
        { uuid::AgogoBlock,                u"percussion" },
        { uuid::Agung,                     u"percussion" },
        { uuid::AgungAtamLang,             u"percussion" },
        { uuid::Ahoko,                     u"percussion" },
        { uuid::Babendil,                  u"percussion" },
        { uuid::BasicIndianPercussion,     u"percussion" },
        { uuid::Berimbau,                  u"percussion" },
        { uuid::Bo,                        u"percussion" },
        { uuid::Bones,                     u"percussion" },
        { uuid::BongoBells,                u"percussion" },
        { uuid::Bullroarer,                u"percussion" },
        { uuid::Caxixi,                    u"percussion" },
        { uuid::ChachaBells,               u"percussion" },
        { uuid::Chabara,                   u"percussion" },
        { uuid::Chanchiki,                 u"percussion" },
        { uuid::Chimta,                    u"percussion" },
        { uuid::ChinaTempleBlocks,         u"percussion" },
        { uuid::ChineseCymbals,            u"percussion" },
        { uuid::ChineseGongs,              u"percussion" },
        { uuid::ChinesePercussionEnsemble, u"percussion" },
        { uuid::Ching,                     u"percussion" },
        { uuid::Chippli,                   u"percussion" },
        { uuid::Daff,                      u"percussion" },
        { uuid::Dafli,                     u"percussion" },
        { uuid::Dawuro,                    u"percussion" },
        { uuid::Def,                       u"percussion" },
        { uuid::Doira,                     u"percussion" },
        { uuid::EweDrumAtoke,              u"percussion" },
        { uuid::EweDrumAxatse,             u"percussion" },
        { uuid::EweDrumGangokui,           u"percussion" },
        { uuid::FlexatonePerc,             u"percussion" },
        { uuid::Gandingan,                 u"percussion" },
        { uuid::Ganza,                     u"percussion" },
        { uuid::Ghatam,                    u"percussion" },
        { uuid::Ghungroo,                  u"percussion" },
        { uuid::Gome,                      u"percussion" },
        { uuid::Guban,                     u"percussion" },
        { uuid::HandCymbal,                u"percussion" },
        { uuid::Hang,                      u"percussion" },
        { uuid::Hatheli,                   u"percussion" },
        { uuid::Hosho,                     u"percussion" },
        { uuid::Hyoushigi,                 u"percussion" },
        { uuid::Ibo,                       u"percussion" },
        { uuid::IndianGong,                u"percussion" },
        { uuid::Ipu,                       u"percussion" },
        { uuid::Jawbone,                   u"percussion" },
        { uuid::KaEkeEke,                  u"percussion" },
        { uuid::Kagul,                     u"percussion" },
        { uuid::Kalaau,                    u"percussion" },
        { uuid::Kashiklar,                 u"percussion" },
        { uuid::Kesi,                      u"percussion" },
        { uuid::Khartal,                   u"percussion" },
        { uuid::Kkwaenggwari,              u"kkwaenggwari" },
        { uuid::Kpokopoko,                 u"percussion" },
        { uuid::KrinSlitDrum,              u"percussion" },
        { uuid::LavaStones,                u"percussion" },
        { uuid::LuoGong,                   u"percussion" },
        { uuid::Manjeera,                  u"percussion" },
        { uuid::PanClappers,               u"percussion" },
        { uuid::Patschen,                  u"percussion" },
        { uuid::RattleCog,                 u"percussion" },
        { uuid::Riq,                       u"percussion" },
        { uuid::Shekere,                   u"shekere" },
        { uuid::Sistre,                    u"percussion" },
        { uuid::Sistrum,                   u"percussion" },
        { uuid::SlideWhistlePercClef,      u"percussion" },
        { uuid::SlitDrum,                  u"slit-drum" },
        { uuid::Snap,                      u"percussion" },
        { uuid::Stamp,                     u"stamp" },
        { uuid::StirDrum,                  u"percussion" },
        { uuid::TebYoshi,                  u"percussion" },
        { uuid::Televi,                    u"percussion" },
        { uuid::Teponaztli,                u"percussion" },
        { uuid::ThaiGong,                  u"percussion" },
        { uuid::TibetanCymbals,            u"percussion" },
        { uuid::TicTocBlock,               u"percussion" },
        { uuid::TimbaleBell,               u"percussion" },
        { uuid::Tinaja,                    u"percussion" },
        { uuid::Tingsha,                   u"percussion" },
        { uuid::Toere,                     u"percussion" },
        { uuid::ToneTang,                  u"percussion" },
        { uuid::Trychel,                   u"percussion" },
        { uuid::Udu,                       u"percussion" },
        { uuid::Zills,                     u"percussion" },
    };
    // todo: different fallback for unpitched percussion
    return muse::value(uuidTable, uuid, u"piano");
}

BracketType toMuseScoreBracketType(details::Bracket::BracketStyle style)
{
    using MusxBracketStyle = details::Bracket::BracketStyle;
    static const std::unordered_map<MusxBracketStyle, BracketType> bracketTypeTable = {
        { MusxBracketStyle::None,                 BracketType::NO_BRACKET },
        { MusxBracketStyle::ThickLine,            BracketType::LINE },
        { MusxBracketStyle::BracketStraightHooks, BracketType::NORMAL },
        { MusxBracketStyle::PianoBrace,           BracketType::BRACE },
        { MusxBracketStyle::BracketCurvedHooks,   BracketType::NORMAL },
        { MusxBracketStyle::DeskBracket,          BracketType::SQUARE },
    };
    return muse::value(bracketTypeTable, style, BracketType::NO_BRACKET);
}

TupletNumberType toMuseScoreTupletNumberType(options::TupletOptions::NumberStyle numberStyle)
{
    using MusxTupletNumberType = options::TupletOptions::NumberStyle;
    static const std::unordered_map<MusxTupletNumberType, TupletNumberType> tupletNumberTypeTable = {
        { MusxTupletNumberType::Nothing,                  TupletNumberType::NO_TEXT },
        { MusxTupletNumberType::Number,                   TupletNumberType::SHOW_NUMBER },
        { MusxTupletNumberType::UseRatio,                 TupletNumberType::SHOW_RELATION },
        { MusxTupletNumberType::RatioPlusDenominatorNote, TupletNumberType::SHOW_RELATION }, // not supported
        { MusxTupletNumberType::RatioPlusBothNotes,       TupletNumberType::SHOW_RELATION }, // not supported
    };
    return muse::value(tupletNumberTypeTable, numberStyle, TupletNumberType::SHOW_NUMBER);
}

Align justifyToAlignment(others::NamePositioning::AlignJustify alignJustify)
{
    static const std::unordered_map<others::NamePositioning::AlignJustify, AlignH> alignTable = {
        { others::NamePositioning::AlignJustify::Left,   AlignH::LEFT },
        { others::NamePositioning::AlignJustify::Right,  AlignH::RIGHT },
        { others::NamePositioning::AlignJustify::Center, AlignH::HCENTER },
    };
    return Align(muse::value(alignTable, alignJustify, AlignH::HCENTER), AlignV::VCENTER);
}

AlignH toAlignH(others::HorizontalTextJustification hTextJustify)
{
    static const std::unordered_map<others::HorizontalTextJustification, AlignH> hAlignTable = {
        { others::HorizontalTextJustification::Left,   AlignH::LEFT },
        { others::HorizontalTextJustification::Center, AlignH::HCENTER },
        { others::HorizontalTextJustification::Right,  AlignH::RIGHT },
    };
    return muse::value(hAlignTable, hTextJustify, AlignH::LEFT);
}

AlignH toAlignH(options::LyricOptions::AlignJustify hTextJustify)
{
    static const std::unordered_map<options::LyricOptions::AlignJustify, AlignH> hAlignTable = {
        { options::LyricOptions::AlignJustify::Left,   AlignH::LEFT },
        { options::LyricOptions::AlignJustify::Center, AlignH::HCENTER },
        { options::LyricOptions::AlignJustify::Right,  AlignH::RIGHT },
    };
    return muse::value(hAlignTable, hTextJustify, AlignH::LEFT);
}

AlignH toAlignH(others::MeasureNumberRegion::AlignJustify align)
{
    static const std::unordered_map<others::MeasureNumberRegion::AlignJustify, AlignH> hAlignTable = {
        { others::MeasureNumberRegion::AlignJustify::Left,   AlignH::LEFT },
        { others::MeasureNumberRegion::AlignJustify::Center, AlignH::HCENTER },
        { others::MeasureNumberRegion::AlignJustify::Right,  AlignH::RIGHT },
    };
    return muse::value(hAlignTable, align, AlignH::LEFT);
}

AlignH toAlignH(others::PageTextAssign::HorizontalAlignment align)
{
    static const std::unordered_map<others::PageTextAssign::HorizontalAlignment, AlignH> hAlignTable = {
        { others::PageTextAssign::HorizontalAlignment::Left,   AlignH::LEFT },
        { others::PageTextAssign::HorizontalAlignment::Center, AlignH::HCENTER },
        { others::PageTextAssign::HorizontalAlignment::Right,  AlignH::RIGHT },
    };
    return muse::value(hAlignTable, align);
}

AlignV toAlignV(others::PageTextAssign::VerticalAlignment align)
{
    static const std::unordered_map<others::PageTextAssign::VerticalAlignment, AlignV> vAlignTable = {
        { others::PageTextAssign::VerticalAlignment::Top,    AlignV::TOP },
        { others::PageTextAssign::VerticalAlignment::Center, AlignV::VCENTER },
        { others::PageTextAssign::VerticalAlignment::Bottom, AlignV::BOTTOM }, // observed: BASELINE produces erratic results
    };
    return muse::value(vAlignTable, align);
}

CourtesyBarlineMode boolToCourtesyBarlineMode(bool useDoubleBarlines)
{
    static const std::unordered_map<bool, CourtesyBarlineMode> courtesyBarlineModeTable = {
        { false, CourtesyBarlineMode::ALWAYS_SINGLE },
        { true,  CourtesyBarlineMode::ALWAYS_DOUBLE },
    };
    return muse::value(courtesyBarlineModeTable, useDoubleBarlines, CourtesyBarlineMode::DOUBLE_BEFORE_COURTESY);
}

NoteVal notePropertiesToNoteVal(const musx::dom::Note::NoteProperties& noteProperties, Key key)
{
    auto [noteType, octave, alteration, staffLine] = noteProperties;
    NoteVal nval;
    nval.pitch = 60 /*middle C*/ + (octave - 4) * PITCH_DELTA_OCTAVE + step2pitch(int(noteType)) + alteration;
    if (alteration < int(AccidentalVal::MIN) || alteration > int(AccidentalVal::MAX) || !pitchIsValid(nval.pitch)) {
        nval.pitch = clampPitch(nval.pitch);
        nval.tpc1 = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
    } else {
        nval.tpc1 = step2tpc(int(noteType), AccidentalVal(alteration));
    }
    nval.tpc2 = nval.tpc1;
    return nval;
}

Fraction musxFractionToFraction(const musx::util::Fraction& fraction)
{
    // unlike with time signatures, remainder does not need to be accounted for
    return Fraction(fraction.numerator(), fraction.denominator());
}

Fraction eduToFraction(Edu edu)
{
    return musxFractionToFraction(musx::util::Fraction::fromEdu(edu));
}

Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig, musx::util::Fraction pickupSpacer, FinaleLoggerPtr& logger)
{
    auto [count, noteType] = simpleMusxTimeSig;
    if (count.remainder()) {
        if ((Edu(noteType) % count.denominator()) == 0) {
            noteType = musx::dom::NoteType(Edu(noteType) / count.denominator());
            count *= count.denominator();
        } else {
            logger->logWarning(String(u"Time signature has fractional portion that could not be reduced."));
            return Fraction(4, 4);
        }
    }
    Fraction result = Fraction(count.quotient(),  musx::util::Fraction::fromEdu(Edu(noteType)).denominator());
    Fraction spacer = musxFractionToFraction(pickupSpacer);
    if (spacer == Fraction(0, 1)) {
        return result;
    }
    if (spacer >= result) {
        logger->logWarning(String("Skipping pickup spacer that is larger than the time signature."));
        return result;
    }
    return result - spacer;
}

Key keyFromAlteration(int musxAlteration)
{
    return Key(musxAlteration);
}

KeyMode keyModeFromDiatonicMode(music_theory::DiatonicMode diatonicMode)
{
    using DiatonicMode = music_theory::DiatonicMode;
    static const std::unordered_map<DiatonicMode, KeyMode> keyModeTypeTable = {
        { DiatonicMode::Ionian,             KeyMode::IONIAN },
        { DiatonicMode::Dorian,             KeyMode::DORIAN },
        { DiatonicMode::Phrygian,           KeyMode::PHRYGIAN },
        { DiatonicMode::Lydian,             KeyMode::LYDIAN },
        { DiatonicMode::Mixolydian,         KeyMode::MIXOLYDIAN },
        { DiatonicMode::Aeolian,            KeyMode::AEOLIAN },
        { DiatonicMode::Locrian,            KeyMode::LOCRIAN },
    };
    return muse::value(keyModeTypeTable, diatonicMode, KeyMode::UNKNOWN);
}

SymId acciSymbolFromAcciAmount(int acciAmount)
{
    /// @todo add support for microtonal symbols (will require access to musx KeySignature instance)
    /// This code assumes each chromatic halfstep is 1 EDO division, but we cannot make that assumption
    /// with microtonal symbols.
    if (acciAmount == std::clamp(acciAmount, int(AccidentalVal::MIN), int(AccidentalVal::MIN))) {
        return Accidental::subtype2symbol(Accidental::value2subtype(AccidentalVal(acciAmount)));
    }
    return SymId::noSym;
}

StaffGroup staffGroupFromNotationStyle(musx::dom::others::Staff::NotationStyle notationStyle)
{
    using NotationStyle = musx::dom::others::Staff::NotationStyle;
    static const std::unordered_map<NotationStyle, StaffGroup> staffGroupMapTable = {
        { NotationStyle::Standard,          StaffGroup::STANDARD },
        { NotationStyle::Percussion,        StaffGroup::PERCUSSION },
        { NotationStyle::Tablature,         StaffGroup::TAB },
    };
    return muse::value(staffGroupMapTable, notationStyle, StaffGroup::STANDARD);

}

ElementType elementTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, ElementType> shapeTypeTable = {
        { ShapeType::SlurDown,            ElementType::SLUR },
        { ShapeType::SlurUp,              ElementType::SLUR },
        { ShapeType::Decrescendo,         ElementType::HAIRPIN },
        { ShapeType::Crescendo,           ElementType::HAIRPIN },
        { ShapeType::OctaveDown,          ElementType::OTTAVA },
        { ShapeType::OctaveUp,            ElementType::OTTAVA },
        // { ShapeType::DashLineUp,          ElementType::TEXTLINE },
        // { ShapeType::DashLineDown,        ElementType::TEXTLINE },
        { ShapeType::DashSlurDown,        ElementType::SLUR },
        { ShapeType::DashSlurUp,          ElementType::SLUR },
        // { ShapeType::DashLine,            ElementType::TEXTLINE },
        // { ShapeType::SolidLine,           ElementType::TEXTLINE },
        // { ShapeType::SolidLineDown,       ElementType::TEXTLINE },
        // { ShapeType::SolidLineUp,         ElementType::TEXTLINE },
        { ShapeType::Trill,               ElementType::TRILL },
        { ShapeType::SlurAuto,            ElementType::SLUR },
        { ShapeType::DashSlurAuto,        ElementType::SLUR },
        { ShapeType::TrillExtension,      ElementType::TRILL },
        // { ShapeType::SolidLineDownBoth,   ElementType::TEXTLINE },
        // { ShapeType::SolidLineUpBoth,     ElementType::TEXTLINE },
        { ShapeType::TwoOctaveDown,       ElementType::OTTAVA },
        { ShapeType::TwoOctaveUp,         ElementType::OTTAVA },
        // { ShapeType::DashLineDownBoth,    ElementType::TEXTLINE },
        // { ShapeType::DashLineUpBoth,      ElementType::TEXTLINE },
        { ShapeType::Glissando,           ElementType::GLISSANDO },
        { ShapeType::TabSlide,            ElementType::GLISSANDO },
        { ShapeType::BendHat,             ElementType::GUITAR_BEND },
        { ShapeType::BendCurve,           ElementType::GUITAR_BEND },
        { ShapeType::CustomLine,          ElementType::INVALID },
        // { ShapeType::SolidLineUpLeft,     ElementType::TEXTLINE },
        // { ShapeType::SolidLineDownLeft,   ElementType::TEXTLINE },
        // { ShapeType::DashLineUpLeft,      ElementType::TEXTLINE },
        // { ShapeType::DashLineDownLeft,    ElementType::TEXTLINE },
        // { ShapeType::SolidLineUpDown,     ElementType::TEXTLINE },
        // { ShapeType::SolidLineDownUp,     ElementType::TEXTLINE },
        // { ShapeType::DashLineUpDown,      ElementType::TEXTLINE },
        // { ShapeType::DashLineDownUp,      ElementType::TEXTLINE },
        /// { ShapeType::Hyphen,              ElementType::INVALID },
        /// { ShapeType::WordExtension,       ElementType::LYRICSLINE },
        { ShapeType::DashContourSlurDown, ElementType::SLUR },
        { ShapeType::DashContourSlurUp,   ElementType::SLUR },
        { ShapeType::DashContourSlurAuto, ElementType::SLUR },
    };
    return muse::value(shapeTypeTable, shapeType, ElementType::TEXTLINE);
}

OttavaType ottavaTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, OttavaType> ottavaTypeTable = {
        { ShapeType::OctaveDown,    OttavaType::OTTAVA_8VB },
        { ShapeType::OctaveUp,      OttavaType::OTTAVA_8VA },
        { ShapeType::TwoOctaveDown, OttavaType::OTTAVA_15MB },
        { ShapeType::TwoOctaveUp,   OttavaType::OTTAVA_15MA },
    };
    return muse::value(ottavaTypeTable, shapeType, OttavaType::OTTAVA_8VA);
}

HairpinType hairpinTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, HairpinType> ottavaTypeTable = {
        { ShapeType::Crescendo,     HairpinType::CRESC_HAIRPIN },
        { ShapeType::Decrescendo,   HairpinType::DIM_HAIRPIN }
    };
    return muse::value(ottavaTypeTable, shapeType, HairpinType::INVALID);
}

SlurStyleType slurStyleTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, SlurStyleType> shapeTypeTable = {
        { ShapeType::SlurDown,            SlurStyleType::Solid },
        { ShapeType::SlurUp,              SlurStyleType::Solid },
        { ShapeType::DashSlurDown,        SlurStyleType::Dashed },
        { ShapeType::DashSlurUp,          SlurStyleType::Dashed },
        { ShapeType::SlurAuto,            SlurStyleType::Solid },
        { ShapeType::DashSlurAuto,        SlurStyleType::Dashed },
        { ShapeType::DashContourSlurDown, SlurStyleType::Dashed },
        { ShapeType::DashContourSlurUp,   SlurStyleType::Dashed },
        { ShapeType::DashContourSlurAuto, SlurStyleType::Dashed },
    };
    return muse::value(shapeTypeTable, shapeType, SlurStyleType::Solid);
}

GlissandoType glissandoTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, GlissandoType> shapeTypeTable = {
        { ShapeType::Glissando,           GlissandoType::WAVY },
        { ShapeType::TabSlide,            GlissandoType::STRAIGHT },
    };
    return muse::value(shapeTypeTable, shapeType, GlissandoType::WAVY);
}

VibratoType vibratoTypeFromSymId(SymId vibratoSym)
{
    static const std::unordered_map<SymId, VibratoType> vibratoTypeTable = {
        { SymId::guitarVibratoStroke,     VibratoType::GUITAR_VIBRATO },
        { SymId::guitarWideVibratoStroke, VibratoType::GUITAR_VIBRATO_WIDE },
        { SymId::wiggleSawtooth,          VibratoType::VIBRATO_SAWTOOTH },
        { SymId::wiggleSawtoothWide,      VibratoType::VIBRATO_SAWTOOTH_WIDE },
    };
    return muse::value(vibratoTypeTable, vibratoSym, VibratoType::GUITAR_VIBRATO);
}

DirectionV directionVFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, DirectionV> shapeTypeTable = {
        { ShapeType::SlurDown,            DirectionV::DOWN },
        { ShapeType::SlurUp,              DirectionV::UP },
        { ShapeType::DashSlurDown,        DirectionV::DOWN },
        { ShapeType::DashSlurUp,          DirectionV::UP },
        // { ShapeType::SlurAuto,            DirectionV::AUTO },
        // { ShapeType::DashSlurAuto,        DirectionV::AUTO },
        { ShapeType::DashContourSlurDown, DirectionV::DOWN },
        { ShapeType::DashContourSlurUp,   DirectionV::UP },
        // { ShapeType::DashContourSlurAuto, DirectionV::AUTO },
    };
    return muse::value(shapeTypeTable, shapeType, DirectionV::AUTO);
}

LineType lineTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, LineType> shapeTypeTable = {
        { ShapeType::DashLineUp,          LineType::DASHED },
        { ShapeType::DashLineDown,        LineType::DASHED },
        { ShapeType::DashLine,            LineType::DASHED },
        // { ShapeType::SolidLine,           LineType::SOLID },
        // { ShapeType::SolidLineDown,       LineType::SOLID },
        // { ShapeType::SolidLineUp,         LineType::SOLID },
        // { ShapeType::SolidLineDownBoth,   LineType::SOLID },
        // { ShapeType::SolidLineUpBoth,     LineType::SOLID },
        { ShapeType::DashLineDownBoth,    LineType::DASHED },
        { ShapeType::DashLineUpBoth,      LineType::DASHED },
        // { ShapeType::SolidLineUpLeft,     LineType::SOLID },
        // { ShapeType::SolidLineDownLeft,   LineType::SOLID },
        { ShapeType::DashLineUpLeft,      LineType::DASHED },
        { ShapeType::DashLineDownLeft,    LineType::DASHED },
        // { ShapeType::SolidLineUpDown,     LineType::SOLID },
        // { ShapeType::SolidLineDownUp,     LineType::SOLID },
        { ShapeType::DashLineUpDown,      LineType::DASHED },
        { ShapeType::DashLineDownUp,      LineType::DASHED },
    };
    return muse::value(shapeTypeTable, shapeType, LineType::SOLID);
}

std::pair<int, int> hookHeightsFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType)
{
    using ShapeType = musx::dom::others::SmartShape::ShapeType;
    static const std::unordered_map<ShapeType, std::pair<int, int> > shapeTypeTable = {
        { ShapeType::DashLineUp,        { 0, -1 } },
        { ShapeType::DashLineDown,      { 0, 1 } },
        { ShapeType::DashLine,          { 0, 0 } },
        { ShapeType::SolidLine,         { 0, 0 } },
        { ShapeType::SolidLineDown,     { 0, 1 } },
        { ShapeType::SolidLineUp,       { 0, -1 } },
        { ShapeType::SolidLineDownBoth, { 1, 1 } },
        { ShapeType::SolidLineUpBoth,   { -1, -1 } },
        { ShapeType::DashLineDownBoth,  { 1, 1 } },
        { ShapeType::DashLineUpBoth,    { -1, -1 } },
        { ShapeType::SolidLineUpLeft,   { -1, 0 } },
        { ShapeType::SolidLineDownLeft, { 1, 0 } },
        { ShapeType::DashLineUpLeft,    { -1, 0 } },
        { ShapeType::DashLineDownLeft,  { 1, 0 } },
        { ShapeType::SolidLineUpDown,   { -1, 1 } },
        { ShapeType::SolidLineDownUp,   { 1, -1 } },
        { ShapeType::DashLineUpDown,    { -1, 1 } },
        { ShapeType::DashLineDownUp,    { 1, -1 } },
    };
    return muse::value(shapeTypeTable, shapeType, { 0, 0 });
}

String fontStylePrefixFromElementType(ElementType elementType)
{
    static const std::unordered_map<ElementType, std::string_view> elementTypeTable = {
        { ElementType::DYNAMIC, "dynamics" },
        { ElementType::EXPRESSION, "expression" },
        { ElementType::TEMPO_TEXT, "tempo" },
//        { ElementType::TEMPO_TEXT, "tempoChange" }, // maybe add "tempoChange" back if we switch to TextStyleType
        { ElementType::STAFF_TEXT, "staffText" },
        { ElementType::REHEARSAL_MARK, "rehearsalMark" },
    };
    return String::fromUtf8(muse::value(elementTypeTable, elementType, "default"));
}

TremoloType tremoloTypeFromSymId(SymId sym)
{
    static const std::unordered_map<SymId, TremoloType> tremoloTypeTable = {
        { SymId::tremolo1,                TremoloType::R8 },
        { SymId::tremoloFingered1,        TremoloType::R8 },
        { SymId::tremolo2,                TremoloType::R16 },
        { SymId::tremoloFingered2,        TremoloType::R16 },
        { SymId::tremolo3,                TremoloType::R32 },
        { SymId::tremoloFingered3,        TremoloType::R32 },
        { SymId::tremolo4,                TremoloType::R64 },
        { SymId::tremoloFingered4,        TremoloType::R64 },
        { SymId::tremolo5,                TremoloType::R64 },
        { SymId::tremoloFingered5,        TremoloType::R64 },
        { SymId::buzzRoll,                TremoloType::BUZZ_ROLL },
        { SymId::pendereckiTremolo,       TremoloType::BUZZ_ROLL },
        { SymId::unmeasuredTremolo,       TremoloType::BUZZ_ROLL },
        { SymId::unmeasuredTremoloSimple, TremoloType::BUZZ_ROLL },
    };
    return muse::value(tremoloTypeTable, sym, TremoloType::INVALID_TREMOLO);
}

engraving::BarLineType toMuseScoreBarLineType(others::Measure::BarlineType blt)
{
    static const std::unordered_map<others::Measure::BarlineType, engraving::BarLineType> barLineTable = {
        { others::Measure::BarlineType::None,           engraving::BarLineType::NORMAL },
        { others::Measure::BarlineType::OptionsDefault, engraving::BarLineType::NORMAL },
        { others::Measure::BarlineType::Normal,         engraving::BarLineType::NORMAL },
        { others::Measure::BarlineType::Double,         engraving::BarLineType::DOUBLE },
        { others::Measure::BarlineType::Final,          engraving::BarLineType::FINAL  },
        { others::Measure::BarlineType::Solid,          engraving::BarLineType::HEAVY  },
        { others::Measure::BarlineType::Dashed,         engraving::BarLineType::DASHED },
        { others::Measure::BarlineType::Tick,           engraving::BarLineType::NORMAL },
        { others::Measure::BarlineType::Custom,         engraving::BarLineType::NORMAL },
    };
    return muse::value(barLineTable, blt, engraving::BarLineType::NORMAL);
}

FretDotType toFretDotType(details::FretboardDiagram::Shape shape)
{
    static const std::unordered_map<details::FretboardDiagram::Shape, FretDotType> dotTable = {
        { details::FretboardDiagram::Shape::Closed, FretDotType::NORMAL },
        { details::FretboardDiagram::Shape::Open,   FretDotType::NORMAL },
        { details::FretboardDiagram::Shape::Muted,  FretDotType::CROSS },
        { details::FretboardDiagram::Shape::Custom, FretDotType::TRIANGLE },
    };
    return muse::value(dotTable, shape, FretDotType::NORMAL);
}

FretMarkerType toFretMarkerType(details::FretboardDiagram::Shape shape)
{
    static const std::unordered_map<details::FretboardDiagram::Shape, FretMarkerType> dotTable = {
        { details::FretboardDiagram::Shape::None,   FretMarkerType::NONE },
        { details::FretboardDiagram::Shape::Closed, FretMarkerType::CIRCLE },
        { details::FretboardDiagram::Shape::Open,   FretMarkerType::CIRCLE },
        { details::FretboardDiagram::Shape::Muted,  FretMarkerType::CROSS },
        { details::FretboardDiagram::Shape::Custom, FretMarkerType::CROSS },
    };
    return muse::value(dotTable, shape, FretMarkerType::NONE);
}

SymId unparenthesisedNoteHead(const std::string& symName)
{
    static const std::unordered_map<std::string, SymId> noteHeadTable = {
        { "noteheadDoubleWholeParens", SymId::noteheadDoubleWhole },
        { "noteheadWholeParens",       SymId::noteheadWhole },
        { "noteheadHalfParens",        SymId::noteheadHalf },
        { "noteheadBlackParens",       SymId::noteheadBlack },
    };
    return muse::value(noteHeadTable, symName, SymId::noSym);
}

double doubleFromEvpu(double evpuDouble)
{
    return evpuDouble / EVPU_PER_SPACE;
}

PointF evpuToPointF(double xEvpu, double yEvpu)
{
    return PointF(doubleFromEvpu(xEvpu), doubleFromEvpu(yEvpu));
}

double doubleFromEfix(double efix)
{
    return efix / EFIX_PER_SPACE;
}

String metaTagFromFileInfo(texts::FileInfoText::TextType textType)
{
    using TextType = texts::FileInfoText::TextType;
    static const std::unordered_map<TextType, String> metaTagTable = {
        { TextType::Title,       u"workTitle" },
        { TextType::Composer,    u"composer" },
        { TextType::Copyright,   u"copyright" },
        { TextType::Description, u"description" }, // created by Finale importer
        { TextType::Lyricist,    u"lyricist" },
        { TextType::Arranger,    u"arranger" },
        { TextType::Subtitle,    u"subtitle" },
    };
    return muse::value(metaTagTable, textType, String());
}

String metaTagFromTextComponent(const std::string& component)
{
    static const std::unordered_map<std::string_view, String> metaTagTable = {
        { "title",       u"workTitle" },
        { "composer",    u"composer" },
        { "copyright",   u"copyright" },
        { "description", u"description" }, // created by Finale importer
        { "lyricist",    u"lyricist" },
        { "arranger",    u"arranger" },
        { "subtitle",    u"subtitle" },
    };
    return muse::value(metaTagTable, component, String());
}

double doubleFromPercent(int percent)
{
    return double(percent) / 100.0;
}

double spatiumScaledFontSize(const MusxInstance<FontInfo>& fontInfo)
{
    return double(fontInfo->fontSize) * (fontInfo->absolute ? 1.0 : MUSE_FINALE_SCALE_DIFFERENTIAL);
}

double absoluteDouble(double value, EngravingItem* e)
{
    return value * e->defaultSpatium();
}

double absoluteDoubleFromEvpu(Evpu evpu, EngravingItem* e)
{
    return absoluteDouble(doubleFromEvpu(evpu), e);
}

double scaledDoubleFromEvpu(Evpu evpu, EngravingItem* e)
{
    return doubleFromEvpu(evpu) * e->spatium();
}

Spatium absoluteSpatium(double value, EngravingItem* e)
{
    // Returns global spatium value adjusted to preserve value for element scaling
    return Spatium(value * e->defaultSpatium() / e->spatium());
}

Spatium absoluteSpatiumFromEvpu(Evpu evpu, EngravingItem* e)
{
    return absoluteSpatium(doubleFromEvpu(evpu), e);
}

}
