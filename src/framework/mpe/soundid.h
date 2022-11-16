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

#ifndef MU_MPE_SOUNDID_H
#define MU_MPE_SOUNDID_H

#include <set>
#include <unordered_map>

#include "types/string.h"

namespace mu::mpe {
enum class SoundId
{
    Undefined = -1,

    Accordion,
    Bandoneon,
    Concertina,
    Harmonica,
    Melodica,
    Sheng,
    Celesta,
    Clavichord,
    Harpsichord,
    Virginal,
    Piano,
    Organ,
    Harmonium,
    Synthesizer,
    Theremin,
    OndesMartenot,

    Harp,
    Cavaquinho,
    Guitar,
    BassGuitar,
    Banjo,
    Ukulele,
    Mandolin,
    MtnDulcimer,
    Lute,
    Theorbo,
    Archlute,
    Balalaika,
    Koto,
    Oud,
    Shamisen,
    Sitar,
    Prim,
    Brac,
    Bugarija,
    Berda,
    Celo,
    Bandurria,
    Laud,
    StringsGroup,
    Violin,
    ViolinSection,
    Viola,
    ViolaDaGamba,
    ViolaSection,
    Violoncello,
    VioloncelloSection,
    Viol,
    PardessusViol,
    Baryton,
    Violone,
    Nyckelharpa,
    Erhu,
    Contrabass,
    ContrabassSection,
    Octobass,

    WindsGroup,
    Piccolo,
    Heckelphone,
    HeckelphoneClarinet,
    Oboe,
    Lupophone,
    Flute,
    PanFlute,
    Danso,
    Traverso,
    Dizi,
    Shakuhachi,
    Fife,
    Whistle,
    Flageolet,
    Recorder,
    Ocarina,
    Gemshorn,
    Quena,
    Horn,
    Bassethorn,
    Shawm,
    Cromorne,
    Crumhorn,
    Cornamuse,
    Kelhorn,
    Rauschpfeife,
    Duduk,
    Shenai,
    Clarinet,
    Chalumeau,
    Xaphoon,
    Tarogato,
    Octavin,
    Saxophone,
    Aulochrome,
    Bassoon,
    Contrabassoon,
    Dulcian,
    Rackett,
    Sarrusophone,
    Bagpipe,
    Tuba,
    Cornet,
    Posthorn,
    BrassGroup,
    Trumpet,
    Bugle,
    MellophoneBugle,
    EuphoniumBugle,
    Euphonium,
    Fiscorn,
    Flugelhorn,
    Kuhlohorn,
    Ophicleide,
    Cornettino,
    Cornett,
    Serpent,
    Trombone,
    Cimbasso,
    Sackbut,
    Sousaphone,
    Helicon,
    Conch,
    Saxhorn,
    Horagai,
    Alphorn,
    RagDung,
    Didgeridoo,
    Shofar,
    Vuvuzela,

    Timpani,
    RotoToms,
    Tubaphone,
    SteelDrums,
    Glockenspiel,
    Xylophone,
    Xylomarimba,
    Vibraphone,
    Dulcimer,
    Cimbalom,
    Marimba,
    Crotales,
    Chimes,
    Carillon,
    Gong,
    Metallophone,
    Flexatone,
    MusicalSaw,
    MusicalGlasses,
    KlaxonHorns,
    Kalimba,
    Bongos,
    TomToms,
    Conga,
    Cuica,
    Drumset,
    Drum,
    Tablas,
    Timbales,
    Anvil,
    Bell,
    Chain,
    Cymbal,
    HiHat,
    Pipe,
    Castanet,
    TamTam,
    Thundersheet,
    Triangle,
    Claves,
    Guiro,
    Block,
    Cabasa,
    Maraca,
    Quijada,
    Ratchet,
    Shaker,
    Stones,
    Tambourine,
    Tubo,
    Vibraslap,
    Whip,
    Snap,
    Clap,
    Slap,
    Stamp,
    Choir,
    Kazoo,

    Last
};

enum class SoundCategory
{
    Undefined = -1,
    Keyboards,
    Strings,
    Winds,
    Percussions,
    Voices,

    Last
};

enum class SoundSubCategory
{
    Undefined = -1,

    English,
    Armenian,
    Alpine,
    Australian,
    Irish,
    French,
    Chinese,
    Vienna,
    Greek,
    Japanese,
    Tibetan,
    African,
    Indian,
    Spanish,
    Swedish,
    Hungarian,
    Romanian,
    CentralEuropean,

    Baroque,
    Classical,
    Modern,
    Orchestral,

    Hammond,
    Wagner,
    Orff,
    Huang,
    Hohner,

    Percussive,
    Piped,
    Rotary,
    Reed,
    Foot,
    Hand,
    Finger,
    Boy,
    Girl,
    Male,
    Female,
    Pad,
    Plucked,

    Temple,
    Military,
    Ride,
    Sleigh,
    Cow,
    Marching,

    Splash,
    Crash,
    Plate,
    Bowl,
    Frame,
    Slit,
    Field,
    Snare,
    Brake,
    Slide,
    Pocket,
    Garklein,
    Toy,
    String,
    TwelveString,

    Grand,
    HonkyTonk,
    Upright,
    Prima,
    Secunda,

    Electric,
    Acoustic,
    Fretless,
    Pedal,
    Steel,
    Metal,
    Iron,
    Brass,
    Tin,
    Nylon,
    Wooden,
    Sandpaper,
    Glass,
    Shell,
    Wind,

    Treble,
    Diatonic,
    Chromatic,
    Octave,

    Piccolo,
    Alto,
    Tenor,
    Baritone,
    Soprano,
    Mezzo_Soprano,
    Sopranino,
    Sopranissimo,
    Counter_Tenor,
    Contra,
    Contra_Alto,
    Sub_Contra_Alto,
    Contra_Bass,
    Sub_Contra_Bass,
    Double_Contra_Bass,
    Bass,
    Bowed,
    Great_Bass,
    Hyper_Bass,
    Melody,

    FX_Goblins,
    FX_Atmosphere,
    FX_Brightness,
    FX_Crystal,
    FX_Echoes,
    FX_Rain,
    FX_SciFi,
    FX_SoundTrack,

    Sweep,
    Warm,
    NewAge,
    Polysynth,
    Halo,
    Metallic,
    Choir,

    Sine_Wave,
    Square_Wave,
    Sawtooth_Wave,

    Last
};

static const String UNDEFINED_STR(u"Undefined");

static const std::unordered_map<SoundId, String> ID_STRINGS
{
    { SoundId::Accordion, String(u"Accordion") },
    { SoundId::Bandoneon, String(u"Bandoneon") },
    { SoundId::Concertina, String(u"Concertina") },
    { SoundId::Harmonica, String(u"Harmonica") },
    { SoundId::Melodica, String(u"Melodica") },
    { SoundId::Sheng, String(u"Sheng") },
    { SoundId::Celesta, String(u"Celesta") },
    { SoundId::Clavichord, String(u"Clavichord") },
    { SoundId::Harpsichord, String(u"Harpsichord") },
    { SoundId::Virginal, String(u"Virginal") },
    { SoundId::Piano, String(u"Piano") },
    { SoundId::Organ, String(u"Organ") },
    { SoundId::Harmonium, String(u"Harmonium") },
    { SoundId::Synthesizer, String(u"Synthesizer") },
    { SoundId::Theremin, String(u"Theremin") },
    { SoundId::OndesMartenot, String(u"OndesMartenot") },

    { SoundId::Harp, String(u"Harp") },
    { SoundId::Cavaquinho, String(u"Cavaquinho") },
    { SoundId::Guitar, String(u"Guitar") },
    { SoundId::BassGuitar, String(u"BassGuitar") },
    { SoundId::Banjo, String(u"Banjo") },
    { SoundId::Ukulele, String(u"Ukulele") },
    { SoundId::Mandolin, String(u"Mandolin") },
    { SoundId::MtnDulcimer, String(u"MtnDulcimer") },
    { SoundId::Lute, String(u"Lute") },
    { SoundId::Theorbo, String(u"Theorbo") },
    { SoundId::Archlute, String(u"Archlute") },
    { SoundId::Balalaika, String(u"Balalaika") },
    { SoundId::Koto, String(u"Koto") },
    { SoundId::Oud, String(u"Oud") },
    { SoundId::Shamisen, String(u"Shamisen") },
    { SoundId::Sitar, String(u"Sitar") },
    { SoundId::Prim, String(u"Prim") },
    { SoundId::Brac, String(u"Brac") },
    { SoundId::Bugarija, String(u"Bugarija") },
    { SoundId::Berda, String(u"Berda") },
    { SoundId::Celo, String(u"Celo") },
    { SoundId::Bandurria, String(u"Bandurria") },
    { SoundId::Laud, String(u"Laud") },
    { SoundId::StringsGroup, String(u"StringsGroup") },
    { SoundId::Violin, String(u"Violin") },
    { SoundId::ViolinSection, String(u"ViolinSection") },
    { SoundId::Viola, String(u"Viola") },
    { SoundId::ViolaDaGamba, String(u"ViolaDaGamba") },
    { SoundId::ViolaSection, String(u"ViolaSection") },
    { SoundId::Violoncello, String(u"Violoncello") },
    { SoundId::VioloncelloSection, String(u"VioloncelloSection") },
    { SoundId::Viol, String(u"Viol") },
    { SoundId::PardessusViol, String(u"PardessusViol") },
    { SoundId::Baryton, String(u"Baryton") },
    { SoundId::Violone, String(u"Violone") },
    { SoundId::Nyckelharpa, String(u"Nyckelharpa") },
    { SoundId::Erhu, String(u"Erhu") },
    { SoundId::Contrabass, String(u"Contrabass") },
    { SoundId::ContrabassSection, String(u"ContrabassSection") },
    { SoundId::Octobass, String(u"Octobass") },

    { SoundId::WindsGroup, String(u"WindsGroup") },
    { SoundId::Piccolo, String(u"Piccolo") },
    { SoundId::Heckelphone, String(u"Heckelphone") },
    { SoundId::HeckelphoneClarinet, String(u"HeckelphoneClarinet") },
    { SoundId::Oboe, String(u"Oboe") },
    { SoundId::Lupophone, String(u"Lupophone") },
    { SoundId::Flute, String(u"Flute") },
    { SoundId::PanFlute, String(u"PanFlute") },
    { SoundId::Danso, String(u"Danso") },
    { SoundId::Traverso, String(u"Traverso") },
    { SoundId::Dizi, String(u"Dizi") },
    { SoundId::Shakuhachi, String(u"Shakuhachi") },
    { SoundId::Fife, String(u"Fife") },
    { SoundId::Whistle, String(u"Whistle") },
    { SoundId::Flageolet, String(u"Flageolet") },
    { SoundId::Recorder, String(u"Recorder") },
    { SoundId::Ocarina, String(u"Ocarina") },
    { SoundId::Gemshorn, String(u"Gemshorn") },
    { SoundId::Quena, String(u"Quena") },
    { SoundId::Horn, String(u"Horn") },
    { SoundId::Bassethorn, String(u"Bassethorn") },
    { SoundId::Shawm, String(u"Shawm") },
    { SoundId::Cromorne, String(u"Cromorne") },
    { SoundId::Crumhorn, String(u"Crumhorn") },
    { SoundId::Cornamuse, String(u"Cornamuse") },
    { SoundId::Kelhorn, String(u"Kelhorn") },
    { SoundId::Rauschpfeife, String(u"Rauschpfeife") },
    { SoundId::Duduk, String(u"Duduk") },
    { SoundId::Shenai, String(u"Shenai") },
    { SoundId::Clarinet, String(u"Clarinet") },
    { SoundId::Chalumeau, String(u"Chalumeau") },
    { SoundId::Xaphoon, String(u"Xaphoon") },
    { SoundId::Tarogato, String(u"Tarogato") },
    { SoundId::Octavin, String(u"Octavin") },
    { SoundId::Saxophone, String(u"Saxophone") },
    { SoundId::Aulochrome, String(u"Aulochrome") },
    { SoundId::Bassoon, String(u"Bassoon") },
    { SoundId::Contrabassoon, String(u"Contrabassoon") },
    { SoundId::Dulcian, String(u"Dulcian") },
    { SoundId::Rackett, String(u"Rackett") },
    { SoundId::Sarrusophone, String(u"Sarrusophone") },
    { SoundId::Bagpipe, String(u"Bagpipe") },
    { SoundId::Tuba, String(u"Tuba") },
    { SoundId::Cornet, String(u"Cornet") },
    { SoundId::Posthorn, String(u"Posthorn") },
    { SoundId::BrassGroup, String(u"BrassGroup") },
    { SoundId::Trumpet, String(u"Trumpet") },
    { SoundId::Bugle, String(u"Bugle") },
    { SoundId::MellophoneBugle, String(u"MellophoneBugle") },
    { SoundId::EuphoniumBugle, String(u"EuphoniumBugle") },
    { SoundId::Euphonium, String(u"Euphonium") },
    { SoundId::Fiscorn, String(u"Fiscorn") },
    { SoundId::Flugelhorn, String(u"Flugelhorn") },
    { SoundId::Kuhlohorn, String(u"Kuhlohorn") },
    { SoundId::Ophicleide, String(u"Ophicleide") },
    { SoundId::Cornettino, String(u"Cornettino") },
    { SoundId::Cornett, String(u"Cornett") },
    { SoundId::Serpent, String(u"Serpent") },
    { SoundId::Trombone, String(u"Trombone") },
    { SoundId::Cimbasso, String(u"Cimbasso") },
    { SoundId::Sackbut, String(u"Sackbut") },
    { SoundId::Sousaphone, String(u"Sousaphone") },
    { SoundId::Helicon, String(u"Helicon") },
    { SoundId::Conch, String(u"Conch") },
    { SoundId::Saxhorn, String(u"Saxhorn") },
    { SoundId::Horagai, String(u"Horagai") },
    { SoundId::Alphorn, String(u"Alphorn") },
    { SoundId::RagDung, String(u"RagDung") },
    { SoundId::Didgeridoo, String(u"Didgeridoo") },
    { SoundId::Shofar, String(u"Shofar") },
    { SoundId::Vuvuzela, String(u"Vuvuzela") },

    { SoundId::Timpani, String(u"Timpani") },
    { SoundId::RotoToms, String(u"RotoToms") },
    { SoundId::Tubaphone, String(u"Tubaphone") },
    { SoundId::SteelDrums, String(u"SteelDrums") },
    { SoundId::Glockenspiel, String(u"Glockenspiel") },
    { SoundId::Xylophone, String(u"Xylophone") },
    { SoundId::Xylomarimba, String(u"Xylomarimba") },
    { SoundId::Vibraphone, String(u"Vibraphone") },
    { SoundId::Dulcimer, String(u"Dulcimer") },
    { SoundId::Cimbalom, String(u"Cimbalom") },
    { SoundId::Marimba, String(u"Marimba") },
    { SoundId::Crotales, String(u"Crotales") },
    { SoundId::Chimes, String(u"Chimes") },
    { SoundId::Carillon, String(u"Carillon") },
    { SoundId::Gong, String(u"Gong") },
    { SoundId::Metallophone, String(u"Metallophone") },
    { SoundId::Flexatone, String(u"Flexatone") },
    { SoundId::MusicalSaw, String(u"MusicalSaw") },
    { SoundId::MusicalGlasses, String(u"MusicalGlasses") },
    { SoundId::KlaxonHorns, String(u"KlaxonHorns") },
    { SoundId::Kalimba, String(u"Kalimba") },
    { SoundId::Bongos, String(u"Bongos") },
    { SoundId::TomToms, String(u"TomToms") },
    { SoundId::Conga, String(u"Conga") },
    { SoundId::Cuica, String(u"Cuica") },
    { SoundId::Drumset, String(u"Drumset") },
    { SoundId::Drum, String(u"Drum") },
    { SoundId::Tablas, String(u"Tablas") },
    { SoundId::Timbales, String(u"Timbales") },
    { SoundId::Anvil, String(u"Anvil") },
    { SoundId::Bell, String(u"Bell") },
    { SoundId::Chain, String(u"Chain") },
    { SoundId::Cymbal, String(u"Cymbal") },
    { SoundId::HiHat, String(u"HiHat") },
    { SoundId::Pipe, String(u"Pipe") },
    { SoundId::Castanet, String(u"Castanet") },
    { SoundId::TamTam, String(u"TamTam") },
    { SoundId::Thundersheet, String(u"Thundersheet") },
    { SoundId::Triangle, String(u"Triangle") },
    { SoundId::Claves, String(u"Claves") },
    { SoundId::Guiro, String(u"Guiro") },
    { SoundId::Block, String(u"Block") },
    { SoundId::Cabasa, String(u"Cabasa") },
    { SoundId::Maraca, String(u"Maraca") },
    { SoundId::Quijada, String(u"Quijada") },
    { SoundId::Ratchet, String(u"Ratchet") },
    { SoundId::Shaker, String(u"Shaker") },
    { SoundId::Stones, String(u"Stones") },
    { SoundId::Tambourine, String(u"Tambourine") },
    { SoundId::Tubo, String(u"Tubo") },
    { SoundId::Vibraslap, String(u"Vibraslap") },
    { SoundId::Whip, String(u"Whip") },
    { SoundId::Snap, String(u"Snap") },
    { SoundId::Clap, String(u"Clap") },
    { SoundId::Slap, String(u"Slap") },
    { SoundId::Stamp, String(u"Stamp") },
    { SoundId::Choir, String(u"Choir") },
    { SoundId::Kazoo, String(u"Kazoo") },
    { SoundId::Last, String(u"Last") }
};

inline const String& soundIdToString(const SoundId id)
{
    auto search = ID_STRINGS.find(id);
    if (search == ID_STRINGS.cend()) {
        return UNDEFINED_STR;
    }

    return search->second;
}

inline SoundId soundIdFromString(const String& str)
{
    auto search = std::find_if(ID_STRINGS.cbegin(),
                               ID_STRINGS.cend(),
                               [str](const auto& pair) { return pair.second == str; });

    if (search == ID_STRINGS.cend()) {
        return SoundId::Undefined;
    }

    return search->first;
}

static const std::unordered_map<SoundCategory, String> CATEGORY_STRINGS
{
    { SoundCategory::Keyboards, String(u"Keyboards") },
    { SoundCategory::Strings, String(u"Strings") },
    { SoundCategory::Winds, String(u"Winds") },
    { SoundCategory::Percussions, String(u"Percussions") },
    { SoundCategory::Voices, String(u"Voices") },
    { SoundCategory::Last, String(u"Last") }
};

inline const String& soundCategoryToString(const SoundCategory category)
{
    auto search = CATEGORY_STRINGS.find(category);
    if (search == CATEGORY_STRINGS.cend()) {
        return UNDEFINED_STR;
    }

    return search->second;
}

inline SoundCategory soundCategoryFromString(const String& str)
{
    auto search = std::find_if(CATEGORY_STRINGS.cbegin(),
                               CATEGORY_STRINGS.cend(),
                               [str](const auto& pair) { return pair.second == str; });

    if (search == CATEGORY_STRINGS.cend()) {
        return SoundCategory::Undefined;
    }

    return search->first;
}

static const std::unordered_map<SoundSubCategory, String> SUBCATEGORY_STRINGS
{
    { SoundSubCategory::English, String(u"English") },
    { SoundSubCategory::Armenian, String(u"Armenian") },
    { SoundSubCategory::Alpine, String(u"Alpine") },
    { SoundSubCategory::Australian, String(u"Australian") },
    { SoundSubCategory::Irish, String(u"Irish") },
    { SoundSubCategory::French, String(u"French") },
    { SoundSubCategory::Chinese, String(u"Chinese") },
    { SoundSubCategory::Vienna, String(u"Vienna") },
    { SoundSubCategory::Greek, String(u"Greek") },
    { SoundSubCategory::Japanese, String(u"Japanese") },
    { SoundSubCategory::Tibetan, String(u"Tibetan") },
    { SoundSubCategory::African, String(u"African") },
    { SoundSubCategory::Indian, String(u"Indian") },
    { SoundSubCategory::Spanish, String(u"Spanish") },
    { SoundSubCategory::Swedish, String(u"Swedish") },
    { SoundSubCategory::Hungarian, String(u"Hungarian") },
    { SoundSubCategory::Romanian, String(u"Romanian") },
    { SoundSubCategory::CentralEuropean, String(u"CentralEuropean") },

    { SoundSubCategory::Baroque, String(u"Baroque") },
    { SoundSubCategory::Classical, String(u"Classical") },
    { SoundSubCategory::Modern, String(u"Modern") },
    { SoundSubCategory::Orchestral, String(u"Orchestral") },

    { SoundSubCategory::Hammond, String(u"Hammond") },
    { SoundSubCategory::Wagner, String(u"Wagner") },
    { SoundSubCategory::Orff, String(u"Orff") },
    { SoundSubCategory::Huang, String(u"Huang") },
    { SoundSubCategory::Hohner, String(u"Hohner") },

    { SoundSubCategory::Percussive, String(u"Percussive") },
    { SoundSubCategory::Piped, String(u"Piped") },
    { SoundSubCategory::Rotary, String(u"Rotary") },
    { SoundSubCategory::Reed, String(u"Reed") },
    { SoundSubCategory::Foot, String(u"Foot") },
    { SoundSubCategory::Hand, String(u"Hand") },
    { SoundSubCategory::Finger, String(u"Finger") },
    { SoundSubCategory::Boy, String(u"Boy") },
    { SoundSubCategory::Girl, String(u"Girl") },
    { SoundSubCategory::Male, String(u"Male") },
    { SoundSubCategory::Female, String(u"Female") },
    { SoundSubCategory::Pad, String(u"Pad") },
    { SoundSubCategory::Plucked, String(u"Plucked") },

    { SoundSubCategory::Temple, String(u"Temple") },
    { SoundSubCategory::Military, String(u"Military") },
    { SoundSubCategory::Ride, String(u"Ride") },
    { SoundSubCategory::Sleigh, String(u"Sleigh") },
    { SoundSubCategory::Cow, String(u"Cow") },
    { SoundSubCategory::Marching, String(u"Marching") },

    { SoundSubCategory::Splash, String(u"Splash") },
    { SoundSubCategory::Crash, String(u"Crash") },
    { SoundSubCategory::Plate, String(u"Plate") },
    { SoundSubCategory::Bowl, String(u"Bowl") },
    { SoundSubCategory::Frame, String(u"Frame") },
    { SoundSubCategory::Slit, String(u"Slit") },
    { SoundSubCategory::Field, String(u"Field") },
    { SoundSubCategory::Snare, String(u"Snare") },
    { SoundSubCategory::Brake, String(u"Brake") },
    { SoundSubCategory::Slide, String(u"Slide") },
    { SoundSubCategory::Pocket, String(u"Pocket") },
    { SoundSubCategory::Garklein, String(u"Garklein") },
    { SoundSubCategory::Toy, String(u"Toy") },
    { SoundSubCategory::TwelveString, String(u"TwelveString") },

    { SoundSubCategory::Grand, String(u"Grand") },
    { SoundSubCategory::HonkyTonk, String(u"HonkyTonk") },
    { SoundSubCategory::Upright, String(u"Upright") },
    { SoundSubCategory::Prima, String(u"Prima") },
    { SoundSubCategory::Secunda, String(u"Secunda") },

    { SoundSubCategory::Electric, String(u"Electric") },
    { SoundSubCategory::Acoustic, String(u"Acoustic") },
    { SoundSubCategory::Fretless, String(u"Fretless") },
    { SoundSubCategory::Pedal, String(u"Pedal") },
    { SoundSubCategory::Steel, String(u"Steel") },
    { SoundSubCategory::Metal, String(u"Metal") },
    { SoundSubCategory::Iron, String(u"Iron") },
    { SoundSubCategory::Brass, String(u"Brass") },
    { SoundSubCategory::Tin, String(u"Tin") },
    { SoundSubCategory::Nylon, String(u"Nylon") },
    { SoundSubCategory::Wooden, String(u"Wooden") },
    { SoundSubCategory::Sandpaper, String(u"Sandpaper") },
    { SoundSubCategory::Glass, String(u"Glass") },
    { SoundSubCategory::Shell, String(u"Shell") },
    { SoundSubCategory::Wind, String(u"Wind") },

    { SoundSubCategory::Treble, String(u"Treble") },
    { SoundSubCategory::Diatonic, String(u"Diatonic") },
    { SoundSubCategory::Chromatic, String(u"Chromatic") },
    { SoundSubCategory::Octave, String(u"Octave") },

    { SoundSubCategory::Piccolo, String(u"Piccolo") },
    { SoundSubCategory::Alto, String(u"Alto") },
    { SoundSubCategory::Tenor, String(u"Tenor") },
    { SoundSubCategory::Baritone, String(u"Baritone") },
    { SoundSubCategory::Soprano, String(u"Soprano") },
    { SoundSubCategory::Mezzo_Soprano, String(u"Mezzo_Soprano") },
    { SoundSubCategory::Sopranino, String(u"Sopranino") },
    { SoundSubCategory::Sopranissimo, String(u"Sopranissimo") },
    { SoundSubCategory::Counter_Tenor, String(u"Counter_Tenor") },
    { SoundSubCategory::Contra, String(u"Contra") },
    { SoundSubCategory::Contra_Alto, String(u"Contra_Alto") },
    { SoundSubCategory::Sub_Contra_Alto, String(u"Sub_Contra_Alto") },
    { SoundSubCategory::Contra_Bass, String(u"Contra_Bass") },
    { SoundSubCategory::Sub_Contra_Bass, String(u"Sub_Contra_Bass") },
    { SoundSubCategory::Double_Contra_Bass, String(u"Double_Contra_Bass") },
    { SoundSubCategory::Bass, String(u"Bass") },
    { SoundSubCategory::Great_Bass, String(u"Great_Bass") },
    { SoundSubCategory::Hyper_Bass, String(u"Hyper_Bass") },
    { SoundSubCategory::Melody, String(u"Melody") },

    { SoundSubCategory::FX_Goblins, String(u"FX_Goblins") },
    { SoundSubCategory::FX_Atmosphere, String(u"FX_Atmosphere") },
    { SoundSubCategory::FX_Brightness, String(u"FX_Brightness") },
    { SoundSubCategory::FX_Crystal, String(u"FX_Crystal") },
    { SoundSubCategory::FX_Echoes, String(u"FX_Echoes") },
    { SoundSubCategory::FX_Rain, String(u"FX_Rain") },
    { SoundSubCategory::FX_SciFi, String(u"FX_SciFi") },
    { SoundSubCategory::FX_SoundTrack, String(u"FX_SoundTrack") },
    { SoundSubCategory::Last, String(u"Last") }
};

inline const String& soundSubCategoryToString(const SoundSubCategory& subCategory)
{
    auto search = SUBCATEGORY_STRINGS.find(subCategory);
    if (search == SUBCATEGORY_STRINGS.cend()) {
        return UNDEFINED_STR;
    }

    return search->second;
}

inline SoundSubCategory soundSubCategoryFromString(const String& str)
{
    auto search = std::find_if(SUBCATEGORY_STRINGS.cbegin(),
                               SUBCATEGORY_STRINGS.cend(),
                               [str](const auto& pair) { return pair.second == str; });

    if (search == SUBCATEGORY_STRINGS.cend()) {
        return SoundSubCategory::Undefined;
    }

    return search->first;
}

using SoundCategories = std::set<SoundCategory>;
struct SoundSubCategories : public std::set<SoundSubCategory>
{
    SoundSubCategories() = default;
    SoundSubCategories(std::initializer_list<SoundSubCategory> initList)
        : std::set<SoundSubCategory>(std::move(initList))
    {
    }

    String toString() const
    {
        StringList subCategoryStrList;
        for (const auto& subCategory : *this) {
            subCategoryStrList.push_back(soundSubCategoryToString(subCategory));
        }

        return subCategoryStrList.join(u",");
    }

    static SoundSubCategories fromString(const String& str)
    {
        SoundSubCategories result;

        StringList subCategoryStrList = str.split(u",");
        for (const String& subStr : subCategoryStrList) {
            result.insert(soundSubCategoryFromString(subStr));
        }

        return result;
    }
};
}

#endif // MU_MPE_SOUNDID_H
