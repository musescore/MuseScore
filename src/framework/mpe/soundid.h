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

#ifndef MUSE_MPE_SOUNDID_H
#define MUSE_MPE_SOUNDID_H

#include <set>
#include <unordered_map>

#include "types/string.h"

namespace muse::mpe {
enum class SoundId
{
    Undefined = -1,
    Unknown,

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
    Viola,
    ViolaDaGamba,
    Violoncello,
    Viol,
    PardessusViol,
    Baryton,
    Violone,
    Nyckelharpa,
    Erhu,
    Contrabass,
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
    BellTree,
    Bell,
    Chain,
    Cymbal,
    HiHat,
    Pipe,
    MarkTree,
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
    Taiko,
    Metronome,

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
    Unknown,

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
    Electronic,
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
    Soft,

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

    Primary,
    Secondary,
    Section,

    In_C,
    In_D,
    In_D_flat,
    In_E,
    In_E_flat,
    In_F,
    In_G,
    In_G_flat,
    In_A,
    In_A_flat,
    In_B,
    In_B_flat,

    Last
};

inline const std::unordered_map<SoundId, String> ID_STRINGS
{
    { SoundId::Undefined, String(u"undefined") },
    { SoundId::Unknown, String(u"unknown") },
    { SoundId::Accordion, String(u"accordion") },
    { SoundId::Bandoneon, String(u"bandoneon") },
    { SoundId::Concertina, String(u"concertina") },
    { SoundId::Harmonica, String(u"harmonica") },
    { SoundId::Melodica, String(u"melodica") },
    { SoundId::Sheng, String(u"sheng") },
    { SoundId::Celesta, String(u"celesta") },
    { SoundId::Clavichord, String(u"clavichord") },
    { SoundId::Harpsichord, String(u"harpsichord") },
    { SoundId::Virginal, String(u"virginal") },
    { SoundId::Piano, String(u"piano") },
    { SoundId::Organ, String(u"organ") },
    { SoundId::Harmonium, String(u"harmonium") },
    { SoundId::Synthesizer, String(u"synthesizer") },
    { SoundId::Theremin, String(u"theremin") },
    { SoundId::OndesMartenot, String(u"ondes_martenot") },

    { SoundId::Harp, String(u"harp") },
    { SoundId::Cavaquinho, String(u"cavaquinho") },
    { SoundId::Guitar, String(u"guitar") },
    { SoundId::BassGuitar, String(u"bass_guitar") },
    { SoundId::Banjo, String(u"banjo") },
    { SoundId::Ukulele, String(u"ukulele") },
    { SoundId::Mandolin, String(u"mandolin") },
    { SoundId::MtnDulcimer, String(u"mtn_dulcimer") },
    { SoundId::Lute, String(u"lute") },
    { SoundId::Theorbo, String(u"theorbo") },
    { SoundId::Archlute, String(u"archlute") },
    { SoundId::Balalaika, String(u"balalaika") },
    { SoundId::Koto, String(u"koto") },
    { SoundId::Oud, String(u"oud") },
    { SoundId::Shamisen, String(u"shamisen") },
    { SoundId::Sitar, String(u"sitar") },
    { SoundId::Prim, String(u"prim") },
    { SoundId::Brac, String(u"brac") },
    { SoundId::Bugarija, String(u"bugarija") },
    { SoundId::Berda, String(u"berda") },
    { SoundId::Celo, String(u"celo") },
    { SoundId::Bandurria, String(u"bandurria") },
    { SoundId::Laud, String(u"laud") },
    { SoundId::StringsGroup, String(u"strings_group") },
    { SoundId::Violin, String(u"violin") },
    { SoundId::Viola, String(u"viola") },
    { SoundId::ViolaDaGamba, String(u"viola_dagamba") },
    { SoundId::Violoncello, String(u"violoncello") },
    { SoundId::Viol, String(u"viol") },
    { SoundId::PardessusViol, String(u"pardessus_viol") },
    { SoundId::Baryton, String(u"varyton") },
    { SoundId::Violone, String(u"violone") },
    { SoundId::Nyckelharpa, String(u"nyckelharpa") },
    { SoundId::Erhu, String(u"erhu") },
    { SoundId::Contrabass, String(u"contrabass") },
    { SoundId::Octobass, String(u"octobass") },

    { SoundId::WindsGroup, String(u"winds_group") },
    { SoundId::Piccolo, String(u"piccolo") },
    { SoundId::Heckelphone, String(u"heckelphone") },
    { SoundId::HeckelphoneClarinet, String(u"heckelphone_clarinet") },
    { SoundId::Oboe, String(u"oboe") },
    { SoundId::Lupophone, String(u"lupophone") },
    { SoundId::Flute, String(u"flute") },
    { SoundId::PanFlute, String(u"pan_flute") },
    { SoundId::Danso, String(u"danso") },
    { SoundId::Traverso, String(u"traverso") },
    { SoundId::Dizi, String(u"dizi") },
    { SoundId::Shakuhachi, String(u"shakuhachi") },
    { SoundId::Fife, String(u"fife") },
    { SoundId::Whistle, String(u"whistle") },
    { SoundId::Flageolet, String(u"flageolet") },
    { SoundId::Recorder, String(u"recorder") },
    { SoundId::Ocarina, String(u"ocarina") },
    { SoundId::Gemshorn, String(u"gemshorn") },
    { SoundId::Quena, String(u"quena") },
    { SoundId::Horn, String(u"horn") },
    { SoundId::Bassethorn, String(u"bassethorn") },
    { SoundId::Shawm, String(u"shawm") },
    { SoundId::Cromorne, String(u"cromorne") },
    { SoundId::Crumhorn, String(u"crumhorn") },
    { SoundId::Cornamuse, String(u"cornamuse") },
    { SoundId::Kelhorn, String(u"kelhorn") },
    { SoundId::Rauschpfeife, String(u"rauschpfeife") },
    { SoundId::Duduk, String(u"duduk") },
    { SoundId::Shenai, String(u"shenai") },
    { SoundId::Clarinet, String(u"clarinet") },
    { SoundId::Chalumeau, String(u"chalumeau") },
    { SoundId::Xaphoon, String(u"xaphoon") },
    { SoundId::Tarogato, String(u"tarogato") },
    { SoundId::Octavin, String(u"octavin") },
    { SoundId::Saxophone, String(u"saxophone") },
    { SoundId::Aulochrome, String(u"aulochrome") },
    { SoundId::Bassoon, String(u"bassoon") },
    { SoundId::Contrabassoon, String(u"contrabassoon") },
    { SoundId::Dulcian, String(u"dulcian") },
    { SoundId::Rackett, String(u"rackett") },
    { SoundId::Sarrusophone, String(u"sarrusophone") },
    { SoundId::Bagpipe, String(u"bagpipe") },
    { SoundId::Tuba, String(u"tuba") },
    { SoundId::Cornet, String(u"cornet") },
    { SoundId::Posthorn, String(u"posthorn") },
    { SoundId::BrassGroup, String(u"brass_group") },
    { SoundId::Trumpet, String(u"trumpet") },
    { SoundId::Bugle, String(u"bugle") },
    { SoundId::MellophoneBugle, String(u"mellophone_bugle") },
    { SoundId::EuphoniumBugle, String(u"euphonium_bugle") },
    { SoundId::Euphonium, String(u"euphonium") },
    { SoundId::Fiscorn, String(u"fiscorn") },
    { SoundId::Flugelhorn, String(u"flugelhorn") },
    { SoundId::Kuhlohorn, String(u"kuhlohorn") },
    { SoundId::Ophicleide, String(u"ophicleide") },
    { SoundId::Cornettino, String(u"cornettino") },
    { SoundId::Cornett, String(u"cornett") },
    { SoundId::Serpent, String(u"serpent") },
    { SoundId::Trombone, String(u"trombone") },
    { SoundId::Cimbasso, String(u"cimbasso") },
    { SoundId::Sackbut, String(u"sackbut") },
    { SoundId::Sousaphone, String(u"sousaphone") },
    { SoundId::Helicon, String(u"helicon") },
    { SoundId::Conch, String(u"conch") },
    { SoundId::Saxhorn, String(u"saxhorn") },
    { SoundId::Horagai, String(u"horagai") },
    { SoundId::Alphorn, String(u"alphorn") },
    { SoundId::RagDung, String(u"rag_dung") },
    { SoundId::Didgeridoo, String(u"didgeridoo") },
    { SoundId::Shofar, String(u"shofar") },
    { SoundId::Vuvuzela, String(u"vuvuzela") },

    { SoundId::Timpani, String(u"timpani") },
    { SoundId::RotoToms, String(u"roto_toms") },
    { SoundId::Tubaphone, String(u"tubaphone") },
    { SoundId::SteelDrums, String(u"steel_drums") },
    { SoundId::Glockenspiel, String(u"glockenspiel") },
    { SoundId::Xylophone, String(u"xylophone") },
    { SoundId::Xylomarimba, String(u"xylomarimba") },
    { SoundId::Vibraphone, String(u"vibraphone") },
    { SoundId::Dulcimer, String(u"dulcimer") },
    { SoundId::Cimbalom, String(u"cimbalom") },
    { SoundId::Marimba, String(u"marimba") },
    { SoundId::Crotales, String(u"crotales") },
    { SoundId::Chimes, String(u"chimes") },
    { SoundId::Carillon, String(u"carillon") },
    { SoundId::Gong, String(u"gong") },
    { SoundId::Metallophone, String(u"metallophone") },
    { SoundId::Flexatone, String(u"flexatone") },
    { SoundId::MusicalSaw, String(u"musical_saw") },
    { SoundId::MusicalGlasses, String(u"musical_glasses") },
    { SoundId::KlaxonHorns, String(u"klaxon_horns") },
    { SoundId::Kalimba, String(u"kalimba") },
    { SoundId::Bongos, String(u"bongos") },
    { SoundId::TomToms, String(u"tom_toms") },
    { SoundId::Conga, String(u"conga") },
    { SoundId::Cuica, String(u"cuica") },
    { SoundId::Drumset, String(u"drumset") },
    { SoundId::Drum, String(u"drum") },
    { SoundId::Tablas, String(u"tablas") },
    { SoundId::Timbales, String(u"timbales") },
    { SoundId::Anvil, String(u"anvil") },
    { SoundId::BellTree, String(u"bell_tree") },
    { SoundId::Bell, String(u"bell") },
    { SoundId::Chain, String(u"chain") },
    { SoundId::Cymbal, String(u"cymbal") },
    { SoundId::HiHat, String(u"hi_hat") },
    { SoundId::Pipe, String(u"pipe") },
    { SoundId::MarkTree, String(u"mark_tree") },
    { SoundId::Castanet, String(u"castanet") },
    { SoundId::TamTam, String(u"tam_tam") },
    { SoundId::Thundersheet, String(u"thunder_sheet") },
    { SoundId::Triangle, String(u"triangle") },
    { SoundId::Claves, String(u"claves") },
    { SoundId::Guiro, String(u"guiro") },
    { SoundId::Block, String(u"block") },
    { SoundId::Cabasa, String(u"cabasa") },
    { SoundId::Maraca, String(u"maraca") },
    { SoundId::Quijada, String(u"quijada") },
    { SoundId::Ratchet, String(u"ratchet") },
    { SoundId::Shaker, String(u"shaker") },
    { SoundId::Stones, String(u"stones") },
    { SoundId::Tambourine, String(u"tambourine") },
    { SoundId::Tubo, String(u"tubo") },
    { SoundId::Vibraslap, String(u"vibraslap") },
    { SoundId::Whip, String(u"whip") },
    { SoundId::Snap, String(u"snap") },
    { SoundId::Clap, String(u"clap") },
    { SoundId::Slap, String(u"slap") },
    { SoundId::Stamp, String(u"stamp") },
    { SoundId::Choir, String(u"choir") },
    { SoundId::Kazoo, String(u"kazoo") },
    { SoundId::Taiko, String(u"taiko") },
    { SoundId::Metronome, String(u"metronome") },
    { SoundId::Last, String(u"last") }
};

inline const String& soundIdToString(const SoundId id)
{
    auto search = ID_STRINGS.find(id);
    if (search == ID_STRINGS.cend()) {
        return ID_STRINGS.at(SoundId::Unknown);
    }

    return search->second;
}

inline SoundId soundIdFromString(const String& str)
{
    if (str.empty()) {
        return SoundId::Undefined;
    }

    auto search = std::find_if(ID_STRINGS.cbegin(),
                               ID_STRINGS.cend(),
                               [str](const auto& pair) { return pair.second == str; });

    if (search == ID_STRINGS.cend()) {
        return SoundId::Unknown;
    }

    return search->first;
}

inline const std::unordered_map<SoundCategory, String> CATEGORY_STRINGS
{
    { SoundCategory::Keyboards, String(u"keyboards") },
    { SoundCategory::Strings, String(u"strings") },
    { SoundCategory::Winds, String(u"winds") },
    { SoundCategory::Percussions, String(u"percussions") },
    { SoundCategory::Voices, String(u"voices") },
    { SoundCategory::Last, String(u"last") }
};

inline const String& soundCategoryToString(const SoundCategory category)
{
    auto search = CATEGORY_STRINGS.find(category);
    if (search == CATEGORY_STRINGS.cend()) {
        static const String UNDEFINED_SUBCATEGORY_STR(u"undefined");
        return UNDEFINED_SUBCATEGORY_STR;
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

inline const std::unordered_map<SoundSubCategory, String> SUBCATEGORY_STRINGS
{
    { SoundSubCategory::Undefined, String(u"undefined") },
    { SoundSubCategory::Unknown, String(u"unknown") },
    { SoundSubCategory::English, String(u"english") },
    { SoundSubCategory::Armenian, String(u"armenian") },
    { SoundSubCategory::Alpine, String(u"alpine") },
    { SoundSubCategory::Australian, String(u"australian") },
    { SoundSubCategory::Irish, String(u"irish") },
    { SoundSubCategory::French, String(u"french") },
    { SoundSubCategory::Chinese, String(u"chinese") },
    { SoundSubCategory::Vienna, String(u"vienna") },
    { SoundSubCategory::Greek, String(u"greek") },
    { SoundSubCategory::Japanese, String(u"japanese") },
    { SoundSubCategory::Tibetan, String(u"tibetan") },
    { SoundSubCategory::African, String(u"african") },
    { SoundSubCategory::Indian, String(u"indian") },
    { SoundSubCategory::Spanish, String(u"spanish") },
    { SoundSubCategory::Swedish, String(u"swedish") },
    { SoundSubCategory::Hungarian, String(u"hungarian") },
    { SoundSubCategory::Romanian, String(u"romanian") },
    { SoundSubCategory::CentralEuropean, String(u"central_european") },

    { SoundSubCategory::Baroque, String(u"baroque") },
    { SoundSubCategory::Classical, String(u"classical") },
    { SoundSubCategory::Modern, String(u"modern") },
    { SoundSubCategory::Orchestral, String(u"orchestral") },

    { SoundSubCategory::Hammond, String(u"hammond") },
    { SoundSubCategory::Wagner, String(u"wagner") },
    { SoundSubCategory::Orff, String(u"orff") },
    { SoundSubCategory::Huang, String(u"huang") },
    { SoundSubCategory::Hohner, String(u"hohner") },

    { SoundSubCategory::Percussive, String(u"percussive") },
    { SoundSubCategory::Piped, String(u"piped") },
    { SoundSubCategory::Rotary, String(u"rotary") },
    { SoundSubCategory::Reed, String(u"reed") },
    { SoundSubCategory::Foot, String(u"foot") },
    { SoundSubCategory::Hand, String(u"hand") },
    { SoundSubCategory::Finger, String(u"finger") },
    { SoundSubCategory::Boy, String(u"boy") },
    { SoundSubCategory::Girl, String(u"girl") },
    { SoundSubCategory::Male, String(u"male") },
    { SoundSubCategory::Female, String(u"female") },
    { SoundSubCategory::Pad, String(u"pad") },
    { SoundSubCategory::Plucked, String(u"plucked") },

    { SoundSubCategory::Temple, String(u"temple") },
    { SoundSubCategory::Military, String(u"military") },
    { SoundSubCategory::Ride, String(u"ride") },
    { SoundSubCategory::Sleigh, String(u"sleigh") },
    { SoundSubCategory::Cow, String(u"cow") },
    { SoundSubCategory::Marching, String(u"marching") },

    { SoundSubCategory::Splash, String(u"splash") },
    { SoundSubCategory::Crash, String(u"crash") },
    { SoundSubCategory::Plate, String(u"plate") },
    { SoundSubCategory::Bowl, String(u"bowl") },
    { SoundSubCategory::Frame, String(u"frame") },
    { SoundSubCategory::Slit, String(u"slit") },
    { SoundSubCategory::Field, String(u"field") },
    { SoundSubCategory::Snare, String(u"snare") },
    { SoundSubCategory::Brake, String(u"brake") },
    { SoundSubCategory::Slide, String(u"slide") },
    { SoundSubCategory::Pocket, String(u"pocket") },
    { SoundSubCategory::Garklein, String(u"garklein") },
    { SoundSubCategory::Toy, String(u"toy") },
    { SoundSubCategory::String, String(u"string") },
    { SoundSubCategory::TwelveString, String(u"twelve_string") },

    { SoundSubCategory::Grand, String(u"grand") },
    { SoundSubCategory::HonkyTonk, String(u"honky_tonk") },
    { SoundSubCategory::Upright, String(u"upright") },
    { SoundSubCategory::Prima, String(u"prima") },
    { SoundSubCategory::Secunda, String(u"secunda") },

    { SoundSubCategory::Electric, String(u"electric") },
    { SoundSubCategory::Electronic, String(u"electronic") },
    { SoundSubCategory::Acoustic, String(u"acoustic") },
    { SoundSubCategory::Fretless, String(u"fretless") },
    { SoundSubCategory::Pedal, String(u"pedal") },
    { SoundSubCategory::Steel, String(u"steel") },
    { SoundSubCategory::Metal, String(u"metal") },
    { SoundSubCategory::Iron, String(u"iron") },
    { SoundSubCategory::Brass, String(u"brass") },
    { SoundSubCategory::Tin, String(u"tin") },
    { SoundSubCategory::Nylon, String(u"nylon") },
    { SoundSubCategory::Wooden, String(u"wooden") },
    { SoundSubCategory::Sandpaper, String(u"sandpaper") },
    { SoundSubCategory::Glass, String(u"glass") },
    { SoundSubCategory::Shell, String(u"shell") },
    { SoundSubCategory::Wind, String(u"wind") },
    { SoundSubCategory::Soft, String(u"soft") },

    { SoundSubCategory::Treble, String(u"treble") },
    { SoundSubCategory::Diatonic, String(u"diatonic") },
    { SoundSubCategory::Chromatic, String(u"chromatic") },
    { SoundSubCategory::Octave, String(u"octave") },

    { SoundSubCategory::Piccolo, String(u"piccolo") },
    { SoundSubCategory::Alto, String(u"alto") },
    { SoundSubCategory::Tenor, String(u"tenor") },
    { SoundSubCategory::Baritone, String(u"baritone") },
    { SoundSubCategory::Soprano, String(u"soprano") },
    { SoundSubCategory::Mezzo_Soprano, String(u"mezzo_soprano") },
    { SoundSubCategory::Sopranino, String(u"sopranino") },
    { SoundSubCategory::Sopranissimo, String(u"sopranissimo") },
    { SoundSubCategory::Counter_Tenor, String(u"counter_tenor") },
    { SoundSubCategory::Contra, String(u"contra") },
    { SoundSubCategory::Contra_Alto, String(u"contra_alto") },
    { SoundSubCategory::Sub_Contra_Alto, String(u"sub_contra_alto") },
    { SoundSubCategory::Contra_Bass, String(u"contra_bass") },
    { SoundSubCategory::Sub_Contra_Bass, String(u"sub_contra_bass") },
    { SoundSubCategory::Double_Contra_Bass, String(u"double_contra_bass") },
    { SoundSubCategory::Bass, String(u"bass") },
    { SoundSubCategory::Bowed, String(u"bowed") },
    { SoundSubCategory::Great_Bass, String(u"great_bass") },
    { SoundSubCategory::Hyper_Bass, String(u"hyper_bass") },
    { SoundSubCategory::Melody, String(u"melody") },

    { SoundSubCategory::FX_Goblins, String(u"fx_goblins") },
    { SoundSubCategory::FX_Atmosphere, String(u"fx_atmosphere") },
    { SoundSubCategory::FX_Brightness, String(u"fx_brightness") },
    { SoundSubCategory::FX_Crystal, String(u"fx_crystal") },
    { SoundSubCategory::FX_Echoes, String(u"fx_echoes") },
    { SoundSubCategory::FX_Rain, String(u"fx_rain") },
    { SoundSubCategory::FX_SciFi, String(u"fx_scifi") },
    { SoundSubCategory::FX_SoundTrack, String(u"fx_soundtrack") },

    { SoundSubCategory::Sweep, String(u"sweep") },
    { SoundSubCategory::Warm, String(u"warm") },
    { SoundSubCategory::NewAge, String(u"new_age") },
    { SoundSubCategory::Polysynth, String(u"polysynth") },
    { SoundSubCategory::Halo, String(u"halo") },
    { SoundSubCategory::Metallic, String(u"metallic") },
    { SoundSubCategory::Choir, String(u"choir") },

    { SoundSubCategory::Sine_Wave, String(u"sine_wave") },
    { SoundSubCategory::Square_Wave, String(u"square_wave") },
    { SoundSubCategory::Sawtooth_Wave, String(u"sawtooth_wave") },

    { SoundSubCategory::Primary, String(u"primary") },
    { SoundSubCategory::Secondary, String(u"secondary") },
    { SoundSubCategory::Section, String(u"section") },

    { SoundSubCategory::In_C, String(u"in_c") },
    { SoundSubCategory::In_D, String(u"in_d") },
    { SoundSubCategory::In_D_flat, String(u"in_d_flat") },
    { SoundSubCategory::In_E, String(u"in_e") },
    { SoundSubCategory::In_E_flat, String(u"in_e_flat") },
    { SoundSubCategory::In_F, String(u"in_f") },
    { SoundSubCategory::In_G, String(u"in_g") },
    { SoundSubCategory::In_G_flat, String(u"in_g_flat") },
    { SoundSubCategory::In_A, String(u"in_a") },
    { SoundSubCategory::In_A_flat, String(u"in_a_flat") },
    { SoundSubCategory::In_B, String(u"in_b") },
    { SoundSubCategory::In_B_flat, String(u"in_b_flat") },

    { SoundSubCategory::Last, String(u"last") }
};

using SoundSubCategories = std::set<SoundSubCategory>;

inline const String& soundSubCategoryToString(const SoundSubCategory& subCategory)
{
    auto search = SUBCATEGORY_STRINGS.find(subCategory);
    if (search == SUBCATEGORY_STRINGS.cend()) {
        return SUBCATEGORY_STRINGS.at(SoundSubCategory::Unknown);
    }

    return search->second;
}

inline SoundSubCategory soundSubCategoryFromString(const String& str)
{
    if (str.empty()) {
        return SoundSubCategory::Undefined;
    }

    auto search = std::find_if(SUBCATEGORY_STRINGS.cbegin(),
                               SUBCATEGORY_STRINGS.cend(),
                               [str](const auto& pair) { return pair.second == str; });

    if (search == SUBCATEGORY_STRINGS.cend()) {
        return SoundSubCategory::Unknown;
    }

    return search->first;
}
}

#endif // MUSE_MPE_SOUNDID_H
