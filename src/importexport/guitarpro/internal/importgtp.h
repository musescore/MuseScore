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

#ifndef __IMPORTGTP_H__
#define __IMPORTGTP_H__

#include <vector>
#include <map>

#include "io/file.h"

#include "gtp/gp67dombuilder.h"
#include "libmscore/score.h"
#include "libmscore/vibrato.h"
#include "libmscore/articulation.h"
#include "engraving/engravingerrors.h"

#include "modularity/ioc.h"
#include "iengravingconfiguration.h"

namespace mu::engraving {
class Chord;
class LetRing;
class Measure;
class Note;
class PalmMute;
class Score;
class Segment;
class Tuplet;
class Vibrato;
class Volta;

static constexpr int GP_MAX_LYRIC_LINES = 5;
static constexpr int GP_MAX_TRACK_NUMBER = 32;
static constexpr int GP_MAX_STRING_NUMBER = 7;
static constexpr int GP_DEFAULT_PERCUSSION_CHANNEL = 9;
static constexpr int GP_INVALID_KEYSIG = 127;
static constexpr int GP_VOLTA_BINARY = 1;
static constexpr int GP_VOLTA_FLAGS = 2;

Err importGTP(Score* score, const String& filename, const char* data, unsigned int data_len);

enum class Repeat : char;

struct GpTrack {
    int patch;
    uint8_t volume, pan, chorus, reverb, phase, tremolo;
};

struct GPVolta {
    int voltaType;
    std::vector<int> voltaInfo;
};

/* How the fermatas are represented in Guitar Pro is two integers, the
 * first is an index value and the second is the time division that
 * index value refers to, and they are givin with respect to a
 * measure. Time division 0 means a minim, 1 is a crotchet, 2 is a
 * quaver and so on, with the index (counting from 0) referring to how
 * many time divisions occur before the fermata. These numbers are
 * separated in GP6 with a '/' character. For example, a note
 * occurring on the third beat of a measure in a 4/4 bar would be
 * represented as 2/1.
 */

struct GPFermata {
    int index;
    int timeDivision;
    String type;
};

struct GPLyrics {
    StringList lyrics;
    std::vector<Segment*> segments;
    std::vector<size_t> lyricPos;
    size_t fromBeat = 0;
    size_t beatCounter = 0;
    size_t lyricTrack = 0;
};

struct GpBar {
    Fraction timesig = Fraction(4, 4);
    bool freeTime = false;
    int keysig = GP_INVALID_KEYSIG;
    String marker;
    BarLineType barLine = BarLineType::NORMAL;
    Repeat repeatFlags = Repeat::NONE;
    int repeats = 2;
    GPVolta volta;
    String direction;
    String directionStyle;
    String section[2];
    std::vector<String> directions;
};

inline Drumset* gpDrumset = nullptr;

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

class GuitarPro
{
    INJECT(mu::engraving::IEngravingConfiguration, engravingConfiguration);

protected:

    enum class TabImportOption {
        STANDARD = 1,
        TAB      = 2,
        BOTH     = 3
    };

    struct GPProperties {
        std::vector<TabImportOption> partsImportOptions;
    };

    GPProperties m_properties;

    std::list<Note*> slideList;   //list of start slide notes

    // note effect bit masks
    static const uint8_t EFFECT_BEND = 0x1;
    static const uint8_t EFFECT_STACCATO = 0x1;
    static const uint8_t EFFECT_HAMMER = 0x2;
    static const uint8_t EFFECT_PALM_MUTE = 0x2;
    static const uint8_t EFFECT_TREMOLO = 0x4;
    static const uint8_t EFFECT_LET_RING = 0x8;
    static const uint8_t EFFECT_SLIDE_OLD = 0x4;
    static const uint8_t EFFECT_SLIDE = 0x8;
    static const uint8_t EFFECT_GRACE = 0x10;
    static const uint8_t EFFECT_ARTIFICIAL_HARMONIC = 0x10;
    static const uint8_t EFFECT_TRILL = 0x20;
    static const uint8_t EFFECT_GHOST = 0x01;

    // arpeggio direction masks
    static const uint8_t ARPEGGIO_UP = 0xa;
    static const uint8_t ARPEGGIO_DOWN = 0x2;

    // note bit masks
    static const uint8_t NOTE_GHOST = 0x04;   // 2
    static const uint8_t NOTE_DEAD = 0x20;   //5
    static const uint8_t NOTE_DYNAMIC = 0x10;   // 4
    static const uint8_t NOTE_FRET = 0x20;   //5
    static const uint8_t NOTE_FINGERING = 0x80;   //7
    static const uint8_t NOTE_MARCATO = 0x02;   //1
    static const uint8_t NOTE_SFORZATO = 0x40;   //6
    static const uint8_t NOTE_SLUR = 0x8;  //3
    static const uint8_t NOTE_APPOGIATURA = 0x02;  //1

    // beat bit masks
    static const uint8_t BEAT_VIBRATO_TREMOLO = 0x02;
    static const uint8_t BEAT_FADE = 0x10;
    static const uint8_t BEAT_EFFECT = 0x20;
    static const uint8_t BEAT_TREMOLO = 0x04;
    static const uint8_t BEAT_ARPEGGIO = 0x40;
    static const uint8_t BEAT_STROKE_DIR = 0x02;
    static const uint8_t BEAT_DOTTED = 0x01;
    static const uint8_t BEAT_PAUSE = 0x40;
    static const uint8_t BEAT_TUPLET = 0x20;
    static const uint8_t BEAT_LYRICS = 0x4;
    static const uint8_t BEAT_EFFECTS = 0x8;
    static const uint8_t BEAT_MIX_CHANGE = 0x10;
    static const uint8_t BEAT_CHORD = 0x2;

    // score bit masks
    static const uint8_t SCORE_TIMESIG_NUMERATOR = 0x1;
    static const uint8_t SCORE_TIMESIG_DENOMINATOR = 0x2;
    static const uint8_t SCORE_REPEAT_START = 0x4;
    static const uint8_t SCORE_REPEAT_END = 0x8;
    static const uint8_t SCORE_MARKER = 0x20;
    static const uint8_t SCORE_VOLTA = 0x10;
    static const uint8_t SCORE_KEYSIG = 0x40;
    static const uint8_t SCORE_DOUBLE_BAR = 0x80;

    // slide kinds
    static const int SHIFT_SLIDE = 1;
    static const int LEGATO_SLIDE = 2;
    static const int SLIDE_OUT_DOWN = 4;
    static const int SLIDE_OUT_UP = 8;
    static const int SLIDE_IN_ABOVE = 16;
    static const int SLIDE_IN_BELOW = 32;

    // harmonic mark types
    static const int HARMONIC_MARK_NATURAL = 1;
    static const int HARMONIC_MARK_ARTIFICIAL = 2;
    static const int HARMONIC_MARK_TAP = 3;
    static const int HARMONIC_MARK_PINCH = 4;
    static const int HARMONIC_MARK_SEMI = 5;

    static const int MAX_PITCH = 127;
    static const char* const errmsg[];
    int version;
    int key { 0 };

    Segment* last_segment   { nullptr };
    Measure* last_measure   { nullptr };
    int last_tempo          { -1 };

    std::map<int, std::vector<GPFermata>*> fermatas;
    std::vector<Ottava*> ottava;
    Hairpin** hairpins = nullptr;
    MasterScore* score = nullptr;
    mu::io::IODevice* f = nullptr;
    int curPos = 0;
    int previousTempo = -1;
    std::vector<int> previousDynamicByTrack;
    constexpr static int INVALID_DYNAMIC = -1;
    constexpr static int DEFAULT_DYNAMIC = 0;
    std::vector<int> ottavaFound;
    std::vector<String> ottavaValue;
    std::map<int, std::pair<int, bool> > tempoMap;
    int tempo = -1;
    std::map<int, int> slides;

    GPLyrics gpLyrics;
    int slide = 0;
    int voltaSequence = 0;
    Slur** slurs       { nullptr };

#ifdef ENGRAVING_USE_STRETCHED_BENDS
    std::vector<StretchedBend*> m_bends;
#else
    std::vector<Bend*> m_bends;
#endif

    void skip(int64_t len);
    void read(void* p, int64_t len);
    uint8_t readUInt8();
    int readChar();
    String readPascalString(int);

    String readWordPascalString();
    String readBytePascalString();
    int readInt();
    String readDelphiString();
    void readVolta(GPVolta*, Measure*);
    virtual void readBend(Note*);
    virtual bool readMixChange(Measure* measure);
    virtual int readBeatEffects(int track, Segment*) = 0;
    void readLyrics();
    void readChannels();
    void setTuplet(Tuplet* tuplet, int tuple);
    void setupTupletStyle(Tuplet* tuplet);
    Fraction len2fraction(int len);
    void addDynamic(Note*, int d);
    void createMeasures();
    void applyBeatEffects(Chord*, int beatEffects);
    void readTremoloBar(int track, Segment*);
    void readChord(Segment* seg, int track, int numStrings, String name, bool gpHeader);
    void restsForEmptyBeats(Segment* seg, Measure* measure, ChordRest* cr, Fraction& l, int track, const Fraction& tick);
    void createSlur(bool hasSlur, staff_idx_t staffIdx, ChordRest* cr);
    void createOttava(bool hasOttava, int track, ChordRest* cr, String value);
    void createSlide(int slide, ChordRest* cr, int staffIdx, Note* note = nullptr);
    void createCrecDim(int staffIdx, int track, const Fraction& tick, bool crec);
    void addTextToNote(String text, Note* note);
    void addTextArticulation(Note* note, ArticulationTextType type);
    void addPalmMute(Note*);
    void addLetRing(Note*);
    void addVibrato(Note*, VibratoType type = VibratoType::GUITAR_VIBRATO);
    void addTap(Note*);
    void addSlap(Note*);
    void addPop(Note*);
    void createTuningString(int strings, int tuning[]);
    virtual std::unique_ptr<IGPDomBuilder> createGPDomBuilder() const { return nullptr; }
    void initDynamics(size_t stavesNum);

    std::vector<PalmMute*> _palmMutes;
    std::vector<LetRing*> _letRings;
    std::vector<Vibrato*> _vibratos;

public:
    std::vector<std::string> tunings;

    static int harmonicOvertone(Note* note, float harmonicValue, int harmonicType);
    void setTempo(int n, Measure* measure);
    void initGuitarProDrumset();
    String title, subtitle, artist, album, composer;
    String transcriber, instructions;
    StringList comments;
    GpTrack channelDefaults[GP_MAX_TRACK_NUMBER * 2];
    size_t staves = 0;
    size_t measures = 0;
    std::vector<GpBar> bars;

    enum class GuitarProError : char {
        GP_NO_ERROR, GP_UNKNOWN_FORMAT,
        GP_EOF, GP_BAD_NUMBER_OF_STRINGS
    };

    GuitarPro(MasterScore*, int v);
    virtual ~GuitarPro();
    virtual bool read(mu::io::IODevice*) = 0;
    String error(GuitarProError n) const { return String::fromUtf8(errmsg[int(n)]); }
};

//---------------------------------------------------------
//   GuitarPro1
//---------------------------------------------------------

class GuitarPro1 : public GuitarPro
{
protected:
    bool readNote(int string, Note* note);
    int readBeatEffects(int track, Segment*) override;

public:
    GuitarPro1(MasterScore* s, int v)
        : GuitarPro(s, v) {}
    bool read(mu::io::IODevice*) override;
};

//---------------------------------------------------------
//   GuitarPro2
//---------------------------------------------------------

class GuitarPro2 : public GuitarPro1
{
public:
    GuitarPro2(MasterScore* s, int v)
        : GuitarPro1(s, v) {}
    bool read(mu::io::IODevice*) override;
};

//---------------------------------------------------------
//   GuitarPro3
//---------------------------------------------------------

class GuitarPro3 : public GuitarPro1
{
    int readBeatEffects(int track, Segment* segment) override;

public:
    GuitarPro3(MasterScore* s, int v)
        : GuitarPro1(s, v) {}
    bool read(mu::io::IODevice*) override;
};

//---------------------------------------------------------
//   GuitarPro4
//---------------------------------------------------------

class GuitarPro4 : public GuitarPro
{
    std::vector<int> curDynam;
    std::vector<int> tupleKind;
    void readInfo();
    bool readNote(int string, int staffIdx, Note* note);
    int readBeatEffects(int track, Segment* segment) override;
    bool readMixChange(Measure* measure) override;
    int convertGP4SlideNum(int slide);

public:
    GuitarPro4(MasterScore* s, int v)
        : GuitarPro(s, v) {}
    bool read(mu::io::IODevice*) override;
};

//---------------------------------------------------------
//   GuitarPro5
//---------------------------------------------------------

class GuitarPro5 : public GuitarPro
{
    std::map<std::pair<int, int>, bool> dead_end;
    int _beat_counter{ 0 };
    std::unordered_map<Chord*, TremoloType> m_tremolosInChords; // for adding tremolo for tied notes
    std::unordered_map<Note*, Note*> m_harmonicNotes; // for adding ties for harmonic notes

    void readInfo();
    void readPageSetup();
    int readBeatEffects(int track, Segment* segment) override;
    bool readNote(int string, Note* note);
    bool readMixChange(Measure* measure) override;
    void readMeasure(Measure* measure, int staffIdx, Tuplet*[], bool mixChange);
    int readArtificialHarmonic();
    bool readTracks();
    void readMeasures(int startingTempo);
    Fraction readBeat(const Fraction& tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets, bool mixChange);
    bool readNoteEffects(Note*);
    float naturalHarmonicFromFret(int fret);

public:
    GuitarPro5(MasterScore* s, int v)
        : GuitarPro(s, v) {}
    bool read(mu::io::IODevice*) override;
};

//---------------------------------------------------------
//   GuitarPro6
//---------------------------------------------------------

class GuitarPro6 : public GuitarPro
{
    // an integer stored in the header indicating that the file is not compressed (BCFS).
    const int GPX_HEADER_UNCOMPRESSED = 1397113666;
    // an integer stored in the header indicating that the file is not compressed (BCFZ).
    const int GPX_HEADER_COMPRESSED = 1514554178;
    int position = 0;
    // a constant storing the amount of bits per byte
    const int BITS_IN_BYTE = 8;
    // contains all the information about notes that will go in the parts
    struct GPPartInfo {
        XmlDomNode masterBars;
        XmlDomNode bars;
        XmlDomNode voices;
        XmlDomNode beats;
        XmlDomNode notes;
        XmlDomNode rhythms;
    };

    void parseFile(const char* filename, ByteArray* data);

    int readBit(ByteArray* buffer);
    ByteArray getBytes(ByteArray* buffer, int offset, int length);
    void readGPX(ByteArray* buffer);
    int readInteger(ByteArray* buffer, int offset);
    ByteArray readString(ByteArray* buffer, int offset, int length);
    int readBits(ByteArray* buffer, int bitsToRead);
    int readBitsReversed(ByteArray* buffer, int bitsToRead);
    int findNumMeasures(GPPartInfo* partInfo);
    void readMasterTracks(XmlDomNode* masterTrack);
    void readDrumNote(Note* note, int element, int variation);
    XmlDomNode getNode(const String& id, XmlDomNode currentDomNode);
    void unhandledNode(String nodeName);
    void makeTie(Note* note);
    int readBeatEffects(int /*track*/, Segment*) override { return 0; }

    std::map<std::pair<int, int>, Note*> slideMap;

protected:
    const static std::map<std::string, std::string> instrumentMapping;
    void readGpif(ByteArray* data);

    virtual std::unique_ptr<IGPDomBuilder> createGPDomBuilder() const override;

public:
    GuitarPro6(MasterScore* s)
        : GuitarPro(s, 6) {}
    GuitarPro6(MasterScore* s, int v)
        : GuitarPro(s, v) {}
    bool read(mu::io::IODevice*) override;
};

class GuitarPro7 : public GuitarPro6
{
    virtual std::unique_ptr<IGPDomBuilder> createGPDomBuilder() const override;

public:
    GuitarPro7(MasterScore* s)
        : GuitarPro6(s, 7) {}
    bool read(mu::io::IODevice*) override;
    GPProperties readProperties(ByteArray* data);
};
} // namespace Ms
#endif
