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
#include "importenigmaxml.h"
#include "dom/sig.h"

#include <zlib.h>
#include <vector>
#include <exception>
#include <string_view>

#include <QFile>
#include <QFileInfo>

#include "musx/musx.h"

#include "global/io/file.h"
#include "global/serialization/zipreader.h"

#include "types/string.h"

#include "engraving/dom/box.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/utils.h"
#include "engraving/engravingerrors.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

static size_t readGzipUncompressedSize(const ByteArray& gzDataInput)
{
    if (gzDataInput.size() < 18) { // Must be at least 18 bytes
        return 0;
    }

    const uint8_t* p = reinterpret_cast<const uint8_t*>(gzDataInput.constData());
    size_t n = gzDataInput.size();

    uint32_t isize = static_cast<uint32_t>(p[n - 4])
                     | (static_cast<uint32_t>(p[n - 3]) << 8)
                     | (static_cast<uint32_t>(p[n - 2]) << 16)
                     | (static_cast<uint32_t>(p[n - 1]) << 24);
    return static_cast<size_t>(isize);
}

static bool gunzipBuffer(const ByteArray& gzDataInput, ByteArray& output)
{
    constexpr size_t chunkSize = 262144; // 256 KB chunks
    output.clear();
    output.reserve(readGzipUncompressedSize(gzDataInput));

    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(gzDataInput.constData());
    stream.avail_in = static_cast<uInt>(gzDataInput.size());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        LOGE() << "inflateInit2 failed";
        return false;
    }

    int ret;
    do {
        size_t oldSize = output.size();
        output.resize(oldSize + chunkSize);
        stream.next_out = output.data() + oldSize;
        stream.avail_out = static_cast<uInt>(chunkSize);

        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            LOGE() << "inflate failed";
            inflateEnd(&stream);
            return false;
        }
    } while (ret != Z_STREAM_END);

    output.resize(output.size() - stream.avail_out); // Trim extra
    inflateEnd(&stream);
    return true;
}

//---------------------------------------------------------
//   importEnigmaXmlfromBuffer
//---------------------------------------------------------

Err importEnigmaXmlfromBuffer(Score* score, ByteArray&& data)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::qt::Document>(data.constChar(), data.size());

    data.clear(); // free up data now that it isn't needed

    EnigmaXmlImporter importer(score, doc);
    importer.import();

    score->setUpTempoMap(); //??
    return engraving::Err::NoError;
}

//---------------------------------------------------------
//   extractScoreFile
//---------------------------------------------------------

/**
Extract EnigmaXml from compressed musx file \a name, return true if OK and false on error.
*/

static bool extractScoreFile(const String& name, ByteArray& data)
{
    ZipReader zip(name);
    data.clear();

    ByteArray gzipData = zip.fileData("score.dat");
    if (gzipData.empty()) {
        LOGE() << "no EngimaXML found: " << name;
        return false;
    }

    constexpr static uint32_t INITIAL_STATE = 0x28006D45; // arbitrary initial value for algorithm
    constexpr static uint32_t RESET_LIMIT = 0x20000; // reset value corresponding (probably) to an internal Finale buffer size

    uint32_t state = INITIAL_STATE;
    for (size_t i = 0; i < gzipData.size(); i++) {
        if (i % RESET_LIMIT == 0) {
            state = INITIAL_STATE;
        }
        // this algorithm is BSD rand()!
        state = state * 0x41c64e6d + 0x3039;
        uint16_t upper = state >> 16;
        uint8_t c = uint8_t(upper + upper / 255);
        gzipData[i] ^= c;
    }

    if (!gunzipBuffer(gzipData, data)) {
        LOGE() << "unable to extract Enigmaxml from file: " << name;
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   importMusx
//---------------------------------------------------------

Err importMusx(MasterScore* score, const QString& name)
{

    if (!io::File::exists(name)) {
        return Err::FileNotFound;
    }

    // extract the root file
    ByteArray data;
    if (!extractScoreFile(name, data)) {
        return Err::FileBadFormat;      // appropriate error message has been printed by extractScoreFile
    }

    return importEnigmaXmlfromBuffer(score, std::move(data));
}

//---------------------------------------------------------
//   importEnigmaXml
//---------------------------------------------------------

Err importEnigmaXml(MasterScore* score, const QString& name)
{
    io::File xmlFile(name);
    if (!xmlFile.exists()) {
        return Err::FileNotFound;
    }

    if (!xmlFile.open(io::IODevice::ReadOnly)) {
        LOGE() << "could not open EngimaXML file: " << name;
        return Err::FileOpenError;
    }

    ByteArray data = xmlFile.readAll();
    xmlFile.close();

    return importEnigmaXmlfromBuffer(score, std::move(data));
}

static ClefType clefTypeFromClefType(ClefIndex clef)
{
    // For now, base this on the default clef definitions.
    // A future todo could be to infer the clef from the actual
    // clef definition record in the Musx document's clef options.

    switch (DefaultClefType(clef)) {
    case DefaultClefType::Treble:         return ClefType::G;
    case DefaultClefType::Alto:           return ClefType::C3;
    case DefaultClefType::Tenor:          return ClefType::C4;
    case DefaultClefType::Bass:           return ClefType::F;
    case DefaultClefType::Percussion:     return ClefType::PERC2;
    case DefaultClefType::Treble8vb:      return ClefType::G8_VB;
    case DefaultClefType::Bass8vb:        return ClefType::F8_VB;
    case DefaultClefType::Baritone:       return ClefType::F_B;
    case DefaultClefType::FrenchViolin:   return ClefType::G_1;
    case DefaultClefType::BaritoneC:      return ClefType::C5;
    case DefaultClefType::MezzoSoprano:   return ClefType::C2;
    case DefaultClefType::Soprano:        return ClefType::C1;
    case DefaultClefType::AltPercussion:  return ClefType::PERC;
    case DefaultClefType::Treble8va:      return ClefType::G8_VA;
    case DefaultClefType::Bass8va:        return ClefType::F_8VA;
    case DefaultClefType::Blank:          return ClefType::INVALID;
    case DefaultClefType::Tab1:
    case DefaultClefType::Tab2:           return ClefType::TAB;
    default:                              return ClefType::INVALID;
    }
}

static std::optional<ClefTypeList> clefTypeListFromMusxStaff(const std::shared_ptr<const others::Staff> musxStaff)
{
    ClefType concertClef = clefTypeFromClefType(musxStaff->calcFirstClefIndex());
    ClefType transposeClef = concertClef;
    if (musxStaff->transposition && musxStaff->transposition->setToClef) {
        transposeClef = clefTypeFromClefType(musxStaff->transposedClef);
    }
    if (concertClef == ClefType::INVALID || transposeClef == ClefType::INVALID) {
        return std::nullopt;
    }
    return ClefTypeList(concertClef, transposeClef);
}

static String instrTemplateIdfromUuid(std::string uuid)
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
        { uuid::Piccolo,                   u"flute" },
        { uuid::Flute,                     u"flute" },
        { uuid::AltoFlute,                 u"flute" },
        { uuid::Oboe,                      u"flute" },
        { uuid::OboeDAmore,                u"flute" },
        { uuid::EnglishHorn,               u"flute" },
        { uuid::ClarinetBFlat,             u"flute" },
        { uuid::ClarinetA,                 u"flute" },
        { uuid::ClarinetEFlat,             u"flute" },
        { uuid::AltoClarinet,              u"flute" },
        { uuid::ContraltoClarinet,         u"flute" },
        { uuid::BassClarinet,              u"flute" },
        { uuid::ContrabassClarinet,        u"flute" },
        { uuid::Bassoon,                   u"flute" },
        { uuid::Contrabassoon,             u"flute" },
        { uuid::WindSection,               u"flute" },
        { uuid::SopranoSax,                u"flute" },
        { uuid::AltoSax,                   u"flute" },
        { uuid::TenorSax,                  u"flute" },
        { uuid::BaritoneSax,               u"flute" },
        { uuid::SopranoRecorder,           u"flute" },
        { uuid::SopraninoRecorder,         u"flute" },
        { uuid::AltoRecorder,              u"flute" },
        { uuid::TenorRecorder,             u"flute" },
        { uuid::BassRecorder,              u"flute" },
        { uuid::DescantRecorder,           u"flute" },
        { uuid::Ocarina,                   u"flute" },
        { uuid::PennyWhistle,              u"flute" },
        { uuid::PennyWhistleD,             u"flute" },
        { uuid::PennyWhistleG,             u"flute" },
        { uuid::LowIrishWhistle,           u"flute" },
        { uuid::TinWhistleBFlat,           u"flute" },
        { uuid::Harmonica,                 u"flute" },
        { uuid::BassHarmonica,             u"flute" },
        { uuid::Concertina,                u"flute" },
        { uuid::Bandoneon,                 u"flute" },
        { uuid::HornF_WWQuintet,           u"flute" },
        { uuid::Bagpipes,                  u"flute" },
        { uuid::UilleannPipes,             u"flute" },
        { uuid::GaidaPipes,                u"flute" },
        { uuid::ContraAltoFlute,           u"flute" },
        { uuid::BassFlute,                 u"flute" },
        { uuid::ContrabassFlute,           u"flute" },
        { uuid::DoubleContrabassFlute,     u"flute" },
        { uuid::HyperbassFlute,            u"flute" },
        { uuid::PanPipes,                  u"flute" },
        { uuid::Fife,                      u"flute" },
        { uuid::BottleBlow,                u"flute" },
        { uuid::Jug,                       u"flute" },
        { uuid::PiccoloOboe,               u"flute" },
        { uuid::PiccoloHeckelphone,        u"flute" },
        { uuid::Heckelphone,               u"flute" },
        { uuid::BassOboe,                  u"flute" },
        { uuid::BassetClarinet,            u"flute" },
        { uuid::BassetHorn,                u"flute" },
        { uuid::Hornpipe,                  u"flute" },
        { uuid::PiccoloClarinet,           u"flute" },
        { uuid::Saxonette,                 u"flute" },
        { uuid::SopraninoSax,              u"flute" },
        { uuid::MezzoSopranoSax,           u"flute" },
        { uuid::Sopranino,                 u"flute" },
        { uuid::CMelodySax,                u"flute" },
        { uuid::Aulochrome,                u"flute" },
        { uuid::Xaphoon,                   u"flute" },
        { uuid::BassSax,                   u"flute" },
        { uuid::ContrabassSax,             u"flute" },
        { uuid::SubContrabassSax,          u"flute" },
        { uuid::Tubax,                     u"flute" },
        { uuid::Bansuri,                   u"flute" },
        { uuid::Danso,                     u"flute" },
        { uuid::Dizi,                      u"flute" },
        { uuid::DilliKaval,                u"flute" },
        { uuid::Diple,                     u"flute" },
        { uuid::DoubleFlute,               u"flute" },
        { uuid::Dvojnice,                  u"flute" },
        { uuid::DvojniceDrone,             u"flute" },
        { uuid::Flageolet,                 u"flute" },
        { uuid::Fujara,                    u"flute" },
        { uuid::Gemshorn,                  u"flute" },
        { uuid::Hocchiku,                  u"flute" },
        { uuid::Hun,                       u"flute" },
        { uuid::IrishFlute,                u"flute" },
        { uuid::Kaval,                     u"flute" },
        { uuid::Khlui,                     u"flute" },
        { uuid::KnotweedFlute,             u"flute" },
        { uuid::KoncovkaAltoFlute,         u"flute" },
        { uuid::Koudi,                     u"flute" },
        { uuid::Ney,                       u"flute" },
        { uuid::Nohkan,                    u"flute" },
        { uuid::NoseFlute,                 u"flute" },
        { uuid::Palendag,                  u"flute" },
        { uuid::Quena,                     u"flute" },
        { uuid::Ryuteki,                   u"flute" },
        { uuid::Shakuhachi,                u"flute" },
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
        { uuid::AltoCrumhorn,              u"flute" },
        { uuid::Arghul,                    u"flute" },
        { uuid::Bawu,                      u"flute" },
        { uuid::Chalumeau,                 u"flute" },
        { uuid::ClarinetteDAmour,          u"flute" },
        { uuid::Cornamuse,                 u"flute" },
        { uuid::Diplica,                   u"flute" },
        { uuid::DoubleClarinet,            u"flute" },
        { uuid::HeckelClarina,             u"flute" },
        { uuid::HeckelphoneClarinet,       u"flute" },
        { uuid::Hirtenschalmei,            u"flute" },
        { uuid::Launeddas,                 u"flute" },
        { uuid::Maqrunah,                  u"flute" },
        { uuid::Mijwiz,                    u"flute" },
        { uuid::Octavin,                   u"flute" },
        { uuid::Pibgorn,                   u"flute" },
        { uuid::Rauschpfeife,              u"flute" },
        { uuid::Sipsi,                     u"flute" },
        { uuid::ModernTarogato,            u"flute" },
        { uuid::TenorCrumhorn,             u"flute" },
        { uuid::Zhaleika,                  u"flute" },
        { uuid::Algaita,                   u"flute" },
        { uuid::Bifora,                    u"flute" },
        { uuid::Bombarde,                  u"flute" },
        { uuid::Cromorne,                  u"flute" },
        { uuid::Duduk,                     u"flute" },
        { uuid::Dulcian,                   u"flute" },
        { uuid::Dulzaina,                  u"flute" },
        { uuid::Guan,                      u"flute" },
        { uuid::Guanzi,                    u"flute" },
        { uuid::Hichiriki,                 u"flute" },
        { uuid::Hne,                       u"flute" },
        { uuid::JogiBaja,                  u"flute" },
        { uuid::KenBau,                    u"flute" },
        { uuid::Mizmar,                    u"flute" },
        { uuid::Nadaswaram,                u"flute" },
        { uuid::OboeDaCaccia,              u"flute" },
        { uuid::Pi,                        u"flute" },
        { uuid::Piri,                      u"flute" },
        { uuid::PungiSnakeCharmer,         u"flute" },
        { uuid::Rackett,                   u"flute" },
        { uuid::ReedContrabass,            u"flute" },
        { uuid::Rhaita,                    u"flute" },
        { uuid::Rothphone,                 u"flute" },
        { uuid::Sarrusophone,              u"flute" },
        { uuid::Shawm,                     u"flute" },
        { uuid::Shehnai,                   u"flute" },
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
        { uuid::Sheng,                     u"flute" },

        // Brass
        { uuid::TrumpetBFlat,              u"trumpet" },
        { uuid::TrumpetC,                  u"trumpet" },
        { uuid::TrumpetD,                  u"trumpet" },
        { uuid::Cornet,                    u"trumpet" },
        { uuid::Flugelhorn,                u"trumpet" },
        { uuid::Mellophone,                u"trumpet" },
        { uuid::HornF,                     u"trumpet" },
        { uuid::Trombone,                  u"trumpet" },
        { uuid::BassTrombone,              u"trumpet" },
        { uuid::Euphonium,                 u"trumpet" },
        { uuid::BaritoneBC,                u"trumpet" },
        { uuid::BaritoneTC,                u"trumpet" },
        { uuid::Tuba,                      u"trumpet" },
        { uuid::BassTuba,                  u"trumpet" },
        { uuid::Sousaphone,                u"trumpet" },
        { uuid::BrassSection,              u"trumpet" },
        { uuid::PiccoloTrumpetA,           u"trumpet" },
        { uuid::Bugle,                     u"trumpet" },
        { uuid::CornetEFlat,               u"trumpet" },
        { uuid::HornEFlat,                 u"trumpet" },
        { uuid::AltoTrombone,              u"trumpet" },
        { uuid::TenorTrombone,             u"trumpet" },
        { uuid::ContrabassTrombone,        u"trumpet" },
        { uuid::Alphorn,                   u"trumpet" },
        { uuid::AltoHorn,                  u"trumpet" },
        { uuid::Didgeridoo,                u"trumpet" },
        { uuid::PostHorn,                  u"trumpet" },
        { uuid::ViennaHorn,                u"trumpet" },
        { uuid::WagnerTuba,                u"trumpet" },
        { uuid::BaroqueTrumpet,            u"trumpet" },
        { uuid::BassTrumpet,               u"trumpet" },
        { uuid::Cornetto,                  u"trumpet" },
        { uuid::Fiscorn,                   u"trumpet" },
        { uuid::Kuhlohorn,                 u"trumpet" },
        { uuid::PocketTrumpet,             u"trumpet" },
        { uuid::Saxhorn,                   u"trumpet" },
        { uuid::SlideTrumpet,              u"trumpet" },
        { uuid::Cimbasso,                  u"trumpet" },
        { uuid::DoubleBellEuphonium,       u"trumpet" },
        { uuid::Sackbut,                   u"trumpet" },
        { uuid::Helicon,                   u"trumpet" },
        { uuid::Ophicleide,                u"trumpet" },
        { uuid::Serpent,                   u"trumpet" },
        { uuid::SubContrabassTuba,         u"trumpet" },
        { uuid::ConchShell,                u"trumpet" },
        { uuid::Horagai,                   u"trumpet" },
        { uuid::Shofar,                    u"trumpet" },
        { uuid::Vuvuzela,                  u"trumpet" },

        // Plucked Strings
        { uuid::Harp,                      u"guitar-steel" },
        { uuid::TroubadorHarp,             u"guitar-steel" },
        { uuid::Guitar,                    u"guitar-steel" },
        { uuid::Guitar8vb,                 u"guitar-steel" },
        { uuid::AcousticGuitar,            u"guitar-steel" },
        { uuid::ClassicalGuitar,           u"guitar-steel" },
        { uuid::ElectricGuitar,            u"guitar-steel" },
        { uuid::SteelGuitar,               u"guitar-steel" },
        { uuid::Banjo,                     u"guitar-steel" },
        { uuid::TenorBanjo,                u"guitar-steel" },
        { uuid::AcousticBass,              u"guitar-steel" },
        { uuid::BassGuitar,                u"guitar-steel" },
        { uuid::ElectricBass,              u"guitar-steel" },
        { uuid::FretlessBass,              u"guitar-steel" },
        { uuid::StringBass,                u"guitar-steel" },
        { uuid::Mandolin,                  u"guitar-steel" },
        { uuid::Dulcimer,                  u"guitar-steel" },
        { uuid::HammeredDulcimer,          u"guitar-steel" },
        { uuid::Dulcimer8vb,               u"guitar-steel" },
        { uuid::Autoharp,                  u"guitar-steel" },
        { uuid::Lute,                      u"guitar-steel" },
        { uuid::Ukulele,                   u"guitar-steel" },
        { uuid::TenorUkulele,              u"guitar-steel" },
        { uuid::Sitar,                     u"guitar-steel" },
        { uuid::Zither,                    u"guitar-steel" },
        { uuid::Archlute,                  u"guitar-steel" },
        { uuid::Baglama,                   u"guitar-steel" },
        { uuid::Balalaika,                 u"guitar-steel" },
        { uuid::Bandura,                   u"guitar-steel" },
        { uuid::Banjolele,                 u"guitar-steel" },
        { uuid::Barbat,                    u"guitar-steel" },
        { uuid::Begena,                    u"guitar-steel" },
        { uuid::Biwa,                      u"guitar-steel" },
        { uuid::Bolon,                     u"guitar-steel" },
        { uuid::Bordonua,                  u"guitar-steel" },
        { uuid::Bouzouki,                  u"guitar-steel" },
        { uuid::BulgarianTambura,          u"guitar-steel" },
        { uuid::ChapmanStick,              u"guitar-steel" },
        { uuid::Charango,                  u"guitar-steel" },
        { uuid::ChitarraBattente,          u"guitar-steel" },
        { uuid::ChaozhouGuzheng,           u"guitar-steel" },
        { uuid::Cimbalom,                  u"guitar-steel" },
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
        { uuid::Koto,                      u"guitar-steel" },
        { uuid::Kutiyapi,                  u"guitar-steel" },
        { uuid::Langeleik,                 u"guitar-steel" },
        { uuid::Lyre,                      u"guitar-steel" },
        { uuid::MandoBass,                 u"guitar-steel" },
        { uuid::MandoCello,                u"guitar-steel" },
        { uuid::Mandola,                   u"guitar-steel" },
        { uuid::Mandora,                   u"guitar-steel" },
        { uuid::Mandore,                   u"guitar-steel" },
        { uuid::Mangbetu,                  u"guitar-steel" },
        { uuid::Marovany,                  u"guitar-steel" },
        { uuid::MohanVeena,                u"guitar-steel" },
        { uuid::MoodSwinger,               u"guitar-steel" },
        { uuid::MusicalBow,                u"guitar-steel" },
        { uuid::Ngoni,                     u"guitar-steel" },
        { uuid::OctaveMandolin,            u"guitar-steel" },
        { uuid::Oud,                       u"guitar-steel" },
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
        { uuid::Shamisen,                  u"guitar-steel" },
        { uuid::Tambura,                   u"guitar-steel" },
        { uuid::TarPlucked,                u"guitar-steel" },
        { uuid::Theorbo,                   u"guitar-steel" },
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
        { uuid::TabGuitar,                 u"guitar-steel" },
        { uuid::TabGuitarNoName,           u"guitar-steel" },
        { uuid::TabGuitarStems,            u"guitar-steel" },
        { uuid::TabGuitarD,                u"guitar-steel" },
        { uuid::TabGuitarDADGAD,           u"guitar-steel" },
        { uuid::TabGuitarDoubled,          u"guitar-steel" },
        { uuid::TabGuitarDropD,            u"guitar-steel" },
        { uuid::TabGuitarG,                u"guitar-steel" },
        { uuid::TabGuitar7String,          u"guitar-steel" },
        { uuid::TabBanjoG,                 u"guitar-steel" },
        { uuid::TabTenorBanjo,             u"guitar-steel" },
        { uuid::TabBanjoC,                 u"guitar-steel" },
        { uuid::TabBanjoD,                 u"guitar-steel" },
        { uuid::TabBanjoDoubleC,           u"guitar-steel" },
        { uuid::TabBanjoGModal,            u"guitar-steel" },
        { uuid::TabBanjoPlectrum,          u"guitar-steel" },
        { uuid::TabBassGuitar4,            u"guitar-steel" },
        { uuid::TabBassGuitar5,            u"guitar-steel" },
        { uuid::TabBassGuitar6,            u"guitar-steel" },
        { uuid::TabDulcimerDAA,            u"guitar-steel" },
        { uuid::TabDulcimerDAAUnison,      u"guitar-steel" },
        { uuid::TabDulcimerDAD,            u"guitar-steel" },
        { uuid::TabGamba,                  u"guitar-steel" },
        { uuid::TabLuteItalian,            u"guitar-steel" },
        { uuid::TabLuteLetters,            u"guitar-steel" },
        { uuid::TabMandolin,               u"guitar-steel" },
        { uuid::TabRequinto,               u"guitar-steel" },
        { uuid::TabSitarShankar,           u"guitar-steel" },
        { uuid::TabSitarKhan,              u"guitar-steel" },
        { uuid::TabUkulele,                u"guitar-steel" },
        { uuid::TabVihuela,                u"guitar-steel" },

        // Pitched Percussion
        { uuid::Timpani,                   u"piano" },
        { uuid::Mallets,                   u"piano" },
        { uuid::Bells,                     u"piano" },
        { uuid::Chimes,                    u"piano" },
        { uuid::Crotales,                  u"piano" },
        { uuid::Glockenspiel,              u"piano" },
        { uuid::SopranoGlockenspiel,       u"piano" },
        { uuid::AltoGlockenspiel,          u"piano" },
        { uuid::Marimba,                   u"piano" },
        { uuid::BassMarimba,               u"piano" },
        { uuid::MarimbaSingleStaff,        u"piano" },
        { uuid::TubularBells,              u"piano" },
        { uuid::Vibraphone,                u"piano" },
        { uuid::Xylophone,                 u"piano" },
        { uuid::SopranoXylophone,          u"piano" },
        { uuid::AltoXylophone,             u"piano" },
        { uuid::BassXylophone,             u"piano" },
        { uuid::Xylorimba,                 u"piano" },
        { uuid::BellLyre,                  u"piano" },
        { uuid::Boomwhackers,              u"piano" },
        { uuid::ChromanotesInstruments,    u"piano" },
        { uuid::Carillon,                  u"piano" },
        { uuid::CrystalGlasses,            u"piano" },
        { uuid::FlexatonePitched,          u"piano" },
        { uuid::GlassHarmonica,            u"piano" },
        { uuid::GlassMarimba,              u"piano" },
        { uuid::Handbells,                 u"piano" },
        { uuid::HandbellsTClef,            u"piano" },
        { uuid::HandbellsBClef,            u"piano" },
        { uuid::HangTClef,                 u"piano" },
        { uuid::JawHarp,                   u"piano" },
        { uuid::Kalimba,                   u"piano" },
        { uuid::SopranoMetallophone,       u"piano" },
        { uuid::AltoMetallophone,          u"piano" },
        { uuid::BassMetallophone,          u"piano" },
        { uuid::MusicalSaw,                u"piano" },
        { uuid::SlideWhistle,              u"piano" },
        { uuid::SteelDrumsTClef,           u"piano" },
        { uuid::SteelDrumsBClef,           u"piano" },
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
        { uuid::Almglocken,                u"piano" },
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
        { uuid::TaikoDrums,                u"piano" },
        { uuid::TempleBells,               u"piano" },
        { uuid::TibetanBells,              u"piano" },
        { uuid::TibetanSingingBowls,       u"piano" },

        // Drums
        { uuid::SnareDrum,                 u"snare-drum" },
        { uuid::BassDrum,                  u"snare-drum" },
        { uuid::DrumSet,                   u"snare-drum" },
        { uuid::TenorDrum,                 u"snare-drum" },
        { uuid::QuadToms,                  u"snare-drum" },
        { uuid::QuintToms,                 u"snare-drum" },
        { uuid::RotoToms,                  u"snare-drum" },
        { uuid::TenorLine,                 u"snare-drum" },
        { uuid::SnareLine,                 u"snare-drum" },
        { uuid::BassDrums5Line,            u"snare-drum" },
        { uuid::Djembe,                    u"snare-drum" },
        { uuid::BongoDrums,                u"snare-drum" },
        { uuid::CongaDrums,                u"snare-drum" },
        { uuid::LogDrum,                   u"snare-drum" },
        { uuid::Tablas,                    u"snare-drum" },
        { uuid::Timbales,                  u"snare-drum" },
        { uuid::AfricanLogDrum,            u"snare-drum" },
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
        { uuid::Dumbek,                    u"snare-drum" },
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
        { uuid::Janggu,                    u"snare-drum" },
        { uuid::Kakko,                     u"snare-drum" },
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
        { uuid::NativeLogDrum,             u"snare-drum" },
        { uuid::NigerianLogDrum,           u"snare-drum" },
        { uuid::Odaiko,                    u"snare-drum" },
        { uuid::Okawa,                     u"snare-drum" },
        { uuid::OkedoDodaiko,              u"snare-drum" },
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
        { uuid::ShimeDaiko,                u"snare-drum" },
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
        { uuid::Tsuzumi,                   u"snare-drum" },
        { uuid::UchiwaDaiko,               u"snare-drum" },
        { uuid::Udaku,                     u"snare-drum" },
        { uuid::Zarb,                      u"snare-drum" },

        // Percussion
        { uuid::PercussionGeneral,         u"percussion" },
        { uuid::PercAccessories,           u"percussion" },
        { uuid::WindChimes,                u"percussion" },
        { uuid::ChimeTree,                 u"percussion" },
        { uuid::BellTree,                  u"percussion" },
        { uuid::JingleBells,               u"percussion" },
        { uuid::Tambourine,                u"percussion" },
        { uuid::Triangle,                  u"percussion" },
        { uuid::Cymbals,                   u"percussion" },
        { uuid::FingerCymbals,             u"percussion" },
        { uuid::CrashCymbal,               u"percussion" },
        { uuid::HiHatCymbal,               u"percussion" },
        { uuid::RideCymbal,                u"percussion" },
        { uuid::SplashCymbal,              u"percussion" },
        { uuid::TamTam,                    u"percussion" },
        { uuid::Gong,                      u"percussion" },
        { uuid::AgogoBells,                u"percussion" },
        { uuid::AirHorn,                   u"percussion" },
        { uuid::BrakeDrum,                 u"percussion" },
        { uuid::Cabasa,                    u"percussion" },
        { uuid::Cajon,                     u"percussion" },
        { uuid::Castanets,                 u"percussion" },
        { uuid::Clap,                      u"percussion" },
        { uuid::Clapper,                   u"percussion" },
        { uuid::Claves,                    u"percussion" },
        { uuid::Cowbell,                   u"percussion" },
        { uuid::Cuica,                     u"percussion" },
        { uuid::Guiro,                     u"percussion" },
        { uuid::Maracas,                   u"percussion" },
        { uuid::PoliceWhistle,             u"percussion" },
        { uuid::Rainstick,                 u"percussion" },
        { uuid::Ratchet,                   u"percussion" },
        { uuid::Rattle,                    u"percussion" },
        { uuid::SandBlock,                 u"percussion" },
        { uuid::Shakers,                   u"percussion" },
        { uuid::Spoons,                    u"percussion" },
        { uuid::TempleBlocks,              u"percussion" },
        { uuid::Vibraslap,                 u"percussion" },
        { uuid::Washboard,                 u"percussion" },
        { uuid::Whip,                      u"percussion" },
        { uuid::WindMachine,               u"percussion" },
        { uuid::WoodBlocks,                u"percussion" },
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
        { uuid::Kkwaenggwari,              u"percussion" },
        { uuid::Kpokopoko,                 u"percussion" },
        { uuid::KrinSlitDrum,              u"percussion" },
        { uuid::LavaStones,                u"percussion" },
        { uuid::LuoGong,                   u"percussion" },
        { uuid::Manjeera,                  u"percussion" },
        { uuid::PanClappers,               u"percussion" },
        { uuid::Patschen,                  u"percussion" },
        { uuid::RattleCog,                 u"percussion" },
        { uuid::Riq,                       u"percussion" },
        { uuid::Shekere,                   u"percussion" },
        { uuid::Sistre,                    u"percussion" },
        { uuid::Sistrum,                   u"percussion" },
        { uuid::SlideWhistlePercClef,      u"percussion" },
        { uuid::SlitDrum,                  u"percussion" },
        { uuid::Snap,                      u"percussion" },
        { uuid::Stamp,                     u"percussion" },
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

static std::string trimNewLineFromString(const std::string& src)
{
    size_t pos = src.find('\n');
    if (pos != std::string::npos) {
        return src.substr(0, pos);  // Truncate at the newline, excluding it
    }
    return src;
}

Staff* EnigmaXmlImporter::createStaff(Part* part, const std::shared_ptr<const others::Staff> musxStaff, const InstrumentTemplate* it)
{
    Staff* s = Factory::createStaff(part);

    // todo: staff settings can change at any tick
    Fraction eventTick{0, 1};

    // initialise MuseScore's default values
    if (it) {
        s->init(it, 0, 0);
    }
    /// @todo This staffLines setting will move to wherever we parse staff styles
    /// @todo Need to intialize the staff type from presets?

    // # of staff lines (only load if custom)
    if (musxStaff->staffLines.has_value()) {
        if (musxStaff->staffLines.value() != s->lines(eventTick)) {
            s->setLines(eventTick, musxStaff->staffLines.value());
        }
    } else if (musxStaff->customStaff.has_value()) {
        int customStaffSize = static_cast<int>(musxStaff->customStaff.value().size());
        if (customStaffSize != s->lines(eventTick)) {
            s->setLines(eventTick, customStaffSize);
        }
    }

    // barline vertical offsets relative to staff
    auto calcBarlineOffsetHalfSpaces = [](Evpu offset, int numLines, bool forTop) -> int {
        if (numLines == 1) {
            // This works for single-line staves. Needs more testing for other non-standard scenarios.
            if (forTop) {
                offset -= 48;
            } else {
                offset += 48;
            }
        }
        double halfSpaces = (double(offset) * 2.0) / EVPU_PER_SPACE;
        return int(std::lround(halfSpaces));
    };
    s->setBarLineFrom(calcBarlineOffsetHalfSpaces(musxStaff->topBarlineOffset, s->lines(eventTick), true));
    s->setBarLineTo(calcBarlineOffsetHalfSpaces(musxStaff->botBarlineOffset, s->lines(eventTick), false));

    // hide when empty
    s->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);

    // clefs
    if (auto defaultClefs = clefTypeListFromMusxStaff(musxStaff)) {
        s->setDefaultClefType(defaultClefs.value());
    } else {
        s->staffType(eventTick)->setGenClef(false);
    }
    m_staff2Inst.emplace(m_score->nstaves(), InstCmper(musxStaff->getCmper()));
    m_inst2Staff.emplace(InstCmper(musxStaff->getCmper()), m_score->nstaves());
    m_score->appendStaff(s);
    return s;
}

void EnigmaXmlImporter::import()
{
    importParts();
    importBrackets();
    importMeasures();
}

static Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig)
{
    auto [count, noteType] = simpleMusxTimeSig;
    if (count.remainder()) {
        if ((Edu(noteType) % count.denominator()) == 0) {
            noteType = musx::dom::NoteType(Edu(noteType) / count.denominator());
            count *= count.denominator();
        } else {
            LOGE() << "Time signature has fractional portion that could not be reduced.";
            return Fraction(4, 4);
        }
    }
    return Fraction(count.quotient(),  musx::util::Fraction::fromEdu(Edu(noteType)).denominator());
}

void EnigmaXmlImporter::importMeasures()
{
    // add default time signature
    Fraction currTimeSig = Fraction(4, 4);
    m_score->sigmap()->clear();
    m_score->sigmap()->add(0, currTimeSig);

    auto musxMeasures = m_doc->getOthers()->getArray<others::Measure>(SCORE_PARTID);
    int counter = 0; // DBG
    for (const auto& musxMeasure : musxMeasures) {
        Fraction tick{ 0, 1 };
        auto lastMeasure = m_score->measures()->last();
        if (lastMeasure) {
            tick = lastMeasure->tick() + lastMeasure->ticks();
        }

        Measure* measure = Factory::createMeasure(m_score->dummy()->system());
        measure->setTick(tick);
        /// @todo eventually we need to import all the TimeSig features we can. Right now it's just the simplified case.
        auto musxTimeSig = musxMeasure->createTimeSignature()->calcSimplified();
        auto scoreTimeSig = simpleMusxTimeSigToFraction(musxTimeSig);
        if (scoreTimeSig != currTimeSig) {
            m_score->sigmap()->add(tick.ticks(), scoreTimeSig);
            currTimeSig = scoreTimeSig;
        }
        measure->setTick(tick);
        measure->setTimesig(scoreTimeSig);
        measure->setTicks(scoreTimeSig);
        m_score->measures()->add(measure);

        // for now, add a full measure rest to each staff for the measure.
        for (mu::engraving::Staff* staff : m_score->staves()) {
            mu::engraving::staff_idx_t staffIdx = staff->idx();
            mu::engraving::Segment* restSeg = measure->getSegment(mu::engraving::SegmentType::ChordRest, tick);
            Rest* rest = mu::engraving::Factory::createRest(restSeg, mu::engraving::TDuration(mu::engraving::DurationType::V_MEASURE));
            rest->setScore(m_score);
            rest->setTicks(measure->ticks());
            rest->setTrack(staffIdx * VOICES);
            restSeg->add(rest);
        }
        if (++counter >= 100) break; // DBG
    }

    /// @todo maybe move this to separate function
    const TimeSigMap& sigmap = *m_score->sigmap();

    for (auto is = sigmap.cbegin(); is != sigmap.cend(); ++is) {
        const SigEvent& se = is->second;
        const int tick = is->first;
        Measure* m = m_score->tick2measure(Fraction::fromTicks(tick));
        if (!m) {
            continue;
        }
        Fraction newTimeSig = se.timesig();
        for (staff_idx_t staffIdx = 0; staffIdx < m_score->nstaves(); ++staffIdx) {
            Segment* seg = m->getSegment(SegmentType::TimeSig, Fraction::fromTicks(tick));
            TimeSig* ts = Factory::createTimeSig(seg);
            ts->setSig(newTimeSig);
            ts->setTrack(static_cast<int>(staffIdx) * VOICES);
            seg->add(ts);
        }
        if (newTimeSig != se.timesig()) {     // was a pickup measure - skip next timesig
            ++is;
        }
    }
}

void EnigmaXmlImporter::importParts()
{
    auto scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(SCORE_PARTID, BASE_SYSTEM_ID);

    int partNumber = 0;
    for (const auto& item : scrollView) {
        auto staff = item->getStaff();
        IF_ASSERT_FAILED(staff) {
            continue; // safety check
        }
        auto compositeStaff = others::StaffComposite::createCurrent(m_doc, SCORE_PARTID, staff->getCmper(), 1, 0);
        IF_ASSERT_FAILED(compositeStaff) {
            continue; // safety check
        }

        auto multiStaffInst = staff->getMultiStaffInstGroup();
        if (multiStaffInst && m_inst2Part.find(staff->getCmper()) != m_inst2Part.end()) {
            continue;
        }

        Part* part = new Part(m_score);

        QString id = QString("P%1").arg(++partNumber);
        part->setId(id);

        // names of part
        auto fullBaseName = staff->getFullInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        if (!fullBaseName.empty()) {
            part->setPartName(QString::fromStdString(trimNewLineFromString(fullBaseName)));
        }
        auto fullEffectiveName = compositeStaff->getFullInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        if (!fullEffectiveName.empty()) {
            part->setLongName(QString::fromStdString(trimNewLineFromString(fullEffectiveName)));
        }
        auto abrvName = compositeStaff->getAbbreviatedInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        if (!abrvName.empty()) {
            part->setShortName(QString::fromStdString(trimNewLineFromString(abrvName)));
        }

        // load default part settings
        // to-do: overwrite most of these settings later
        const InstrumentTemplate* it = searchTemplate(instrTemplateIdfromUuid(compositeStaff->instUuid));
        if (it) {
            part->initFromInstrTemplate(it);
        }

        if (multiStaffInst) {
            m_part2Inst.emplace(id, multiStaffInst->staffNums);
            for (auto inst : multiStaffInst->staffNums) {
                if (auto instStaff = others::StaffComposite::createCurrent(m_doc, SCORE_PARTID, inst, 1, 0)) {
                    createStaff(part, instStaff, it);
                    m_inst2Part.emplace(inst, id);
                }
            }
        } else {
            createStaff(part, compositeStaff, it);
            m_part2Inst.emplace(id, std::vector<InstCmper>({ InstCmper(staff->getCmper()) }));
            m_inst2Part.emplace(staff->getCmper(), id);
        }
        m_score->appendPart(part);
    }
}

void EnigmaXmlImporter::importBrackets()
{
    struct StaffGroupLayer
    {
        details::StaffGroupInfo info;
        int layer{};
    };

    auto computeStaffGroupLayers = [](std::vector<details::StaffGroupInfo> groups) -> std::vector<StaffGroupLayer> {
        const size_t n = groups.size();
        std::vector<StaffGroupLayer> result;
        result.reserve(n);

        for (auto& g : groups)
            result.push_back({ std::move(g), 0 });

        for (size_t i = 0; i < n; ++i) {
            const auto& gi = result[i].info;
            if (!gi.startSlot || !gi.endSlot)
                continue;

            for (size_t j = 0; j < n; ++j) {
                if (i == j) continue;
                const auto& gj = result[j].info;
                if (!gj.startSlot || !gj.endSlot)
                    continue;

                if (*gi.startSlot >= *gj.startSlot && *gi.endSlot <= *gj.endSlot &&
                    (*gi.startSlot > *gj.startSlot || *gi.endSlot < *gj.endSlot)) {
                    result[i].layer = std::max(result[i].layer, result[j].layer + 1);
                }
            }
        }

        std::sort(result.begin(), result.end(), [](const StaffGroupLayer& a, const StaffGroupLayer& b) {
            if (a.layer != b.layer)
                return a.layer < b.layer;
            if (!a.info.startSlot || !b.info.startSlot)
                return static_cast<bool>(b.info.startSlot);
            return *a.info.startSlot < *b.info.startSlot;
        });

        return result;
    };

    auto toMuseScoreBracketType = [](details::StaffGroup::BracketStyle style) -> BracketType {
        using MusxBracketStyle = details::StaffGroup::BracketStyle;
        switch (style) {
        case MusxBracketStyle::None:                  return BracketType::NO_BRACKET;
        case MusxBracketStyle::ThickLine:             return BracketType::LINE;
        case MusxBracketStyle::BracketStraightHooks:  return BracketType::NORMAL;
        case MusxBracketStyle::PianoBrace:            return BracketType::BRACE;
        case MusxBracketStyle::BracketCurvedHooks:    return BracketType::NORMAL;
        case MusxBracketStyle::DeskBracket:           return BracketType::SQUARE;
        default:                                      return BracketType::NO_BRACKET;
        }
    };

    auto scorePartInfo = m_doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, SCORE_PARTID);
    if (!scorePartInfo) {
        LOGE() << "Unable to read PartDefinition for score";
        return;
    }
    auto scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(SCORE_PARTID, BASE_SYSTEM_ID);

    auto staffGroups = details::StaffGroupInfo::getGroupsAtMeasure(1, scorePartInfo, scrollView);
    auto groupsByLayer = computeStaffGroupLayers(staffGroups);
    for (const auto& groupInfo : groupsByLayer) {
        IF_ASSERT_FAILED(groupInfo.info.startSlot && groupInfo.info.endSlot) {
            LOGE() << "Group info encountered without start or end slot information";
            continue;
        }
        auto musxStartStaff = others::InstrumentUsed::getStaffAtIndex(scrollView, groupInfo.info.startSlot.value());
        auto musxEndStaff = others::InstrumentUsed::getStaffAtIndex(scrollView, groupInfo.info.endSlot.value());
        IF_ASSERT_FAILED(musxStartStaff && musxEndStaff) {
            LOGE() << "Group info encountered without start or end slot information";
            continue;
        }
        auto getStaffIdx = [&](InstCmper inst) -> std::optional<size_t> {
            auto it = m_inst2Staff.find(inst);
            IF_ASSERT_FAILED_X(it != m_inst2Staff.end(), "Musx inst value not found in m_inst2Staff") {
                return std::nullopt;
            }
            return it->second;
        };
        auto staffIdx = getStaffIdx(musxStartStaff->getCmper());
        IF_ASSERT_FAILED(staffIdx) {
            continue;
        }
        staff_idx_t startStaffIdx = staffIdx.value();
        BracketItem* bi = Factory::createBracketItem(m_score->dummy());
        bi->setBracketType(toMuseScoreBracketType(groupInfo.info.group->bracket->style));
        int groupSpan = groupInfo.info.endSlot.value() - groupInfo.info.startSlot.value() + 1;
        bi->setBracketSpan(groupSpan);
        bi->setColumn(size_t(groupInfo.layer));
        m_score->staff(startStaffIdx)->addBracket(bi);
        if (groupInfo.info.group->drawBarlines == details::StaffGroup::DrawBarlineStyle::ThroughStaves) {
            for (staff_idx_t idx = startStaffIdx; idx < startStaffIdx + groupSpan - 1; idx++) {
                m_score->staff(idx)->setBarLineSpan(true);
            }
        }
    }
}

}
