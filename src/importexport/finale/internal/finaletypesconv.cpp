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

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/accidental.h"
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

TDuration noteInfoToDuration(std::pair<musx::dom::NoteType, unsigned> noteInfo)
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

bool isValidUuid(std::string uuid) {
    return uuid != uuid::BlankStaff && uuid != uuid::Unknown;
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
        { uuid::TenorDrum,                 u"snare-drum" },
        { uuid::QuadToms,                  u"tom-toms" },
        { uuid::QuintToms,                 u"tom-toms" },
        { uuid::RotoToms,                  u"roto-toms" },
        { uuid::TenorLine,                 u"snare-drum" },
        { uuid::SnareLine,                 u"marching-snareline" },
        { uuid::BassDrums5Line,            u"snare-drum" },
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

AlignH toAlignH(others::MeasureNumberRegion::AlignJustify align)
{
    static const std::unordered_map<others::MeasureNumberRegion::AlignJustify, AlignH> hAlignTable = {
        { others::MeasureNumberRegion::AlignJustify::Left,   AlignH::LEFT },
        { others::MeasureNumberRegion::AlignJustify::Center, AlignH::HCENTER },
        { others::MeasureNumberRegion::AlignJustify::Right,  AlignH::RIGHT },
    };
    return muse::value(hAlignTable, align, AlignH::LEFT);
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

Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig, FinaleLoggerPtr& logger)
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
    return Fraction(count.quotient(),  musx::util::Fraction::fromEdu(Edu(noteType)).denominator());
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
    AccidentalType at = Accidental::value2subtype(AccidentalVal(acciAmount));
    return at != AccidentalType::NONE ? Accidental::subtype2symbol(at) : SymId::noSym;
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
    // Finale uses music font size 24 to fill a space.
    // MuseScore uses music font size 20 to fill a space.
    // This scaling carries over to any font setting whose font size scales with spatium.
    constexpr static double MUSE_FINALE_SCALE_DIFFERENTIAL = 20.0 / 24.0;

    return double(fontInfo->fontSize) * (fontInfo->absolute ? 1.0 : MUSE_FINALE_SCALE_DIFFERENTIAL);
}

Spatium absoluteSpatium(double value, EngravingItem* e)
{
    // Returns global spatium value adjusted to preserve value for element scaling
    // Use style .spatium value or .defaultSpatium ??? or SPATIUM20??
    return Spatium(value * e->score()->style().defaultSpatium() / e->spatium());
}

Spatium absoluteSpatiumFromEvpu(Evpu evpu, EngravingItem* e)
{
    return absoluteSpatium(doubleFromEvpu(evpu), e);
}

}
