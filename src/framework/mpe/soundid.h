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

static const std::unordered_map<SoundId, String> ID_STRINGS
{
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
        static const String UNDEFINED_ID_STR(u"undefined_id");
        return UNDEFINED_ID_STR;
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
    { SoundSubCategory::Primary, String(u"Primary") },
    { SoundSubCategory::Secondary, String(u"Secondary") },
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
