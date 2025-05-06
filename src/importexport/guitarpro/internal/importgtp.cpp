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

#include "importgtp.h"

#include <cmath>

#include "importptb.h"
#include "internal/guitarprodrumset.h"

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/bend.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurebase.h"
#include "engraving/dom/note.h"
#include "engraving/dom/notedot.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/volta.h"
#include "engraving/rw/xmlwriter.h"
#include "engraving/types/symid.h"

#include "log.h"

using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::iex::guitarpro {
//---------------------------------------------------------
//   errmsg
//---------------------------------------------------------

const char* const GuitarPro::errmsg[] = {
    "no error",
    "unknown file format",
    "unexpected end of file",
    "bad number of strings",
};

#ifdef _MSC_VER
#pragma optimize("", off)
#endif

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

GuitarPro::GuitarPro(MasterScore* s, int v, const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
    score   = s;
    version = v;
    voltaSequence = 1;
    tempo = -1;
}

GuitarPro::~GuitarPro()
{
    delete[] slurs;
}

//---------------------------------------------------------
//   skip
//---------------------------------------------------------

void GuitarPro::skip(int64_t len)
{
    f->seek(f->pos() + len);
    /*char c;
    while (len--)
          read(&c, 1);*/
}

//---------------------------------------------------------
//   createTuningString
//---------------------------------------------------------
static const std::unordered_map<uint64_t, std::string> flatPresets = {
    // 63 58 54 49 44 39
    { 0x3f3a36312c27, { "Eb Ab Db Gb Bb Eb" } },
    // 60 55 51 46 41 36
    { 0x3c37332e2924, { "C F Bb Eb G C" } },
    // 63 58 54 49 44 37
    { 0x3f3a36312c25, { "Db Ab Db Gb Bb Eb" } },
};

bool GuitarPro::createTuningString(int strings, int tuning[])
{
    bool useFlats = false;
    const char* tune[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    std::vector<int> pitch;
    uint64_t k = 0;
    for (int i = 0; i < strings; ++i) {
        pitch.push_back(tuning[i]);
        k |= (uint64_t)tuning[i] << 8 * i;
    }
    if (auto preset = flatPresets.find(k); preset != flatPresets.end()) {
        tunings.push_back(preset->second);
        useFlats = true;
        return useFlats;
    }
    std::string t;
    for (auto i : pitch) {
        t += tune[i % 12];
        t += " ";
    }
    tunings.push_back(t);
    return useFlats;
}

void GuitarPro::initDynamics(size_t stavesNum)
{
    previousDynamicByTrack.resize(stavesNum * mu::engraving::VOICES);
    std::fill(previousDynamicByTrack.begin(), previousDynamicByTrack.end(), INVALID_DYNAMIC);
}

//---------------------------------------------------------
//    read
//---------------------------------------------------------

void GuitarPro::read(void* p, int64_t len)
{
    if (len == 0) {
        return;
    }
    int64_t rv = f->read((uint8_t*)p, len);
    if (rv != len) {
        assert(rv == len);     //to have assert in debug and no warnings from AppVeyor in release
    }
    curPos += len;
}

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

int GuitarPro::readChar()
{
    signed char c;
    read(&c, 1);
    return c;
}

//---------------------------------------------------------
//   readUInt8
//---------------------------------------------------------

uint8_t GuitarPro::readUInt8()
{
    uint8_t c;
    read(&c, 1);
    return c;
}

//---------------------------------------------------------
//   readPascalString
//---------------------------------------------------------

String GuitarPro::readPascalString(int n)
{
    uint8_t l = readUInt8();
    std::vector<char> s(l + 1);
    read(&s[0], l);
    s[l] = 0;
    if (n - l > 0) {
        skip(n - l);
    }
    std::string_view sw(&s[0], l);
    if (UtfCodec::isValidUtf8(sw)) {
        return String::fromUtf8(&s[0]);
    }
    return String::fromAscii(&s[0], l);
}

//---------------------------------------------------------
//   readWordPascalString
//---------------------------------------------------------

String GuitarPro::readWordPascalString()
{
    int l = readInt();
    std::vector<char> c(l + 1);
    read(&c[0], l);
    c[l] = 0;
    std::string_view sw(&c[0], l);
    if (UtfCodec::isValidUtf8(sw)) {
        return String::fromUtf8(&c[0]);
    }
    return String::fromAscii(&c[0], l);
}

//---------------------------------------------------------
//   readBytePascalString
//---------------------------------------------------------

String GuitarPro::readBytePascalString()
{
    int l = readUInt8();
    std::vector<char> c(l + 1);
    read(&c[0], l);
    c[l] = 0;
    std::string_view sw(&c[0], l);
    if (UtfCodec::isValidUtf8(sw)) {
        return String::fromUtf8(&c[0]);
    }
    return String::fromAscii(&c[0], l);
}

//---------------------------------------------------------
//   readDelphiString
//---------------------------------------------------------

String GuitarPro::readDelphiString()
{
    int maxl = readInt();
    int l    = readUInt8();
    if (maxl != l + 1 && maxl > 255) {
        ASSERT_X("readDelphiString: first word doesn't match second byte");
        l = maxl - 1;
    }
    std::vector<char> c(l + 1);
    read(&c[0], l);
    c[l] = 0;
    return String::fromAscii(&c[0]);
}

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

int GuitarPro::readInt()
{
    uint8_t x;
    read(&x, 1);
    int r = x;
    read(&x, 1);
    r += x << 8;
    read(&x, 1);
    r += x << 16;
    read(&x, 1);
    r += x << 24;
    return r;
}

//---------------------------------------------------------
//   addPalmMate
//---------------------------------------------------------

void GuitarPro::addPalmMute(ChordRest* cr, bool hasPalmMute)
{
    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::PALM_MUTE, ContiniousElementsBuilder::ImportType::PALM_MUTE,
                                                        hasPalmMute);
}

//---------------------------------------------------------
//   addLetRing
//---------------------------------------------------------

void GuitarPro::addLetRing(ChordRest* cr, bool hasLetRing)
{
    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::LET_RING, ContiniousElementsBuilder::ImportType::LET_RING,
                                                        hasLetRing);
}

//---------------------------------------------------------
//   addTrill
//---------------------------------------------------------

void GuitarPro::addTrill(ChordRest* cr, bool hasTrill)
{
    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::TRILL, ContiniousElementsBuilder::ImportType::TRILL, hasTrill);
}

//---------------------------------------------------------
//   addRasgueado
//---------------------------------------------------------

void GuitarPro::addRasgueado(ChordRest* cr, bool hasRasgueado)
{
    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::RASGUEADO, ContiniousElementsBuilder::ImportType::RASGUEADO,
                                                        hasRasgueado);
}

//---------------------------------------------------------
//   addVibratoLeftHand
//---------------------------------------------------------

void GuitarPro::addVibratoLeftHand(ChordRest* cr, bool hasVibratoLeftHand)
{
    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::VIBRATO,
                                                        ContiniousElementsBuilder::ImportType::VIBRATO_LEFT_HAND_SLIGHT, hasVibratoLeftHand,
                                                        ContiniousElementsBuilder::VibratoSubType::LEFT_HAND_SLIGHT);
}

//---------------------------------------------------------
//   addVibratoWTremBar
//---------------------------------------------------------

void GuitarPro::addVibratoWTremBar(ChordRest* cr, bool hasVibratoWTremBar)
{
    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::VIBRATO,
                                                        ContiniousElementsBuilder::ImportType::VIBRATO_W_TREM_BAR_SLIGHT,
                                                        hasVibratoWTremBar, ContiniousElementsBuilder::VibratoSubType::W_TREM_BAR_SLIGHT);
}

//---------------------------------------------------------
//   addHarmonicMarks
//---------------------------------------------------------

void GuitarPro::addHarmonicMarks(ChordRest* cr, bool hasHarmonicArtificial, bool hasHarmonicPinch, bool hasHarmonicTap,
                                 bool hasHarmonicSemi)
{
    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::HARMONIC_MARK,
                                                        ContiniousElementsBuilder::ImportType::HARMONIC_ARTIFICIAL,
                                                        hasHarmonicArtificial, ContiniousElementsBuilder::HarmonicMarkSubType::ARTIFICIAL);

    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::HARMONIC_MARK,
                                                        ContiniousElementsBuilder::ImportType::HARMONIC_PINCH,
                                                        hasHarmonicPinch, ContiniousElementsBuilder::HarmonicMarkSubType::PINCH);

    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::HARMONIC_MARK,
                                                        ContiniousElementsBuilder::ImportType::HARMONIC_TAP,
                                                        hasHarmonicTap, ContiniousElementsBuilder::HarmonicMarkSubType::TAP);

    m_continiousElementsBuilder->buildContiniousElement(cr, ElementType::HARMONIC_MARK,
                                                        ContiniousElementsBuilder::ImportType::HARMONIC_SEMI,
                                                        hasHarmonicSemi, ContiniousElementsBuilder::HarmonicMarkSubType::SEMI);
}

//---------------------------------------------------------
//   addTap
//---------------------------------------------------------

void GuitarPro::addTap(Note* note)
{
    addTextArticulation(note, ArticulationTextType::TAP);
}

//---------------------------------------------------------
//   addSlap
//---------------------------------------------------------

void GuitarPro::addSlap(Note* note)
{
    addTextArticulation(note, ArticulationTextType::SLAP);
}

//---------------------------------------------------------
//   addPop
//---------------------------------------------------------

void GuitarPro::addPop(Note* note)
{
    addTextArticulation(note, ArticulationTextType::POP);
}

void GuitarPro::addTextArticulation(Note* note, ArticulationTextType type)
{
    if (type == ArticulationTextType::NO_TEXT) {
        return;
    }

    if (Chord* ch = toChord(note->parent())) {
        Articulation* art = mu::engraving::Factory::createArticulation(score->dummy()->chord());
        art->setTextType(type);
        ch->add(art);
    }
}

//---------------------------------------------------------
//   addTextToNote
//---------------------------------------------------------

void GuitarPro::addTextToNote(String string, Note* note)
{
    Segment* segment = note->chord()->segment();
    StaffText* text = Factory::createStaffText(segment);

    if (!string.isEmpty()) {
        bool useHarmony = string[string.size() - 1] == '\\';
        if (useHarmony) {
            string.chop(1);
        }
    }
    text->setPlainText(string);
    text->setTrack(note->chord()->track());
    segment->add(text);
}

void GuitarPro::setupTupletStyle(Tuplet* tuplet)
{
    bool real;
    switch (tuplet->ratio().numerator()) {
    case 2: real = (tuplet->ratio().denominator() == 3);
        break;
    case 3:
    case 4: real = (tuplet->ratio().denominator() == 2);
        break;
    case 5:
    case 6:
    case 7: real = (tuplet->ratio().denominator() == 4);
        break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13: real = (tuplet->ratio().denominator() == 8);
        break;
    default: real = false;
    }
    if (!real) {
        tuplet->setNumberType(TupletNumberType::SHOW_RELATION);
        tuplet->setPropertyFlags(Pid::NUMBER_TYPE, PropertyFlags::UNSTYLED);
    }
}

//---------------------------------------------------------
//   setTuplet
//---------------------------------------------------------

void GuitarPro::setTuplet(Tuplet* tuplet, int tuple)
{
    switch (tuple) {
    case 3:
        tuplet->setRatio(Fraction(3, 2));
        break;
    case 5:
        tuplet->setRatio(Fraction(5, 4));
        break;
    case 6:
        tuplet->setRatio(Fraction(6, 4));
        break;
    case 7:
        tuplet->setRatio(Fraction(7, 4));
        break;
    case 9:
        tuplet->setRatio(Fraction(9, 8));
        break;
    case 10:
        tuplet->setRatio(Fraction(10, 8));
        break;
    case 11:
        tuplet->setRatio(Fraction(11, 8));
        break;
    case 12:
        tuplet->setRatio(Fraction(12, 8));
        break;
    case 13:
        tuplet->setRatio(Fraction(13, 8));
        break;
    default:
        ASSERT_X(String(u"unsupported tuplet %d\n").arg(tuple));
    }
}

//---------------------------------------------------------
//   addDynamic
//---------------------------------------------------------

void GuitarPro::addDynamic(Note* note, int d)
{
    if (d < 0) {
        return;
    }
    if (!note->chord()) {
        LOGD() << "addDynamics: No chord associated with this note";
        return;
    }
    Segment* s = nullptr;
    if (note->chord()->isGrace()) {
        Chord* parent = static_cast<Chord*>(note->chord()->parent());
        s = parent->segment();
    } else {
        s = note->chord()->segment();
    }
    if (!s->findAnnotation(ElementType::DYNAMIC, note->staffIdx() * VOICES, note->staffIdx() * VOICES + VOICES - 1)) {
        Dynamic* dyn = new Dynamic(s);
        // guitar pro only allows their users to go from ppp to fff
        const String map_dyn[] = { u"f", u"ppp", u"pp", u"p", u"mp", u"mf", u"f", u"ff", u"fff" };
        dyn->setDynamicType(map_dyn[d]);
        dyn->setTrack(note->track());
        s->add(dyn);
    }
}

//---------------------------------------------------------
//   readVolta
//---------------------------------------------------------

void GuitarPro::readVolta(GPVolta* gpVolta, Measure* m)
{
    /* Volta information is at most eight bits
     * signifying which numbers should appear in the
     * volta. A single bit 1 represents we should show
     * 1, 100 represents 3, 10000 represents 6, 10101
     * represents 1,3,5 etc. */
    if (gpVolta->voltaInfo.size() != 0) {
        // we have volta information - set up a volta
        mu::engraving::Volta* volta = new mu::engraving::Volta(score->dummy());
        volta->endings().clear();
        String voltaTextString = u"";
        // initialise count to 1 as the first bit processed with represent first time volta
        int count = 0;
        int binaryNumber = 0;
        // iterate through the volta information and determine the decimal numbers for voltas
        auto iter = gpVolta->voltaInfo.begin();
        while (iter != gpVolta->voltaInfo.end()) {
            switch (gpVolta->voltaType) {
            case GP_VOLTA_FLAGS:
                count++;
                if (*iter == 1) {                 // we want this number to be displayed in the volta
                    if (voltaTextString.empty()) {
                        voltaTextString += String::number(count);
                    } else {
                        voltaTextString += u',' + String::number(count);
                    }
                    // add the decimal number to the endings field of voltas as well as the text
                    volta->endings().push_back(count);
                }
                ++iter;
                break;
            case GP_VOLTA_BINARY:
                // find the binary number in decimal
                if (*iter == 1) {
                    binaryNumber += pow(2, count);
                }
                ++iter;
                if (iter == gpVolta->voltaInfo.end()) {
                    // display all numbers in the volta from voltaSequence to the decimal
                    while (voltaSequence <= binaryNumber) {
                        if (voltaTextString.empty()) {
                            voltaTextString = String::number(voltaSequence);
                        } else {
                            voltaTextString += u',' + String::number(voltaSequence);
                        }
                        volta->endings().push_back(voltaSequence);
                        voltaSequence++;
                    }
                }
                count++;
                break;
            }
        }
        volta->setText(XmlWriter::xmlString(voltaTextString));
        volta->setTick(m->tick());
        volta->setTick2(m->tick() + m->ticks());
        score->addElement(volta);
    }
}

std::vector<PitchValue> GuitarPro::readBendDataFromFile()
{
    std::vector<PitchValue> bendData;

    readUInt8();                          // icon
    /*int amplitude =*/ readInt();        // shown amplitude
    int numPoints = readInt();            // the number of points in the bend

    for (int i = 0; i < numPoints; ++i) {
        int bendTime  = readInt();
        int bendPitch = readInt();
        int bendVibrato = readUInt8();
        bendData.emplace_back(bendTime, bendPitch, bendVibrato);
    }

    return bendData;
}

void GuitarPro::createBend(Note* note, std::vector<PitchValue>& bendData)
{
    if (bendData.size() < 2) {
        return;
    }

    /// not adding "hold" on 0 pitch
    int maxPitch = (std::max_element(bendData.begin(), bendData.end(), [](const PitchValue& l, const PitchValue& r) {
        return l.pitch < r.pitch;
    }))->pitch;

    if (maxPitch == 0) {
        return;
    }

    if (engravingConfiguration()->experimentalGuitarBendImport()) {
        m_guitarBendImporter->collectBend(note, bendData);
    } else {
        Bend* bend = Factory::createBend(note);
        bend->setPoints(bendData);
        bend->setTrack(note->track());
        note->add(bend);
    }
}

//---------------------------------------------------------
//   readBend
//    bend graph
//---------------------------------------------------------

void GuitarPro::readBend(Note* note)
{
    std::vector<PitchValue> bendData = readBendDataFromFile();
    createBend(note, bendData);
}

//---------------------------------------------------------
//   readLyrics
//---------------------------------------------------------

void GuitarPro::readLyrics()
{
    gpLyrics.lyricTrack = static_cast<size_t>(readInt());          // lyric track
    gpLyrics.fromBeat = static_cast<size_t>(readInt());
    gpLyrics.beatCounter = 0;

    String lyrics = readWordPascalString();
    lyrics.replace(u'\n', u' ');
    lyrics.replace(u'\r', u' ');
    auto sl = lyrics.split(u' ', KeepEmptyParts);
    for (auto& str : sl) {
        gpLyrics.lyrics.push_back(str);
    }

    for (int i = 0; i < 4; ++i) {
        readInt();
        readWordPascalString();
    }
}

//---------------------------------------------------------
//   createSlide
//---------------------------------------------------------

void GuitarPro::createSlide(int sl, ChordRest* cr, int staffIdx, Note* note)
{
    UNUSED(note);
    // shift / legato slide
    if (sl == SHIFT_SLIDE || sl == LEGATO_SLIDE) {
        Glissando* s = mu::engraving::Factory::createGlissando(cr);
        //s->setXmlText("");
        s->setGlissandoType(GlissandoType::STRAIGHT);
        cr->add(s);
        s->setAnchor(Spanner::Anchor::NOTE);
        Segment* prevSeg = cr->segment()->prev1(SegmentType::ChordRest);
        EngravingItem* prevElem = prevSeg->element(staffIdx);
        if (prevElem) {
            if (prevElem->type() == ElementType::CHORD) {
                Chord* prevChord = static_cast<Chord*>(prevElem);
                /** TODO we should not just take the top note here
                * but the /correct/ note need to check whether GP
                * supports multi-note gliss. I think it can in modern
                * versions */
                s->setStartElement(prevChord->upNote());
                s->setTick(prevSeg->tick());
                s->setTrack(staffIdx);
                s->setParent(prevChord->upNote());
                s->setText(u"");
                s->setGlissandoType(GlissandoType::STRAIGHT);
                s->setGlissandoShift(sl == SHIFT_SLIDE);

                if (sl == LEGATO_SLIDE) {
                    createSlur(true, staffIdx, prevChord);
                }
            }
        }

        Chord* chord = (Chord*)cr;
        /* TODO again here, we should not just set the up note but the
         * /correct/ note need to check whether GP supports
         * multi-note gliss. I think it can in modern versions */
        s->setEndElement(chord->upNote());
        s->setTick2(chord->segment()->tick());
        s->setTrack2(staffIdx);
        score->addElement(s);
        if (sl == LEGATO_SLIDE) {
            createSlur(false, staffIdx, cr);
        }
    }
    // slide out downwards (fall)
    if (sl & SLIDE_OUT_DOWN) {
        ChordLine* sld = Factory::createChordLine(mu::engraving::toChord(cr));
        sld->setChordLineType(ChordLineType::FALL);
        cr->add(sld);
    }
    // slide out upwards (doit)
    if (sl & SLIDE_OUT_UP) {
        ChordLine* slu = Factory::createChordLine(mu::engraving::toChord(cr));
        slu->setChordLineType(ChordLineType::DOIT);
        cr->add(slu);
    }
    // slide in from below (plop)
    if (sl & SLIDE_IN_BELOW) {
        ChordLine* slb = Factory::createChordLine(mu::engraving::toChord(cr));
        slb->setChordLineType(ChordLineType::PLOP);
        cr->add(slb);
    }
    // slide in from above (scoop)
    if (sl & SLIDE_IN_ABOVE) {
        ChordLine* sla = Factory::createChordLine(mu::engraving::toChord(cr));
        sla->setChordLineType(ChordLineType::SCOOP);
        cr->add(sla);
    }
}

//---------------------------------------------------------
//   readChannels
//---------------------------------------------------------

void GuitarPro::readChannels()
{
    for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
        channelDefaults[i].patch   = readInt();
        channelDefaults[i].volume  = readUInt8() * 8 - 1;
        channelDefaults[i].pan     = readUInt8() * 8 - 1;
        channelDefaults[i].chorus  = readUInt8() * 8 - 1;
        channelDefaults[i].reverb  = readUInt8() * 8 - 1;
        channelDefaults[i].phase   = readUInt8() * 8 - 1;
        channelDefaults[i].tremolo = readUInt8() * 8 - 1;

        // defaults of 255, or any value above 127, are set to 0 (Musescore range is 0-127)
        if (channelDefaults[i].patch > 127) {
            channelDefaults[i].patch = 0;
        }
        if (channelDefaults[i].volume > 127) {
            channelDefaults[i].volume = 0;
        }
        if (channelDefaults[i].pan > 127) {
            channelDefaults[i].pan = 0;
        }
        if (channelDefaults[i].chorus > 127) {
            channelDefaults[i].chorus = 0;
        }
        if (channelDefaults[i].reverb > 127) {
            channelDefaults[i].reverb = 0;
        }
        if (channelDefaults[i].phase > 127) {
            channelDefaults[i].phase = 0;
        }
        if (channelDefaults[i].tremolo > 127) {
            channelDefaults[i].tremolo = 0;
        }

        // skip over blank information included for backwards compatibility with 3.0
        skip(2);
    }
}

//---------------------------------------------------------
//   len2fraction
//---------------------------------------------------------

Fraction GuitarPro::len2fraction(int len)
{
    Fraction l;
    switch (len) {
    case -2: l.set(1, 1);
        break;
    case -1: l.set(1, 2);
        break;
    case  0: l.set(1, 4);
        break;
    case  1: l.set(1, 8);
        break;
    case  2: l.set(1, 16);
        break;
    case  3: l.set(1, 32);
        break;
    case  4: l.set(1, 64);
        break;
    case  5: l.set(1, 128);
        break;
    // set to len - in some cases we get whacky numbers for this (40, 28...)
    default:
        l.set(1, len);
    }
    return l;
}

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

bool GuitarPro::readMixChange(Measure* measure)
{
    /*char patch   =*/
    readChar();
    signed char volume  = readChar();
    signed char pan     = readChar();
    signed char chorus  = readChar();
    signed char reverb  = readChar();
    signed char phase   = readChar();
    signed char tremolo = readChar();
    int temp    = readInt();

    if (volume >= 0) {
        readChar();
    }
    if (pan >= 0) {
        readChar();
    }
    if (chorus >= 0) {
        readChar();
    }
    if (reverb >= 0) {
        readChar();
    }
    if (phase >= 0) {
        readChar();
    }
    if (tremolo >= 0) {
        readChar();
    }
    if (temp >= 0) {
        if (temp != previousTempo) {
            previousTempo = temp;
            setTempo(temp, measure);
        }
        readChar();
    }
    return true;
}

//---------------------------------------------------------
//   createMeasures
//---------------------------------------------------------

void GuitarPro::createMeasures()
{
    Fraction tick = Fraction(0, 1);
    Fraction ts;
    LOGD("measures %zu bars.size %zu", measures, bars.size());

    //      for (int i = 0; i < measures; ++i) {
    for (size_t i = 0; i < bars.size(); ++i) {     // ?? (ws)
        Fraction nts = bars[i].timesig;
        Measure* m = Factory::createMeasure(score->dummy()->system());
        m->setTick(tick);
        m->setTimesig(nts);
        m->setTicks(nts);

        if (i == 0 || ts.numerator() != nts.numerator() || ts.denominator() != nts.denominator()) {
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                const Staff* staff = score->staff(staffIdx);
                const StaffType* staffType = staff->staffType(Fraction(0, 1));             // at tick 0
                if (staffType->genTimesig()) {
                    Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                    TimeSig* t = Factory::createTimeSig(s);
                    t->setTrack(staffIdx * VOICES);
                    t->setSig(nts);
                    s->add(t);
                }
            }
        }
        if (i == 0 || (bars[i].keysig != GP_INVALID_KEYSIG)) {
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                int keysig = bars[i].keysig != GP_INVALID_KEYSIG ? bars[i].keysig : key;
                if (tick.isZero() || (int)score->staff(staffIdx)->key(tick) != (int)Key(keysig)) {
                    Segment* s = m->getSegment(SegmentType::KeySig, tick);
                    KeySig* t = Factory::createKeySig(s);
                    t->setKey(Key(keysig));
                    t->setTrack(staffIdx * VOICES);
                    s->add(t);
                }
            }
        }
        readVolta(&bars[i].volta, m);
        m->setRepeatEnd(bars[i].repeatFlags == mu::engraving::Repeat::END);
        m->setRepeatStart(bars[i].repeatFlags == mu::engraving::Repeat::START);
        m->setRepeatJump(bars[i].repeatFlags == mu::engraving::Repeat::JUMP);
        //            m->setRepeatFlags(bars[i].repeatFlags);
        m->setRepeatCount(bars[i].repeats);           // supported in gp5

        if (bars[i].repeatFlags == (mu::engraving::Repeat::START | mu::engraving::Repeat::END)) {
            m->setRepeatEnd(true);
            m->setRepeatStart(true);
            voltaSequence = 1;
        }
        // reset the volta sequence if we have an opening repeat
        if (bars[i].repeatFlags == mu::engraving::Repeat::START) {
            voltaSequence = 1;
        }
        // otherwise, if we see an end repeat symbol, only reset if the bar after it does not contain a volta
        else if (bars[i].repeatFlags == mu::engraving::Repeat::END && i < bars.size() - 1) {
            if (bars[i + 1].volta.voltaInfo.size() == 0) {
                voltaSequence = 1;              // reset  the volta count
            }
        }

        score->measures()->append(m);
        tick += nts;
        ts = nts;
    }
}

//---------------------------------------------------------
//   applyBeatEffects
//---------------------------------------------------------

void GuitarPro::applyBeatEffects(Chord* chord, int beatEffect, bool& hasVibratoLeftHand, bool& hasVibratoWTremBar)
{
    /* tap/slap/pop implemented as text until SMuFL has
     * specifications and we can add them to fonts. Note that
     * tap/slap/pop are just added to the top note in the chord,
     * technically these can be applied to individual notes on the
     * UI, but Guitar Pro has no way to express that on the
     * score. To get the same result, we should just add the marking
     * to above the top note.
     */
    if (beatEffect == 1) {
        if (version > 300) {
            addTap(chord->upNote());
        } else {
            hasVibratoLeftHand = true;
        }
    } else if (beatEffect == 2) {
        addSlap(chord->upNote());
    } else if (beatEffect == 3) {
        addPop(chord->upNote());
    } else if (beatEffect == 4) {
        if (version >= 400) {
            Articulation* a = Factory::createArticulation(chord);
            a->setSymId(SymId::guitarFadeIn);
            a->setAnchor(ArticulationAnchor::TOP);
            a->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
            chord->add(a);
        }
        //TODO-ws		else for (auto n : chord->notes())
        //			n->setHarmonic(true);
    } else if (beatEffect == 5) {
        Articulation* a = Factory::createArticulation(chord);
        a->setSymId(SymId::stringsUpBow);
        chord->add(a);
    } else if (beatEffect == 6) {
        Articulation* art = Factory::createArticulation(chord);
        art->setSymId(SymId::stringsDownBow);
        chord->add(art);
    } else if (beatEffect == 7) {
        hasVibratoWTremBar = true;
    }
}

#ifdef _MSC_VER
#pragma optimize("", on)
#endif

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro1::read(IODevice* io)
{
    m_continiousElementsBuilder = std::make_unique<ContiniousElementsBuilder>(score);
    f      = io;
    curPos = 30;

    title  = readDelphiString();
    artist = readDelphiString();
    readDelphiString();

    int temp = readInt();
    /*uint8_t num =*/ readUInt8();        // Shuffle rhythm feel

    // int octave = 0;
    key    = 0;
    if (version > 102) {
        key = readInt();        // key
    }
    staves  = version > 102 ? 8 : 1;

    slurs = new Slur*[staves];
    for (size_t i = 0; i < staves; ++i) {
        slurs[i] = nullptr;
    }

    //int tnumerator   = 4;
    //int tdenominator = 4;

    //
    // create a part for every staff
    //
    for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
        Part* part = new Part(score);
        Staff* s = Factory::createStaff(part);

        score->appendStaff(s);
        score->appendPart(part);
    }

    for (size_t i = 0; i < staves; ++i) {
        int tuning[GP_MAX_STRING_NUMBER];

        int strings  = version > 101 ? readInt() : 6;
        if (strings <= 0 || strings > GP_MAX_STRING_NUMBER) {
            return false;
        }

        for (int j = 0; j < strings; ++j) {
            tuning[j] = readInt();
        }
        std::vector<int> tuning2(strings);
        //int tuning2[strings];
        for (int k = 0; k < strings; ++k) {
            tuning2[strings - k - 1] = tuning[k];
        }

        int frets = 32;       // TODO
        bool useFlats = createTuningString(strings, &tuning2[0]);
        StringData stringData(frets, strings, &tuning2[0], useFlats);
        Part* part = score->staff(i)->part();
        Instrument* instr = part->instrument();
        instr->setStringData(stringData);
        instr->setSingleNoteDynamics(false);
    }

    measures = readInt();

    Fraction ts;
    Fraction tick = { 0, 1 };
    for (size_t i = 0; i < measures; ++i) {
        Fraction nts = bars[i].timesig;
        Measure* m = Factory::createMeasure(score->dummy()->system());
        m->setTick(tick);
        m->setTimesig(nts);
        m->setTicks(nts);

        if (i == 0 || ts != nts) {
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                TimeSig* t = Factory::createTimeSig(s);
                t->setTrack(staffIdx * VOICES);
                t->setSig(nts);
                s->add(t);
            }
        }

        score->measures()->append(m);
        tick += nts;
        ts = nts;
    }

    previousTempo = temp;
    Measure* measure = score->firstMeasure();
    bool mixChange = false;
    for (size_t bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
        const GpBar& gpbar = bars[bar];

        if (!gpbar.marker.isEmpty()) {
            Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
            RehearsalMark* s = new RehearsalMark(segment);
            s->setPlainText(gpbar.marker.trimmed());
            s->setTrack(0);
            segment->add(s);
        }
        std::vector<Tuplet*> tuplets(staves);
        //Tuplet* tuplets[staves];
        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            tuplets[staffIdx] = 0;
        }

        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Fraction measureLen = { 0, 1 };
            track_idx_t track = staffIdx * VOICES;
            Fraction fraction  = measure->tick();
            int beats = readInt();
            for (int beat = 0; beat < beats; ++beat) {
                //                        int pause = 0;
                uint8_t beatBits = readUInt8();
                bool dotted = beatBits & BEAT_DOTTED;
                if (beatBits & BEAT_PAUSE) {
                    /*pause =*/
                    readUInt8();
                }
                int len = readChar();
                int tuple = 0;
                if (beatBits & BEAT_TUPLET) {
                    tuple = readInt();
                }
                Segment* segment = measure->getSegment(SegmentType::ChordRest, fraction);
                if (beatBits & BEAT_CHORD) {
                    int numStrings = static_cast<int>(score->staff(staffIdx)->part()->instrument()->stringData()->strings());
                    int header = readUInt8();
                    String name;
                    if ((header & 1) == 0) {
                        name = readDelphiString();
                        readChord(segment, static_cast<int>(track), numStrings, name, false);
                    } else {
                        skip(25);
                        name = readPascalString(34);
                        readChord(segment, static_cast<int>(track), numStrings, name, true);
                        skip(36);
                    }
                }
                Lyrics* lyrics = 0;
                if (beatBits & BEAT_LYRICS) {
                    lyrics = Factory::createLyrics(score->dummy()->chord());
                    lyrics->setPlainText(readDelphiString());
                }
                if (beatBits & BEAT_EFFECTS) {
                    readBeatEffects(static_cast<int>(track), segment);
                }

                if (beatBits & BEAT_MIX_CHANGE) {
                    readMixChange(measure);
                    mixChange = true;
                }

                int strings = readUInt8();           // used strings mask

                Fraction l = len2fraction(len);
                ChordRest* cr;
                if (strings) {
                    cr = Factory::createChord(score->dummy()->segment());
                } else {
                    cr = Factory::createRest(score->dummy()->segment());
                }
                cr->setTrack(track);
                if (lyrics) {
                    cr->add(lyrics);
                }

                TDuration d(l);
                d.setDots(dotted ? 1 : 0);

                if (dotted) {
                    l = l + (l * Fraction(1, 2));
                }

                if (tuple) {
                    Tuplet* tuplet = tuplets[staffIdx];
                    if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                        tuplet = Factory::createTuplet(measure);
                        tuplet->setTick(fraction);
                        tuplet->setTrack(cr->track());
                        tuplets[staffIdx] = tuplet;
                        setTuplet(tuplet, tuple);
                    }
                    tuplet->setTrack(track);
                    tuplet->setBaseLen(l);
                    tuplet->setTicks(l * tuplet->ratio().denominator());
                    cr->setTuplet(tuplet);
                    tuplet->add(cr);            //TODOxxx
                }

                cr->setTicks(l);
                cr->setDurationType(d);
                segment->add(cr);
                Staff* staff = cr->staff();
                int numStrings = static_cast<int>(staff->part()->instrument()->stringData()->strings());
                ReadNoteResult readResult;
                bool hasLetRing = false;
                bool hasPalmMute = false;
                for (int i = 6; i >= 0; --i) {
                    if (strings & (1 << i) && ((6 - i) < numStrings)) {
                        Note* note = Factory::createNote(static_cast<Chord*>(cr));
                        static_cast<Chord*>(cr)->add(note);
                        readResult = readNote(6 - i, note);
                        hasLetRing = readResult.letRing || hasLetRing;
                        hasPalmMute = readResult.palmMute || hasPalmMute;
                        note->setTpcFromPitch();
                    }
                }
                restsForEmptyBeats(segment, measure, cr, l, static_cast<int>(track), fraction);
                fraction += cr->actualTicks();
                measureLen += cr->actualTicks();
                addLetRing(cr, hasLetRing);
                addPalmMute(cr, hasPalmMute);
            }
            if (measureLen < measure->ticks()) {
                score->setRest(fraction, track, measure->ticks() - measureLen, false, nullptr, false);
                m_continiousElementsBuilder->notifyUncompletedMeasure();
            }
        }
        if (bar == 1 && !mixChange) {
            setTempo(temp, score->firstMeasure());
        }
    }

    m_continiousElementsBuilder->addElementsToScore();
    return true;
}

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void GuitarPro::setTempo(int newTempo, Measure* measure)
{
    if (!measure || measure == last_measure) {
        return;
    }

    last_measure = measure;

    if (last_tempo != newTempo) {
        Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
        TempoText* tt = Factory::createTempoText(segment);
        tt->setTempo(BeatsPerSecond::fromBPM(newTempo));
        tt->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(newTempo));
        tt->setTrack(0);
        segment->add(tt);
        tempo = newTempo;
        last_tempo = newTempo;
        score->setTempo(measure->tick(), BeatsPerSecond::fromBPM(newTempo));
    }
}

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro::readChord(Segment* seg, int track, int numStrings, String name, bool gpHeader)
{
    int firstFret = readInt();
    if (firstFret || gpHeader) {
        /// showing chord instead of fret diagrams
        if (true) {
            StaffText* staffText = Factory::createStaffText(seg);
            staffText->setTrack(track);
            staffText->setPlainText(name);
            seg->add(staffText);
            skip(4 * (gpHeader ? 7 : 6));
        } else {
            FretDiagram* fret = Factory::createFretDiagram(seg);
            fret->setTrack(track);
            fret->setStrings(numStrings);
            fret->setFretOffset(firstFret - 1);
            for (int i = 0; i < (gpHeader ? 7 : 6); ++i) {
                int currentFret =  readInt();
                // read the frets and add them to the fretboard
                // subtract 1 extra from numStrings as we count from 0
                if (i > numStrings - 1) {
                } else if (currentFret > 0) {
                    fret->setDot(numStrings - 1 - i, currentFret - firstFret + 1, true);
                } else if (currentFret == 0) {
                    fret->setDot(numStrings - 1 - i, 0, true);
                    fret->setMarker(numStrings - 1 - i, FretMarkerType::CIRCLE);
                } else if (currentFret == -1) {
                    fret->setDot(numStrings - 1 - i, 0, true);
                    fret->setMarker(numStrings - 1 - i, FretMarkerType::CROSS);
                }
            }
            seg->add(fret);
            if (!name.isEmpty()) {
                Harmony* harmony = new Harmony(seg);
                harmony->setHarmony(name);
                harmony->setTrack(track);
                fret->add(harmony);
            }
        }
    } else if (!name.isEmpty()) {
        Harmony* harmony = new Harmony(seg);
        harmony->setHarmony(name);
        harmony->setTrack(track);
        seg->add(harmony);
    }
}

//---------------------------------------------------------
//   restsForEmptyBeats
//---------------------------------------------------------

void GuitarPro::restsForEmptyBeats(Segment* seg, Measure* measure, ChordRest* cr, Fraction& l, int track, const Fraction& tick)
{
    /* this can happen as Guitar Pro versions 5 and below allows
     * users to create empty segments. Here, we create rests and
     * make them invisible so users get the same visual if they are
     * at a valid tick of the score. */
    if (seg->empty()) {
        if (tick < measure->first()->tick() + measure->ticks()) {
            cr = Factory::createRest(seg);
            cr->setTrack(track);
            TDuration d(l);
            cr->setDurationType(d);
            cr->setVisible(false);
            seg->add(cr);
        } else {
            measure->remove(seg);
        }
    }
}

//---------------------------------------------------------
//   createSlur
//---------------------------------------------------------

void GuitarPro::createSlur(bool hasSlur, staff_idx_t staffIdx, ChordRest* cr)
{
    if (hasSlur && (slurs[staffIdx] == 0)) {
        Slur* slur = Factory::createSlur(score->dummy());
        slur->setParent(0);
        slur->setTrack(cr->track());
        slur->setTrack2(cr->track());
        slur->setTick(cr->tick());
        slur->setTick2(cr->tick());
        slur->setStartElement(cr);
        slur->setEndElement(cr);
        slurs[staffIdx] = slur;
        score->addElement(slur);
    } else if (slurs[staffIdx] && !hasSlur) {
        Slur* s = slurs[staffIdx];
        slurs[staffIdx] = 0;
        s->setEndElement(cr);
        s->setTick2(cr->tick());
        s->setTrack2(cr->track());
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro2::read(IODevice* io)
{
    m_continiousElementsBuilder = std::make_unique<ContiniousElementsBuilder>(score);
    f      = io;
    curPos = 30;

    title        = readDelphiString();
    subtitle     = readDelphiString();
    artist       = readDelphiString();
    album        = readDelphiString();
    composer     = readDelphiString();
    String copyright = readDelphiString();
    if (!copyright.isEmpty()) {
        score->setMetaTag(u"copyright", copyright);
    }

    readDelphiString(); // transcriber
    readDelphiString(); // instructions
    int n = readInt();
    for (int i = 0; i < n; ++i) {
        comments.append(readDelphiString());
    }

    /*uint8_t num =*/ readUInt8();        // Shuffle rhythm feel

    int temp = readInt();

    // int octave = 0;
    /*int key =*/ readInt();      // key

    for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
        channelDefaults[i].patch   = readInt();
        channelDefaults[i].volume  = readUInt8() * 8 - 1;
        channelDefaults[i].pan     = readUInt8() * 8 - 1;
        channelDefaults[i].chorus  = readUInt8() * 8 - 1;
        channelDefaults[i].reverb  = readUInt8() * 8 - 1;
        channelDefaults[i].phase   = readUInt8() * 8 - 1;
        channelDefaults[i].tremolo = readUInt8() * 8 - 1;
        readUInt8();          // padding
        readUInt8();
    }
    measures   = readInt();
    staves = readInt();
    initDynamics(staves);

    int tnumerator   = 4;
    int tdenominator = 4;
    for (size_t i = 0; i < measures; ++i) {
        GpBar bar;
        uint8_t barBits = readUInt8();
        if (barBits & SCORE_TIMESIG_NUMERATOR) {
            tnumerator = readUInt8();
        }
        if (barBits & SCORE_TIMESIG_DENOMINATOR) {
            tdenominator = readUInt8();
        }
        if (barBits & SCORE_REPEAT_START) {
            bar.repeatFlags = bar.repeatFlags | mu::engraving::Repeat::START;
        }
        if (barBits & SCORE_REPEAT_END) {
            bar.repeatFlags = bar.repeatFlags | mu::engraving::Repeat::END;
            bar.repeats = readUInt8() + 1;
        }
        if (barBits & SCORE_VOLTA) {
            uint8_t voltaNumber = readUInt8();
            while (voltaNumber > 0) {
                // volta information is represented as a binary number
                bar.volta.voltaType = GP_VOLTA_BINARY;
                bar.volta.voltaInfo.push_back(voltaNumber & 1);
                voltaNumber >>= 1;
            }
        }
        if (barBits & SCORE_MARKER) {
            bar.marker = readDelphiString();           // new section?
            /*int color =*/ readInt();          // color?
        }
        if (barBits & SCORE_KEYSIG) {
            bar.keysig = readUInt8();
            /*uint8_t c    =*/ readUInt8();              // minor
        }
        if (barBits & SCORE_DOUBLE_BAR) {
            bar.barLine = BarLineType::DOUBLE;
        }
        bar.timesig = Fraction(tnumerator, tdenominator);
        bars.push_back(bar);
    }

    //
    // create a part for every staff
    //
    for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
        Part* part = new Part(score);
        Staff* s = Factory::createStaff(part);

        score->appendStaff(s);
        score->appendPart(part);
    }

    Fraction ts;
    Fraction tick = { 0, 1 };
    for (size_t i = 0; i < measures; ++i) {
        Fraction nts = bars[i].timesig;
        Measure* m = Factory::createMeasure(score->dummy()->system());
        m->setTick(tick);
        m->setTimesig(nts);
        m->setTicks(nts);

        if (i == 0 || ts != nts) {
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                TimeSig* t = Factory::createTimeSig(s);
                t->setTrack(staffIdx * VOICES);
                t->setSig(nts);
                s->add(t);
            }
        }

        score->measures()->append(m);
        tick += nts;
        ts = nts;
    }

    for (size_t i = 0; i < staves; ++i) {
        int tuning[GP_MAX_STRING_NUMBER];

        uint8_t c      = readUInt8();       // simulations bitmask
        if (c & 0x2) {                    // 12 stringed guitar
        }
        if (c & 0x4) {                    // banjo track
        }
        String name = readPascalString(40);
        int strings  = readInt();
        if (strings <= 0 || strings > GP_MAX_STRING_NUMBER) {
            return false;
        }
        for (int j = 0; j < strings; ++j) {
            tuning[j] = readInt();
        }
        for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j) {
            readInt();
        }
        /*int midiPort     =*/ readInt();     //  - 1;
        int midiChannel  = readInt() - 1;
        /*int midiChannel2 =*/ readInt();     // - 1;
        int frets        = readInt();
        int capo         = readInt();
        /*int color        =*/ readInt();

        std::vector<int> tuning2(strings);
        //int tuning2[strings];
        for (int k = 0; k < strings; ++k) {
            tuning2[strings - k - 1] = tuning[k];
        }
        bool useFlats = createTuningString(strings, &tuning2[0]);
        StringData stringData(frets, strings, &tuning2[0], useFlats);
        Part* part = score->staff(i)->part();
        Instrument* instr = part->instrument();
        instr->setStringData(stringData);
        instr->setSingleNoteDynamics(false);
        part->setPartName(name);
        part->setPlainLongName(name);

        //
        // determine clef
        //
        Staff* staff = score->staff(i);
        int patch = channelDefaults[midiChannel].patch;
        ClefType clefId = ClefType::G;
        if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
            clefId = ClefType::PERC;
            StaffTypes type = StaffTypes::PERC_DEFAULT;
            if (auto it = drumset::PERC_STAFF_LINES_FROM_INSTRUMENT.find(name.toStdString());
                it != drumset::PERC_STAFF_LINES_FROM_INSTRUMENT.end()) {
                drumset::initGuitarProPercussionSet(it->second);
                drumset::setInstrumentDrumset(instr, it->second);
                switch (it->second.numLines) {
                case 1:
                    type = StaffTypes::PERC_1LINE;
                    break;
                case 2:
                    type = StaffTypes::PERC_2LINE;
                    break;
                case 3:
                    type = StaffTypes::PERC_3LINE;
                    break;
                default:
                    type = StaffTypes::PERC_DEFAULT;
                    break;
                }
            } else {
                drumset::initGuitarProDrumset();
                instr->setDrumset(drumset::gpDrumset);
            }
            staff->setStaffType(Fraction(0, 1), *StaffType::preset(type));
        } else {
            clefId = defaultClef(patch);
        }
        Measure* measure = score->firstMeasure();
        Segment* segment = measure->getSegment(SegmentType::HeaderClef, Fraction(0, 1));
        Clef* clef = Factory::createClef(segment);
        clef->setClefType(clefId);
        clef->setTrack(i * VOICES);
        segment->add(clef);

        if (capo > 0 && !engravingConfiguration()->guitarProImportExperimental()) {
            Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
            StaffText* st = new StaffText(s);
            //                  st->setTextStyleType(TextStyleType::STAFF);
            st->setPlainText(String(u"Capo. fret ") + String::number(capo));
            st->setTrack(i * VOICES);
            s->add(st);
        }

        InstrChannel* ch = instr->channel(0);
        if (midiChannel == int(StaffTypes::PERC_DEFAULT)) {
            ch->setProgram(0);
            ch->setBank(128);
        } else {
            ch->setProgram(patch);
            ch->setBank(0);
        }
        ch->setVolume(channelDefaults[midiChannel].volume);
        ch->setPan(channelDefaults[midiChannel].pan);
        ch->setChorus(channelDefaults[midiChannel].chorus);
        ch->setReverb(channelDefaults[midiChannel].reverb);
        // missing: phase, tremolo
    }

    previousTempo = temp;
    Measure* measure = score->firstMeasure();
    bool mixChange = false;
    for (size_t bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
        const GpBar& gpbar = bars[bar];

        if (!gpbar.marker.isEmpty()) {
            Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
            RehearsalMark* s = new RehearsalMark(segment);
            s->setPlainText(gpbar.marker.trimmed());
            s->setTrack(0);
            segment->add(s);
        }

        std::vector<Tuplet*> tuplets(staves);
        // Tuplet* tuplets[staves];
        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            tuplets[staffIdx] = 0;
        }

        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Fraction measureLen = { 0, 1 };
            track_idx_t track = staffIdx * VOICES;
            Fraction fraction = measure->tick();
            int beats = readInt();
            for (int beat = 0; beat < beats; ++beat) {
                //                        int pause = 0;
                uint8_t beatBits = readUInt8();
                bool dotted = beatBits & BEAT_DOTTED;
                if (beatBits & BEAT_PAUSE) {
                    /*pause =*/
                    readUInt8();
                }
                int len = readChar();
                int tuple = 0;
                if (beatBits & BEAT_TUPLET) {
                    tuple = readInt();
                }
                Segment* segment = measure->getSegment(SegmentType::ChordRest, fraction);
                if (beatBits & BEAT_CHORD) {
                    int numStrings = static_cast<int>(score->staff(staffIdx)->part()->instrument()->stringData()->strings());
                    int header = readUInt8();
                    String name;
                    if ((header & 1) == 0) {
                        name = readDelphiString();
                        readChord(segment, static_cast<int>(track), numStrings, name, false);
                    } else {
                        skip(25);
                        name = readPascalString(34);
                        readChord(segment, static_cast<int>(track), numStrings, name, true);
                        skip(36);
                    }
                }
                Lyrics* lyrics = 0;
                if (beatBits & BEAT_LYRICS) {
                    String txt = readDelphiString();
                    lyrics = Factory::createLyrics(score->dummy()->chord());
                    lyrics->setPlainText(txt);
                }
                if (beatBits & BEAT_EFFECTS) {
                    readBeatEffects(static_cast<int>(track), segment);
                }

                if (beatBits & BEAT_MIX_CHANGE) {
                    readMixChange(measure);
                    mixChange = true;
                }

                int strings = readUInt8();           // used strings mask

                Fraction l = len2fraction(len);
                ChordRest* cr;
                if (strings) {
                    cr = Factory::createChord(score->dummy()->segment());
                } else {
                    cr = Factory::createRest(score->dummy()->segment());
                }
                cr->setTrack(track);
                if (lyrics) {
                    cr->add(lyrics);
                }

                TDuration d(l);
                d.setDots(dotted ? 1 : 0);

                if (dotted) {
                    l = l + (l * Fraction(1, 2));
                }

                if (tuple) {
                    Tuplet* tuplet = tuplets[staffIdx];
                    if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                        tuplet = Factory::createTuplet(measure);
                        tuplet->setTick(fraction);
                        tuplet->setTrack(cr->track());
                        tuplets[staffIdx] = tuplet;
                        setTuplet(tuplet, tuple);
                        tuplet->setParent(measure);
                    }
                    tuplet->setTrack(track);
                    tuplet->setBaseLen(l);
                    tuplet->setTicks(l * tuplet->ratio().denominator());
                    cr->setTuplet(tuplet);
                    tuplet->add(cr);
                }

                cr->setTicks(l);
                cr->setDurationType(d);
                segment->add(cr);
                Staff* staff = cr->staff();
                int numStrings = static_cast<int>(staff->part()->instrument()->stringData()->strings());
                ReadNoteResult readResult;
                bool hasLetRing = false;
                bool hasPalmMute = false;
                for (int i = 6; i >= 0; --i) {
                    if (strings & (1 << i) && ((6 - i) < numStrings)) {
                        Note* note = Factory::createNote(static_cast<Chord*>(cr));
                        static_cast<Chord*>(cr)->add(note);
                        readResult = readNote(6 - i, note);
                        hasLetRing = readResult.letRing || hasLetRing;
                        hasPalmMute = readResult.palmMute || hasPalmMute;
                        note->setTpcFromPitch();
                    }
                }
                restsForEmptyBeats(segment, measure, cr, l, static_cast<int>(track), fraction);
                fraction += cr->actualTicks();
                measureLen += cr->actualTicks();
                addLetRing(cr, hasLetRing);
                addPalmMute(cr, hasPalmMute);
            }
            if (measureLen < measure->ticks()) {
                score->setRest(fraction, track, measure->ticks() - measureLen, false, nullptr, false);
                m_continiousElementsBuilder->notifyUncompletedMeasure();
            }
        }
        if (bar == 1 && !mixChange) {
            setTempo(temp, score->firstMeasure());
        }
    }

    m_continiousElementsBuilder->addElementsToScore();
    return true;
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

GuitarPro::ReadNoteResult GuitarPro1::readNote(int string, Note* note)
{
    ReadNoteResult readResult;
    uint8_t noteBits = readUInt8();
    if (noteBits & NOTE_GHOST) {
        if (version == 300) {
            note->setGhost(true);
        } else {
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setGhost(true);
        }
    }

    bool tieNote = false;
    uint8_t variant = 1;
    if (noteBits & NOTE_DEAD) {
        variant = readUInt8();
        if (variant == 1) {         // normal note
        } else if (variant == 2) {
            tieNote = true;
        } else if (variant == 3) {                   // dead notes
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setDeadNote(true);
        } else {
            LOGD("unknown note variant: %d", variant);
        }
    }

    //
    // noteBits:
    //    7 - Right hand or left hand fingering;
    //    6 - Accentuated note
    //    5 - Note type (rest, empty note, normal note);
    //    4 - note dynamic;
    //    3 - Presence of effects linked to the note;
    //    2 - Ghost note;
    //    1 - Dotted note;  ?
    //    0 - Time-independent duration

    if (noteBits & 0x1) {                 // note != beat
        int a = readUInt8();              // length
        int b = readUInt8();              // t
        LOGD("Time independent note len, len %d t %d", a, b);
    }
    if (noteBits & 0x2) {                 // note is dotted
        //readUInt8();
    }

    int& previousDynamic = previousDynamicByTrack[note->track()];

    // set dynamic information on note if different from previous note
    if (noteBits & NOTE_DYNAMIC) {
        int d = readChar();
        if (previousDynamic != d) {
            previousDynamic = d;
            addDynamic(note, d);
        }
    } else if (previousDynamic != DEFAULT_DYNAMIC) {
        previousDynamic = DEFAULT_DYNAMIC;
        addDynamic(note, previousDynamic);
    }

    int fretNumber = -1;
    if (noteBits & NOTE_FRET) {
        fretNumber = readUInt8();
    }

    if (noteBits & NOTE_FINGERING) {                // fingering
        int a = readUInt8();
        int b = readUInt8();
        LOGD("Fingering=========%d %d", a, b);
    }
    if (noteBits & BEAT_EFFECTS) {
        uint8_t modMask1 = readUInt8();
        uint8_t modMask2 = 0;
        if (version >= 400) {
            modMask2 = readUInt8();
        }
        if (modMask1 & EFFECT_BEND) {
            readBend(note);
        }
        if (modMask1 & EFFECT_GRACE) {
            // GP3 grace note
            int fret = readUInt8();                  // grace fret
            int dynamic = readUInt8();                  // grace dynamic
            int transition = readUInt8();                  // grace transition
            int duration = readUInt8();                  // grace duration

            int grace_len = Constants::DIVISION / 8;
            if (duration == 1) {
                grace_len = Constants::DIVISION / 8;       //32nd
            } else if (duration == 2) {
                grace_len = Constants::DIVISION / 6;       //24th
            } else if (duration == 3) {
                grace_len = Constants::DIVISION / 4;       //16th
            }
            Note* gn = Factory::createNote(score->dummy()->chord());

            if (fret == 255) {
                gn->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                gn->setGhost(true);
            }
            if (fret == 255) {
                fret = 0;
            }
            gn->setFret(fret);
            gn->setString(string);
            int grace_pitch = note->staff()->part()->instrument()->stringData()->getPitch(string, fret, nullptr);
            gn->setPitch(grace_pitch);
            gn->setTpcFromPitch();

            Chord* gc = nullptr;
            if (note->chord()->graceNotes().size()) {
                gc = note->chord()->graceNotes().front();
            }
            if (!gc) {
                gc = Factory::createChord(score->dummy()->segment());
                TDuration d;
                d.setVal(grace_len);
                if (grace_len == Constants::DIVISION / 6) {
                    d.setDots(1);
                }
                gc->setDurationType(d);
                gc->setTicks(d.fraction());
                gc->setNoteType(NoteType::ACCIACCATURA);
                gc->mutldata()->setMag(note->chord()->staff()->staffMag(Fraction(0, 1)) * score->style().styleD(Sid::graceNoteMag));
                note->chord()->add(gc);         // sets parent + track
                addDynamic(gn, dynamic);
            }

            gc->add(gn);

            if (transition == 0) {
                // no transition
            } else if (transition == 1) {
                //note->setSlideNote(gn);
                Glissando* glis = new Glissando(score->dummy());
                glis->setGlissandoType(GlissandoType::STRAIGHT);
                gn->chord()->add(glis);
                glis->setAnchor(Spanner::Anchor::NOTE);
                glis->setStartElement(gn);
                glis->setTick(gn->chord()->tick());
                glis->setTrack(gn->track());
                glis->setParent(gn);
                glis->setEndElement(note);
                glis->setTick2(note->chord()->tick());
                glis->setTrack2(note->track());
                score->addElement(glis);
                //HammerOn here??? Maybe version...

                Slur* slur1 = Factory::createSlur(score->dummy());
                slur1->setStartElement(gc);
                slur1->setEndElement(note->chord());
                slur1->setTick(gc->tick());
                slur1->setTick2(note->chord()->tick());
                slur1->setTrack(gc->track());
                slur1->setTrack2(note->track());
                score->addElement(slur1);

                //TODO: Add a 'slide' guitar effect when implemented
            } else if (transition == 2 && fretNumber >= 0 && fretNumber <= 255 && fretNumber != gn->fret()) {
                /*QList<PitchValue> points;
                points.append(PitchValue(0,0, false));
                points.append(PitchValue(60,(fretNumber-gn->fret())*100, false));

                Bend* b = Factory::createBend(note->score());
                b->setPoints(points);
                b->setTrack(gn->track());
                gn->add(b);*/
            } else if (transition == 3) {
                // TODO:
                //     major: replace with a 'hammer-on' guitar effect when implemented
                //     minor: make slurs for parts

                ChordRest* cr1 = static_cast<Chord*>(gc);
                ChordRest* cr2 = static_cast<Chord*>(note->chord());

                Slur* slur1 = Factory::createSlur(score->dummy());
                slur1->setStartElement(cr1);
                slur1->setEndElement(cr2);
                slur1->setTick(cr1->tick());
                slur1->setTick2(cr2->tick());
                slur1->setTrack(cr1->track());
                slur1->setTrack2(cr2->track());
                score->addElement(slur1);
            }
        }
        if (modMask1 & EFFECT_HAMMER) {         // hammer on / pull off
            readResult.slur = true;
        }
        if (modMask1 & EFFECT_LET_RING) {       // let ring
            readResult.letRing = true;
        }
        if (modMask1 & EFFECT_SLIDE_OLD) {
            slideList.push_back(note);
        }

        if (version >= 400) {
            if (modMask2 & EFFECT_STACCATO) {
            }
            if (modMask2 & EFFECT_PALM_MUTE) {
                readResult.palmMute = true;
            }
            if (modMask2 & EFFECT_TREMOLO) {
                readUInt8();
            }
            if (modMask2 & EFFECT_ARTIFICIAL_HARMONIC) {
                /*int type =*/
                readUInt8();
                //TODO-ws			if (type == 1 || type == 4 || type == 5)
                //				      note->setHarmonic(true);
            }
            if (modMask2 & EFFECT_TRILL) {
                //TODO-ws                        note->setTrillFret(readUInt8());      // trill fret
                readUInt8();              // trill length
            }
        }
    }
    if (fretNumber == -1) {
        LOGD("Note: no fret number, tie %d", tieNote);
    }
    Staff* staff = note->staff();
    if (fretNumber == 255) {
        fretNumber = 0;
        note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        note->setGhost(true);
    }
    // dead note represented as high numbers - fix to zero
    if (fretNumber > 99 || fretNumber == -1) {
        fretNumber = 0;
    }
    int pitch = staff->part()->instrument()->stringData()->getPitch(string, fretNumber, nullptr);

    /* it's possible to specify extraordinarily high pitches by
    specifying fret numbers that don't exist. This is an issue that
    comes from tuxguitar. Just set to maximum pitch. GP6 actually
    sets the fret number to 0 also, so that's what I've opted to do
    here. */
    if (pitch > MAX_PITCH) {
        fretNumber = 0;
        pitch = MAX_PITCH;
    }

    note->setFret(fretNumber);
    note->setString(string);
    note->setPitch(pitch);

    if (tieNote) {
        bool found = false;
        Chord* chord     = note->chord();
        Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
        track_idx_t track = note->track();
        std::vector<Chord*> chords;
        Note* true_note = nullptr;
        while (segment) {
            EngravingItem* e = segment->element(track);
            if (e) {
                if (e->isChord()) {
                    Chord* chord2 = toChord(e);
                    for (Note* note2 : chord2->notes()) {
                        if (note2->string() == string) {
                            if (chords.empty()) {
                                Tie* tie = Factory::createTie(note2);
                                tie->setEndNote(note);
                                note2->add(tie);
                            }
                            note->setFret(note2->fret());
                            note->setPitch(note2->pitch());
                            found = true;
                            true_note = note2;
                            break;
                        }
                    }
                    if (!found) {
                        chords.push_back(chord2);
                    }
                }
                if (found) {
                    break;
                }
            }
            segment = segment->prev1(SegmentType::ChordRest);
        }

        if (chords.size() && true_note) {
            Note* end_note = note;
            for (unsigned int i = 0; i < chords.size(); ++i) {
                Note* note2 = Factory::createNote(chords[i]);
                note2->setString(true_note->string());
                note2->setFret(true_note->fret());
                note2->setPitch(true_note->pitch());
                note2->setTpcFromPitch();
                chords[i]->add(note2);
                Tie* tie = Factory::createTie(note2);
                tie->setEndNote(end_note);
                end_note = note2;
                note2->add(tie);
            }
            Tie* tie = Factory::createTie(true_note);
            tie->setEndNote(end_note);
            true_note->add(tie);
        }
    }

    return readResult;
}

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro1::readBeatEffects(int, Segment*)
{
    uint8_t fxBits1 = readUInt8();
    if (fxBits1 & BEAT_EFFECT) {
        uint8_t num = readUInt8();
        switch (num) {
        case 0:                     // tremolo bar
            readInt();
            break;
        default:
            readInt();
            break;
        }
    }
    if (fxBits1 & BEAT_ARPEGGIO) {
        readUInt8();                // down stroke length
        readUInt8();                // up stroke length
    }
    return 0;
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro3::read(IODevice* io)
{
    m_continiousElementsBuilder = std::make_unique<ContiniousElementsBuilder>(score);
    f      = io;
    curPos = 30;

    title        = readDelphiString();
    subtitle     = readDelphiString();
    artist       = readDelphiString();
    album        = readDelphiString();
    composer     = readDelphiString();
    String copyright = readDelphiString();

    if (!copyright.isEmpty()) {
        score->setMetaTag(u"copyright", copyright);
    }

    readDelphiString(); // transcriber
    readDelphiString(); // instructions
    for (int i = 0, n = readInt(); i < n; ++i) {
        comments.append(readDelphiString());
    }

    /*uint8_t num =*/ readUInt8();        // Shuffle rhythm feel

    int temp = readInt();

    // int octave = 0;
    key = readInt();      // key

    for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
        channelDefaults[i].patch   = readInt();
        channelDefaults[i].volume  = readUInt8() * 8 - 1;
        channelDefaults[i].pan     = readUInt8() * 8 - 1;
        channelDefaults[i].chorus  = readUInt8() * 8 - 1;
        channelDefaults[i].reverb  = readUInt8() * 8 - 1;
        channelDefaults[i].phase   = readUInt8() * 8 - 1;
        channelDefaults[i].tremolo = readUInt8() * 8 - 1;
        readUInt8();          // padding
        readUInt8();
    }
    measures   = readInt();
    staves = readInt();
    initDynamics(staves);

    slurs = new Slur*[staves];
    for (size_t i = 0; i < staves; ++i) {
        slurs[i] = nullptr;
    }

    int tnumerator   = 4;
    int tdenominator = 4;
    for (size_t i = 0; i < measures; ++i) {
        GpBar bar;
        uint8_t barBits = readUInt8();
        if (barBits & SCORE_TIMESIG_NUMERATOR) {
            tnumerator = readUInt8();
        }
        if (barBits & SCORE_TIMESIG_DENOMINATOR) {
            tdenominator = readUInt8();
        }
        if (barBits & SCORE_REPEAT_START) {
            bar.repeatFlags = bar.repeatFlags | mu::engraving::Repeat::START;
        }
        if (barBits & SCORE_REPEAT_END) {                    // number of repeats
            bar.repeatFlags = bar.repeatFlags | mu::engraving::Repeat::END;
            bar.repeats = readUInt8() + 1;
        }
        if (barBits & SCORE_VOLTA) {                          // a volta
            uint8_t voltaNumber = readUInt8();
            // voltas are represented as a binary number
            bar.volta.voltaType = GP_VOLTA_BINARY;
            while (voltaNumber > 0) {
                bar.volta.voltaInfo.push_back(voltaNumber & 1);
                voltaNumber >>= 1;
            }
        }
        if (barBits & SCORE_MARKER) {
            bar.marker = readDelphiString();           // new section?
            /*int color =*/ readInt();          // color?
        }
        if (barBits & SCORE_KEYSIG) {
            int currentKey = readUInt8();
            /* key signatures are specified as
             * 1# = 1, 2# = 2, ..., 7# = 7
             * 1b = 255, 2b = 254, ... 7b = 249 */
            bar.keysig = currentKey <= 7 ? currentKey : -256 + currentKey;
            readUInt8();              // specifies major/minor mode
        }

        if (barBits & SCORE_DOUBLE_BAR) {
            bar.barLine = BarLineType::DOUBLE;
        }
        bar.timesig = Fraction(tnumerator, tdenominator);
        bars.push_back(bar);
    }

    //
    // create a part for every staff
    //
    for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
        Part* part = new Part(score);
        Staff* s = Factory::createStaff(part);

        score->appendStaff(s);
        score->appendPart(part);
    }

    Fraction ts;
    Fraction tick = { 0, 1 };
    for (size_t i = 0; i < measures; ++i) {
        Fraction nts = bars[i].timesig;
        Measure* m = Factory::createMeasure(score->dummy()->system());
        m->setTick(tick);
        m->setTimesig(nts);
        m->setTicks(nts);

        if (i == 0 || ts != nts) {
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                TimeSig* t = Factory::createTimeSig(s);
                t->setTrack(staffIdx * VOICES);
                t->setSig(nts);
                s->add(t);
            }
        }
        if (i == 0 && key) {
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                Segment* s = m->getSegment(SegmentType::KeySig, tick);
                KeySig* t = Factory::createKeySig(s);
                t->setKey(Key(key));
                t->setTrack(staffIdx * VOICES);
                s->add(t);
            }
        }

        readVolta(&bars[i].volta, m);
        m->setRepeatEnd(bars[i].repeatFlags == mu::engraving::Repeat::END);
        m->setRepeatStart(bars[i].repeatFlags == mu::engraving::Repeat::START);
        m->setRepeatJump(bars[i].repeatFlags == mu::engraving::Repeat::JUMP);
        //            m->setRepeatFlags(bars[i].repeatFlags);
        m->setRepeatCount(bars[i].repeats);

        // reset the volta sequence if we have an opening repeat
        if (bars[i].repeatFlags == mu::engraving::Repeat::START) {
            voltaSequence = 1;
        }
        // otherwise, if we see an end repeat symbol, only reset if the bar after it does not contain a volta
        else if (bars[i].repeatFlags == mu::engraving::Repeat::END && i < bars.size() - 1) {
            if (bars[i + 1].volta.voltaInfo.size() == 0) {
                voltaSequence = 1;
            }
        }

        score->measures()->append(m);
        tick += nts;
        ts = nts;
    }

    for (size_t i = 0; i < staves; ++i) {
        int tuning[GP_MAX_STRING_NUMBER];

        uint8_t c      = readUInt8();       // simulations bitmask
        if (c & 0x2) {                    // 12 stringed guitar
        }
        if (c & 0x4) {                    // banjo track
        }
        String name = readPascalString(40);
        int strings  = readInt();
        if (strings <= 0 || strings > GP_MAX_STRING_NUMBER) {
            return false;
        }
        for (int j = 0; j < strings; ++j) {
            tuning[j] = readInt();
        }
        for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j) {
            readInt();
        }
        int midiPort     = readInt() - 1;
        int midiChannel  = readInt() - 1;
        /*int midiChannel2 =*/ readInt();     // - 1;
        int frets        = readInt();
        int capo         = readInt();
        /*int color        =*/ readInt();

        std::vector<int> tuning2(strings);
        //int tuning2[strings];
        for (int k = 0; k < strings; ++k) {
            tuning2[strings - k - 1] = tuning[k];
        }
        bool useFlats = createTuningString(strings, &tuning2[0]);
        StringData stringData(frets, strings, &tuning2[0], useFlats);
        Part* part = score->staff(i)->part();
        Instrument* instr = part->instrument();
        instr->setStringData(stringData);
        instr->setSingleNoteDynamics(false);
        part->setPartName(name);
        part->setPlainLongName(name);
        //
        // determine clef
        //
        Staff* staff = score->staff(i);
        int patch = channelDefaults[midiChannel].patch;
        ClefType clefId = ClefType::G;
        if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
            clefId = ClefType::PERC;
            StaffTypes type = StaffTypes::PERC_DEFAULT;
            if (auto it = drumset::PERC_STAFF_LINES_FROM_INSTRUMENT.find(name.toStdString());
                it != drumset::PERC_STAFF_LINES_FROM_INSTRUMENT.end()) {
                drumset::initGuitarProPercussionSet(it->second);
                drumset::setInstrumentDrumset(instr, it->second);
                switch (it->second.numLines) {
                case 1:
                    type = StaffTypes::PERC_1LINE;
                    break;
                case 2:
                    type = StaffTypes::PERC_2LINE;
                    break;
                case 3:
                    type = StaffTypes::PERC_3LINE;
                    break;
                default:
                    type = StaffTypes::PERC_DEFAULT;
                    break;
                }
            } else {
                drumset::initGuitarProDrumset();
                instr->setDrumset(drumset::gpDrumset);
            }
            staff->setStaffType(Fraction(0, 1), *StaffType::preset(type));
        } else {
            clefId = defaultClef(patch);
        }
        Measure* measure = score->firstMeasure();
        Segment* segment = measure->getSegment(SegmentType::HeaderClef, Fraction(0, 1));
        Clef* clef = Factory::createClef(segment);
        clef->setClefType(clefId);
        clef->setTrack(i * VOICES);
        segment->add(clef);

        if (capo > 0) {
            Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
            StaffText* st = new StaffText(s);
            //                  st->setTextStyleType(TextStyleType::STAFF);
            st->setPlainText(String(u"Capo. fret ") + String::number(capo));
            st->setTrack(i * VOICES);
            s->add(st);
        }

        InstrChannel* ch = instr->channel(0);
        if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
            ch->setProgram(0);
            ch->setBank(128);
        } else {
            ch->setProgram(patch);
            ch->setBank(0);
        }
        ch->setVolume(channelDefaults[midiChannel].volume);
        ch->setPan(channelDefaults[midiChannel].pan);
        ch->setChorus(channelDefaults[midiChannel].chorus);
        ch->setReverb(channelDefaults[midiChannel].reverb);
        staff->part()->setMidiChannel(midiChannel, midiPort);
        // missing: phase, tremolo
    }

    previousTempo = temp;
    Measure* measure = score->firstMeasure();
    bool mixChange = false;
    for (size_t bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
        const GpBar& gpbar = bars[bar];

        if (!gpbar.marker.isEmpty()) {
            Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
            RehearsalMark* s = new RehearsalMark(segment);
            s->setPlainText(gpbar.marker.trimmed());
            s->setTrack(0);
            segment->add(s);
        }

        std::vector<Tuplet*> tuplets(staves);
        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            tuplets[staffIdx] = 0;
        }

        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Fraction measureLen = { 0, 1 };
            track_idx_t track = staffIdx * VOICES;
            Fraction fraction = measure->tick();
            int beats = readInt();
            if (beats > 200) {
                return false;
            }
            for (int beat = 0; beat < beats; ++beat) {
                bool hasSlur = false;
                bool hasLetRing = false;
                bool hasPalmMute = false;
                bool hasVibratoLeftHand = false;
                bool hasVibratoWTremBar = false;

                //                        int pause = 0;
                uint8_t beatBits = readUInt8();
                bool dotted = beatBits & BEAT_DOTTED;
                if (beatBits & BEAT_PAUSE) {
                    /*pause =*/
                    readUInt8();
                }

                slide = -1;
                if (muse::contains(slides, static_cast<int>(track))) {
                    slide = muse::take(slides, static_cast<int>(track));
                }

                int len = readChar();
                int tuple = 0;
                if (beatBits & BEAT_TUPLET) {
                    tuple = readInt();
                }

                Segment* segment = measure->getSegment(SegmentType::ChordRest, fraction);
                if (beatBits & BEAT_CHORD) {
                    int numStrings = static_cast<int>(score->staff(staffIdx)->part()->instrument()->stringData()->strings());
                    int header = readUInt8();
                    String name;
                    if ((header & 1) == 0) {
                        name = readDelphiString();
                        readChord(segment, static_cast<int>(track), numStrings, name, false);
                    } else {
                        skip(25);
                        name = readPascalString(34);
                        readChord(segment, static_cast<int>(track), numStrings, name, false);
                        skip(36);
                    }
                }
                Lyrics* lyrics = 0;
                if (beatBits & BEAT_LYRICS) {
                    String txt = readDelphiString();
                    Lyrics* lyrics2 = Factory::createLyrics(score->dummy()->chord());
                    lyrics2->setPlainText(txt);
                }
                int beatEffects = 0;

                if (beatBits & BEAT_EFFECTS) {
                    beatEffects = readBeatEffects(static_cast<int>(track), segment);
                }
                bool vibrato = beatEffects & 0x1 || beatEffects & 0x2;

                if (beatBits & BEAT_MIX_CHANGE) {
                    readMixChange(measure);
                    mixChange = true;
                }

                int strings = readUInt8();           // used strings mask

                Fraction l = len2fraction(len);

                // Some beat effects could add a Chord before this
                ChordRest* cr = segment->cr(track);
                // if (!pause || strings)
                if (strings) {
                    if (!segment->cr(track)) {
                        cr = Factory::createChord(score->dummy()->segment());
                    }
                } else {
                    if (segment->cr(track)) {
                        segment->remove(segment->cr(track));
                        delete cr;
                        cr = 0;
                    }
                    cr = Factory::createRest(score->dummy()->segment());
                }

                cr->setTrack(track);
                if (lyrics) {
                    cr->add(lyrics);
                }

                TDuration d(l);
                d.setDots(dotted ? 1 : 0);

                if (dotted) {
                    l = l + (l * Fraction(1, 2));
                }

                if (tuple) {
                    Tuplet* tuplet = tuplets[staffIdx];
                    if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                        tuplet = Factory::createTuplet(measure);
                        tuplet->setTick(fraction);
                        tuplet->setTrack(cr->track());
                        tuplets[staffIdx] = tuplet;
                        setTuplet(tuplet, tuple);
                        tuplet->setParent(measure);
                    }
                    tuplet->setTrack(track);
                    tuplet->setBaseLen(l);
                    tuplet->setTicks(l * tuplet->ratio().denominator());
                    cr->setTuplet(tuplet);
                    tuplet->add(cr);
                }

                cr->setTicks(l);

                if (cr->type() == ElementType::REST && l >= measure->ticks()) {
                    cr->setDurationType(DurationType::V_MEASURE);
                    cr->setTicks(measure->ticks());
                } else {
                    cr->setDurationType(d);
                }

                if (!segment->cr(track)) {
                    segment->add(cr);
                }

                Staff* staff = cr->staff();
                int numStrings = static_cast<int>(staff->part()->instrument()->stringData()->strings());
                for (int i = 6; i >= 0; --i) {
                    if (strings & (1 << i) && ((6 - i) < numStrings)) {
                        Note* note = Factory::createNote(toChord(cr));
                        toChord(cr)->add(note);
                        if (vibrato) {
                            hasVibratoLeftHand = true;
                        }
                        if (dotted) {
                            NoteDot* dot = Factory::createNoteDot(note);
                            // there is at most one dotted note in this guitar pro version - set 0 index
                            dot->setParent(note);
                            dot->setTrack(track);                // needed to know the staff it belongs to (and detect tablature)
                            dot->setVisible(true);
                            note->add(dot);
                        }
                        ReadNoteResult readResult = readNote(6 - i, note);
                        hasSlur = readResult.slur || hasSlur;
                        hasLetRing = readResult.letRing || hasLetRing;
                        hasPalmMute = readResult.palmMute || hasPalmMute;
                        hasVibratoLeftHand = readResult.vibrato || hasVibratoLeftHand;
                        note->setTpcFromPitch();
                    }
                }
                if (cr && cr->type() == ElementType::CHORD && static_cast<Chord*>(cr)->notes().empty()) {
                    if (segment->cr(track)) {
                        segment->remove(cr);
                    }
                    delete cr;
                    cr = Factory::createRest(segment);
                    cr->setTicks(l);
                    cr->setTrack(track);
                    cr->setDurationType(d);
                    segment->add(cr);
                }
                createSlur(hasSlur, staffIdx, cr);

                if (cr && (cr->isChord())) {
                    if (beatEffects >= 200) {
                        beatEffects -= 200;
                        Articulation* art = Factory::createArticulation(cr);
                        art->setSymId(SymId::guitarFadeOut);
                        art->setAnchor(ArticulationAnchor::TOP);
                        art->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
                        if (!score->toggleArticulation(cr, art)) {
                            delete art;
                        }
                    }

                    bool hasVibratoLeftHandOnBeat = false;
                    bool hasVibratoWTremBarOnBeat = false;
                    applyBeatEffects(static_cast<Chord*>(cr), beatEffects, hasVibratoLeftHandOnBeat, hasVibratoWTremBarOnBeat);
                    hasVibratoLeftHand = hasVibratoLeftHand || hasVibratoLeftHandOnBeat;
                    hasVibratoWTremBar = hasVibratoWTremBar || hasVibratoWTremBarOnBeat;
                    if (slide > 0) {
                        createSlide(slide, cr, static_cast<int>(staffIdx));
                    }
                }

                if (cr) {
                    addLetRing(cr, hasLetRing);
                    addPalmMute(cr, hasPalmMute);
                    addVibratoLeftHand(cr, hasVibratoLeftHand);
                    addVibratoWTremBar(cr, hasVibratoWTremBar);
                }

                restsForEmptyBeats(segment, measure, cr, l, static_cast<int>(track), fraction);
                fraction += cr->actualTicks();
                measureLen += cr->actualTicks();
            }
            if (measureLen < measure->ticks()) {
                score->setRest(fraction, track, measure->ticks() - measureLen, false, nullptr, false);
                m_continiousElementsBuilder->notifyUncompletedMeasure();
            }
            bool removeRests = true;
            int counter = 0;
            Rest* lastRest = nullptr;
            for (auto seg = measure->first(); seg; seg = seg->next()) {
                if (seg->segmentType() == SegmentType::ChordRest) {
                    auto cr = seg->cr(track);
                    if (cr && cr->type() == ElementType::CHORD) {
                        removeRests = false;
                        break;
                    } else if (cr) {
                        ++counter;
                        lastRest = static_cast<Rest*>(cr);
                    }
                }
            }
            if (removeRests && counter < 2) {
                removeRests = false;
                if (counter == 1) {
                    lastRest->setTicks(measure->timesig());
                    lastRest->setDurationType(DurationType::V_MEASURE);
                }
            }
            if (removeRests) {
                auto seg = measure->first();
                while (seg && seg != measure->last())
                {
                    if (seg->segmentType() == SegmentType::ChordRest) {
                        auto cr = seg->cr(track);
                        if (cr) {
                            seg->remove(cr);
                            delete cr;
                        }
                    }
                    seg = seg->next();
                }
                auto cr = Factory::createRest(seg);
                cr->setTicks(measure->timesig());
                cr->setDurationType(DurationType::V_MEASURE);
                cr->setTrack(track);
                seg->add(cr);
            }
        }
        if (bar == 1 && !mixChange) {
            setTempo(temp, score->firstMeasure());
        }
    }
    for (auto n : slideList) {
        auto segment = n->chord()->segment();
        auto measure1 = segment->measure();
        while ((segment = segment->next1(SegmentType::ChordRest))
               || ((measure1 = measure1->nextMeasure()) && (segment = measure1->first()))) {
            // bool br = false;
            auto crest = segment->cr(n->track());
            if (!crest) {
                continue;
            }
            if (crest->type() == mu::engraving::ElementType::REST) {
                break;
            }
            auto cr = static_cast<Chord*>(crest);
            if (!cr) {
                continue;
            }
            if (cr->graceNotes().size()) {
                cr = cr->graceNotes().front();
            }
            if (cr) {
                for (auto nt : cr->notes()) {
                    if (nt->string() == n->string()) {
                        // auto mg = nt->magS();
                        Glissando* s = new Glissando(n);
                        s->setAnchor(Spanner::Anchor::NOTE);
                        s->setStartElement(n);
                        s->setTick(n->chord()->segment()->tick());
                        s->setTrack(n->track());
                        s->setParent(n);
                        s->setGlissandoType(GlissandoType::STRAIGHT);
                        s->setEndElement(nt);
                        s->setTick2(cr->segment()->tick());
                        s->setTrack2(n->track());
                        score->addElement(s);
                        break;
                    }
                }
            }
            break;
        }
    }

    m_continiousElementsBuilder->addElementsToScore();
    if (engravingConfiguration()->experimentalGuitarBendImport()) {
        m_guitarBendImporter->applyBendsToChords();
    }

    return true;
}

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro3::readBeatEffects(int track, Segment* segment)
{
    int effects = 0;
    uint8_t fxBits = readUInt8();

    if (fxBits & BEAT_EFFECT) {
        effects = readUInt8();          // effect 1-tapping, 2-slapping, 3-popping
        readInt();     // we don't need this integer
    }

    if (fxBits & BEAT_ARPEGGIO) {
        int strokeup = readUInt8();                // up stroke length
        int strokedown = readUInt8();                // down stroke length

        Arpeggio* a = Factory::createArpeggio(score->dummy()->chord());
        if (strokeup > 0) {
            a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
        } else if (strokedown > 0) {
            a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
        } else {
            delete a;
            a = 0;
        }

        if (a) {
            ChordRest* cr = Factory::createChord(segment);
            cr->setTrack(track);
            cr->add(a);
            segment->add(cr);
        }
    }
    if (fxBits & BEAT_TREMOLO) {
    }
    if (fxBits & BEAT_FADE) {
        effects += 200;
    }
    if (fxBits & BEAT_DOTTED) {
    }
    if (fxBits & BEAT_CHORD) {
    }
    if (effects == 0) {
        return fxBits;
    }
    return effects;
}

//---------------------------------------------------------
//   readTremoloBar
//---------------------------------------------------------

void GuitarPro::readTremoloBar(int /*track*/, Segment* /*segment*/)
{
    /*int a1 =*/
    readChar();
    /*int a2 =*/ readChar();
    /*int a3 =*/ readChar();
    /*int a4 =*/ readChar();
    /*int a5 =*/ readChar();
    int n  =  readInt();
    std::vector<PitchValue> points;
    for (int i = 0; i < n; ++i) {
        int time    = readInt();
        int pitch   = readInt();
        int vibrato = readUInt8();
        points.push_back(PitchValue(time, pitch, vibrato));
    }
}

//---------------------------------------------------------
//   addMetaInfo
//---------------------------------------------------------

static void addMetaInfo(MasterScore* score, GuitarPro* gp, bool experimental)
{
    std::vector<String> fieldNames = { gp->title, gp->subtitle, gp->artist,
                                       gp->album, gp->composer };

    bool createTitleField
        = std::any_of(fieldNames.begin(), fieldNames.end(), [](const String& fieldName) { return !fieldName.isEmpty(); });

    if (!createTitleField && !experimental) {
        return;
    }

    MeasureBase* m = nullptr;
    if (!score->measures()->first()) {
        m = Factory::createTitleVBox(score->dummy()->system());
        score->measures()->append(m);
    } else {
        m = score->measures()->first();
        if (!m->isVBox()) {
            MeasureBase* mb = Factory::createTitleVBox(score->dummy()->system());
            score->addMeasure(mb, m);
            m = mb;
        }
    }
    if (!gp->title.isEmpty() || experimental) {
        Text* s = Factory::createText(m, TextStyleType::TITLE);
        s->setPlainText(gp->title);
        m->add(s);
    }
    if (!gp->subtitle.isEmpty() || !gp->artist.isEmpty() || !gp->album.isEmpty() || experimental) {
        Text* s = Factory::createText(m, TextStyleType::SUBTITLE);
        String str;
        if (!gp->subtitle.isEmpty()) {
            str.append(gp->subtitle);
        }
        if (!gp->artist.isEmpty()) {
            if (!str.isEmpty()) {
                str.append(u'\n');
            }
            str.append(gp->artist);
        }
        if (!gp->album.isEmpty()) {
            if (!str.isEmpty()) {
                str.append(u'\n');
            }
            str.append(gp->album);
        }
        s->setPlainText(str);
        m->add(s);
    }
    if (!gp->composer.isEmpty()) {
        Text* s = Factory::createText(m, TextStyleType::COMPOSER);
        s->setPlainText(gp->composer);
        m->add(s);
    }

    //TODO: Temporary for experimental import, will be deleted later
    if (experimental) {
        Text* s = Factory::createText(score->dummy(), TextStyleType::LYRICIST);
        m->add(s);
    }
}

//---------------------------------------------------------
//   createLinkedTabs
//---------------------------------------------------------

static void createLinkedTabs(MasterScore* score)
{
    // store map of all initial spanners
    std::unordered_map<staff_idx_t, std::vector<Spanner*> > spanners;
    // for moving initial spanner to new index
    std::unordered_map<staff_idx_t, staff_idx_t> indexMapping;
    std::set<staff_idx_t> staffIndexesToCopy;
    constexpr size_t stavesInPart = 2;

    for (auto it = score->spanner().cbegin(); it != score->spanner().cend(); ++it) {
        Spanner* s = it->second;
        spanners[s->staffIdx()].push_back(s);
    }

    size_t curStaffIdx = 0;
    size_t stavesOperated = 0;

    // creating linked staves and recalculating spanners indexes
    for (size_t partNum = 0; partNum < score->parts().size(); partNum++) {
        Part* part = score->parts()[partNum];
        Fraction fr = Fraction(0, 1);
        size_t lines = part->instrument()->stringData()->strings();
        size_t stavesNum = part->nstaves();

        if (stavesNum != 1) {
            for (size_t i = 0; i < stavesNum; i++) {
                indexMapping[curStaffIdx] = stavesOperated + i;
                curStaffIdx++;
            }

            stavesOperated += stavesNum;
            continue;
        }

        bool needsTabStaff = !part->staff(0)->isDrumStaff(fr);

        if (needsTabStaff) {
            part->setStaves(static_cast<int>(stavesInPart));

            Staff* srcStaff = part->staff(0);
            Staff* dstStaff = part->staff(1);
            Excerpt::cloneStaff(srcStaff, dstStaff, false);

            static const std::vector<StaffTypes> types {
                StaffTypes::TAB_4SIMPLE,
                StaffTypes::TAB_5SIMPLE,
                StaffTypes::TAB_6SIMPLE,
                StaffTypes::TAB_7SIMPLE,
                StaffTypes::TAB_8SIMPLE,
                StaffTypes::TAB_9SIMPLE,
                StaffTypes::TAB_10SIMPLE,
            };

            size_t index = (lines >= 4 && lines <= 10) ? lines - 4 : 2;

            dstStaff->setStaffType(fr, *StaffType::preset(types.at(index)));
            dstStaff->setLines(fr, static_cast<int>(lines));

            staffIndexesToCopy.insert(curStaffIdx);
        }

        // each spanner moves down to the staff with index,
        // equal to number of spanners operated before it
        indexMapping[curStaffIdx] = stavesOperated;
        curStaffIdx++;

        stavesOperated += needsTabStaff ? stavesInPart : 1;
    }

    // moving and copying spanner segments
    for (auto& spannerMapElem : spanners) {
        auto& spannerList = spannerMapElem.second;
        staff_idx_t idx = spannerMapElem.first;
        bool needsCopy = staffIndexesToCopy.find(idx) != staffIndexesToCopy.end();
        for (Spanner* s : spannerList) {
            /// moving
            staff_idx_t newIdx = indexMapping[idx];
            track_idx_t newTrackIdx = staff2track(newIdx);
            s->setTrack(newTrackIdx);
            s->setTrack2(newTrackIdx);
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setTrack(newTrackIdx);
            }

            /// copying
            if (needsCopy) {
                staff_idx_t dstStaffIdx = newIdx + 1;

                track_idx_t dstTrack = dstStaffIdx * VOICES + s->voice();
                track_idx_t dstTrack2 = dstStaffIdx * VOICES + (s->track2() % VOICES);

                Excerpt::cloneSpanner(s, score, dstTrack, dstTrack2);
            }
        }
    }
}

//---------------------------------------------------------
//   importScore
//---------------------------------------------------------

static Err importScore(MasterScore* score, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx, bool experimental = false)
{
    if (!io->open(IODevice::ReadOnly)) {
        return Err::FileOpenError;
    }

    score->checkChordList();
    io->seek(0);
    char header[5];
    io->read((uint8_t*)(header), 4);
    header[4] = 0;
    io->seek(0);
    if (strcmp(header, "ptab") == 0) {
        PowerTab ptb(io, score);
        return ptb.read();
    }

    GuitarPro* gp;
    bool readResult = false;
    // check to see if we are dealing with a GP file via the extension
    if (strcmp(header, "PK\x3\x4") == 0) {
        gp = new GuitarPro7(score, iocCtx);
        readResult = gp->read(io);
        gp->setTempo(0, 0);
    }
    // check to see if we are dealing with a GPX file via the extension
    else if (strcmp(header, "BCFZ") == 0) {
        gp = new GuitarPro6(score, iocCtx);
        drumset::initGuitarProDrumset();
        readResult = gp->read(io);
        gp->setTempo(0, 0);
    }
    // otherwise it's an older version - check the header
    else if (strcmp(&header[1], "FIC") == 0) {
        uint8_t l;
        io->read((uint8_t*)&l, 1);
        char ss[30];
        io->read((uint8_t*)(ss), 30);
        ss[l] = 0;
        String s = String::fromUtf8(ss);
        if (s.startsWith(u"FICHIER GUITAR PRO ")) {
            s = s.mid(20);
        } else if (s.startsWith(u"FICHIER GUITARE PRO ")) {
            s = s.mid(21);
        } else {
            LOGD("unknown gtp format <%s>", ss);
            return Err::FileBadFormat;
        }
        int a = s.left(1).toInt();
        int b = s.mid(2).toInt();
        int version = a * 100 + b;
        if (a == 1) {
            gp = new GuitarPro1(score, version, iocCtx);
        } else if (a == 2) {
            gp = new GuitarPro2(score, version, iocCtx);
        } else if (a == 3) {
            gp = new GuitarPro3(score, version, iocCtx);
        } else if (a == 4) {
            gp = new GuitarPro4(score, version, iocCtx);
        } else if (a == 5) {
            gp = new GuitarPro5(score, version, iocCtx);
        } else {
            LOGD("unknown gtp format %d", version);
            return Err::FileBadFormat;
        }
        readResult = gp->read(io);
        gp->setTempo(0, 0);
    } else {
        return Err::FileBadFormat;
    }
    if (readResult == false) {
        LOGD("guitar pro import error====");
        // avoid another error message box
        return Err::NoError;
    }

    addMetaInfo(score, gp, experimental);

    int idx = 0;

    for (Measure* m1 = score->firstMeasure(); m1; m1 = m1->nextMeasure(), ++idx) {
        if (gp->bars.empty()) {
            break;
        }
        const GpBar& bar = gp->bars[idx];
        if (bar.barLine != BarLineType::NORMAL && bar.barLine != BarLineType::END_REPEAT && bar.barLine != BarLineType::START_REPEAT
            && bar.barLine != BarLineType::END_START_REPEAT) {
            m1->setEndBarLineType(bar.barLine, 0);
        }
    }
    if (score->lastMeasure() && score->lastMeasure()->endBarLineType() != BarLineType::NORMAL) {
        score->lastMeasure()->setEndBarLineType(BarLineType::END, false);
    }

    for (Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            pair.second->updateInstrumentId();
        }
    }

    score->setUpTempoMap();
    for (Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }

    delete gp;

    return Err::NoError;
}

//---------------------------------------------------------
//   importGTP
//---------------------------------------------------------

Err importGTP(MasterScore* score, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx, bool createLinkedTabForce,
              bool experimental)
{
    Err error = importScore(score, io, iocCtx, experimental);

    if (error != Err::NoError) {
        return error;
    }

    if (createLinkedTabForce) {
        createLinkedTabs(score);
    }

    return Err::NoError;
}
} // namespace mu::iex::guitarpro
