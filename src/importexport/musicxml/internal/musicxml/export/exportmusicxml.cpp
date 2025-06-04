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

/**
 MusicXML export.
 */

// TODO: trill lines need to be handled the same way as slurs
// in MuseScore they are measure level elements, while in MusicXML
// they are attached to notes (as ornaments)

//=========================================================
//  LVI FIXME
//
//  Evaluate parameter handling between the various classes, could be simplified
//=========================================================

// TODO LVI 2011-10-30: determine how to report export errors.
// Currently all output (both debug and error reports) are done using LOGD.

#include "exportmusicxml.h"

#include <math.h>
#include <set>

#include "containers.h"
#include "realfn.h"
#include "io/iodevice.h"
#include "io/buffer.h"
#include "io/file.h"
#include "io/fileinfo.h"
#include "global/serialization/zipwriter.h"

#include "engraving/style/style.h"
#include "engraving/rw/xmlwriter.h"
#include "engraving/types/typesconv.h"
#include "engraving/types/symnames.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/figuredbass.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/hammeronpulloff.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmonicmark.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/letring.h"
#include "engraving/dom/linkedobjects.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/page.h"
#include "engraving/dom/palmmute.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/pickscrape.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/rasgueado.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tempo.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textlinebase.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/undo.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/whammybar.h"

#include "../shared/musicxmlfonthandler.h"
#include "../shared/musicxmlsupport.h"
#include "../shared/musicxmltypes.h"

#include "modularity/ioc.h"
#include "../../../imusicxmlconfiguration.h"
#include "global/iapplication.h"
#include "global/realfn.h"
#include "engraving/iengravingconfiguration.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::iex::musicxml;
using namespace mu::engraving;

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

// #define DEBUG_CLEF true
// #define DEBUG_REPEATS true
// #define DEBUG_TICK true

#ifdef DEBUG_CLEF
#define clefDebug(...) LOGD(__VA_ARGS__)
#else
#define clefDebug(...) {}
#endif

constexpr int MAX_PART_GROUPS  = 8;

//---------------------------------------------------------
//   typedefs
//---------------------------------------------------------

typedef std::map<track_idx_t, const FiguredBass*> FigBassMap;

//---------------------------------------------------------
//   attributes -- prints <attributes> tag when necessary
//---------------------------------------------------------

class Attributes
{
public:
    Attributes() { start(); }
    void doAttr(XmlWriter& xml, bool attr);
    void start();
    void stop(XmlWriter& xml);

private:
    bool m_inAttributes = false;
};

//---------------------------------------------------------
//   doAttr - when necessary change state and print <attributes> tag
//---------------------------------------------------------

void Attributes::doAttr(XmlWriter& xml, bool attr)
{
    if (!m_inAttributes && attr) {
        xml.startElement("attributes");
        m_inAttributes = true;
    } else if (m_inAttributes && !attr) {
        xml.endElement();
        m_inAttributes = false;
    }
}

//---------------------------------------------------------
//   start -- initialize
//---------------------------------------------------------

void Attributes::start()
{
    m_inAttributes = false;
}

//---------------------------------------------------------
//   stop -- print </attributes> tag when necessary
//---------------------------------------------------------

void Attributes::stop(XmlWriter& xml)
{
    if (m_inAttributes) {
        xml.endElement();
        m_inAttributes = false;
    }
}

//---------------------------------------------------------
//   notations -- prints <notations> tag when necessary
//---------------------------------------------------------

class Notations
{
public:
    void tag(XmlWriter& xml, const EngravingItem* e);
    void etag(XmlWriter& xml);

private:
    bool m_notationsPrinted = false;
    bool m_prevElementVisible = true;
};

//---------------------------------------------------------
//   articulations -- prints <articulations> tag when necessary
//---------------------------------------------------------

class Articulations
{
public:
    void tag(XmlWriter& xml);
    void etag(XmlWriter& xml);

private:
    bool m_articulationsPrinted = false;
};

//---------------------------------------------------------
//   ornaments -- prints <ornaments> tag when necessary
//---------------------------------------------------------

class Ornaments
{
public:
    void tag(XmlWriter& xml);
    void etag(XmlWriter& xml);
private:
    bool m_ornamentsPrinted = false;
};

//---------------------------------------------------------
//   technical -- prints <technical> tag when necessary
//---------------------------------------------------------

class Technical
{
public:
    void tag(XmlWriter& xml);
    void etag(XmlWriter& xml);
private:
    bool m_technicalPrinted = false;
};

//---------------------------------------------------------
//   slur handler -- prints <slur> tags
//---------------------------------------------------------

class SlurHandler
{
public:
    SlurHandler();
    void doSlurs(const ChordRest* chordRest, Notations& notations, XmlWriter& xml);

private:
    void doSlurStart(const Slur* s, Notations& notations, String tagName, XmlWriter& xml);
    void doSlurStop(const Slur* s, Notations& notations, String tagName, XmlWriter& xml);
    int findSlur(const Slur* s) const;

    const Slur* m_slur[MAX_NUMBER_LEVEL];
    bool m_started[MAX_NUMBER_LEVEL];
};

//---------------------------------------------------------
//   glissando handler -- prints <glissando> tags
//---------------------------------------------------------

class GlissandoHandler
{
public:
    GlissandoHandler();
    void doGlissandoStart(Glissando* gliss, Notations& notations, XmlWriter& xml);
    void doGlissandoStop(Glissando* gliss, Notations& notations, XmlWriter& xml);

private:
    int findNote(const Note* note, int type) const;

    const Note* m_glissNote[MAX_NUMBER_LEVEL];
    const Note* m_slideNote[MAX_NUMBER_LEVEL];
};

//---------------------------------------------------------
//  MeasureNumberStateHandler
//---------------------------------------------------------

/**
 State handler used to calculate measure number including implicit flag.
 To be called once at the start of each measure in a part.
 */

class MeasureNumberStateHandler final
{
public:
    MeasureNumberStateHandler();
    void updateForMeasure(const Measure* const m);
    String measureNumber() const;
    bool isFirstActualMeasure() const;
private:
    void init();
    int m_measureNo = 0;                             // number of next regular measure
    int m_measureNoOffset = 0;                       // measure number offset
    int m_irregularMeasureNo = 0;                    // number of next irregular measure
    int m_pickupMeasureNo = 0;                       // number of next pickup measure
    String m_cachedAttributes;                  // attributes calculated by updateForMeasure()
};

//---------------------------------------------------------
//   MeasurePrintContext
//---------------------------------------------------------

struct MeasurePrintContext final
{
    void measureWritten(const Measure* m);
    bool scoreStart = true;
    bool pageStart = true;
    bool systemStart = true;
    const Measure* prevMeasure = nullptr;
    const System* prevSystem = nullptr;
    const System* lastSystemPrevPage = nullptr;
};

//---------------------------------------------------------
//   ExportMusicXml
//---------------------------------------------------------

typedef std::unordered_map<const ChordRest*, const Trill*> TrillHash;
typedef std::map<const Instrument*, int> MusicXmlInstrumentMap;

class ExportMusicXml : public muse::Injectable
{
public:
    static inline muse::GlobalInject<mu::iex::musicxml::IMusicXmlConfiguration> configuration;
    muse::Inject<muse::IApplication> application  = { this };

public:
    ExportMusicXml(Score* s)
        : muse::Injectable(s->iocContext())
    {
        m_score = s;
        m_tick = { 0, 1 };
        m_div = 1;
        m_tenths = 40;
        m_millimeters = m_score->style().spatium() * m_tenths / (10 * DPMM);
    }

    void write(muse::io::IODevice* dev);
    void credits(XmlWriter& xml);
    void moveToTick(const Fraction& t);
    void moveToTickIfNeed(const Fraction& t);
    void words(TextBase const* const text, staff_idx_t staff);
    void tboxTextAsWords(TextBase const* const text, const staff_idx_t staff, PointF position);
    void rehearsal(RehearsalMark const* const rmk, staff_idx_t staff);
    void harpPedals(HarpPedalDiagram const* const hpd, staff_idx_t staff);
    void hairpin(Hairpin const* const hp, staff_idx_t staff, const Fraction& tick);
    void ottava(Ottava const* const ot, staff_idx_t staff, const Fraction& tick);
    void pedal(Pedal const* const pd, staff_idx_t staff, const Fraction& tick);
    void playText(PlayTechAnnotation const* const annot, staff_idx_t staff);
    void textLine(TextLineBase const* const tl, staff_idx_t staff, const Fraction& tick);
    void dynamic(Dynamic const* const dyn, staff_idx_t staff);
    void systemText(StaffTextBase const* const text, staff_idx_t staff);
    void tempoText(TempoText const* const text, staff_idx_t staff);
    void tempoSound(TempoText const* const text);
    void harmony(Harmony const* const, FretDiagram const* const fd, const Fraction& offset = Fraction(0, 1));
    Score* score() const { return m_score; }
    double getTenthsFromInches(double) const;
    double getTenthsFromDots(double) const;
    Fraction tick() const { return m_tick; }
    void writeInstrumentChange(const InstrumentChange* instrChange);
    void writeInstrumentDetails(const Instrument* instrument, const bool concertPitch);

    static bool canWrite(const EngravingItem* e);

    static String positioningAttributes(EngravingItem const* const el, bool isSpanStart = true);
    static String fermataPosition(const Fermata* const fermata);

private:
    int findBracket(const TextLineBase* tl) const;
    int findDashes(const TextLineBase* tl) const;
    int findHairpin(const Hairpin* tl) const;
    int findOttava(const Ottava* tl) const;
    int findTrill(const Trill* tl) const;
    void chord(Chord* chord, staff_idx_t staff, const std::vector<Lyrics*>& ll, bool useDrumset);
    void rest(Rest* chord, staff_idx_t staff, const std::vector<Lyrics*>& ll);
    void clef(staff_idx_t staff, const ClefType ct, const String& extraAttributes = u"");
    void timesig(const TimeSig* tsig);
    void keysig(const KeySig* ks, ClefType ct, staff_idx_t staff = 0, bool visible = true);
    void barlineLeft(const Measure* const m, const track_idx_t track);
    void barlineMiddle(const BarLine* bl);
    void barlineRight(const Measure* const m, const track_idx_t strack, const track_idx_t etrack);
    void lyrics(const std::vector<Lyrics*>& ll, const track_idx_t trk);
    void work(const MeasureBase* measure);
    void calcDivMoveToTick(const Fraction& t);
    void calcDivisions();
    void keysigTimesig(const Measure* m, const Part* p);
    void chordAttributes(Chord* chord, Notations& notations, Technical& technical, TrillHash& trillStart, TrillHash& trillStop);
    void wavyLineStartStop(const ChordRest* cr, Notations& notations, Ornaments& ornaments, TrillHash& trillStart, TrillHash& trillStop);
    void print(const Measure* const m, const int partNr, const int firstStaffOfPart, const size_t nrStavesInPart,
               const MeasurePrintContext& mpc);
    void measureLayout(const double distance);
    void findAndExportClef(const Measure* const m, const int staves, const track_idx_t strack, const track_idx_t etrack);
    void exportDefaultClef(const Part* const part, const Measure* const m);
    void writeElement(EngravingItem* el, const Measure* m, staff_idx_t sstaff, bool useDrumset);
    void writeMeasureTracks(const Measure* const m, const int partIndex, const staff_idx_t strack, const staff_idx_t partRelStaffNo,
                            const bool useDrumset, const bool isLastStaffOfPart, FigBassMap& fbMap,
                            std::set<const Spanner*>& spannersStopped);
    void writeMeasureStaves(const Measure* m, const int partIndex, const staff_idx_t startStaff, const size_t nstaves,
                            const bool useDrumset, FigBassMap& fbMap, std::set<const Spanner*>& spannersStopped);
    void writeMeasure(const Measure* const m, const int idx, const int staffCount, MeasureNumberStateHandler& mnsh, FigBassMap& fbMap,
                      const MeasurePrintContext& mpc, std::set<const Spanner*>& spannersStopped);
    void repeatAtMeasureStart(Attributes& attr, const Measure* const m, track_idx_t strack, track_idx_t etrack, track_idx_t track);
    void repeatAtMeasureStop(const Measure* const m, track_idx_t strack, track_idx_t etrack, track_idx_t track);
    void writeParts();

    static String elementPosition(const ExportMusicXml* const expMxml, const EngravingItem* const elm);
    static String positioningAttributesForTboxText(const PointF position, float spatium);
    void identification(XmlWriter& xml, Score const* const score);

    Score* m_score = nullptr;
    XmlWriter m_xml;
    SlurHandler m_sh;
    GlissandoHandler m_gh;
    Fraction m_tick;
    Attributes m_attr;
    const TextLineBase* m_brackets[MAX_NUMBER_LEVEL];
    const TextLineBase* m_dashes[MAX_NUMBER_LEVEL];
    const Hairpin* m_hairpins[MAX_NUMBER_LEVEL];
    const Ottava* m_ottavas[MAX_NUMBER_LEVEL];
    const Trill* m_trills[MAX_NUMBER_LEVEL];
    std::vector<const Jump*> m_jumpElements;
    ArpeggioMap m_measArpeggios;
    int m_div = 0;
    double m_millimeters = 0.0;
    int m_tenths = 0;
    bool m_tboxesAboveWritten = false;
    bool m_tboxesBelowWritten = false;
    std::vector<size_t> m_hiddenStaves;
    TrillHash m_trillStart;
    TrillHash m_trillStop;
    MusicXmlInstrumentMap m_instrMap;
    PlayingTechniqueType m_currPlayTechnique;
};

//---------------------------------------------------------
//   fractionToStdString
//---------------------------------------------------------

#ifdef DEBUG_TICK
static std::string fractionToStdString(const Fraction& f)
{
    if (!f.isValid()) {
        return "<invalid>";
    }
    String res { f.toString() };
    res += String(u" (%1)").arg(String::number(f.ticks()));
    return res.toStdString();
}

#endif

//---------------------------------------------------------
//   durElemTicksToStdString
//---------------------------------------------------------

#ifdef DEBUG_TICK
static std::string durElemTicksToStdString(const DurationElement& d)
{
    String res;
    res += String(u" ticks %1").arg(d.ticks().toString());
    res += String(u" (%1)").arg(String::number(d.ticks().ticks()));
    res += String(u" globalTicks %1").arg(d.globalTicks().toString());
    res += String(u" (%1)").arg(String::number(d.globalTicks().ticks()));
    res += String(u" actualTicks %1").arg(d.actualTicks().toString());
    res += String(u" (%1)").arg(String::number(d.actualTicks().ticks()));
    return res.toStdString();
}

#endif

//---------------------------------------------------------
//   positionToString
//---------------------------------------------------------

static String positionToString(const PointF def, const PointF rel, const float spatium)
{
    // minimum value to export
    const float positionElipson = 0.1f;

    // convert into tenths for MusicXML
    const float defaultX =  10 * def.x() / spatium;
    const float defaultY =  -10 * def.y() / spatium;
    const float relativeX =  10 * rel.x() / spatium;
    const float relativeY =  -10 * rel.y() / spatium;

    // generate string representation
    String res;
    if (fabsf(defaultX) > positionElipson) {
        res += String(u" default-x=\"%1\"").arg(String::number(defaultX, 2));
    }
    if (fabsf(defaultY) > positionElipson) {
        res += String(u" default-y=\"%1\"").arg(String::number(defaultY, 2));
    }
    if (fabsf(relativeX) > positionElipson) {
        res += String(u" relative-x=\"%1\"").arg(String::number(relativeX, 2));
    }
    if (fabsf(relativeY) > positionElipson) {
        res += String(u" relative-y=\"%1\"").arg(String::number(relativeY, 2));
    }

    return res;
}

//---------------------------------------------------------
//   positioningAttributes
//   According to the specs (common.dtd), all direction-type and note elements must be relative to the measure
//   while all other elements are relative to their position or the nearest note.
//---------------------------------------------------------

String ExportMusicXml::positioningAttributes(EngravingItem const* const el, bool isSpanStart)
{
    if (!configuration()->exportLayout()) {
        return String();
    }

    //LOGD("single el %p _pos x,y %f %f _userOff x,y %f %f spatium %f",
    //       el, el->ldata()->pos.x(), el->ldata()->pos.y(), el->offset().x(), el->offset().y(), el->spatium());

    PointF def;
    PointF rel;
    float spatium = el->spatium();

    const SLine* span = nullptr;
    if (el->isSLine()) {
        span = static_cast<const SLine*>(el);
    }

    if (span && !span->segmentsEmpty()) {
        if (isSpanStart) {
            const LineSegment* seg = span->frontSegment();
            const PointF offset = seg->offset();
            const PointF p = seg->pos();
            rel.setX(offset.x());
            def.setY(p.y());

            //LOGD("sline start seg %p seg->pos x,y %f %f seg->userOff x,y %f %f spatium %f",
            //       seg, p.x(), p.y(), seg->offset().x(), seg->offset().y(), seg->spatium());
        } else {
            const LineSegment* seg = span->backSegment();
            const PointF userOff = seg->offset();       // This is the offset accessible from the inspector
            const PointF userOff2 = seg->userOff2();       // Offset of the actual dragged anchor, which doesn't affect the inspector offset
            //auto pos = seg->pos();
            //auto pos2 = seg->pos2();

            //LOGD("sline stop seg %p seg->pos2 x,y %f %f seg->userOff2 x,y %f %f spatium %f",
            //       seg, pos2.x(), pos2.y(), seg->userOff2().x(), seg->userOff2().y(), seg->spatium());

            // For an SLine, the actual offset equals the sum of userOff and userOff2,
            // as userOff moves the SLine as a whole
            rel.setX(userOff.x() + userOff2.x());

            // Following would probably required for non-horizontal SLines:
            //defaultY = pos.y() + pos2.y();
        }
    } else {
        def = el->ldata()->pos();       // Note: for some elements, Finale Notepad seems to work slightly better w/o default-x
        rel = el->offset();
    }

    return positionToString(def, rel, spatium);
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Notations::tag(XmlWriter& xml, const EngravingItem* e)
{
    if (m_notationsPrinted && m_prevElementVisible != e->visible()) {
        etag(xml);
    }

    if (!m_notationsPrinted) {
        if (e->visible()) {
            xml.startElement("notations");
        } else {
            xml.startElement("notations", { { "print-object", "no" } });
        }
    }

    m_notationsPrinted = true;
    m_prevElementVisible = e->visible();
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Notations::etag(XmlWriter& xml)
{
    if (m_notationsPrinted) {
        xml.endElement();
    }
    m_notationsPrinted = false;
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Articulations::tag(XmlWriter& xml)
{
    if (!m_articulationsPrinted) {
        xml.startElement("articulations");
    }
    m_articulationsPrinted = true;
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Articulations::etag(XmlWriter& xml)
{
    if (m_articulationsPrinted) {
        xml.endElement();
    }
    m_articulationsPrinted = false;
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Ornaments::tag(XmlWriter& xml)
{
    if (!m_ornamentsPrinted) {
        xml.startElement("ornaments");
    }
    m_ornamentsPrinted = true;
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Ornaments::etag(XmlWriter& xml)
{
    if (m_ornamentsPrinted) {
        xml.endElement();
    }
    m_ornamentsPrinted = false;
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Technical::tag(XmlWriter& xml)
{
    if (!m_technicalPrinted) {
        xml.startElement("technical");
    }
    m_technicalPrinted = true;
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Technical::etag(XmlWriter& xml)
{
    if (m_technicalPrinted) {
        xml.endElement();
    }
    m_technicalPrinted = false;
}

static std::shared_ptr<mu::engraving::IEngravingConfiguration> engravingConfiguration()
{
    return muse::modularity::globalIoc()->resolve<mu::engraving::IEngravingConfiguration>("iex_musicxml");
}

//---------------------------------------------------------
//   color2xml
//---------------------------------------------------------

static String color2xml(const EngravingItem* el)
{
    if (el->color() != engravingConfiguration()->defaultColor()) {
        return String(u" color=\"%1\"").arg(String::fromStdString(el->color().toString()));
    } else if (el->isSLine() && ((SLine*)el)->lineColor() != engravingConfiguration()->defaultColor()) {
        return String(u" color=\"%1\"").arg(String::fromStdString(((SLine*)el)->lineColor().toString()));
    } else {
        return String();
    }
}

static void addColorAttr(const EngravingItem* el, XmlWriter::Attributes& attrs)
{
    if (el->color() != engravingConfiguration()->defaultColor()) {
        attrs.emplace_back(std::make_pair("color", String::fromStdString(el->color().toString())));
    }
}

//---------------------------------------------------------
//   frame2xml
//---------------------------------------------------------

static String frame2xml(const TextBase* el)
{
    switch (el->frameType()) {
    case FrameType::CIRCLE:
        return u" enclosure=\"circle\"";
    case FrameType::SQUARE:
        return u" enclosure=\"rectangle\"";
    default:
        return String();
    }
}

//---------------------------------------------------------
//   fontStyleToXML
//---------------------------------------------------------

static String fontStyleToXML(const FontStyle style, bool allowUnderline = true)
{
    String res;
    if (style & FontStyle::Bold) {
        res += u" font-weight=\"bold\"";
    }
    if (style & FontStyle::Italic) {
        res += u" font-style=\"italic\"";
    }
    if (allowUnderline && style & FontStyle::Underline) {
        res += u" underline=\"1\"";
    }
    // at places where underline is not wanted (e.g. fingering, pluck), strike is not wanted too
    if (allowUnderline && style & FontStyle::Strike) {
        res += u" line-through=\"1\"";
    }
    return res;
}

//---------------------------------------------------------
//   slurHandler
//---------------------------------------------------------

SlurHandler::SlurHandler()
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        m_slur[i] = 0;
        m_started[i] = false;
    }
}

static String slurTieLineStyle(const SlurTie* s)
{
    String lineType;
    String rest;
    switch (s->styleType()) {
    case SlurStyleType::Dashed:
    case SlurStyleType::WideDashed:
        lineType = u"dashed";
        break;
    case SlurStyleType::Dotted:
        lineType = u"dotted";
        break;
    case SlurStyleType::Solid:
    default:
        lineType.clear();
    }
    if (!lineType.isEmpty()) {
        rest = String(u" line-type=\"%1\"").arg(lineType);
    }
    if (s->slurDirection() != engraving::DirectionV::AUTO) {
        rest += String(u" orientation=\"%1\"").arg(s->up() ? u"over" : u"under");
        rest += String(u" placement=\"%1\"").arg(s->up() ? u"above" : u"below");
    }
    rest += color2xml(s);
    return rest;
}

//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//---------------------------------------------------------

int SlurHandler::findSlur(const Slur* s) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (m_slur[i] == s) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   findFirstChordRest -- find first chord or rest (in musical order) for slur s
//   note that this is not necessarily the same as s->startElement()
//---------------------------------------------------------

static const ChordRest* findFirstChordRest(const Slur* s)
{
    const EngravingItem* e1 = s->startElement();
    if (!e1 || !(e1->isChordRest())) {
        LOGD("no valid start element for slur %p", s);
        return nullptr;
    }

    const EngravingItem* e2 = s->endElement();
    if (!e2 || !(e2->isChordRest())) {
        LOGD("no valid end element for slur %p", s);
        return nullptr;
    }

    if (e1->tick() < e2->tick()) {
        return static_cast<const ChordRest*>(e1);
    } else if (e1->tick() > e2->tick()) {
        return static_cast<const ChordRest*>(e2);
    }

    if (e1->isRest() || e2->isRest()) {
        return nullptr;
    }

    const Chord* c1 = static_cast<const Chord*>(e1);
    const Chord* c2 = static_cast<const Chord*>(e2);

    // c1->tick() == c2->tick()
    if (!c1->isGrace() && !c2->isGrace()) {
        // slur between two regular notes at the same tick
        // probably shouldn't happen but handle just in case
        LOGD("invalid slur between chords %p and %p at tick %d", c1, c2, c1->tick().ticks());
        return 0;
    } else if (c1->isGraceBefore() && !c2->isGraceBefore()) {
        return c1;            // easy case: c1 first
    } else if (c1->isGraceAfter() && !c2->isGraceAfter()) {
        return c2;            // easy case: c2 first
    } else if (c2->isGraceBefore() && !c1->isGraceBefore()) {
        return c2;            // easy case: c2 first
    } else if (c2->isGraceAfter() && !c1->isGraceAfter()) {
        return c1;            // easy case: c1 first
    } else {
        // both are grace before or both are grace after -> compare grace indexes
        // (note: higher means closer to the non-grace chord it is attached to)
        if ((c1->isGraceBefore() && c1->graceIndex() < c2->graceIndex())
            || (c1->isGraceAfter() && c1->graceIndex() > c2->graceIndex())) {
            return c1;
        } else {
            return c2;
        }
    }
}

//---------------------------------------------------------
//   doSlurs
//---------------------------------------------------------

void SlurHandler::doSlurs(const ChordRest* chordRest, Notations& notations, XmlWriter& xml)
{
    // loop over all slurs twice, first to handle the stops, then the starts
    for (int i = 0; i < 2; ++i) {
        // search for slur(s) starting or stopping at this chord
        for (const auto& it : chordRest->score()->spanner()) {
            String tagName = u"slur";
            auto sp = it.second;
            if (sp->generated() || ( sp->type() != ElementType::SLUR && sp->type() != ElementType::HAMMER_ON_PULL_OFF )
                || !ExportMusicXml::canWrite(sp)) {
                continue;
            }
            if (sp->isHammerOnPullOff()) {
                const HammerOnPullOffSegment* hopoSeg = static_cast<const HammerOnPullOffSegment*>(sp->spannerSegments().front());
                tagName = hopoSeg->hopoText().front()->isHammerOn() ? u"hammer-on" : u"pull-off";
            }
            if (chordRest == sp->startElement() || chordRest == sp->endElement()) {
                const Slur* s = static_cast<const Slur*>(sp);
                const ChordRest* firstChordRest = findFirstChordRest(s);
                if (firstChordRest) {
                    if (i == 0) {
                        // first time: do slur stops
                        if (firstChordRest != chordRest) {
                            doSlurStop(s, notations, tagName, xml);
                        }
                    } else {
                        // second time: do slur starts
                        if (firstChordRest == chordRest) {
                            doSlurStart(s, notations, tagName, xml);
                        }
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   doSlurStart
//---------------------------------------------------------

void SlurHandler::doSlurStart(const Slur* s, Notations& notations, String tagName, XmlWriter& xml)
{
    // check if on slur list (i.e. stop already seen)
    int i = findSlur(s);
    // compose tag
    tagName += u" type=\"start\"";
    tagName += (tagName == u"slur") ? slurTieLineStyle(s) : color2xml(s);
    tagName += ExportMusicXml::positioningAttributes(s, true);

    if (i >= 0) {
        // remove from list and print start
        m_slur[i] = 0;
        m_started[i] = false;
        notations.tag(xml, s);
        tagName += String(u" number=\"%1\"").arg(i + 1);
        xml.tagRaw(tagName);
    } else {
        // find free slot to store it
        i = findSlur(0);
        if (i >= 0) {
            m_slur[i] = s;
            m_started[i] = true;
            notations.tag(xml, s);
            tagName += String(u" number=\"%1\"").arg(i + 1);
            xml.tagRaw(tagName);
        } else {
            LOGD("no free slur slot");
        }
    }
}

//---------------------------------------------------------
//   doSlurStop
//---------------------------------------------------------

// Note: a slur may start in a higher voice in the same measure.
// In that case it is not yet started (i.e. on the active slur list)
// when doSlurStop() is executed. Handle this slur as follows:
// - generate stop anyway and put it on the slur list
// - doSlurStart() starts slur but doesn't store it

void SlurHandler::doSlurStop(const Slur* s, Notations& notations, String tagName, XmlWriter& xml)
{
    // check if on slur list
    int i = findSlur(s);
    if (i < 0) {
        // if not, find free slot to store it
        i = findSlur(0);
        if (i >= 0) {
            m_slur[i] = s;
            m_started[i] = false;
            notations.tag(xml, s);
            tagName += String(u" type=\"stop\" number=\"%1\"").arg(i + 1);
            tagName += ExportMusicXml::positioningAttributes(s, false);
            xml.tagRaw(tagName);
        } else {
            LOGD("no free slur slot");
        }
    } else {
        // found (already started), stop it and remove from list
        m_slur[i] = 0;
        m_started[i] = false;
        notations.tag(xml, s);
        tagName += String(u" type=\"stop\" number=\"%1\"").arg(i + 1);
        tagName += ExportMusicXml::positioningAttributes(s, false);
        xml.tagRaw(tagName);
    }
}

//---------------------------------------------------------
//   glissando
//---------------------------------------------------------

// <notations>
//   <slide line-type="solid" number="1" type="start"/>
//   </notations>

// <notations>
//   <glissando line-type="wavy" number="1" type="start"/>
//   </notations>

static void glissando(const Glissando* gli, int number, bool start, Notations& notations, XmlWriter& xml)
{
    String tagName;
    if (gli->glissandoType() == GlissandoType::STRAIGHT) {
        switch (gli->lineStyle()) {
        case LineType::SOLID:
            tagName = u"slide line-type=\"solid\"";
            break;
        case LineType::DASHED:
            tagName = u"slide line-type=\"dashed\"";
            break;
        case LineType::DOTTED:
            tagName = u"slide line-type=\"dotted\"";
            break;
        }
    } else {
        tagName = u"glissando line-type=\"wavy\"";
    }
    tagName += String(u" number=\"%1\" type=\"%2\"").arg(number).arg(start ? u"start" : u"stop");
    if (start) {
        tagName += color2xml(gli);
        tagName += ExportMusicXml::positioningAttributes(gli, start);
    }
    notations.tag(xml, gli);
    if (start && gli->showText() && !gli->text().empty()) {
        xml.tagRaw(tagName, gli->text());
    } else {
        xml.tagRaw(tagName);
    }
}

//---------------------------------------------------------
//   GlissandoHandler
//---------------------------------------------------------

GlissandoHandler::GlissandoHandler()
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        m_glissNote[i] = 0;
        m_slideNote[i] = 0;
    }
}

//---------------------------------------------------------
//   findNote -- get index of Note in note table for subtype type
//   return -1 if not found
//---------------------------------------------------------

int GlissandoHandler::findNote(const Note* note, int type) const
{
    if (type != 0 && type != 1) {
        LOGD("GlissandoHandler::findNote: unknown glissando subtype %d", type);
        return -1;
    }
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (type == 0 && m_slideNote[i] == note) {
            return i;
        }
        if (type == 1 && m_glissNote[i] == note) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   doGlissandoStart
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStart(Glissando* gliss, Notations& notations, XmlWriter& xml)
{
    GlissandoType type = gliss->glissandoType();
    if (type != GlissandoType::STRAIGHT && type != GlissandoType::WAVY) {
        LOGD("doGlissandoStart: unknown glissando subtype %d", int(type));
        return;
    }
    Note* note = static_cast<Note*>(gliss->startElement());
    // check if on chord list
    int i = findNote(note, int(type));
    if (i >= 0) {
        // print error and remove from list
        LOGD("doGlissandoStart: note for glissando/slide %p already on list", gliss);
        if (type == GlissandoType::STRAIGHT) {
            m_slideNote[i] = 0;
        }
        if (type == GlissandoType::WAVY) {
            m_glissNote[i] = 0;
        }
    }
    // find free slot to store it
    i = findNote(0, int(type));
    if (i >= 0) {
        if (type == GlissandoType::STRAIGHT) {
            m_slideNote[i] = note;
        }
        if (type == GlissandoType::WAVY) {
            m_glissNote[i] = note;
        }
        glissando(gliss, i + 1, true, notations, xml);
    } else {
        LOGD("doGlissandoStart: no free slot");
    }
}

//---------------------------------------------------------
//   doGlissandoStop
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStop(Glissando* gliss, Notations& notations, XmlWriter& xml)
{
    GlissandoType type = gliss->glissandoType();
    if (type != GlissandoType::STRAIGHT && type != GlissandoType::WAVY) {
        LOGD("doGlissandoStart: unknown glissando subtype %d", int(type));
        return;
    }
    Note* note = static_cast<Note*>(gliss->startElement());
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (type == GlissandoType::STRAIGHT && m_slideNote[i] == note) {
            m_slideNote[i] = 0;
            glissando(gliss, i + 1, false, notations, xml);
            return;
        }
        if (type == GlissandoType::WAVY && m_glissNote[i] == note) {
            m_glissNote[i] = 0;
            glissando(gliss, i + 1, false, notations, xml);
            return;
        }
    }
    LOGD("doGlissandoStop: glissando note %p not found", note);
}

//---------------------------------------------------------
//   directions anchor -- anchor directions at another element or a specific tick
//---------------------------------------------------------

class DirectionsAnchor
{
    EngravingItem* direct;          // the element containing the direction
    EngravingItem* anchor;          // the element it is attached to
    bool start;               // whether it is attached to start or end
    Fraction tick;            // the timestamp

public:
    DirectionsAnchor(EngravingItem* a, bool s, const Fraction& t) { direct = 0; anchor = a; start = s; tick = t; }
    DirectionsAnchor(const Fraction& t) { direct = 0; anchor = 0; start = true; tick = t; }
    EngravingItem* getDirect() { return direct; }
    EngravingItem* getAnchor() { return anchor; }
    bool getStart() { return start; }
    Fraction getTick() { return tick; }
    void setDirect(EngravingItem* d) { direct = d; }
};

//---------------------------------------------------------
// trill handling
//---------------------------------------------------------

// find all trills in this measure and this part

static void findTrills(const Measure* const measure, track_idx_t strack, track_idx_t etrack, TrillHash& trillStart, TrillHash& trillStop)
{
    // loop over all spanners in this measure
    Fraction stick = measure->tick();
    Fraction etick = measure->tick() + measure->ticks();
    for (auto it = measure->score()->spanner().lower_bound(stick.ticks());
         it != measure->score()->spanner().upper_bound(etick.ticks()); ++it) {
        EngravingItem* e = it->second;
        //LOGD("1 trill %p type %d track %d tick %s", e, e->type(), e->track(), muPrintable(e->tick().print()));
        if (e->isTrill() && ExportMusicXml::canWrite(e) && strack <= e->track() && e->track() < etrack
            && e->tick() >= measure->tick() && e->tick() < (measure->tick() + measure->ticks())) {
            //LOGD("2 trill %p", e);
            // a trill is found starting in this segment, trill end time is known
            // determine notes to write trill start and stop

            const Trill* tr = toTrill(e);
            EngravingItem* elem1 = tr->startElement();
            EngravingItem* elem2 = tr->endElement();

            if (elem1 && elem1->isChordRest() && elem2 && elem2->isChordRest()) {
                trillStart.insert({ toChordRest(elem1), tr });
                trillStop.insert({ toChordRest(elem2), tr });
            }
        }
    }
}

//---------------------------------------------------------
// helpers for ::calcDivisions
//---------------------------------------------------------

typedef std::set<Fraction> FractionSet;
static FractionSet fractions;

static void addFraction(const Fraction& len)
{
    fractions.insert(len.reduced());
}

//---------------------------------------------------------
//   calcDivMoveToTick
//---------------------------------------------------------

void ExportMusicXml::calcDivMoveToTick(const Fraction& t)
{
    if (t < tick()) {
#ifdef DEBUG_TICK
        LOGD() << "backup " << fractionToStdString(tick() - t);
#endif
        addFraction(tick() - t);
    } else if (t > tick()) {
#ifdef DEBUG_TICK
        LOGD() << "forward " << fractionToStdString(t - tick());
#endif
        addFraction(t - tick());
    }
    tick() = t;
}

//---------------------------------------------------------
// isTwoNoteTremolo - determine is chord is part of two note tremolo
//---------------------------------------------------------

static bool isTwoNoteTremolo(Chord* chord)
{
    return chord->tremoloTwoChord();
}

//---------------------------------------------------------
//  calcDivisions
//---------------------------------------------------------

// Loop over all voices in all staves and determine a suitable value for divisions.

// Length of time in MusicXML is expressed in "units", which should allow expressing all time values
// as an integral number of units. Divisions contains the number of units in a quarter note.
// Compute divisions by finding all fractions used to move through the score in MusicXML
// and computing the least common multiple of these fractions numerator and of fraction 1/4.

void ExportMusicXml::calcDivisions()
{
    // init
    fractions.clear();

    const std::vector<Part*>& il = m_score->parts();

    for (size_t idx = 0; idx < il.size(); ++idx) {
        Part* part = il.at(idx);
        m_tick = { 0, 1 };

        size_t staves = part->nstaves();
        track_idx_t strack = m_score->staffIdx(part) * VOICES;
        track_idx_t etrack = strack + staves * VOICES;

        for (MeasureBase* mb = m_score->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE) {
                continue;
            }
            Measure* m = (Measure*)mb;

            for (track_idx_t st = strack; st < etrack; ++st) {
                for (Segment* seg = m->first(); seg; seg = seg->next()) {
                    for (const EngravingItem* e : seg->annotations()) {
                        if (e->track() == st && e->type() == ElementType::FIGURED_BASS) {
                            const FiguredBass* fb = toFiguredBass(e);
#ifdef DEBUG_TICK
                            LOGD("figuredbass tick %d duration %d", fb->tick().ticks(), fb->ticks().ticks());
#endif
                            addFraction(fb->ticks());
                        }
                    }

                    EngravingItem* el = seg->element(st);
                    if (!el) {
                        continue;
                    }

                    // must ignore start repeat to prevent spurious backup/forward
                    if (el->type() == ElementType::BAR_LINE && toBarLine(el)->barLineType() == BarLineType::START_REPEAT) {
                        continue;
                    }

                    if (m_tick != seg->tick()) {
                        calcDivMoveToTick(seg->tick());
                    }

                    if (el->isChordRest()) {
                        Fraction l = toChordRest(el)->actualTicks();
                        if (el->isChord()) {
                            if (isTwoNoteTremolo(toChord(el))) {
                                l = l * Fraction(1, 2);
                            }
                        }
#ifdef DEBUG_TICK
                        LOGD() << "chordrest tick " << fractionToStdString(el->tick())
                               << " tickLen" << durElemTicksToStdString(*toChordRest(el));
#endif
                        addFraction(l);
                        m_tick += l;
                    }
                }
            }
            // move to end of measure (in case of incomplete last voice)
            calcDivMoveToTick(m->endTick());
        }
    }

    // compute divisions
    int divisions { 4 };  // ensure divisions > 0 for half and whole note
    for (auto f : fractions) {
        divisions = std::lcm(divisions, f.denominator());
    }
    divisions /= 4;

#ifdef DEBUG_TICK
    LOGD("divisions %d", divisions);
#endif
    m_div = divisions;
}

//---------------------------------------------------------
//   writePageFormat
//---------------------------------------------------------

static void writePageFormat(const MStyle& s, XmlWriter& xml, double conversion)
{
    xml.startElement("page-layout");

    xml.tag("page-height", s.styleD(Sid::pageHeight) * conversion);
    xml.tag("page-width", s.styleD(Sid::pageWidth) * conversion);

    String type(u"both");
    if (s.styleB(Sid::pageTwosided)) {
        type = u"even";
        xml.startElement("page-margins", { { "type", type } });
        xml.tag("left-margin",   s.styleD(Sid::pageEvenLeftMargin) * conversion);
        xml.tag("right-margin",  s.styleD(Sid::pageOddLeftMargin) * conversion);
        xml.tag("top-margin",    s.styleD(Sid::pageEvenTopMargin) * conversion);
        xml.tag("bottom-margin", s.styleD(Sid::pageEvenBottomMargin) * conversion);
        xml.endElement();
        type = u"odd";
    }
    xml.startElement("page-margins", { { "type", type } });
    xml.tag("left-margin",   s.styleD(Sid::pageOddLeftMargin) * conversion);
    xml.tag("right-margin",  s.styleD(Sid::pageEvenLeftMargin) * conversion);
    xml.tag("top-margin",    s.styleD(Sid::pageOddTopMargin) * conversion);
    xml.tag("bottom-margin", s.styleD(Sid::pageOddBottomMargin) * conversion);
    xml.endElement();

    xml.endElement();
}

//---------------------------------------------------------
//   defaults
//---------------------------------------------------------

// _spatium = DPMM * (millimeter * 10.0 / tenths);

static void defaults(XmlWriter& xml, const MStyle& s, double& millimeters, const int& tenths)
{
    xml.startElement("defaults");
    {
        xml.startElement("scaling");
        xml.tag("millimeters", millimeters);
        xml.tag("tenths", tenths);
        xml.endElement();
    }

    if (s.styleB(Sid::concertPitch)) {
        xml.tag("concert-score");
    }

    writePageFormat(s, xml, INCH / millimeters * tenths);

    // when exporting only manual or no breaks, system-distance is not written at all
    if (s.styleB(Sid::dividerLeft) || s.styleB(Sid::dividerRight)) {
        xml.startElement("system-layout");
        xml.startElement("system-dividers");
        if (s.styleB(Sid::dividerLeft)) {
            xml.tag("left-divider", { { "print-object", "yes" },
                        { "relative-x", s.styleD(Sid::dividerLeftX) * 10 },
                        { "relative-y", s.styleD(Sid::dividerLeftY) * 10 } });
        } else {
            xml.tag("left-divider", { { "print-object", "no" } });
        }
        if (s.styleB(Sid::dividerRight)) {
            xml.tag("right-divider", { { "print-object", "yes" },
                        { "relative-x", s.styleD(Sid::dividerRightX) * 10 },
                        { "relative-y", s.styleD(Sid::dividerRightY) * 10 } });
        } else {
            xml.tag("right-divider", { { "print-object", "no" } });
        }
        xml.endElement();
        xml.endElement();
    }

    {
        xml.startElement("appearance");
        // line width values in tenth
        xml.tag("line-width", { { "type", "light barline" } }, s.styleS(Sid::barWidth).val() * 10);
        xml.tag("line-width", { { "type", "heavy barline" } }, s.styleS(Sid::endBarWidth).val() * 10);
        xml.tag("line-width", { { "type", "beam" } }, s.styleS(Sid::beamWidth).val() * 10);
        xml.tag("line-width", { { "type", "bracket" } }, s.styleS(Sid::bracketWidth).val() * 10);
        xml.tag("line-width", { { "type", "dashes" } }, s.styleS(Sid::lyricsDashLineThickness).val() * 10);
        xml.tag("line-width", { { "type", "enclosure" } }, s.styleD(Sid::staffTextFrameWidth) * 10);
        xml.tag("line-width", { { "type", "ending" } }, s.styleS(Sid::voltaLineWidth).val() * 10);
        xml.tag("line-width", { { "type", "extend" } }, s.styleS(Sid::lyricsLineThickness).val() * 10);
        xml.tag("line-width", { { "type", "leger" } }, s.styleS(Sid::ledgerLineWidth).val() * 10);
        xml.tag("line-width", { { "type", "pedal" } }, s.styleS(Sid::pedalLineWidth).val() * 10);
        xml.tag("line-width", { { "type", "octave shift" } }, s.styleS(Sid::ottavaLineWidth).val() * 10);
        xml.tag("line-width", { { "type", "slur middle" } }, s.styleS(Sid::slurMidWidth).val() * 10);
        xml.tag("line-width", { { "type", "slur tip" } }, s.styleS(Sid::slurEndWidth).val() * 10);
        xml.tag("line-width", { { "type", "staff" } }, s.styleS(Sid::staffLineWidth).val() * 10);
        xml.tag("line-width", { { "type", "stem" } }, s.styleS(Sid::stemWidth).val() * 10);
        xml.tag("line-width", { { "type", "tie middle" } }, s.styleS(Sid::tieMidWidth).val() * 10);
        xml.tag("line-width", { { "type", "tie tip" } }, s.styleS(Sid::tieEndWidth).val() * 10);
        xml.tag("line-width", { { "type", "tuplet bracket" } }, s.styleS(Sid::tupletBracketWidth).val() * 10);
        xml.tag("line-width", { { "type", "wedge" } }, s.styleS(Sid::hairpinLineWidth).val() * 10);
        // note size values in percent
        xml.tag("note-size", { { "type", "cue" } }, s.styleD(Sid::smallNoteMag) * 100);
        xml.tag("note-size", { { "type", "grace" } }, s.styleD(Sid::graceNoteMag) * 100);
        xml.tag("note-size", { { "type", "grace-cue" } }, s.styleD(Sid::graceNoteMag) * s.styleD(Sid::smallNoteMag) * 100);
        xml.endElement();
    }

    // font defaults
    // as MuseScore supports dozens of different styles, while MusicXML only has defaults
    // for music, words and lyrics, use Tid STAFF (typically used for words)
    // and LYRIC1 to get MusicXML defaults

    xml.tag("music-font", { { "font-family", s.styleSt(Sid::musicalSymbolFont) } });
    xml.tag("word-font", { { "font-family", s.styleSt(Sid::staffTextFontFace) }, { "font-size", s.styleD(Sid::staffTextFontSize) } });
    xml.tag("lyric-font",
            { { "font-family", s.styleSt(Sid::lyricsOddFontFace) }, { "font-size", s.styleD(Sid::lyricsOddFontSize) } });
    xml.endElement();
}

//---------------------------------------------------------
//   formatForWords
//---------------------------------------------------------

static CharFormat formatForWords(const MStyle& s)
{
    CharFormat defFmt;
    defFmt.setFontFamily(s.styleSt(Sid::staffTextFontFace));
    defFmt.setFontSize(s.styleD(Sid::staffTextFontSize));
    return defFmt;
}

//---------------------------------------------------------
//   creditWords
//---------------------------------------------------------

static void creditWords(XmlWriter& xml, const MStyle& s, const page_idx_t pageNr,
                        const double x, const double y, const String& just, const String& val,
                        const std::list<TextFragment>& words, const String& creditType)
{
    // prevent incorrect MusicXML for empty text
    if (words.empty()) {
        return;
    }

    const String mtf = s.styleSt(Sid::musicalTextFont);
    const CharFormat defFmt = formatForWords(s);

    // export formatted
    xml.startElement("credit", { { "page", pageNr } });
    if (!creditType.empty()) {
        xml.tag("credit-type", creditType);
    }
    String attr = String(u" default-x=\"%1\"").arg(x);
    attr += String(u" default-y=\"%1\"").arg(y);
    attr += u" justify=\"" + just + u"\"";
    attr += u" valign=\"" + val + u"\"";
    MScoreTextToMusicXml mttm(u"credit-words", attr, defFmt, mtf);
    mttm.writeTextFragments(words, xml);
    xml.endElement();
}

//---------------------------------------------------------
//   parentHeight
//---------------------------------------------------------

static double parentHeight(const EngravingItem* element)
{
    const EngravingItem* parent = element->parentItem();

    if (!parent) {
        return 0;
    }

    if (parent->type() == ElementType::VBOX) {
        return parent->height();
    }

    return 0;
}

//---------------------------------------------------------
//   tidToCreditType
//---------------------------------------------------------

static String tidToCreditType(const TextStyleType tid)
{
    String res;
    switch (tid) {
    case TextStyleType::COMPOSER:
        res = u"composer";
        break;
    case TextStyleType::LYRICIST:
        res = u"lyricist";
        break;
    case TextStyleType::SUBTITLE:
        res = u"subtitle";
        break;
    case TextStyleType::TITLE:
        res = u"title";
        break;
    default:
        break;
    }
    return res;
}

//---------------------------------------------------------
//   textAsCreditWords
//---------------------------------------------------------

// Refactor suggestion: make getTenthsFromInches static instead of ExportMusicXml member function

static void textAsCreditWords(const ExportMusicXml* const expMxml, XmlWriter& xml, const MStyle& s, const int pageNr,
                              const Text* const text)
{
    // determine page formatting
    const double h  = expMxml->getTenthsFromInches(s.styleD(Sid::pageHeight));
    const double w  = expMxml->getTenthsFromInches(s.styleD(Sid::pageWidth));
    const double lm = expMxml->getTenthsFromInches(s.styleD(Sid::pageOddLeftMargin));
    const double rm = expMxml->getTenthsFromInches(s.styleD(Sid::pageEvenLeftMargin));
    const double ph = expMxml->getTenthsFromDots(parentHeight(text));

    double tx = w / 2;
    double ty = h - expMxml->getTenthsFromDots(text->pagePos().y());

    Align al = text->align();
    String just;
    String val;

    if (al == AlignH::RIGHT) {
        just = u"right";
        tx   = w - rm;
    } else if (al == AlignH::HCENTER) {
        just = u"center";
        // tx already set correctly
    } else {
        just = u"left";
        tx   = lm;
    }

    if (al == AlignV::BOTTOM) {
        val = u"bottom";
        ty -= ph;
    } else if (al == AlignV::VCENTER) {
        val = u"middle";
        ty -= ph / 2;
    } else if (al == AlignV::BASELINE) {
        val = u"baseline";
        ty -= ph / 2;
    } else {
        val = u"top";
        // ty already set correctly
    }

    const String creditType = tidToCreditType(text->textStyleType());

    creditWords(xml, s, pageNr, tx, ty, just, val, text->fragmentList(), creditType);
}

//---------------------------------------------------------
//   credits
//---------------------------------------------------------

void ExportMusicXml::credits(XmlWriter& xml)
{
    // find the vboxes in every page and write their elements as credit-words
    for (const Page* page : m_score->pages()) {
        const page_idx_t pageIdx = m_score->pageIdx(page);
        for (const System* system : page->systems()) {
            for (const MeasureBase* mb : system->measures()) {
                if (mb->isVBox()) {
                    for (const EngravingItem* element : mb->el()) {
                        if (element->isText()) {
                            const Text* text = toText(element);
                            textAsCreditWords(this, xml, m_score->style(), static_cast<int>(pageIdx) + 1, text);
                        }
                    }
                }
            }
        }
    }

    // put copyright at the bottom center of every page
    // note: as the copyright metatag contains plain text, special XML characters must be escaped
    // determine page formatting
    const String rights = m_score->metaTag(u"copyright");
    if (!rights.isEmpty()) {
        const MStyle& style = m_score->style();
        const double bm = getTenthsFromInches(style.styleD(Sid::pageOddBottomMargin));
        const double w  = getTenthsFromInches(style.styleD(Sid::pageWidth));
        /*
        const double h  = getTenthsFromInches(_score->styleD(Sid::pageHeight));
        const double lm = getTenthsFromInches(_score->styleD(Sid::pageOddLeftMargin));
        const double rm = getTenthsFromInches(_score->styleD(Sid::pageEvenLeftMargin));
        const double tm = getTenthsFromInches(_score->styleD(Sid::pageOddTopMargin));
        LOGD("page h=%g w=%g lm=%g rm=%g tm=%g bm=%g", h, w, lm, rm, tm, bm);
        */
        TextFragment f(XmlWriter::xmlString(rights));
        f.changeFormat(FormatId::FontFamily, style.styleSt(Sid::footerFontFace));
        f.changeFormat(FormatId::FontSize, style.styleD(Sid::footerFontSize));
        std::list<TextFragment> list;
        list.push_back(f);
        for (page_idx_t pageIdx = 0; pageIdx < m_score->npages(); ++pageIdx) {
            creditWords(xml, style, pageIdx + 1, w / 2, bm, u"center", u"bottom", list, u"rights");
        }
    }
}

//---------------------------------------------------------
//   midipitch2xml
//---------------------------------------------------------

static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char16_t noteTab[12] = { u'C', u'C', u'D', u'D', u'E', u'F', u'F', u'G', u'G', u'A', u'A', u'B' };

static void midipitch2xml(int pitch, char16_t& c, int& alter, int& octave)
{
    // 60 = C 4
    c      = noteTab[pitch % 12];
    alter  = alterTab[pitch % 12];
    octave = pitch / 12 - 1;
    //LOGD("midipitch2xml(pitch %d) step %c, alter %d, octave %d", pitch, c, alter, octave);
}

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

// TODO validation

static void pitch2xml(const Note* note, String& s, int& alter, int& octave)
{
    const Staff* st = note->staff();
    const Fraction tick = note->tick();
    const Instrument* instr = st->part()->instrument(tick);
    const Interval intval = note->concertPitch() ? 0 : instr->transpose();

    s      = tpc2stepName(note->tpc());
    alter  = tpc2alterByKey(note->tpc(), Key::C);
    // note that pitch must be converted to concert pitch
    // in order to calculate the correct octave
    octave = (note->pitch() - intval.chromatic - alter) / 12 - 1;

    //
    // HACK:
    // On percussion clefs there is no relationship between
    // note->pitch() and note->line()
    // note->line() is determined by drumMap
    //
    ClefType ct     = st->clef(tick);
    if (ct == ClefType::PERC || ct == ClefType::PERC2) {
        alter = 0;
        octave = line2pitch(note->line(), ct, Key::C) / 12 - 1;
    }

    // correct for ottava lines
    int ottava = 0;
    switch (note->ppitch() - note->pitch()) {
    case  24: ottava =  2;
        break;
    case  12: ottava =  1;
        break;
    case   0: ottava =  0;
        break;
    case -12: ottava = -1;
        break;
    case -24: ottava = -2;
        break;
    default:  LOGD("pitch2xml() tick=%d pitch()=%d ppitch()=%d",
                   tick.ticks(), note->pitch(), note->ppitch());
    }
    octave += ottava;

    //LOGD("pitch2xml(pitch %d, tpc %d, ottava %d clef %hhd) step    %s, alter    %d, octave    %d",
    //       note->pitch(), note->tpc(), ottava, clef, muPrintable(s), alter, octave);
}

// unpitch2xml -- calculate display-step and display-octave for an unpitched note
// note:
// even though this produces the correct step/octave according to Recordare's tutorial
// Finale Notepad 2012 does not import a three line staff with percussion clef correctly
// Same goes for Sibelius 6 in case of three or five line staff with percussion clef

static void unpitch2xml(const Note* note, String& s, int& octave)
{
    static char16_t table1[]  = u"FEDCBAG";

    Fraction tick        = note->chord()->tick();
    Staff* st       = note->staff();
    ClefType ct     = st->clef(tick);
    // offset in lines between staff with current clef and with G clef
    int clefOffset  = ClefInfo::pitchOffset(ct) - ClefInfo::pitchOffset(ClefType::G);
    // line note would be on on a five line staff with G clef
    // note top line is line 0, bottom line is line 8
    int line5g      = note->line() - clefOffset;
    // in MusicXML with percussion clef, step and octave are determined as if G clef is used
    // when stafflines is not equal to five, in MusicXML the bottom line is still E4.
    // in MuseScore assumes line 0 is F5
    // MS line numbers (top to bottom) plus correction to get lowest line at E4 (line 8)
    // 1 line staff: 0             -> correction 8
    // 3 line staff: 2, 4, 6       -> correction 2
    // 5 line staff: 0, 2, 4, 6, 8 -> correction 0
    // TODO handle other # staff lines ?
    if (st->lines(Fraction(0, 1)) == 1) {
        line5g += 8;
    }
    if (st->lines(Fraction(0, 1)) == 3) {
        line5g += 2;
    }
    // index in table1 to get step
    int stepIdx     = (line5g + 700) % 7;
    // get step
    s               = Char(table1[stepIdx]);
    // calculate octave, offset "3" correcting for the fact that an octave starts
    // with C instead of F
    octave =(3 - line5g + 700) / 7 + 5 - 100;
    // LOGD("ExportMusicXml::unpitch2xml(%p) clef %d clef.po %d clefOffset %d staff.lines %d note.line %d line5g %d step %c oct %d",
    //        note, ct, clefTable[ct].pitchOffset, clefOffset, st->lines(), note->line(), line5g, step, octave);
}

//---------------------------------------------------------
//   tick2xml
//    set type + dots depending on tick len
//---------------------------------------------------------

static String tick2xml(const Fraction& ticks, int* dots)
{
    TDuration t(ticks);
    *dots = t.dots();
    if (ticks == Fraction(0, 1)) {
        t.setType(DurationType::V_MEASURE);
        *dots = 0;
    }
    return String::fromAscii(TConv::toXml(t.type()).ascii());
}

//---------------------------------------------------------
//   findVolta -- find volta starting in measure m
//---------------------------------------------------------

static Volta* findVolta(const Measure* const m, bool left, const track_idx_t track)
{
    Fraction stick = m->tick();
    Fraction etick = m->tick() + m->ticks();
    auto spanners = m->score()->spannerMap().findOverlapping(stick.ticks(), etick.ticks());
    for (auto i : spanners) {
        Spanner* el = i.value;
        if (el->type() != ElementType::VOLTA || track2staff(el->track()) != track2staff(track)) {
            continue;
        }
        if (left && el->tick() == stick) {
            return (Volta*)el;
        }
        if (!left && el->tick2() == etick) {
            return (Volta*)el;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   ending
//---------------------------------------------------------

static void ending(XmlWriter& xml, Volta* v, bool left)
{
    String number;
    String type;
    for (int i : v->endings()) {
        if (!number.isEmpty()) {
            number += u", ";
        }
        number += String::number(i);
    }
    if (left) {
        type = u"start";
    } else {
        Volta::Type st = v->voltaType();
        switch (st) {
        case Volta::Type::OPEN:
            type = u"discontinue";
            break;
        case Volta::Type::CLOSED:
            type = u"stop";
            break;
        default:
            LOGD("unknown volta subtype %d", int(st));
            return;
        }
    }
    String voltaXml = String(u"ending number=\"%1\" type=\"%2\"").arg(number, type);
    voltaXml += ExportMusicXml::positioningAttributes(v, left);
    if (left) {
        if (!v->visible()) {
            voltaXml += u" print-object=\"no\"";
        }
        voltaXml += color2xml(v);
        xml.tagRaw(voltaXml, v->text().toXmlEscaped());
    } else {
        xml.tagRaw(voltaXml);
    }
}

//---------------------------------------------------------
//   barlineLeft -- search for and handle barline left
//---------------------------------------------------------

void ExportMusicXml::barlineLeft(const Measure* const m, const track_idx_t track)
{
    bool rs = m->repeatStart();
    Volta* volta = findVolta(m, true, track);
    if (!rs && !volta) {
        return;
    }
    m_attr.doAttr(m_xml, false);
    m_xml.startElement("barline", { { "location", "left" } });
    if (rs) {
        m_xml.tag("bar-style", String(u"heavy-light"));
    }
    if (volta) {
        ending(m_xml, volta, true);
    }
    if (rs) {
        m_xml.tag("repeat", { { "direction", "forward" } });
    }
    m_xml.endElement();
}

//---------------------------------------------------------
//   shortBarlineStyle -- recognize normal but shorter barline styles
//---------------------------------------------------------

static String shortBarlineStyle(const BarLine* bl)
{
    if (bl->barLineType() == BarLineType::NORMAL && !bl->spanStaff()) {
        if (bl->spanTo() < 0) {
            // lowest point of barline above lowest staff line
            if (bl->spanFrom() < 0) {
                return u"tick";               // highest point of barline above highest staff line
            } else {
                return u"short";               // highest point of barline below highest staff line
            }
        }
    }

    return String();
}

//---------------------------------------------------------
//   normalBarlineStyle -- recognize other barline styles
//---------------------------------------------------------

static String normalBarlineStyle(const BarLine* bl)
{
    const BarLineType bst = bl->barLineType();

    switch (bst) {
    case BarLineType::NORMAL:
        return u"regular";
    case BarLineType::DOUBLE:
        return u"light-light";
    case BarLineType::REVERSE_END:
        return u"heavy-light";
    case BarLineType::BROKEN:
        return u"dashed";
    case BarLineType::DOTTED:
        return u"dotted";
    case BarLineType::END:
    case BarLineType::END_REPEAT:
    case BarLineType::END_START_REPEAT:
        return u"light-heavy";
    case BarLineType::HEAVY:
        return u"heavy";
    case BarLineType::DOUBLE_HEAVY:
        return u"heavy-heavy";
    default:
        LOGD("bar subtype %d not supported", int(bst));
    }

    return String();
}

//---------------------------------------------------------
//   barlineMiddle -- handle barline middle
//---------------------------------------------------------

void ExportMusicXml::barlineMiddle(const BarLine* bl)
{
    bool vis = bl->visible();
    String shortStyle = shortBarlineStyle(bl);
    String normalStyle = normalBarlineStyle(bl);
    String barStyle;
    if (!vis) {
        barStyle = u"none";
    } else if (!shortStyle.empty()) {
        barStyle = shortStyle;
    } else {
        barStyle = normalStyle;
    }

    if (!barStyle.empty()) {
        m_xml.startElement("barline", { { "location", "middle" } });
        m_xml.tag("bar-style", barStyle);
        m_xml.endElement();
    }
}

//---------------------------------------------------------
//   fermataPosition -  return fermata y position as MusicXML string
//---------------------------------------------------------

String ExportMusicXml::fermataPosition(const Fermata* const fermata)
{
    String res;

    if (configuration()->exportLayout()) {
        constexpr double SPATIUM2TENTHS = 10;
        constexpr double EPSILON = 0.01;
        const double spatium = fermata->spatium();
        const double defY = -1 * SPATIUM2TENTHS * fermata->ldata()->pos().y() / spatium;
        const double relY = -1 * SPATIUM2TENTHS * fermata->offset().y() / spatium;

        if (std::abs(defY) >= EPSILON) {
            res += String(u" default-y=\"%1\"").arg(String::number(defY, 2));
        }
        if (std::abs(relY) >= EPSILON) {
            res += String(u" relative-y=\"%1\"").arg(String::number(relY, 2));
        }
    }

    return res;
}

//---------------------------------------------------------
//   fermata - write a fermata
//---------------------------------------------------------

static void fermata(const Fermata* const a, XmlWriter& xml)
{
    String tagName = u"fermata";
    tagName += String(u" type=\"%1\"").arg(a->placement() == PlacementV::ABOVE ? u"upright" : u"inverted");
    tagName += ExportMusicXml::fermataPosition(a);
    tagName += color2xml(a);
    SymId id = a->symId();
    if (id == SymId::fermataAbove || id == SymId::fermataBelow) {
        xml.tagRaw(tagName);
    } else if (id == SymId::fermataShortAbove || id == SymId::fermataShortBelow) {
        xml.tagRaw(tagName, "angled");
    } else if (id == SymId::fermataLongAbove || id == SymId::fermataLongBelow) {
        xml.tagRaw(tagName, "square");
    } else if (id == SymId::fermataVeryShortAbove || id == SymId::fermataVeryShortBelow) {
        xml.tagRaw(tagName, "double-angled");
    } else if (id == SymId::fermataVeryLongAbove || id == SymId::fermataVeryLongBelow) {
        xml.tagRaw(tagName, "double-square");
    } else if (id == SymId::fermataLongHenzeAbove || id == SymId::fermataLongHenzeBelow) {
        xml.tagRaw(tagName, "double-dot");
    } else if (id == SymId::fermataShortHenzeAbove || id == SymId::fermataShortHenzeBelow) {
        xml.tagRaw(tagName, "half-curve");
    } else if (id == SymId::curlewSign) {
        xml.tagRaw(tagName, "curlew");
    } else {
        LOGD("unsupported fermata SymId %d", static_cast<int>(id));
    }
}

//---------------------------------------------------------
//   barlineHasFermata -- search for fermata on barline
//---------------------------------------------------------

static bool barlineHasFermata(const BarLine* const barline, const track_idx_t strack, const track_idx_t etrack)       // TODO: track
{
    const Segment* seg = barline ? barline->segment() : 0;
    if (seg) {
        for (const EngravingItem* anno : seg->annotations()) {
            if (anno->isFermata() && strack <= anno->track() && anno->track() < etrack) {
                return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------
//   writeBarlineFermata -- write fermata on barline
//---------------------------------------------------------

static void writeBarlineFermata(const BarLine* const barline, XmlWriter& xml, const track_idx_t strack, const track_idx_t etrack)
{
    const Segment* seg = barline ? barline->segment() : 0;
    if (seg) {
        for (const EngravingItem* anno : seg->annotations()) {
            if (anno->isFermata() && strack <= anno->track() && anno->track() < etrack) {
                fermata(toFermata(anno), xml);
            }
        }
    }
}

//---------------------------------------------------------
//   barlineRight -- search for and handle barline right
//---------------------------------------------------------

void ExportMusicXml::barlineRight(const Measure* const m, const track_idx_t strack, const track_idx_t etrack)
{
    const Measure* mmR1 = m->coveringMMRestOrThis();   // the multi measure rest this measure is covered by
    const Measure* mmRLst = mmR1->isMMRest() ? mmR1->mmRestLast() : 0;   // last measure of replaced sequence of empty measures
    // note: use barlinetype as found in multi measure rest for last measure of replaced sequence
    BarLineType bst = m == mmRLst ? mmR1->endBarLineType() : m->endBarLineType();
    const bool visible = m->endBarLineVisible();
    String color;

    bool needBarStyle = (bst != BarLineType::NORMAL && bst != BarLineType::START_REPEAT) || !visible;
    Volta* volta = findVolta(m, false, strack);
    // detect short and tick barlines
    String special;
    const BarLine* bl = m->endBarLine();
    if (bl) {
        color = color2xml(bl);
        if (bst == BarLineType::NORMAL && !bl->spanStaff()) {
            if (bl->spanFrom() == BARLINE_SPAN_TICK1_FROM && bl->spanTo() == BARLINE_SPAN_TICK1_TO) {
                special = u"tick";
            }
            if (bl->spanFrom() == BARLINE_SPAN_TICK2_FROM && bl->spanTo() == BARLINE_SPAN_TICK2_TO) {
                special = u"tick";
            }
            if (bl->spanFrom() == BARLINE_SPAN_SHORT1_FROM && bl->spanTo() == BARLINE_SPAN_SHORT1_TO) {
                special = u"short";
            }
            if (bl->spanFrom() == BARLINE_SPAN_SHORT2_FROM && bl->spanTo() == BARLINE_SPAN_SHORT2_FROM) {
                special = u"short";
            }
        }
    }

    // check fermata
    // no need to take mmrest into account, MS does not create mmrests for measure with fermatas
    const bool hasFermata = barlineHasFermata(m->endBarLine(), strack, etrack);

    if (!needBarStyle && !volta && special.isEmpty() && !hasFermata && color.isEmpty()) {
        return;
    }

    m_xml.startElement("barline", { { "location", "right" } });
    String tagName = u"bar-style";
    tagName += color;
    if (needBarStyle) {
        if (!visible) {
            m_xml.tagRaw(tagName, "none");
        } else {
            switch (bst) {
            case BarLineType::DOUBLE:
                m_xml.tagRaw(tagName, "light-light");
                break;
            case BarLineType::REVERSE_END:
                m_xml.tagRaw(tagName, "heavy-light");
                break;
            case BarLineType::BROKEN:
                m_xml.tagRaw(tagName, "dashed");
                break;
            case BarLineType::DOTTED:
                m_xml.tagRaw(tagName, "dotted");
                break;
            case BarLineType::END:
            case BarLineType::END_REPEAT:
            case BarLineType::END_START_REPEAT:
                m_xml.tagRaw(tagName, "light-heavy");
                break;
            case BarLineType::HEAVY:
                m_xml.tagRaw(tagName, "heavy");
                break;
            case BarLineType::DOUBLE_HEAVY:
                m_xml.tagRaw(tagName, "heavy-heavy");
                break;
            default:
                LOGD("ExportMusicXml::bar(): bar subtype %d not supported", int(bst));
                break;
            }
        }
    } else if (!special.isEmpty()) {
        m_xml.tagRaw(tagName, special);
    } else if (!color.isEmpty()) {
        m_xml.tagRaw(tagName, "regular");
    }

    writeBarlineFermata(m->endBarLine(), m_xml, strack, etrack);

    if (volta) {
        ending(m_xml, volta, false);
    }

    if (bst == BarLineType::END_REPEAT || bst == BarLineType::END_START_REPEAT) {
        if (m->repeatCount() > 2) {
            m_xml.tag("repeat", { { "direction", "backward" }, { "times", m->repeatCount() } });
        } else {
            m_xml.tag("repeat", { { "direction", "backward" } });
        }
    }

    m_xml.endElement();
}

//---------------------------------------------------------
//   calculateTimeDeltaInDivisions
//---------------------------------------------------------

static int calculateTimeDeltaInDivisions(const Fraction& t1, const Fraction& t2, const int divisions)
{
    const Fraction resAsFraction { (4 * divisions * (t1 - t2)) };
    return resAsFraction.reduced().numerator();
}

//---------------------------------------------------------
//   calculateDurationInDivisions
//---------------------------------------------------------

static int calculateDurationInDivisions(const Fraction& tick, const int divisions)
{
    return calculateTimeDeltaInDivisions(tick, Fraction { 0, 1 }, divisions);
}

//---------------------------------------------------------
//   moveToTick
//---------------------------------------------------------

void ExportMusicXml::moveToTick(const Fraction& t)
{
    //LOGD("ExportMusicXml::moveToTick(t=%s) _tick=%s", muPrintable(t.print()), muPrintable(_tick.print()));
    if (t < m_tick) {
#ifdef DEBUG_TICK
        LOGD(" -> backup");
#endif
        m_attr.doAttr(m_xml, false);
        m_xml.startElement("backup");
        m_xml.tag("duration", calculateTimeDeltaInDivisions(m_tick, t, m_div));
        m_xml.endElement();
    } else if (t > m_tick) {
#ifdef DEBUG_TICK
        LOGD(" -> forward");
#endif
        m_attr.doAttr(m_xml, false);
        m_xml.startElement("forward");
        m_xml.tag("duration", calculateTimeDeltaInDivisions(t, m_tick, m_div));
        m_xml.endElement();
    }
    m_tick = t;
}

void ExportMusicXml::moveToTickIfNeed(const Fraction& t)
{
    if (m_tick != t) {
        m_attr.doAttr(m_xml, false);
        moveToTick(t);
    }
}

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

void ExportMusicXml::timesig(const TimeSig* tsig)
{
    const TimeSigType st = tsig->timeSigType();
    const Fraction ts = tsig->sig();
    const int z = ts.numerator();
    const int n = ts.denominator();
    const String ns = tsig->numeratorString();

    m_attr.doAttr(m_xml, true);
    XmlWriter::Attributes attrs;
    if (st == TimeSigType::FOUR_FOUR) {
        attrs = { { "symbol", "common" } };
    } else if (st == TimeSigType::ALLA_BREVE) {
        attrs = { { "symbol", "cut" } };
    } else if (!ns.empty() && tsig->denominatorString().empty()) {
        attrs = { { "symbol", "single-number" } };
    }
    if (!tsig->visible()) {
        attrs.emplace_back(std::make_pair("print-object", "no"));
    }

    addColorAttr(tsig, attrs);

    m_xml.startElement("time", attrs);

    static const std::regex beats_re("^\\d+(\\+\\d+)+$");
    if (std::regex_match(ns.toStdString(), beats_re)) {
        // if compound numerator, exported as is
        m_xml.tag("beats", ns);
    } else {
        // else fall back and use the numerator as integer
        m_xml.tag("beats", z);
    }
    m_xml.tag("beat-type", n);
    m_xml.endElement();
}

//---------------------------------------------------------
//   accSymId2alter
//---------------------------------------------------------

static double accSymId2alter(SymId id)
{
    double res = 0;
    switch (id) {
    case SymId::accidentalFlat:                            res = -1;
        break;
    case SymId::accidentalNatural:                         res =  0;
        break;
    case SymId::accidentalSharp:                           res =  1;
        break;
    case SymId::accidentalDoubleSharp:                     res =  2;
        break;
    case SymId::accidentalDoubleFlat:                      res = -2;
        break;
    case SymId::accidentalTripleSharp:                     res =  3;
        break;
    case SymId::accidentalTripleFlat:                      res = -3;
        break;
    case SymId::accidentalNaturalFlat:                     res = -1;
        break;
    case SymId::accidentalNaturalSharp:                    res =  1;
        break;
    case SymId::accidentalSharpSharp:                      res =  2;
        break;

    // Gould arrow quartertone
    case SymId::accidentalQuarterToneFlatArrowUp:          res = -0.5;
        break;
    case SymId::accidentalThreeQuarterTonesFlatArrowDown:  res = -1.5;
        break;
    case SymId::accidentalQuarterToneSharpNaturalArrowUp:  res =  0.5;
        break;
    case SymId::accidentalQuarterToneFlatNaturalArrowDown: res = -0.5;
        break;
    case SymId::accidentalThreeQuarterTonesSharpArrowUp:   res =  1.5;
        break;
    case SymId::accidentalQuarterToneSharpArrowDown:       res =  0.5;
        break;
    case SymId::accidentalFiveQuarterTonesSharpArrowUp:    res =  2.5;
        break;
    case SymId::accidentalThreeQuarterTonesSharpArrowDown: res =  1.5;
        break;
    case SymId::accidentalThreeQuarterTonesFlatArrowUp:    res = -1.5;
        break;
    case SymId::accidentalFiveQuarterTonesFlatArrowDown:   res = -2.5;
        break;
    case SymId::accidentalArrowDown:                       res = -0.5;
        break;
    case SymId::accidentalArrowUp:                         res =  0.5;
        break;

    // Stein-Zimmermann
    case SymId::accidentalQuarterToneFlatStein:            res = -0.5;
        break;
    case SymId::accidentalThreeQuarterTonesFlatZimmermann: res = -1.5;
        break;
    case SymId::accidentalQuarterToneSharpStein:           res =  0.5;
        break;
    case SymId::accidentalThreeQuarterTonesSharpStein:     res =  1.5;
        break;

    // Arel-Ezgi-Uzdilek (AEU)
    case SymId::accidentalBuyukMucennebFlat:               res = -0.89;
        break;
    case SymId::accidentalBakiyeFlat:                      res = -0.44;
        break;
    case SymId::accidentalKucukMucennebSharp:              res =  0.56;
        break;
    case SymId::accidentalBuyukMucennebSharp:              res =  0.89;
        break;

    // Persian
    case SymId::accidentalSori:                            res =  0.33;
        break;
    case SymId::accidentalKoron:                           res = -0.67;
        break;

    // Wyschnegradsky
    case SymId::accidentalWyschnegradsky10TwelfthsFlat:    res = -1.67;
        break;
    case SymId::accidentalWyschnegradsky10TwelfthsSharp:   res =  1.67;
        break;
    case SymId::accidentalWyschnegradsky11TwelfthsFlat:    res = -1.83;
        break;
    case SymId::accidentalWyschnegradsky11TwelfthsSharp:   res =  1.83;
        break;
    case SymId::accidentalWyschnegradsky1TwelfthsFlat:     res = -0.17;
        break;
    case SymId::accidentalWyschnegradsky1TwelfthsSharp:    res =  0.17;
        break;
    case SymId::accidentalWyschnegradsky2TwelfthsFlat:     res = -0.33;
        break;
    case SymId::accidentalWyschnegradsky2TwelfthsSharp:    res =  0.33;
        break;
    case SymId::accidentalWyschnegradsky3TwelfthsFlat:     res = -0.50;
        break;
    case SymId::accidentalWyschnegradsky3TwelfthsSharp:    res =  0.50;
        break;
    case SymId::accidentalWyschnegradsky4TwelfthsFlat:     res = -0.67;
        break;
    case SymId::accidentalWyschnegradsky4TwelfthsSharp:    res =  0.67;
        break;
    case SymId::accidentalWyschnegradsky5TwelfthsFlat:     res = -0.83;
        break;
    case SymId::accidentalWyschnegradsky5TwelfthsSharp:    res =  0.83;
        break;
    case SymId::accidentalWyschnegradsky6TwelfthsFlat:     res =    -1;
        break;
    case SymId::accidentalWyschnegradsky6TwelfthsSharp:    res =     1;
        break;
    case SymId::accidentalWyschnegradsky7TwelfthsFlat:     res = -1.16;
        break;
    case SymId::accidentalWyschnegradsky7TwelfthsSharp:    res =  1.16;
        break;
    case SymId::accidentalWyschnegradsky8TwelfthsFlat:     res = -1.33;
        break;
    case SymId::accidentalWyschnegradsky8TwelfthsSharp:    res =  1.33;
        break;
    case SymId::accidentalWyschnegradsky9TwelfthsFlat:     res = -1.50;
        break;
    case SymId::accidentalWyschnegradsky9TwelfthsSharp:    res =  1.50;
        break;

    // the most important (Spartan) Sagittal accidentals
    case SymId::accSagittal5v7KleismaDown:                 res = -0.06;
        break;
    case SymId::accSagittal5v7KleismaUp:                   res =  0.06;
        break;
    case SymId::accSagittal5CommaDown:                     res = -0.22;
        break;
    case SymId::accSagittal5CommaUp:                       res =  0.22;
        break;
    case SymId::accSagittal7CommaDown:                     res = -0.27;
        break;
    case SymId::accSagittal7CommaUp:                       res =  0.27;
        break;
    case SymId::accSagittal25SmallDiesisDown:              res = -0.43;
        break;
    case SymId::accSagittal25SmallDiesisUp:                res =  0.43;
        break;
    case SymId::accSagittal35MediumDiesisDown:             res = -0.49;
        break;
    case SymId::accSagittal35MediumDiesisUp:               res =  0.49;
        break;
    case SymId::accSagittal11MediumDiesisDown:             res = -0.53;
        break;
    case SymId::accSagittal11MediumDiesisUp:               res =  0.53;
        break;
    case SymId::accSagittal11LargeDiesisDown:              res = -0.60;
        break;
    case SymId::accSagittal11LargeDiesisUp:                res =  0.60;
        break;
    case SymId::accSagittal35LargeDiesisDown:              res = -0.65;
        break;
    case SymId::accSagittal35LargeDiesisUp:                res =  0.65;
        break;
    case SymId::accSagittalFlat25SUp:                      res = -0.71;
        break;
    case SymId::accSagittalSharp25SDown:                   res =  0.71;
        break;
    case SymId::accSagittalFlat7CUp:                       res = -0.86;
        break;
    case SymId::accSagittalSharp7CDown:                    res =  0.86;
        break;
    case SymId::accSagittalFlat5CUp:                       res = -0.92;
        break;
    case SymId::accSagittalSharp5CDown:                    res =  0.92;
        break;
    case SymId::accSagittalFlat5v7kUp:                     res = -1.08;
        break;
    case SymId::accSagittalSharp5v7kDown:                  res =  1.08;
        break;
    case SymId::accSagittalFlat:                           res = -1.14;
        break;
    case SymId::accSagittalSharp:                          res =  1.14;
        break;

    // Turkish folk music accidentals
    case SymId::accidental1CommaFlat:                      res = -0.22;
        break;
    case SymId::accidental1CommaSharp:                     res =  0.22;
        break;
    case SymId::accidental2CommaFlat:                      res = -0.44;
        break;
    case SymId::accidental2CommaSharp:                     res =  0.44;
        break;
    case SymId::accidental3CommaFlat:                      res = -0.67;
        break;
    case SymId::accidental3CommaSharp:                     res =  0.67;
        break;
    case SymId::accidental4CommaFlat:                      res = -0.89;
        break;
    case SymId::accidental5CommaSharp:                     res =  1.11;
        break;
    default: LOGD("accSymId2alter: unsupported sym %s", SymNames::nameForSymId(id).ascii());
    }
    return res;
}

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void ExportMusicXml::keysig(const KeySig* ks, ClefType ct, staff_idx_t staff, bool visible)
{
    static char16_t table2[]  = u"CDEFGAB";
    int po = ClefInfo::pitchOffset(ct);   // actually 7 * oct + step for topmost staff line
    //LOGD("keysig st %d key %d custom %d ct %hhd st %d", staff, ks->key(), ks->isCustom(), ct, staff);
    //LOGD(" pitch offset clef %d stp %d oct %d ", po, po % 7, po / 7);

    XmlWriter::Attributes attrs;
    if (staff) {
        attrs.emplace_back(std::make_pair("number", staff));
    }
    if (!visible) {
        attrs.emplace_back(std::make_pair("print-object", "no"));
    }
    addColorAttr(ks, attrs);

    m_attr.doAttr(m_xml, true);
    m_xml.startElement("key", attrs);

    //! NOTE It looks like there is some kind of problem here,
    //! layout data should not be used to write to a file or export
    const KeySig::LayoutData* ldata = ks->ldata();
    const std::vector<KeySym>& keysyms = ldata->keySymbols;
    if (ks->isCustom() && !ks->isAtonal() && keysyms.size() > 0) {
        // non-traditional key signature
        // MusicXML order is left-to-right order, while KeySims in keySymbols()
        // are in insertion order -> sorting required

        // first put the KeySyms in a map
        std::map<double, KeySym> map;
        for (const KeySym& ksym : keysyms) {
            map.insert({ ksym.xPos, ksym });
        }
        // then write them (automatically sorted on key)
        for (const KeySym& ksym : muse::values(map)) {
            int step = (po - ksym.line) % 7;
            //LOGD(" keysym sym %d -> line %d step %d", ksym.sym, ksym.line, step);
            m_xml.tag("key-step", String(Char(table2[step])));
            m_xml.tag("key-alter", accSymId2alter(ksym.sym));
            XmlWriter::Attributes accidentalAttrs;
            String s = accSymId2MusicXmlString(ksym.sym);
            if (s == u"other") {
                accidentalAttrs = { { "smufl", accSymId2SmuflMusicXmlString(ksym.sym) } };
            }
            m_xml.tag("key-accidental", accidentalAttrs, s);
        }
    } else {
        // traditional key signature
        m_xml.tag("fifths", static_cast<int>(ks->key()));
        switch (ks->mode()) {
        case KeyMode::NONE:       m_xml.tag("mode", "none");
            break;
        case KeyMode::MAJOR:      m_xml.tag("mode", "major");
            break;
        case KeyMode::MINOR:      m_xml.tag("mode", "minor");
            break;
        case KeyMode::DORIAN:     m_xml.tag("mode", "dorian");
            break;
        case KeyMode::PHRYGIAN:   m_xml.tag("mode", "phrygian");
            break;
        case KeyMode::LYDIAN:     m_xml.tag("mode", "lydian");
            break;
        case KeyMode::MIXOLYDIAN: m_xml.tag("mode", "mixolydian");
            break;
        case KeyMode::AEOLIAN:    m_xml.tag("mode", "aeolian");
            break;
        case KeyMode::IONIAN:     m_xml.tag("mode", "ionian");
            break;
        case KeyMode::LOCRIAN:    m_xml.tag("mode", "locrian");
            break;
        case KeyMode::UNKNOWN:              // fall thru
        default:
            if (ks->isCustom()) {
                m_xml.tag("mode", "none");
            }
        }
    }
    m_xml.endElement();
}

//---------------------------------------------------------
//   clef
//---------------------------------------------------------
struct MusicXmlClefInfo
{
    ClefType type;
    const char* sign;
    int octChng;
};

// table must be in sync with enum ClefType in types.h
static const std::vector<MusicXmlClefInfo> CLEF_INFOS = {
    { ClefType::G,          "G", 0 },
    { ClefType::G15_MB,     "G", -2 },
    { ClefType::G8_VB,      "G", -1 },
    { ClefType::G8_VA,      "G", 1 },
    { ClefType::G15_MA,     "G", 2 },
    { ClefType::G8_VB_O,    "G", -1 },
    { ClefType::G8_VB_P,    "G", 0 },
    { ClefType::G_1,        "G", 0 },

    { ClefType::C1,         "C", 0 },
    { ClefType::C2,         "C", 0 },
    { ClefType::C3,         "C", 0 },
    { ClefType::C4,         "C", 0 },
    { ClefType::C5,         "C", 0 },

    { ClefType::C_19C,      "G", 0 },

    { ClefType::C1_F18C,    "C", 0 },
    { ClefType::C3_F18C,    "C", 0 },
    { ClefType::C4_F18C,    "C", 0 },
    { ClefType::C1_F20C,    "C", 0 },
    { ClefType::C3_F20C,    "C", 0 },
    { ClefType::C4_F20C,    "C", 0 },

    { ClefType::F,          "F", 0 },
    { ClefType::F15_MB,     "F", -2 },
    { ClefType::F8_VB,      "F", -1 },
    { ClefType::F_8VA,      "F", 1 },
    { ClefType::F_15MA,     "F", 2 },
    { ClefType::F_B,        "F", 0 },
    { ClefType::F_C,        "F", 0 },
    { ClefType::F_F18C,     "F", 0 },
    { ClefType::F_19C,      "F", 0 },

    { ClefType::PERC,       "percussion", 0 },
    { ClefType::PERC2,      "percussion", 0 },

    { ClefType::TAB,        "TAB", 0 },
    { ClefType::TAB4,       "TAB", 0 },
    { ClefType::TAB_SERIF,  "TAB", 0 },
    { ClefType::TAB4_SERIF, "TAB", 0 },

    { ClefType::C4_8VB,     "C", -1 },
    { ClefType::G8_VB_C,    "G", -1 },
};

static const MusicXmlClefInfo findClefInfoByType(const ClefType& v)
{
    auto it = std::find_if(CLEF_INFOS.cbegin(), CLEF_INFOS.cend(), [v](const MusicXmlClefInfo& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != CLEF_INFOS.cend()) {
        static MusicXmlClefInfo dummy_;
        return dummy_;
    }
    return *it;
}

void ExportMusicXml::clef(staff_idx_t staff, const ClefType ct, const String& extraAttributes)
{
    clefDebug("ExportMusicXml::clef(staff %zu, clef %hhd)", staff, ct);

    String tagName = u"clef";
    if (staff) {
        tagName += String(u" number=\"%1\"").arg(static_cast<int>(staff));
    }
    tagName += extraAttributes;
    m_attr.doAttr(m_xml, true);
    m_xml.startElementRaw(tagName);

    MusicXmlClefInfo info = findClefInfoByType(ct);

    int line = ClefInfo::line(ct);
    m_xml.tag("sign", info.sign);
    m_xml.tag("line", line);
    if (info.octChng) {
        m_xml.tag("clef-octave-change", info.octChng);
    }
    m_xml.endElement();
}

//---------------------------------------------------------
//   tupletNesting
//---------------------------------------------------------

/*
 * determine the tuplet nesting level for cr
 * 0 = not part of tuplet
 * 1 = part of a single tuplet
 * 2 = part of two nested tuplets
 * etc.
 */

static int tupletNesting(const ChordRest* const cr)
{
    const DurationElement* el { cr->tuplet() };
    int nesting { 0 };
    while (el) {
        nesting++;
        el = el->tuplet();
    }
    return nesting;
}

//---------------------------------------------------------
//   isSimpleTuplet
//---------------------------------------------------------

/*
 * determine if t is simple, i.e. all its children are chords or rests
 */

static bool isSimpleTuplet(const Tuplet* const t)
{
    if (t->tuplet()) {
        return false;
    }
    for (const EngravingItem* el : t->elements()) {
        if (!el->isChordRest()) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   isTupletStart
//---------------------------------------------------------

/*
 * determine if t is the starting element of a tuplet
 */

static bool isTupletStart(const DurationElement* const el)
{
    const Tuplet* t = el->tuplet();
    if (!t) {
        return false;
    }
    return el == t->elements().front();
}

//---------------------------------------------------------
//   isTupletStop
//---------------------------------------------------------

/*
 * determine if t is the stopping element of a tuplet
 */

static bool isTupletStop(const DurationElement* const el)
{
    const Tuplet* t = el->tuplet();
    if (!t) {
        return false;
    }
    return el == t->elements().back();
}

//---------------------------------------------------------
//   startTupletAtLevel
//---------------------------------------------------------

/*
 * return the tuplet starting at tuplet nesting level, if any
 */

static const Tuplet* startTupletAtLevel(const DurationElement* const cr, const int level)
{
    const DurationElement* el { cr };
    if (!el->tuplet()) {
        return nullptr;
    }
    for (int i = 0; i < level; ++i) {
        if (!isTupletStart(el)) {
            return nullptr;
        }
        el = el->tuplet();
    }
    return toTuplet(el);
}

//---------------------------------------------------------
//   stopTupletAtLevel
//---------------------------------------------------------

/*
 * return the tuplet stopping at tuplet nesting level, if any
 */

static const Tuplet* stopTupletAtLevel(const DurationElement* const cr, const int level)
{
    const DurationElement* el { cr };
    if (!el->tuplet()) {
        return nullptr;
    }
    for (int i = 0; i < level; ++i) {
        if (!isTupletStop(el)) {
            return nullptr;
        }
        el = el->tuplet();
    }
    return toTuplet(el);
}

//---------------------------------------------------------
//   tupletTypeAndDots
//---------------------------------------------------------

static void tupletTypeAndDots(const String& type, const int dots, XmlWriter& xml)
{
    xml.tag("tuplet-type", type);
    for (int i = 0; i < dots; ++i) {
        xml.tag("tuplet-dot");
    }
}

//---------------------------------------------------------
//   tupletActualAndNormal
//---------------------------------------------------------

static void tupletActualAndNormal(const Tuplet* const t, XmlWriter& xml)
{
    xml.startElement("tuplet-actual");
    XmlWriter::Attributes tNumAttrs;
    addColorAttr(t, tNumAttrs);
    xml.tag("tuplet-number", tNumAttrs, t->ratio().numerator());
    int dots { 0 };
    const String s = tick2xml(t->baseLen().ticks(), &dots);
    tupletTypeAndDots(s, dots, xml);
    xml.endElement();
    xml.startElement("tuplet-normal");
    xml.tag("tuplet-number", tNumAttrs, t->ratio().denominator());
    tupletTypeAndDots(s, dots, xml);
    xml.endElement();
}

//---------------------------------------------------------
//   tupletStart
//---------------------------------------------------------

// LVIFIX: add placement to tuplet support
// <notations>
//   <tuplet type="start" placement="above" bracket="no"/>
// </notations>

static void tupletStart(const Tuplet* const t, const int number, const bool needActualAndNormal, Notations& notations, XmlWriter& xml)
{
    notations.tag(xml, t);
    String tupletTag = u"tuplet type=\"start\"";
    if (!isSimpleTuplet(t)) {
        tupletTag += String(u" number=\"%1\"").arg(number);
    }
    tupletTag += u" bracket=";
    tupletTag += t->hasBracket() ? u"\"yes\"" : u"\"no\"";
    if (t->numberType() == TupletNumberType::SHOW_RELATION) {
        tupletTag += u" show-number=\"both\"";
    } else if (t->numberType() == TupletNumberType::NO_TEXT) {
        tupletTag += u" show-number=\"none\"";
    }
    if (t->direction() != engraving::DirectionV::AUTO) {
        if (t->direction() == engraving::DirectionV::UP) {
            tupletTag += u" placement=\"above\"";
        } else if (t->direction() == engraving::DirectionV::DOWN) {
            tupletTag += u" placement=\"below\"";
        }
    }
    if (needActualAndNormal) {
        xml.startElementRaw(tupletTag);
        tupletActualAndNormal(t, xml);
        xml.endElement();
    } else {
        xml.tagRaw(tupletTag);
    }
}

//---------------------------------------------------------
//   tupletStop
//---------------------------------------------------------

static void tupletStop(const Tuplet* const t, const int number, Notations& notations, XmlWriter& xml)
{
    notations.tag(xml, t);
    XmlWriter::Attributes tupletAttrs = { { "type", "stop" } };
    if (!isSimpleTuplet(t)) {
        tupletAttrs.emplace_back(std::make_pair("number", number));
    }
    xml.tag("tuplet", tupletAttrs);
}

//---------------------------------------------------------
//   tupletStartStop
//---------------------------------------------------------

static void tupletStartStop(ChordRest* cr, Notations& notations, XmlWriter& xml)
{
    const int nesting = tupletNesting(cr);
    bool doActualAndNormal = (nesting > 1);
    if (cr->isChord() && isTwoNoteTremolo(toChord(cr))) {
        doActualAndNormal = true;
    }
    for (int level = nesting - 1; level >= 0; --level) {
        const Tuplet* startTuplet = startTupletAtLevel(cr, level + 1);
        if (startTuplet) {
            tupletStart(startTuplet, nesting - level, doActualAndNormal, notations, xml);
        }
        const Tuplet* stopTuplet = stopTupletAtLevel(cr, level + 1);
        if (stopTuplet) {
            tupletStop(stopTuplet, nesting - level, notations, xml);
        }
    }
}

//---------------------------------------------------------
//   findTrill -- get index of trill in trill table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findTrill(const Trill* tr) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (m_trills[i] == tr) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   writeAccidental
//---------------------------------------------------------

static void writeAccidental(XmlWriter& xml, const String& tagName, const Accidental* const acc)
{
    if (acc) {
        String s = accidentalType2MusicXmlString(acc->accidentalType());
        if (!s.empty()) {
            XmlWriter::Attributes attrs;
            if (s == "other") {
                attrs = { { "smufl", accidentalType2SmuflMusicXmlString(acc->accidentalType()) } };
            }
            String tag = tagName;
            if (acc->bracket() == AccidentalBracket::BRACKET) {
                if (acc->role() == AccidentalRole::USER) {
                    attrs.emplace_back(std::make_pair("editorial", "yes"));
                }
                attrs.emplace_back(std::make_pair("bracket", "yes"));
            } else if (acc->bracket() == AccidentalBracket::PARENTHESIS) {
                if (acc->role() == AccidentalRole::USER) {
                    attrs.emplace_back(std::make_pair("cautionary", "yes"));
                }
                attrs.emplace_back(std::make_pair("parentheses", "yes"));
            } else if (acc->role() == AccidentalRole::USER) {            // no way to tell "cautionary" from "editorial"
                attrs.emplace_back(std::make_pair("cautionary", "yes")); // so pick one
                attrs.emplace_back(std::make_pair("parentheses", "no")); // but use neither parenthesis nor bracket ;-)
            }
            if (tagName == "accidental-mark") {
                if (acc->placeAbove()) {
                    attrs.emplace_back(std::make_pair("placement", "above"));
                } else if (acc->placeBelow()) {
                    attrs.emplace_back(std::make_pair("placement", "below"));
                }
            }
            if (acc->isSmall()) {
                // only set if the accidental is smaller than the notehead
                const bool tiny = acc->note()->isSmall() || acc->note()->chord()->isSmall();
                attrs.emplace_back(std::make_pair("size", tiny ? "grace-cue" : "cue"));
            }
            addColorAttr(acc, attrs);
            xml.tag(AsciiStringView(tag.toStdString()), attrs, s);
        }
    }
}

//---------------------------------------------------------
//   writeDisplayName
//---------------------------------------------------------

static void writeDisplayName(XmlWriter& xml, const String& partName)
{
    // TODO: add text style attributes
    String displayText;
    for (size_t i = 0; i < partName.size(); ++i) {
        Char ch = partName.at(i);
        if (ch != u'♭' && ch != u'♯') {
            displayText += ch;
        } else {
            if (!displayText.empty()) {
                xml.tag("display-text", displayText);
            }
            if (ch == u'♭') {
                xml.tag("accidental-text", "flat");
            } else if (ch == u'♯') {
                xml.tag("accidental-text", "sharp");
            }
            displayText.clear();
        }
    }
    if (!displayText.empty()) {
        xml.tag("display-text", displayText);
    }
}

//---------------------------------------------------------
//   wavyLineStart
//---------------------------------------------------------

static void wavyLineStart(const Trill* tr, const int number, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
{
    notations.tag(xml, tr);
    ornaments.tag(xml);
    switch (tr->trillType()) {
    case TrillType::TRILL_LINE:
        xml.tagRaw(u"trill-mark" + color2xml(tr));
        break;
    case TrillType::UPPRALL_LINE:
        xml.tagRaw(u"other-ornament smufl=\"ornamentBottomLeftConcaveStroke\"" + color2xml(tr));
        break;
    case TrillType::DOWNPRALL_LINE:
        xml.tagRaw(u"other-ornament smufl=\"ornamentLeftVerticalStroke\"" + color2xml(tr));
        break;
    case TrillType::PRALLPRALL_LINE:
    default:
        break;
    }
    String tagName = u"wavy-line type=\"start\"";
    tagName += String(u" number=\"%1\"").arg(number + 1);
    tagName += color2xml(tr);
    tagName += ExportMusicXml::positioningAttributes(tr, true);
    xml.tagRaw(tagName);
    writeAccidental(xml, u"accidental-mark", tr->accidental());
}

//---------------------------------------------------------
//   wavyLineStop
//---------------------------------------------------------

static void wavyLineStop(const Trill* tr, const int number, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
{
    notations.tag(xml, tr);
    ornaments.tag(xml);
    String trillXml = String(u"wavy-line type=\"stop\" number=\"%1\"").arg(number + 1);
    trillXml += ExportMusicXml::positioningAttributes(tr, false);
    xml.tagRaw(trillXml);
}

//---------------------------------------------------------
//   wavyLineStartStop
//---------------------------------------------------------

void ExportMusicXml::wavyLineStartStop(const ChordRest* cr, Notations& notations, Ornaments& ornaments,
                                       TrillHash& trillStart, TrillHash& trillStop)
{
    if (muse::contains(trillStart, cr) && muse::contains(trillStop, cr)) {
        const Trill* tr = trillStart.at(cr);
        int n = findTrill(0);
        if (n >= 0) {
            wavyLineStart(tr, n, notations, ornaments, m_xml);
            wavyLineStop(tr, n, notations, ornaments, m_xml);
        } else {
            LOGD("too many overlapping trills (cr %p staff %zu tick %d)",
                 cr, cr->staffIdx(), cr->tick().ticks());
        }
    } else {
        if (muse::contains(trillStop, cr)) {
            const Trill* tr = trillStop.at(cr);
            int n = findTrill(tr);
            if (n >= 0) {
                // trill stop after trill start
                m_trills[n] = 0;
            } else {
                // trill stop before trill start
                n = findTrill(0);
                if (n >= 0) {
                    m_trills[n] = tr;
                } else {
                    LOGD("too many overlapping trills (cr %p staff %zu tick %d)",
                         cr, cr->staffIdx(), cr->tick().ticks());
                }
            }
            if (n >= 0) {
                wavyLineStop(tr, n, notations, ornaments, m_xml);
            }
            muse::remove(trillStop, cr);
        }
        if (muse::contains(trillStart, cr)) {
            const Trill* tr = trillStart.at(cr);
            int n = findTrill(tr);
            if (n >= 0) {
                LOGD("wavyLineStartStop error");
            } else {
                n = findTrill(0);
                if (n >= 0) {
                    m_trills[n] = tr;
                    wavyLineStart(tr, n, notations, ornaments, m_xml);
                } else {
                    LOGD("too many overlapping trills (cr %p staff %zu tick %d)",
                         cr, cr->staffIdx(), cr->tick().ticks());
                }
                muse::remove(trillStart, cr);
            }
        }
    }
}

//---------------------------------------------------------
//   tremoloSingleStartStop
//---------------------------------------------------------

static void tremoloSingleStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
{
    TremoloType st = chord->tremoloType();
    const EngravingItem* tr = chord->tremoloSingleChord()
                              ? static_cast<const EngravingItem*>(chord->tremoloSingleChord())
                              : static_cast<const EngravingItem*>(chord->tremoloTwoChord());

    if (st != TremoloType::INVALID_TREMOLO && ExportMusicXml::canWrite(tr)) {
        int count = 0;
        String type;

        if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
            if (st == TremoloType::BUZZ_ROLL) {
                type = u"unmeasured";
                count = 0;
            } else {
                type = u"single";
                switch (st) {
                case TremoloType::R8:  count = 1;
                    break;
                case TremoloType::R16: count = 2;
                    break;
                case TremoloType::R32: count = 3;
                    break;
                case TremoloType::R64: count = 4;
                    break;
                default: LOGD("unknown tremolo single %d", int(st));
                    break;
                }
            }
        } else if (chord->tremoloChordType() == TremoloChordType::TremoloFirstChord) {
            type = u"start";
            switch (st) {
            case TremoloType::C8:  count = 1;
                break;
            case TremoloType::C16: count = 2;
                break;
            case TremoloType::C32: count = 3;
                break;
            case TremoloType::C64: count = 4;
                break;
            default: LOGD("unknown tremolo double %d", int(st));
                break;
            }
        } else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondChord) {
            type = u"stop";
            switch (st) {
            case TremoloType::C8:  count = 1;
                break;
            case TremoloType::C16: count = 2;
                break;
            case TremoloType::C32: count = 3;
                break;
            case TremoloType::C64: count = 4;
                break;
            default: LOGD("unknown tremolo double %d", int(st));
                break;
            }
        } else {
            LOGD("unknown tremolo subtype %d", int(st));
        }

        if (!type.empty() && ((count > 0 && type != u"unmeasured") || (count == 0 && type == u"unmeasured"))) {
            notations.tag(xml, tr);
            ornaments.tag(xml);
            XmlWriter::Attributes attrs = { { "type", type } };
            if (type != u"stop") {
                addColorAttr(tr, attrs);
            }
            xml.tag("tremolo", attrs, count);
        }
    }
}

//---------------------------------------------------------
//   fermatas
//---------------------------------------------------------

static void fermatas(const std::vector<EngravingItem*>& cra, XmlWriter& xml, Notations& notations)
{
    for (const EngravingItem* e : cra) {
        if (!e->isFermata() || !ExportMusicXml::canWrite(e)) {
            continue;
        }
        notations.tag(xml, e);
        fermata(toFermata(e), xml);
    }
}

//---------------------------------------------------------
//   symIdToArtic
//---------------------------------------------------------

static std::vector<String> symIdToArtic(const SymId sid)
{
    switch (sid) {
    case SymId::articAccentAbove:
    case SymId::articAccentBelow:
        return { u"accent" };
    case SymId::articStaccatoAbove:
    case SymId::articStaccatoBelow:
        return { u"staccato" };
    case SymId::articStaccatissimoAbove:
    case SymId::articStaccatissimoBelow:
    case SymId::articStaccatissimoWedgeAbove:
    case SymId::articStaccatissimoWedgeBelow:
        return { u"staccatissimo" };
    case SymId::articTenutoAbove:
    case SymId::articTenutoBelow:
        return { u"tenuto" };
    case SymId::articMarcatoAbove:
    case SymId::articMarcatoBelow:
        return { u"strong-accent" };
    case SymId::articTenutoStaccatoAbove:
    case SymId::articTenutoStaccatoBelow:
        return { u"detached-legato" };
    case SymId::articSoftAccentAbove:
    case SymId::articSoftAccentBelow:
        return { u"soft-accent" };
    case SymId::articSoftAccentStaccatoAbove:
    case SymId::articSoftAccentStaccatoBelow:
        return { u"soft-accent", u"staccato" };
    case SymId::articSoftAccentTenutoAbove:
    case SymId::articSoftAccentTenutoBelow:
        return { u"soft-accent", u"tenuto" };
    case SymId::articSoftAccentTenutoStaccatoAbove:
    case SymId::articSoftAccentTenutoStaccatoBelow:
        return { u"soft-accent", u"detached-legato" };
    case SymId::articStressAbove:
    case SymId::articStressBelow:
        return { u"stress" };
    case SymId::articUnstressAbove:
    case SymId::articUnstressBelow:
        return { u"unstress" };
    case SymId::articAccentStaccatoAbove:
    case SymId::articAccentStaccatoBelow:
        return { u"accent", u"staccato" };
    case SymId::articMarcatoStaccatoAbove:
    case SymId::articMarcatoStaccatoBelow:
        return { u"strong-accent", u"staccato" };
    case SymId::articMarcatoTenutoAbove:
    case SymId::articMarcatoTenutoBelow:
        return { u"strong-accent", u"tenuto" };
    case SymId::articTenutoAccentAbove:
    case SymId::articTenutoAccentBelow:
        return { u"tenuto", u"accent" };
    case SymId::articStaccatissimoStrokeAbove:
    case SymId::articStaccatissimoStrokeBelow:
        return { u"spiccato" };
    default:
        return {}; // nothing
    }
}

//---------------------------------------------------------
//   symIdToOrnam
//---------------------------------------------------------

static String symIdToOrnam(const SymId sid)
{
    switch (sid) {
    case SymId::ornamentTrill:
        return u"trill-mark";
    case SymId::ornamentTurn:
        return u"turn";
    case SymId::ornamentTurnInverted:
        return u"inverted-turn";
    case SymId::ornamentTurnSlash:
        return u"turn slash=\"yes\"";
    case SymId::ornamentTurnUp:
        return u"vertical-turn";
    case SymId::ornamentTurnUpS:
        return u"inverted-vertical-turn";
    case SymId::ornamentMordent:
        return u"mordent";
    case SymId::ornamentShortTrill:
        return u"inverted-mordent";
    case SymId::ornamentTremblement:
        return u"inverted-mordent long=\"yes\"";
    case SymId::ornamentPrallMordent:
        return u"mordent long=\"yes\"";
    case SymId::ornamentUpPrall:
        return u"inverted-mordent long=\"yes\" approach=\"below\"";
    case SymId::ornamentPrecompMordentUpperPrefix:
        return u"inverted-mordent long=\"yes\" approach=\"above\"";
    case SymId::ornamentUpMordent:
        return u"mordent long=\"yes\" approach=\"below\"";
    case SymId::ornamentDownMordent:
        return u"mordent long=\"yes\" approach=\"above\"";
    case SymId::ornamentPrallDown:
        return u"inverted-mordent long=\"yes\" departure=\"below\"";
    case SymId::ornamentPrallUp:
        return u"inverted-mordent long=\"yes\" departure=\"above\"";
    case SymId::ornamentLinePrall:
        // MusicXML 3.0 did not distinguish between downprall and lineprall
        return u"inverted-mordent long=\"yes\" approach=\"above\"";
    case SymId::ornamentHaydn:
        return u"haydn";
    case SymId::ornamentPrecompSlide:
        return u"schleifer";
    default:
        // use other-ornament
        const AsciiStringView name = SymNames::nameForSymId(sid);
        return String(u"other-ornament smufl=\"%1\"").arg(String::fromAscii(name.ascii()));
    }
}

//---------------------------------------------------------
//   symIdToTechn
//---------------------------------------------------------

static String symIdToTechn(const SymId sid)
{
    switch (sid) {
    case SymId::stringsUpBow:
        return u"up-bow";
    case SymId::stringsDownBow:
        return u"down-bow";
    case SymId::stringsHarmonic:
        return u"harmonic";
    case SymId::stringsThumbPosition:
    case SymId::stringsThumbPositionTurned:
        return u"thumb-position";
    case SymId::doubleTongueAbove:
    case SymId::doubleTongueBelow:
        return u"double-tongue";
    case SymId::tripleTongueAbove:
    case SymId::tripleTongueBelow:
        return u"triple-tongue";
    case SymId::brassMuteClosed:
        return u"stopped";
    case SymId::pluckedSnapPizzicatoAbove:
    case SymId::pluckedSnapPizzicatoBelow:
        return u"snap-pizzicato";
    case SymId::guitarLeftHandTapping:
        return u"tap hand=\"left\"";
    case SymId::guitarRightHandTapping:
        return u"tap hand=\"right\"";
    case SymId::keyboardPedalHeel1:
    case SymId::keyboardPedalHeel2:
    case SymId::keyboardPedalHeel3:
        return u"heel";
    case SymId::keyboardPedalToe1:
    case SymId::keyboardPedalToe2:
        return u"toe";
    case SymId::pluckedWithFingernails:
        return u"fingernails";
    case SymId::brassBend:
        return u"brass-bend";
    case SymId::brassFlip:
        return u"brass-flip";
    case SymId::brassSmear:
        return u"smear";
    case SymId::brassMuteOpen:
        return u"open";
    case SymId::brassMuteHalfClosed:
        return u"half-muted";
    case SymId::brassHarmonMuteClosed:
    case SymId::brassHarmonMuteStemHalfLeft:
    case SymId::brassHarmonMuteStemHalfRight:
    case SymId::brassHarmonMuteStemOpen:
        return u"harmon-mute";
    case SymId::windClosedHole:
    case SymId::windHalfClosedHole1:
    case SymId::windHalfClosedHole2:
    case SymId::windHalfClosedHole3:
    case SymId::windOpenHole:
        return u"hole";
    case SymId::guitarGolpe:
        return u"golpe";
    case SymId::handbellsBelltree:
        return u"belltree";
    case SymId::handbellsDamp3:
        return u"damp";
    case SymId::handbellsEcho1:
        return u"echo";
    case SymId::handbellsGyro:
        return u"gyro";
    case SymId::handbellsHandMartellato:
        return u"hand martellato";
    case SymId::handbellsMalletLft:
        return u"mallet lift";
    case SymId::handbellsMalletBellOnTable:
        return u"mallet table";
    case SymId::handbellsMartellato:
        return u"martellato";
    case SymId::handbellsMartellatoLift:
        return u"martellato lift";
    case SymId::handbellsMutedMartellato:
        return u"muted martellato";
    case SymId::handbellsPluckLift:
        return u"pluck lift";
    case SymId::handbellsSwing:
        return u"swing";
    case SymId::guitarClosePedal:
    case SymId::pictOpenRimShot:
        return String(u"stopped smufl=\"%1\"").arg(String::fromAscii(SymNames::nameForSymId(sid).ascii()));
    case SymId::guitarHalfOpenPedal:
    case SymId::pictHalfOpen1:
    case SymId::pictHalfOpen2:
        return String(u"half-muted smufl=\"%1\"").arg(String::fromAscii(SymNames::nameForSymId(sid).ascii()));
    case SymId::guitarOpenPedal:
    case SymId::pictOpen:
        return String(u"open smufl=\"%1\"").arg(String::fromAscii(SymNames::nameForSymId(sid).ascii()));
    default:
        return String(); // nothing
    }
}

//---------------------------------------------------------
//   writeChordLines
//---------------------------------------------------------

static void writeChordLines(const Chord* const chord, XmlWriter& xml, Notations& notations, Articulations& articulations)
{
    for (EngravingItem* e : chord->el()) {
        LOGD("writeChordLines: el %p type %d (%s)", e, int(e->type()), e->typeName());
        if (e->type() == ElementType::CHORDLINE) {
            ChordLine const* const cl = static_cast<ChordLine*>(e);
            String subtype;
            switch (cl->chordLineType()) {
            case ChordLineType::FALL:
                subtype = u"falloff";
                break;
            case ChordLineType::DOIT:
                subtype = u"doit";
                break;
            case ChordLineType::PLOP:
                subtype = u"plop";
                break;
            case ChordLineType::SCOOP:
                subtype = u"scoop";
                break;
            default:
                LOGD("unknown ChordLine subtype %d", int(cl->chordLineType()));
            }
            subtype += color2xml(cl);
            if (!subtype.empty()) {
                notations.tag(xml, cl);
                articulations.tag(xml);
                xml.tagRaw(subtype);
            }
        }
    }
}

//---------------------------------------------------------
//   writeBreathMark
//---------------------------------------------------------

static void writeBreathMark(const Breath* const breath, XmlWriter& xml, Notations& notations, Articulations& articulations)
{
    if (breath && ExportMusicXml::canWrite(breath)) {
        String tagName;
        String type;

        notations.tag(xml, breath);
        articulations.tag(xml);
        if (breath->isCaesura()) {
            tagName = u"caesura";
            switch (breath->symId()) {
            case SymId::caesuraCurved:
                type = u"curved";
                break;
            case SymId::caesuraShort:
                type = u"short";
                break;
            case SymId::caesuraThick:
                type = u"thick";
                break;
            case SymId::caesuraSingleStroke:
            case SymId::chantCaesura:
                type = u"single";
                break;
            case SymId::caesura:
            default:
                ; //type = u"normal"; // is correct too, but see below
            }
        } else {
            tagName = u"breath-mark";
            switch (breath->symId()) {
            case SymId::breathMarkTick:
                type = u"tick";
                break;
            case SymId::breathMarkUpbow:
                type = u"upbow";
                break;
            case SymId::breathMarkSalzedo:
                type = u"salzedo";
                break;
            case SymId::breathMarkComma:
            default:
                type = u"comma";
            }
        }
        tagName += color2xml(breath);
        tagName += ExportMusicXml::positioningAttributes(breath);
        if (breath->placement() == PlacementV::BELOW) {
            tagName += u" placement=\"below\"";
        } else if (ExportMusicXml::configuration()->exportMu3Compat()) {
            // MuseScore versions prior to 4.3 otherwise default to below on import
            tagName += u" placement=\"above\"";
        }

        if (breath->isCaesura() && (type.isEmpty() || ExportMusicXml::configuration()->exportMu3Compat())) {
            // for backwards compatibility, as 3.6.2 and earlier can't import those special caesuras,
            // but reports corruption on all subsequent measures and imports them entirely empty.
            xml.tagRaw(tagName);
        } else {
            xml.tagRaw(tagName, type);
        }
    }
}

//---------------------------------------------------------
//   chordAttributes
//---------------------------------------------------------

void ExportMusicXml::chordAttributes(Chord* chord, Notations& notations, Technical& technical,
                                     TrillHash& trillStart, TrillHash& trillStop)
{
    if (!chord->isGrace()) {
        std::vector<EngravingItem*> fl;
        for (EngravingItem* e : chord->segment()->annotations()) {
            if (e->track() == chord->track() && e->isFermata()) {
                fl.push_back(e);
            }
        }
        fermatas(fl, m_xml, notations);
    }

    const std::vector<Articulation*> na = chord->articulations();
    // first the attributes whose elements are children of <articulations>
    Articulations articulations;
    for (const Articulation* a : na) {
        if (!ExportMusicXml::canWrite(a)) {
            continue;
        }

        SymId sid = a->symId();
        std::vector<String> mxmlArtics = symIdToArtic(sid);

        for (String mxmlArtic : mxmlArtics) {
            if (mxmlArtic == u"strong-accent") {
                if (a->up()) {
                    mxmlArtic += u" type=\"up\"";
                } else {
                    mxmlArtic += u" type=\"down\"";
                }
            } else if (a->anchor() != ArticulationAnchor::AUTO) {
                if (a->anchor() == ArticulationAnchor::TOP) {
                    mxmlArtic += u" placement=\"above\"";
                } else {
                    mxmlArtic += u" placement=\"below\"";
                }
            }
            mxmlArtic += color2xml(a);
            mxmlArtic += ExportMusicXml::positioningAttributes(a);

            notations.tag(m_xml, a);
            articulations.tag(m_xml);
            m_xml.tagRaw(mxmlArtic);
        }
    }

    writeBreathMark(chord->hasBreathMark(), m_xml, notations, articulations);
    writeChordLines(chord, m_xml, notations, articulations);

    articulations.etag(m_xml);

    // then the attributes whose elements are children of <ornaments>
    Ornaments ornaments;
    for (const Articulation* art : na) {
        if (!ExportMusicXml::canWrite(art)) {
            continue;
        }
        if (!art->isOrnament()) {
            continue;
        }
        const Ornament* ornam = toOrnament(art);
        const SymId sid = ornam->symId();
        String mxmlOrnam = symIdToOrnam(sid);

        String placement;
        if (!ornam->isStyled(Pid::ARTICULATION_ANCHOR) && ornam->anchor() != ArticulationAnchor::AUTO) {
            placement = (ornam->anchor() == ArticulationAnchor::BOTTOM ? u"below" : u"above");
        }
        if (!placement.empty()) {
            mxmlOrnam += String(u" placement=\"%1\"").arg(placement);
        }
        mxmlOrnam += color2xml(ornam);

        notations.tag(m_xml, ornam);
        ornaments.tag(m_xml);
        m_xml.tagRaw(mxmlOrnam);
        for (const Accidental* accidental : ornam->accidentalsAboveAndBelow()) {
            writeAccidental(m_xml, u"accidental-mark", accidental);
        }
    }
    tremoloSingleStartStop(chord, notations, ornaments, m_xml);
    wavyLineStartStop(chord, notations, ornaments, trillStart, trillStop);
    ornaments.etag(m_xml);

    // and finally the attributes whose elements are children of <technical>
    for (const Articulation* a : na) {
        if (!ExportMusicXml::canWrite(a)) {
            continue;
        }

        SymId sid = a->symId();
        String placement;
        String direction;

        if (!a->isStyled(Pid::ARTICULATION_ANCHOR) && a->anchor() != ArticulationAnchor::AUTO) {
            placement = (a->anchor() == ArticulationAnchor::BOTTOM) ? u"below" : u"above";
        } else if (!a->isStyled(Pid::DIRECTION) && a->direction() != DirectionV::AUTO) {
            direction = (a->direction() == DirectionV::DOWN) ? u"down" : u"up";
        }
        /* For future use if/when implemented font details for articulation
        if (!a->isStyled(Pid::FONT_FACE))
              attr += String(" font-family=\"%1\"").arg(a->getProperty(Pid::FONT_FACE).toString());
        if (!a->isStyled(Pid::FONT_SIZE))
              attr += String(" font-size=\"%1\"").arg(a->getProperty(Pid::FONT_SIZE).toReal());
        if (!a->isStyled(Pid::FONT_STYLE))
              attr += fontStyleToXML(static_cast<FontStyle>(a->getProperty(Pid::FONT_STYLE).toInt()), false);
        */

        String mxmlTechn = symIdToTechn(sid);
        if (!mxmlTechn.empty()) {
            notations.tag(m_xml, a);
            technical.tag(m_xml);
            mxmlTechn += color2xml(a);
            mxmlTechn += ExportMusicXml::positioningAttributes(a);
            if (!placement.empty()) {
                mxmlTechn += String(u" placement=\"%1\"").arg(placement);
            }
            if (sid == SymId::stringsHarmonic) {
                m_xml.startElementRaw(mxmlTechn);
                m_xml.tag("natural");
                m_xml.endElement();
            } else if (String::fromAscii(SymNames::nameForSymId(sid).ascii()).startsWith(u"handbells")) {
                String handbell = u"handbell";
                handbell += color2xml(a);
                handbell += ExportMusicXml::positioningAttributes(a);
                if (!placement.empty()) {
                    handbell += String(u" placement=\"%1\"").arg(placement);
                }
                m_xml.tagRaw(handbell, symIdToTechn(sid));
            } else if (mxmlTechn.startsWith(u"harmon")) {
                m_xml.startElementRaw(mxmlTechn);
                XmlWriter::Attributes location = {};
                String harmonClosedValue;
                switch (sid) {
                case SymId::brassHarmonMuteClosed:
                    harmonClosedValue = u"yes";
                    break;
                case SymId::brassHarmonMuteStemOpen:
                    harmonClosedValue = u"no";
                    break;
                default:
                    harmonClosedValue = u"half";
                    location = { { "location", (sid == SymId::brassHarmonMuteStemHalfLeft) ? "left" : "right" } };
                    break;
                }
                m_xml.tag("harmon-closed", location, harmonClosedValue);
                m_xml.endElement();
            } else if (mxmlTechn.startsWith(u"hole")) {
                m_xml.startElementRaw(mxmlTechn);
                XmlWriter::Attributes location = {};
                String holeClosedValue;
                switch (sid) {
                case SymId::windClosedHole:
                    holeClosedValue = u"yes";
                    break;
                case SymId::windOpenHole:
                    holeClosedValue = u"no";
                    break;
                default:
                    holeClosedValue = u"half";
                    location = { { "location", (sid == SymId::windHalfClosedHole1) ? "right" : "bottom" } };
                    break;
                }
                m_xml.tag("hole-closed", location, holeClosedValue);
                m_xml.endElement();
            } else {
                m_xml.tagRaw(mxmlTechn);
            }
        } else if (a->isTapping()) {
            const Tapping* tap = toTapping(a);
            notations.tag(m_xml, a);
            technical.tag(m_xml);
            mxmlTechn = u"tap";
            if (tap->hand() != TappingHand::INVALID) {
                mxmlTechn += String(u" hand=\"%1\"").arg(String::fromAscii(TConv::toXml(tap->hand()).ascii()));
            }
            mxmlTechn += color2xml(a);
            mxmlTechn += ExportMusicXml::positioningAttributes(a);
            if (!placement.empty()) {
                mxmlTechn += String(u" placement=\"%1\"").arg(placement);
            }
            m_xml.tagRaw(mxmlTechn);
        }
    }

    // check if all articulations were handled
    for (const Articulation* a : na) {
        if (!ExportMusicXml::canWrite(a)) {
            continue;
        }

        SymId sid = a->symId();
        if (symIdToArtic(sid).empty()
            && symIdToTechn(sid) == ""
            && !a->isOrnament() && !a->isTapping()
            && !isLaissezVibrer(sid)) {
            LOGD("unknown chord attribute %d %s", static_cast<int>(sid), muPrintable(a->translatedTypeUserName()));
        }
    }
}

//---------------------------------------------------------
//   arpeggiate
//---------------------------------------------------------

// <notations>
//   <arpeggiate direction="up"/>
//   </notations>

static void arpeggiate(Arpeggio* arp, bool front, bool back, XmlWriter& xml, Notations& notations, ArpeggioMap& arps,
                       bool spanArp = false)
{
    if (!ExportMusicXml::canWrite(arp)) {
        return;
    }
    bool found = false;
    int arpNo = 1;

    // Number arpeggios spanning multiple voices correctly
    const std::vector<MusicXmlArpeggioDesc> foundArps = muse::values(arps, arp->tick().ticks());
    for (const MusicXmlArpeggioDesc arpDesc : foundArps) {
        if (arpDesc.arp == arp) {
            found = true;
            arpNo = arpDesc.no;
        } else {
            arpNo = std::max(arpNo, arpDesc.no) + 1;
        }
    }

    if (!found && spanArp) {
        LOGD("span arpeggio without main arpeggio found at tick %d", arp->tick().ticks());
        return;
    }

    String tagName;
    switch (arp->arpeggioType()) {
    case ArpeggioType::NORMAL:
        notations.tag(xml, arp);
        tagName = u"arpeggiate number=\"" + String::number(arpNo) + u"\"";
        break;
    case ArpeggioType::UP:                  // fall through
    case ArpeggioType::UP_STRAIGHT:         // not supported by MusicXML, export as normal arpeggio
        notations.tag(xml, arp);
        tagName = u"arpeggiate direction=\"up\" number=\"" + String::number(arpNo) + u"\"";
        break;
    case ArpeggioType::DOWN:                  // fall through
    case ArpeggioType::DOWN_STRAIGHT:         // not supported by MusicXML, export as normal arpeggio
        notations.tag(xml, arp);
        tagName = u"arpeggiate direction=\"down\" number=\"" + String::number(arpNo) + u"\"";
        break;
    case ArpeggioType::BRACKET:
        if (front) {
            notations.tag(xml, arp);
            tagName = u"non-arpeggiate type=\"bottom\" number=\"" + String::number(arpNo) + u"\"";
        }
        if (back) {
            notations.tag(xml, arp);
            tagName = u"non-arpeggiate type=\"top\" number=\"" + String::number(arpNo) + u"\"";
        }
        break;
    default:
        LOGD("unknown arpeggio subtype %d", int(arp->arpeggioType()));
        break;
    }

    if (!tagName.empty()) {
        tagName += color2xml(arp);
        tagName += ExportMusicXml::positioningAttributes(arp);
        xml.tagRaw(tagName);
        if (!found) {
            MusicXmlArpeggioDesc arpDesc(arp, arpNo);
            arps.insert(std::pair<int, MusicXmlArpeggioDesc>(arp->tick().ticks(), arpDesc));
        }
    }
}

//---------------------------------------------------------
//   determineTupletNormalTicks
//---------------------------------------------------------

/**
 Determine the ticks in the normal type for the tuplet \a chord.
 This is non-zero only if chord if part of a tuplet containing
 different length duration elements.
 TODO determine how to handle baselen with dots and verify correct behaviour.
 TODO verify if baseLen should always be correctly set
      (it seems after MusicXMLimport this is not the case)
 */

static int determineTupletNormalTicks(Tuplet const* const t)
{
    if (!t) {
        return 0;
    }
    /*
    LOGD("determineTupletNormalTicks t %p baselen %s", t, muPrintable(t->baseLen().ticks().print()));
    for (int i = 0; i < t->elements().size(); ++i)
          LOGD("determineTupletNormalTicks t %p i %d ticks %s", t, i, muPrintable(t->elements().at(i)->ticks().print()));
          */
    for (unsigned int i = 1; i < t->elements().size(); ++i) {
        if (t->elements().at(0)->ticks() != t->elements().at(i)->ticks()) {
            return t->baseLen().ticks().ticks();
        }
    }
    if (t->elements().size() != (unsigned)(t->ratio().numerator())) {
        return t->baseLen().ticks().ticks();
    }
    return 0;
}

//---------------------------------------------------------
//   beamFanAttribute
//---------------------------------------------------------

static String beamFanAttribute(const Beam* const b)
{
    const double epsilon = 0.1;

    String fan;
    if ((b->growRight() - b->growLeft() > epsilon)) {
        fan = u"accel";
    }

    if ((b->growLeft() - b->growRight() > epsilon)) {
        fan = u"rit";
    }

    if (!fan.empty()) {
        return String(u" fan=\"%1\"").arg(fan);
    }

    return String();
}

//---------------------------------------------------------
//   writeBeam
//---------------------------------------------------------

//  beaming
//    <beam number="1">start</beam>
//    <beam number="1">end</beam>
//    <beam number="1">continue</beam>
//    <beam number="1">backward hook</beam>
//    <beam number="1">forward hook</beam>

static void writeBeam(XmlWriter& xml, ChordRest* const cr, Beam* const b)
{
    const auto& elements = b->elements();
    const size_t idx = muse::indexOf(elements, cr);
    if (idx == muse::nidx) {
        LOGD("writeBeam: cannot find ChordRest");
        return;
    }
    int blp = -1;   // beam level previous chord or rest
    int blc = -1;   // beam level current chord or rest
    int bln = -1;   // beam level next chord or rest
    BeamMode bmc = BeamMode::AUTO; // beam mode current chord or rest
    BeamMode bmn = BeamMode::AUTO; // beam mode next chord or rest
    // find beam level previous chord or rest
    for (size_t i = idx - 1; blp == -1 && i != muse::nidx; --i) {
        const ChordRest* crst = elements[i];
        blp = crst->beams();
    }
    // find beam level current chord or rest
    blc = cr->beams();
    bmc = cr->beamMode();
    // find beam level next chord or rest
    for (size_t i = idx + 1; bln == -1 && i < elements.size(); ++i) {
        const ChordRest* crst = elements[i];
        bln = crst->beams();
        bmn = crst->beamMode();
    }
    // find beam type and write
    for (int i = 1; i <= blc; ++i) {
        String text;
        // TODO: correctly handle Beam::Mode::AUTO
        // when equivalent to BEGIN32 or BEGIN64
        if ((blp < i && bln >= i)
            || (bmc == BeamMode::BEGIN16 && i > 1)
            || (bmc == BeamMode::BEGIN32 && i > 2)) {
            text = u"begin";
        } else if (blp < i && bln < i) {
            if (bln > 0) {
                text = u"forward hook";
            } else if (blp > 0) {
                text = u"backward hook";
            }
        } else if ((blp >= i && bln < i)
                   || (bmn == BeamMode::BEGIN16 && i > 1)
                   || (bmn == BeamMode::BEGIN32 && i > 2)) {
            text = u"end";
        } else if (blp >= i && bln >= i) {
            text = u"continue";
        }
        if (!text.empty()) {
            String tag = u"beam";
            tag += String(u" number=\"%1\"").arg(i);
            if (text == u"begin") {
                tag += beamFanAttribute(b);
                tag += color2xml(b);
            }
            xml.tagRaw(tag, text);
        }
    }
}

//---------------------------------------------------------
//   instrId
//---------------------------------------------------------

static String instrId(int partNr, int instrNr)
{
    return String(u"id=\"P%1-I%2\"").arg(partNr).arg(instrNr);
}

//---------------------------------------------------------
//   isNoteheadParenthesis
//---------------------------------------------------------

static bool isNoteheadParenthesis(const Symbol* symbol)
{
    const SymId sym = symbol->sym();
    return sym == SymId::noteheadParenthesisLeft || sym == SymId::noteheadParenthesisRight;
}

//---------------------------------------------------------
//   writeNotehead
//---------------------------------------------------------

static void writeNotehead(XmlWriter& xml, const Note* const note)
{
    String noteheadTagname = u"notehead";
    noteheadTagname += color2xml(note);
    bool leftParenthesis = false, rightParenthesis = false;
    for (EngravingItem* elem : note->el()) {
        if (elem->type() == ElementType::SYMBOL) {
            Symbol* s = static_cast<Symbol*>(elem);
            if (s->sym() == SymId::noteheadParenthesisLeft) {
                leftParenthesis = true;
            } else if (s->sym() == SymId::noteheadParenthesisRight) {
                rightParenthesis = true;
            }
        }
    }
    if (rightParenthesis && leftParenthesis) {
        noteheadTagname += u" parentheses=\"yes\"";
    }
    if (note->headType() == NoteHeadType::HEAD_QUARTER) {
        noteheadTagname += u" filled=\"yes\"";
    } else if ((note->headType() == NoteHeadType::HEAD_HALF) || (note->headType() == NoteHeadType::HEAD_WHOLE)) {
        noteheadTagname += u" filled=\"no\"";
    }
    if (!note->visible()) {
        // The notehead is invisible but other parts of the note might
        // still be visible so don't export <note print-object="no">.
        xml.tagRaw(noteheadTagname, "none");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_SLASH) {
        xml.tagRaw(noteheadTagname, "slash");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_TRIANGLE_UP) {
        xml.tagRaw(noteheadTagname, "triangle");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_DIAMOND) {
        xml.tagRaw(noteheadTagname, "diamond");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_PLUS) {
        xml.tagRaw(noteheadTagname, "cross");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_CROSS) {
        xml.tagRaw(noteheadTagname, "x");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_CIRCLED) {
        xml.tagRaw(noteheadTagname, "circled");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_XCIRCLE) {
        xml.tagRaw(noteheadTagname, "circle-x");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_TRIANGLE_DOWN) {
        xml.tagRaw(noteheadTagname, "inverted triangle");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_SLASHED1) {
        xml.tagRaw(noteheadTagname, "slashed");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_SLASHED2) {
        xml.tagRaw(noteheadTagname, "back slashed");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_DO) {
        xml.tagRaw(noteheadTagname, "do");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_RE) {
        xml.tagRaw(noteheadTagname, "re");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_MI) {
        xml.tagRaw(noteheadTagname, "mi");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_FA && !note->chord()->up()) {
        xml.tagRaw(noteheadTagname, "fa");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_FA && note->chord()->up()) {
        xml.tagRaw(noteheadTagname, "fa up");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_LA) {
        xml.tagRaw(noteheadTagname, "la");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_TI) {
        xml.tagRaw(noteheadTagname, "ti");
    } else if (note->headGroup() == NoteHeadGroup::HEAD_SOL) {
        xml.tagRaw(noteheadTagname, "so");
    } else if (note->color() != engravingConfiguration()->defaultColor()) {
        xml.tagRaw(noteheadTagname, "normal");
    } else if (rightParenthesis && leftParenthesis) {
        xml.tagRaw(noteheadTagname, "normal");
    } else if (note->headType() != NoteHeadType::HEAD_AUTO) {
        xml.tagRaw(noteheadTagname, "normal");
    } else if (note->headGroup() != NoteHeadGroup::HEAD_NORMAL) {
        AsciiStringView noteheadName = SymNames::nameForSymId(note->noteHead());
        noteheadTagname += String(u" smufl=\"%1\"").arg(String::fromAscii(noteheadName.ascii()));
        xml.tagRaw(noteheadTagname, "other");
    }

    if (note->headScheme() == NoteHeadScheme::HEAD_PITCHNAME
        || note->headScheme() == NoteHeadScheme::HEAD_PITCHNAME_GERMAN
        || note->headScheme() == NoteHeadScheme::HEAD_SOLFEGE
        || note->headScheme() == NoteHeadScheme::HEAD_SOLFEGE_FIXED) {
        static const std::regex nameparts("^note([A-Z][a-z]*)(Sharp|Flat)?");
        AsciiStringView noteheadName = SymNames::nameForSymId(note->noteHead());
        StringList matches = String::fromAscii(noteheadName.ascii()).search(nameparts, { 1, 2 }, SplitBehavior::SkipEmptyParts);
        xml.startElement("notehead-text");
        xml.tag("display-text", matches.at(0));
        if (matches.size() > 1) {
            xml.tag("accidental-text", matches.at(1).toLower());
        }
        xml.endElement();
    }
}

//---------------------------------------------------------
//   writeGuitarBend
//---------------------------------------------------------

static void writeGuitarBend(XmlWriter& xml, Notations& notations, Technical& technical, const Note* const note)
{
    if (note->bendBack()) {
        const GuitarBend* bend = note->bendBack();
        if (bend->type() == GuitarBendType::PRE_BEND || bend->type() == GuitarBendType::GRACE_NOTE_BEND) {
            XmlWriter::Attributes bendAttrs;
            notations.tag(xml, note);
            technical.tag(xml);
            bendAttrs.push_back({ "first-beat", bend->startTimeFactor() * 100 });
            bendAttrs.push_back({ "last-beat", bend->endTimeFactor() * 100 });
            addColorAttr(bend, bendAttrs);
            xml.startElement("bend", bendAttrs);
            xml.tag("bend-alter", String::number(-0.5 * bend->bendAmountInQuarterTones(), 2));
            xml.tag("pre-bend");
            xml.endElement();
        }
    }
    if (note->bendFor()) {
        const GuitarBend* bend = note->bendFor();
        if (bend->type() == GuitarBendType::BEND || bend->type() == GuitarBendType::SLIGHT_BEND) {
            notations.tag(xml, note);
            technical.tag(xml);
            XmlWriter::Attributes bendAttrs;
            bendAttrs.push_back({ "first-beat", bend->startTimeFactor() * 100 });
            bendAttrs.push_back({ "last-beat", bend->endTimeFactor() * 100 });
            addColorAttr(bend, bendAttrs);
            xml.startElement("bend", bendAttrs);
            xml.tag("bend-alter", String::number(0.5 * bend->bendAmountInQuarterTones(), 2));
            xml.endElement();
        }
    }
}

//---------------------------------------------------------
//   writeFingering
//---------------------------------------------------------

static void writeFingering(XmlWriter& xml, Notations& notations, Technical& technical, const Note* const note)
{
    for (const EngravingItem* e : note->el()) {
        if (!ExportMusicXml::canWrite(e)) {
            continue;
        }

        if (e->type() == ElementType::FINGERING) {
            const TextBase* f = toTextBase(e);
            notations.tag(xml, e);
            technical.tag(xml);
            String t = MScoreTextToMusicXml::toPlainText(f->xmlText());
            String attr;
            if (!f->isStyled(Pid::PLACEMENT) || f->placement() == PlacementV::BELOW) {
                attr = String(u" placement=\"%1\"").arg((f->placement() == PlacementV::BELOW) ? u"below" : u"above");
            }
            if (!f->isStyled(Pid::FONT_FACE)) {
                attr += String(u" font-family=\"%1\"").arg(f->getProperty(Pid::FONT_FACE).value<String>());
            }
            if (!f->isStyled(Pid::FONT_SIZE)) {
                attr += String(u" font-size=\"%1\"").arg(f->getProperty(Pid::FONT_SIZE).toReal());
            }
            if (!f->isStyled(Pid::FONT_STYLE)) {
                attr += fontStyleToXML(static_cast<FontStyle>(f->getProperty(Pid::FONT_STYLE).toInt()), false);
            }
            attr += color2xml(f);
            attr += ExportMusicXml::positioningAttributes(f);

            if (f->textStyleType() == TextStyleType::RH_GUITAR_FINGERING) {
                xml.tagRaw(u"pluck" + attr, t);
            } else if (f->textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
                xml.tagRaw(u"fingering" + attr, t);
            } else if (f->textStyleType() == TextStyleType::FINGERING) {
                // for generic fingering, try to detect plucking
                // (backwards compatibility with MuseScore 1.x)
                // p, i, m, a, c represent the plucking finger
                if (t == "p" || t == "i" || t == "m" || t == "a" || t == "c") {
                    xml.tagRaw(u"pluck" + attr, t);
                } else {
                    xml.tagRaw(u"fingering" + attr, t);
                }
            } else if (f->textStyleType() == TextStyleType::STRING_NUMBER) {
                bool ok;
                int i = t.toInt(&ok);
                if (ok) {
                    if (i == 0) {
                        xml.tagRaw(u"open-string" + attr);
                    } else if (i > 0) {
                        xml.tagRaw(u"string" + attr, t);
                    }
                }
                if (!ok || i < 0) {
                    LOGD("invalid string number '%s'", muPrintable(t));
                }
            } else {
                LOGD("unknown fingering style");
            }
        } else {
            // TODO
        }
    }
}

//---------------------------------------------------------
//   writeNotationSymbols
//---------------------------------------------------------

static void writeNotationSymbols(XmlWriter& xml, Notations& notations, const ElementList& elist, bool excludeParentheses)
{
    for (const EngravingItem* e : elist) {
        if (!e->isSymbol() || !ExportMusicXml::canWrite(e)) {
            continue;
        }

        const Symbol* symbol = toSymbol(e);

        if (excludeParentheses && isNoteheadParenthesis(symbol)) {
            // Already handled when writing notehead properties.
            continue;
        }

        notations.tag(xml, symbol);
        xml.tag("other-notation", { { "type", "single" }, { "smufl", symbol->symName() } });
    }
}

//---------------------------------------------------------
//   stretchCorrActFraction
//---------------------------------------------------------

static Fraction stretchCorrActFraction(const Note* const note)
{
    // time signature stretch factor
    const Fraction str = note->chord()->staff()->timeStretch(note->chord()->tick());
    // chord's actual ticks corrected for stretch
    return note->chord()->actualTicks() * str;
}

//---------------------------------------------------------
//   tremoloCorrection
//---------------------------------------------------------

// duration correction for two note tremolo
static int tremoloCorrection(const Note* const note)
{
    int tremCorr = 1;
    if (isTwoNoteTremolo(note->chord())) {
        tremCorr = 2;
    }
    return tremCorr;
}

//---------------------------------------------------------
//   isSmallNote
//---------------------------------------------------------

static bool isSmallNote(const Note* const note)
{
    return note->isSmall() || note->chord()->isSmall();
}

//---------------------------------------------------------
//   isCueNote
//---------------------------------------------------------

static bool isCueNote(const Note* const note)
{
    return isSmallNote(note) && !note->play();
}

//---------------------------------------------------------
//   timeModification
//---------------------------------------------------------

static Fraction timeModification(const Tuplet* const tuplet, const int tremolo = 1)
{
    int actNotes { tremolo };
    int nrmNotes { 1 };
    const Tuplet* t { tuplet };

    while (t) {
        // cannot use Fraction::operator*() as it contains a reduce(),
        // which would change a 6:4 tuplet into 3:2
        actNotes *= t->ratio().numerator();
        nrmNotes *= t->ratio().denominator();
        t = t->tuplet();
    }

    return { actNotes, nrmNotes };
}

//---------------------------------------------------------
//   writeType
//---------------------------------------------------------

static void writeType(XmlWriter& xml, const Note* const note)
{
    int dots = 0;
    const Fraction ratio = timeModification(note->chord()->tuplet());

    const Fraction strActFraction = stretchCorrActFraction(note);
    const Fraction tt  = strActFraction * ratio * tremoloCorrection(note);
    const String s = tick2xml(tt, &dots);
    if (s.isEmpty()) {
        LOGD("no note type found for fraction %d / %d", strActFraction.numerator(), strActFraction.denominator());
    }

    // small notes are indicated by size=cue, but for grace and cue notes this is implicit
    if (isSmallNote(note) && !isCueNote(note) && !note->chord()->isGrace()) {
        xml.tag("type", { { "size", "cue" } }, s);
    } else if (isSmallNote(note) && !isCueNote(note) && note->chord()->isGrace()) {
        xml.tag("type", { { "size", "grace-cue" } }, s);
    } else {
        xml.tag("type", s);
    }

    if (note->dots().empty()) {
        for (int ni = dots; ni > 0; --ni) {
            xml.tag("dot");
        }
    }
}

//---------------------------------------------------------
//   writeTimeModification
//---------------------------------------------------------

static void writeTimeModification(XmlWriter& xml, const Tuplet* const tuplet, const int tremolo = 1)
{
    const Fraction ratio = timeModification(tuplet, tremolo);
    if (ratio != Fraction(1, 1)) {
        const int actNotes = ratio.numerator();
        const int nrmNotes = ratio.denominator();

        const int nrmTicks = determineTupletNormalTicks(tuplet);
        xml.startElement("time-modification");
        xml.tag("actual-notes", actNotes);
        xml.tag("normal-notes", nrmNotes);
        //LOGD("nrmTicks %d", nrmTicks);
        if (nrmTicks > 0) {
            int nrmDots { 0 };
            const String nrmType = tick2xml(Fraction::fromTicks(nrmTicks), &nrmDots);
            if (nrmType.isEmpty()) {
                LOGD("no note type found for ticks %d", nrmTicks);
            } else {
                xml.tag("normal-type", nrmType);
                for (int ni = nrmDots; ni > 0; ni--) {
                    xml.tag("normal-dot");
                }
            }
        }
        xml.endElement();
    }
}

//---------------------------------------------------------
//   writePitch
//---------------------------------------------------------

static void writePitch(XmlWriter& xml, const Note* const note, const bool useDrumset)
{
    // step / alter / octave
    String step;
    int alter = 0;
    int octave = 0;
    if (!useDrumset) {
        pitch2xml(note, step, alter, octave);
    } else {
        unpitch2xml(note, step, octave);
    }
    xml.startElement(useDrumset ? "unpitched" : "pitch");
    xml.tag(useDrumset ? "display-step" : "step", step);
    // Check for microtonal accidentals and overwrite "alter" tag
    const Accidental* acc = note->accidental();
    double microtonalAlter = 0.0;
    if (acc) {
        switch (acc->accidentalType()) {
        case AccidentalType::MIRRORED_FLAT:  microtonalAlter = -0.5;
            break;
        case AccidentalType::SHARP_SLASH:    microtonalAlter = 0.5;
            break;
        case AccidentalType::MIRRORED_FLAT2: microtonalAlter = -1.5;
            break;
        case AccidentalType::SHARP_SLASH4:   microtonalAlter = 1.5;
            break;
        default:                                             break;
        }
    }
    // Override accidental with explicit note tuning
    double tuning = note->tuning();
    if (!muse::RealIsNull(tuning)) {
        microtonalAlter = tuning / 100.0;
    }
    if (alter || microtonalAlter) {
        xml.tag("alter", alter + microtonalAlter);
    }
    xml.tag(useDrumset ? "display-octave" : "octave", octave);
    xml.endElement();
}

//---------------------------------------------------------
//   elementPosition
//---------------------------------------------------------

String ExportMusicXml::elementPosition(const ExportMusicXml* const expMxml, const EngravingItem* const elm)
{
    String res;

    if (configuration()->exportLayout()) {
        const double pageHeight  = expMxml->getTenthsFromInches(expMxml->score()->style().styleD(Sid::pageHeight));

        const Measure* meas = elm->findMeasure();
        IF_ASSERT_FAILED(meas) {
            return res;
        }

        double measureX = expMxml->getTenthsFromDots(meas->pagePos().x());
        double measureY = pageHeight - expMxml->getTenthsFromDots(meas->pagePos().y());
        double noteX = expMxml->getTenthsFromDots(elm->pagePos().x());
        double noteY = pageHeight - expMxml->getTenthsFromDots(elm->pagePos().y());

        res += String(u" default-x=\"%1\"").arg(String::number(noteX - measureX, 2));
        res += String(u" default-y=\"%1\"").arg(String::number(noteY - measureY, 2));
    }

    return res;
}

//---------------------------------------------------------
//   chord
//---------------------------------------------------------

/**
 Write \a chord on \a staff with lyriclist \a ll.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::chord(Chord* chord, staff_idx_t staff, const std::vector<Lyrics*>& ll, bool useDrumset)
{
    Part* part = chord->score()->staff(chord->track() / VOICES)->part();
    size_t partNr = muse::indexOf(m_score->parts(), part);
    int instNr = muse::value(m_instrMap, part->instrument(m_tick), -1);
    /*
    LOGD("chord() %p parent %p isgrace %d #gracenotes %d graceidx %d",
           chord, chord->parent(), chord->isGrace(), chord->graceNotes().size(), chord->graceIndex());
    LOGD("track %d tick %d part %p nr %d instr %p nr %d",
           chord->track(), chord->tick(), part, partNr, part->instrument(tick), instNr);
    for (EngravingItem* e : chord->el())
          LOGD("chord %p el %p", chord, e);
     */
    std::vector<Note*> nl = chord->notes();
    bool grace = chord->isGrace();
#ifdef DEBUG_TICK
    LOGD() << "oldtick " << fractionToStdString(tick())
           << " grace " << grace;
#endif
    if (!grace) {
        m_tick += chord->actualTicks();
    }
#ifdef DEBUG_TICK
    LOGD() << "newtick " << fractionToStdString(tick());
#endif

    for (Note* note : nl) {
        m_attr.doAttr(m_xml, false);
        String noteTag = u"note";

        noteTag += elementPosition(this, note);

        int velo = note->userVelocity();
        if (velo != 0) {
            noteTag += String(u" dynamics=\"%1\"").arg(String::number(velo * 100.0 / 90.0, 2));
        }

        m_xml.startElementRaw(noteTag);

        if (grace) {
            if (note->chord()->showStemSlash()) {
                m_xml.tag("grace", { { "slash", "yes" } });
            } else {
                m_xml.tag("grace");
            }
        }
        if (isCueNote(note)) {
            m_xml.tag("cue");
        }
        if (note != nl.front()) {
            m_xml.tag("chord");
        }

        writePitch(m_xml, note, useDrumset);

        // duration
        if (!grace) {
            m_xml.tag("duration", calculateDurationInDivisions(stretchCorrActFraction(note), m_div));
        }

        if (!isCueNote(note)) {
            if (note->tieBackNonPartial()) {
                m_xml.tag("tie", { { "type", "stop" } });
            }
            if (note->tieForNonPartial()) {
                m_xml.tag("tie", { { "type", "start" } });
            }
        }

        // instrument for multi-instrument or unpitched parts
        if (!useDrumset) {
            if (m_instrMap.size() > 1 && instNr >= 0) {
                m_xml.tagRaw(String(u"instrument %1").arg(instrId(static_cast<int>(partNr) + 1, instNr + 1)));
            }
        } else {
            m_xml.tagRaw(String(u"instrument %1").arg(instrId(static_cast<int>(partNr) + 1, note->pitch() + 1)));
        }

        // voice
        // for a single-staff part, staff is 0, which needs to be corrected
        // to calculate the correct voice number
        voice_idx_t voice = (static_cast<int>(staff) - 1) * VOICES + note->chord()->voice() + 1;
        if (staff == 0) {
            voice += VOICES;
        }

        m_xml.tag("voice", static_cast<int>(voice));

        writeType(m_xml, note);
        for (const NoteDot* dot : note->dots()) {
            String dotTag = u"dot";
            if (note->userDotPosition() != engraving::DirectionV::AUTO) {
                if (note->dotPosition() == engraving::DirectionV::UP) {
                    dotTag += u" placement=\"above\"";
                } else {
                    dotTag += u" placement=\"below\"";
                }
            }
            dotTag += color2xml(dot);
            dotTag += elementPosition(this, dot);
            m_xml.tagRaw(dotTag);
        }
        writeAccidental(m_xml, u"accidental", note->accidental());
        writeTimeModification(m_xml, note->chord()->tuplet(), tremoloCorrection(note));

        // no stem for whole notes and beyond
        if (chord->noStem() || chord->measure()->stemless(chord->staffIdx()) || (chord->stem() && !chord->stem()->visible())) {
            m_xml.tag("stem", "none");
        } else if (const Stem* stem = note->chord()->stem()) {
            String stemTag = u"stem";
            stemTag += color2xml(stem);
            m_xml.tagRaw(stemTag, note->chord()->up() ? "up" : "down");
        }

        writeNotehead(m_xml, note);

        // LVIFIX: check move() handling
        if (staff) {
            m_xml.tag("staff", static_cast<int>(staff + note->chord()->staffMove()));
        }

        if (note == nl.front() && chord->beam()) {
            writeBeam(m_xml, chord, chord->beam());
        }

        Notations notations;
        Technical technical;

        const Tie* tieBack = note->tieBack();
        if (tieBack && ExportMusicXml::canWrite(tieBack)) {
            notations.tag(m_xml, tieBack);
            m_xml.tag("tied", { { "type", "stop" } });
        }

        const LaissezVib* laissezVib = note->laissezVib();
        if (laissezVib && ExportMusicXml::canWrite(laissezVib)) {
            notations.tag(m_xml, laissezVib);
            String rest = slurTieLineStyle(laissezVib);
            m_xml.tagRaw(String(u"tied type=\"let-ring\"%1").arg(rest));
        }

        const Tie* tieFor = note->tieFor();
        if (tieFor && !laissezVib && ExportMusicXml::canWrite(tieFor)) {
            notations.tag(m_xml, tieFor);
            String rest = slurTieLineStyle(tieFor);
            m_xml.tagRaw(String(u"tied type=\"start\"%1").arg(rest));
        }

        if (note == nl.front()) {
            if (!grace) {
                tupletStartStop(chord, notations, m_xml);
            }

            m_sh.doSlurs(chord, notations, m_xml);

            chordAttributes(chord, notations, technical, m_trillStart, m_trillStop);
        }

        writeFingering(m_xml, notations, technical, note);
        writeNotationSymbols(m_xml, notations, note->el(), true);

        // write tablature string / fret
        if (chord->staff() && chord->staff()->isTabStaff(Fraction(0, 1))) {
            if (note->fret() >= 0 && note->string() >= 0) {
                notations.tag(m_xml, note);
                technical.tag(m_xml);
                m_xml.tag("string", note->string() + 1);
                m_xml.tag("fret", note->fret());
            }
        }

        writeGuitarBend(m_xml, notations, technical, note);

        technical.etag(m_xml);
        if (chord->arpeggio()) {
            arpeggiate(chord->arpeggio(), note == nl.front(), note == nl.back(), m_xml, notations, m_measArpeggios);
        } else if (chord->spanArpeggio()) {
            arpeggiate(chord->spanArpeggio(), note == nl.front(), note == nl.back(), m_xml, notations, m_measArpeggios, /*spanArp=*/ true);
        }
        for (Spanner* spanner : note->spannerFor()) {
            if (spanner->type() == ElementType::GLISSANDO && ExportMusicXml::canWrite(spanner)) {
                m_gh.doGlissandoStart(static_cast<Glissando*>(spanner), notations, m_xml);
            }
        }
        for (Spanner* spanner : note->spannerBack()) {
            if (spanner->type() == ElementType::GLISSANDO && ExportMusicXml::canWrite(spanner)) {
                m_gh.doGlissandoStop(static_cast<Glissando*>(spanner), notations, m_xml);
            }
        }
        // write glissando (only for last note)
        /*
        Chord* ch = nextChord(chord);
        if ((note == nl.back()) && ch && ch->glissando()) {
              gh.doGlissandoStart(ch, notations, xml);
              }
        if (chord->glissando()) {
              gh.doGlissandoStop(chord, notations, xml);
              }
        */
        notations.etag(m_xml);
        // write lyrics (only for first note)
        if (!grace && (note == nl.front())) {
            lyrics(ll, chord->track());
        }
        m_xml.endElement();
    }
}

//---------------------------------------------------------
//   rest
//---------------------------------------------------------

/**
 Write \a rest on \a staff.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::rest(Rest* rest, staff_idx_t staff, const std::vector<Lyrics*>& ll)
{
    static char16_t table2[]  = u"CDEFGAB";
#ifdef DEBUG_TICK
    LOGD() << "oldtick " << fractionToStdString(tick());
#endif
    m_attr.doAttr(m_xml, false);

    String noteTag = u"note";
    noteTag += color2xml(rest);
    noteTag += elementPosition(this, rest);
    if (!rest->visible()) {
        noteTag += u" print-object=\"no\"";
    }
    m_xml.startElementRaw(noteTag);

    int yOffsSt   = 0;
    int oct       = 0;
    int stp       = 0;
    ClefType clef = rest->staff()->clef(rest->tick());
    int po        = ClefInfo::pitchOffset(clef);

    // Determine y position, but leave at zero in case of tablature staff
    // as no display-step or display-octave should be written for a tablature staff,

    if (clef != ClefType::TAB && clef != ClefType::TAB_SERIF && clef != ClefType::TAB4 && clef != ClefType::TAB4_SERIF) {
        double yOffsSp = -2 * rest->offset().y() / rest->spatium();              // positive = up, one spatium is two pitches
        yOffsSt = int(yOffsSp > 0.0 ? yOffsSp + 0.5 : yOffsSp - 0.5);            // same rounded to int

        po -= 4;        // pitch middle staff line (two lines times two steps lower than top line)
        po += yOffsSt;  // rest "pitch"
        po -= 7;        // correct octave
        // at this point:
        // C0 (lowest value allowed in MusicXML) has po == 0   (y offset = 17 (treble clef) 11 (bass clef))
        // B9 (highest value allowed in MusicXML) has po == 69 (y offset = -17.5 (treble clef) -23.5 (bass clef))

        po = std::clamp(po, 0, 69);
        oct = po / 7;   // octave (MusicXML spec: must be 0..9, C4 is middle C)
        stp = po % 7;   // step (must be 0..6, as display-step must be A..G)
        LOGN()
            << " pitchOffset (initial po) " << ClefInfo::pitchOffset(clef)
            << " offset().y() " << rest->offset().y()
            << " spatium() " << rest->spatium()
            << " yOffsSp " << yOffsSp
            << " yOffsSt " << yOffsSt
            << " po " << po
            << " oct " << oct
            << " stp " << stp
            << " (" << char(table2[stp]) << oct << ")";
    }

    String restTag = u"rest";
    const TDuration d = rest->durationType();
    if (d.type() == DurationType::V_MEASURE) {
        restTag += u" measure=\"yes\"";
    }
    // Either <rest/>
    // or <rest><display-step>F</display-step><display-octave>5</display-octave></rest>
    if (yOffsSt == 0) {
        m_xml.tagRaw(restTag);
    } else {
        m_xml.startElementRaw(restTag);
        m_xml.tag("display-step", String(Char(table2[stp])));
        m_xml.tag("display-octave", oct);
        m_xml.endElement();
    }

    Fraction tickLen = rest->actualTicks();
    if (d.type() == DurationType::V_MEASURE) {
        // to avoid forward since rest->ticklen=0 in this case.
        tickLen = rest->measure()->ticks();
    }
    m_tick += tickLen;
#ifdef DEBUG_TICK
    LOGD() << "tickLen " << fractionToStdString(tickLen)
           << "newtick " << fractionToStdString(tick());
#endif

    m_xml.tag("duration", calculateDurationInDivisions(tickLen, m_div));

    // for a single-staff part, staff is 0, which needs to be corrected
    // to calculate the correct voice number
    voice_idx_t voice = (static_cast<int>(staff) - 1) * VOICES + rest->voice() + 1;
    if (staff == 0) {
        voice += VOICES;
    }
    m_xml.tag("voice", static_cast<int>(voice));

    // do not output a "type" element for whole measure rest
    if (d.type() != DurationType::V_MEASURE) {
        AsciiStringView s = TConv::toXml(d.type());
        if (rest->isSmall()) {
            m_xml.tag("type", { { "size", "cue" } }, s);
        } else {
            m_xml.tag("type", s);
        }
        for (NoteDot* dot : rest->dotList()) {
            String dotTag = u"dot";
            dotTag += color2xml(dot);
            dotTag += elementPosition(this, dot);
            m_xml.tagRaw(dotTag);
        }
    }

    writeTimeModification(m_xml, rest->tuplet());

    if (staff) {
        m_xml.tag("staff", static_cast<int>(staff));
    }

    if (rest->beam()) {
        writeBeam(m_xml, rest, rest->beam());
    }

    Notations notations;
    std::vector<EngravingItem*> fl;
    for (EngravingItem* e : rest->segment()->annotations()) {
        if (e->isFermata() && e->track() == rest->track()) {
            fl.push_back(e);
        }
    }
    fermatas(fl, m_xml, notations);

    Articulations articulations;
    writeBreathMark(rest->hasBreathMark(), m_xml, notations, articulations);
    articulations.etag(m_xml);

    Ornaments ornaments;
    wavyLineStartStop(rest, notations, ornaments, m_trillStart, m_trillStop);
    ornaments.etag(m_xml);

    writeNotationSymbols(m_xml, notations, rest->el(), false);

    m_sh.doSlurs(rest, notations, m_xml);

    tupletStartStop(rest, notations, m_xml);
    notations.etag(m_xml);

    lyrics(ll, rest->track());

    m_xml.endElement();
}

//---------------------------------------------------------
//   directionTag
//---------------------------------------------------------

static void directionTag(XmlWriter& xml, Attributes& attr, EngravingItem const* const el = 0)
{
    attr.doAttr(xml, false);
    String tagName = u"direction";
    if (el) {
        /*
         LOGD("directionTag() spatium=%g elem=%p tp=%d (%s)\ndirectionTag()  x=%g y=%g xsp,ysp=%g,%g w=%g h=%g userOff.y=%g",
                el->spatium(),
                el,
                el->type(),
                el->typeName(),
                el->x(), el->y(),
                el->x()/el->spatium(), el->y()/el->spatium(),
                el->width(), el->height(),
                el->offset().y()
               );
         */
        const EngravingItem* pel = 0;
        const LineSegment* seg = 0;
        if (el->type() == ElementType::HAIRPIN || el->type() == ElementType::OTTAVA
            || el->type() == ElementType::PEDAL || el->type() == ElementType::TEXTLINE
            || el->type() == ElementType::LET_RING || el->type() == ElementType::PALM_MUTE
            || el->type() == ElementType::WHAMMY_BAR || el->type() == ElementType::RASGUEADO
            || el->type() == ElementType::HARMONIC_MARK || el->type() == ElementType::PICK_SCRAPE
            || el->type() == ElementType::GRADUAL_TEMPO_CHANGE) {
            // handle elements derived from SLine
            // find the system containing the first linesegment
            const SLine* sl = static_cast<const SLine*>(el);
            if (!sl->segmentsEmpty()) {
                seg = toLineSegment(sl->frontSegment());
                /*
                 LOGD("directionTag()  seg=%p x=%g y=%g w=%g h=%g cpx=%g cpy=%g userOff.y=%g",
                        seg, seg->x(), seg->y(),
                        seg->width(), seg->height(),
                        seg->pagePos().x(), seg->pagePos().y(),
                        seg->offset().y());
                 */
                pel = seg->parentItem();
            }
        } else if (el->type() == ElementType::CAPO
                   || el->type() == ElementType::DYNAMIC
                   || el->type() == ElementType::HARP_DIAGRAM
                   || el->type() == ElementType::INSTRUMENT_CHANGE
                   || el->type() == ElementType::PLAYTECH_ANNOTATION
                   || el->type() == ElementType::REHEARSAL_MARK
                   || el->type() == ElementType::STAFF_TEXT
                   || el->type() == ElementType::STRING_TUNINGS
                   || el->type() == ElementType::SYMBOL
                   || el->type() == ElementType::TEXT
                   || el->type() == ElementType::SYSTEM_TEXT) {
            // handle other elements attached (e.g. via Segment / Measure) to a system
            // find the system containing this element
            for (const EngravingItem* e = el; e; e = e->parentItem()) {
                if (e->type() == ElementType::SYSTEM) {
                    pel = e;
                }
            }
        } else {
            LOGD("directionTag() element %p tp=%d (%s) not supported",
                 el, int(el->type()), el->typeName());
        }

        /*
         if (pel) {
         LOGD("directionTag()  prnt tp=%d (%s) x=%g y=%g w=%g h=%g userOff.y=%g",
                pel->type(),
                pel->typeName(),
                pel->x(), pel->y(),
                pel->width(), pel->height(),
                pel->offset().y());
              }
         */

        if (pel && pel->type() == ElementType::SYSTEM) {
            /*
            const System* sys = static_cast<const System*>(pel);
            RectF bb = sys->staff(el->staffIdx())->ldata()->bbox;
            LOGD("directionTag()  syst=%p sys x=%g y=%g cpx=%g cpy=%g",
                   sys, sys->pos().x(),  sys->pos().y(),
                   sys->pagePos().x(),
                   sys->pagePos().y()
                  );
            LOGD("directionTag()  staff x=%g y=%g w=%g h=%g",
                   bb.x(), bb.y(),
                   bb.width(), bb.height());
            // element is above the staff if center of bbox is above center of staff
            LOGD("directionTag()  center diff=%g", el->y() + el->height() / 2 - bb.y() - bb.height() / 2);
             */

            if (el->isHairpin() || el->isOttava() || el->isPedal() || el->isTextLine()) {
                // for the line type elements the reference point is vertically centered
                // actual position info is in the segments
                // compare the segment's canvas ypos with the staff's center height
                // if (seg->pagePos().y() < sys->pagePos().y() + bb.y() + bb.height() / 2)
                if (el->placement() == PlacementV::ABOVE) {
                    tagName += u" placement=\"above\"";
                } else {
                    tagName += u" placement=\"below\"";
                }
            } else if (el->isDynamic()) {
                tagName += u" placement=\"";
                tagName += el->placement() == PlacementV::ABOVE ? u"above" : u"below";
                tagName += u"\"";
            } else {
                /*
                LOGD("directionTag()  staff ely=%g elh=%g bby=%g bbh=%g",
                       el->y(), el->height(),
                       bb.y(), bb.height());
                 */
                // if (el->y() + el->height() / 2 < /*bb.y() +*/ bb.height() / 2)
                if (el->placement() == PlacementV::ABOVE) {
                    tagName += u" placement=\"above\"";
                } else {
                    tagName += u" placement=\"below\"";
                }
            }
        }           // if (pel && ...

        if (el->systemFlag() && !ExportMusicXml::configuration()->exportMu3Compat()) {
            tagName += u" system=\"only-top\"";
        }
    }
    xml.startElementRaw(tagName);
}

//---------------------------------------------------------
//   directionETag
//---------------------------------------------------------

static void directionETag(XmlWriter& xml, staff_idx_t staff, int offs = 0)
{
    if (offs) {
        xml.tag("offset", offs);
    }
    if (staff) {
        xml.tag("staff", static_cast<int>(staff));
    }
    xml.endElement();
}

//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

static void partGroupStart(XmlWriter& xml, int number, const BracketItem* const bracket, const bool barlineSpan)
{
    xml.startElement("part-group", { { "type", "start" }, { "number", number } });
    String br;
    switch (bracket->bracketType()) {
    case BracketType::NO_BRACKET:
        br = u"none";
        break;
    case BracketType::NORMAL:
        br = u"bracket";
        break;
    case BracketType::BRACE:
        br = u"brace";
        break;
    case BracketType::LINE:
        br = u"line";
        break;
    case BracketType::SQUARE:
        br = u"square";
        break;
    default:
        LOGD("bracket subtype %d not understood", int(bracket->bracketType()));
    }
    if (!br.empty()) {
        String tag = u"group-symbol";
        tag += color2xml(bracket);
        xml.tagRaw(tag, br);
    }
    if (barlineSpan) {
        xml.tag("group-barline", "yes");
    }
    xml.endElement();
}

//---------------------------------------------------------
//   findMetronome
//---------------------------------------------------------
static size_t indexOf(const String& src_, const std::wregex& re, size_t from, std::wsmatch* m)
{
    std::u16string u16 = src_.toStdU16String();
    std::wstring src;
    src.resize(u16.size());

    static_assert(sizeof(wchar_t) >= sizeof(char16_t));

    for (size_t i = 0; i < u16.size(); ++i) {
        src[i] = static_cast<wchar_t>(u16.at(i));
    }

    auto begin = std::wsregex_iterator(src.begin(), src.end(), re);
    auto end = std::wsregex_iterator();
    for (auto it = begin; it != end; ++it) {
        std::wsmatch match = *it;
        size_t pos = src.find(match.str(), from);
        if (pos != std::u16string::npos) {
            if (m) {
                *m = match;
            }
            return pos;
        }
    }
    return std::u16string::npos;
}

static bool findMetronome(const std::list<TextFragment>& list,
                          std::list<TextFragment>& wordsLeft,  // words left of metronome
                          bool& hasParen,      // parenthesis
                          String& metroLeft,  // left part of metronome
                          String& metroRight, // right part of metronome
                          std::list<TextFragment>& wordsRight // words right of metronome
                          )
{
    String words = MScoreTextToMusicXml::toPlainTextPlusSymbols(list);
    //LOGD("findMetronome('%s')", muPrintable(words));
    hasParen = false;
    metroLeft.clear();
    metroRight.clear();
    int metroPos = -1;     // metronome start position
    int metroLen = 0;      // metronome length

    size_t indEq  = words.indexOf(u'=');
    if (indEq == 0 || indEq == muse::nidx) {
        return false;
    }

    int len1 = 0;
    TDuration dur;

    // find first note, limiting search to the part left of the first '=',
    // to prevent matching the second note in a "note1 = note2" metronome
    int pos1 = TempoText::findTempoDuration(words.left(indEq), len1, dur);
    static const std::wregex equationRegEx(L"\\s*=\\s*");
    std::wsmatch equationMatch;
    size_t pos2 = indexOf(words, equationRegEx, pos1 + len1, &equationMatch);
    if (pos1 != -1 && int(pos2) == pos1 + len1) {
        int len2 = equationMatch.length();
        if (words.size() > pos2 + len2) {
            String s1 = words.mid(0, pos1);           // string to the left of metronome
            String s2 = words.mid(pos1, len1);        // first note
            String s3 = words.mid(pos2, len2);        // equals sign
            String s4 = words.mid(pos2 + len2);       // string to the right of equals sign

            // now determine what is to the right of the equals sign
            // must have either a (dotted) note or a number at start of s4
            int len3 = 0;
            // One or more digits, optionally followed by a single dot or comma and one or more digits
            static const std::wregex numberRegEx(L"\\d+([,\\.]{1}\\d+)?");
            int pos3 = TempoText::findTempoDuration(s4, len3, dur);
            if (pos3 == -1) {
                // did not find note, try to find a number
                std::wsmatch numberMatch;
                pos3 = static_cast<int>(indexOf(s4, numberRegEx, 0, &numberMatch));
                if (pos3 == 0) {
                    len3 = numberMatch.length();
                }
            }
            if (pos3 == -1) {
                // neither found
                return false;
            }

            String s5 = s4.mid(0, len3);       // number or second note
            String s6 = s4.mid(len3);          // string to the right of metronome

            // determine if metronome has parentheses
            // left part of string must end with parenthesis plus optional spaces
            // right part of string must have parenthesis (but not in first pos)
            size_t lparen = s1.indexOf(u'(');
            size_t rparen = s6.indexOf(u')');
            hasParen = (lparen == s1.size() - 1 && rparen == 0);

            metroLeft = s2;
            metroRight = s5;

            metroPos = pos1;                     // metronome position
            metroLen = len1 + len2 + len3;       // metronome length
            if (hasParen) {
                metroPos -= 1;                   // move left one position
                metroLen += 2;                   // add length of '(' and ')'
            }

            // calculate starting position corrected for surrogate pairs
            // (which were ignored by toPlainTextPlusSymbols())
            int corrPos = metroPos;
            for (int i = 0; i < metroPos; ++i) {
                if (words.at(i).isHighSurrogate()) {
                    --corrPos;
                }
            }
            metroPos = corrPos;

            std::list<TextFragment> mid;       // not used
            MScoreTextToMusicXml::split(list, metroPos, metroLen, wordsLeft, mid, wordsRight);
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   beatUnit
//---------------------------------------------------------

static void beatUnit(XmlWriter& xml, const TDuration dur)
{
    int dots = dur.dots();
    xml.tag("beat-unit", TConv::toXml(dur.type()));
    while (dots > 0) {
        xml.tag("beat-unit-dot");
        --dots;
    }
}

//---------------------------------------------------------
//   wordsMetronome
//---------------------------------------------------------

static void wordsMetronome(XmlWriter& xml, const MStyle& s, TextBase const* const text, const int offset)
{
    const std::list<TextFragment> list = text->fragmentList();
    std::list<TextFragment> wordsLeft;          // words left of metronome
    bool hasParen;                          // parenthesis
    String metroLeft;                      // left part of metronome
    String metroRight;                     // right part of metronome
    std::list<TextFragment> wordsRight;         // words right of metronome

    // set the default words format
    const String mtf = s.styleSt(Sid::musicalTextFont);
    const CharFormat defFmt = formatForWords(s);

    if (findMetronome(list, wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {
        if (wordsLeft.size() > 0) {
            xml.startElement("direction-type");
            String attr = ExportMusicXml::positioningAttributes(text);
            MScoreTextToMusicXml mttm(u"words", attr, defFmt, mtf);
            mttm.writeTextFragments(wordsLeft, xml);
            xml.endElement();
        }

        xml.startElement("direction-type");
        String tagName = String(u"metronome parentheses=\"%1\"").arg(hasParen ? u"yes" : u"no");
        tagName += color2xml(text);
        tagName += ExportMusicXml::positioningAttributes(text);
        if (!text->visible()) {
            tagName += u" print-object=\"no\"";
        }
        xml.startElementRaw(tagName);
        int len1 = 0;
        TDuration dur;
        TempoText::findTempoDuration(metroLeft, len1, dur);
        beatUnit(xml, dur);

        if (TempoText::findTempoDuration(metroRight, len1, dur) != -1) {
            beatUnit(xml, dur);
        } else {
            xml.tag("per-minute", metroRight);
        }

        xml.endElement();
        xml.endElement();

        if (wordsRight.size() > 0) {
            xml.startElement("direction-type");
            String attr = ExportMusicXml::positioningAttributes(text);
            MScoreTextToMusicXml mttm(u"words", attr, defFmt, mtf);
            mttm.writeTextFragments(wordsRight, xml);
            xml.endElement();
        }
    } else {
        xml.startElement("direction-type");
        String attr;
        attr += frame2xml(text);
        attr += color2xml(text);
        attr += ExportMusicXml::positioningAttributes(text);
        MScoreTextToMusicXml mttm(u"words", attr, defFmt, mtf);
        //LOGD("words('%s')", muPrintable(text->text()));
        mttm.writeTextFragments(text->fragmentList(), xml);
        xml.endElement();
    }

    if (offset) {
        xml.tag("offset", offset);
    }
}

//---------------------------------------------------------
//   tempoText
//---------------------------------------------------------

void ExportMusicXml::tempoText(TempoText const* const text, staff_idx_t staff)
{
    const int offset = calculateTimeDeltaInDivisions(text->tick(), tick(), m_div);
    /*
    LOGD("tick %s text->tick %s offset %d xmlText='%s')",
           muPrintable(tick().print()),
           muPrintable(text->tick().print()),
           offset,
           muPrintable(text->xmlText()));
    */
    m_attr.doAttr(m_xml, false);

    XmlWriter::Attributes tempoAttrs;
    tempoAttrs = { { "placement", (text->placement() == PlacementV::BELOW) ? "below" : "above" } };
    if (text->systemFlag() && !ExportMusicXml::configuration()->exportMu3Compat()) {
        tempoAttrs.emplace_back(std::make_pair("system", text->isLinked() ? "also-top" : "only-top"));
    }

    m_xml.startElement("direction", tempoAttrs);
    wordsMetronome(m_xml, m_score->style(), text, offset);

    if (staff) {
        m_xml.tag("staff", static_cast<int>(staff));
    }
    tempoSound(text);
    m_xml.endElement();
}

void ExportMusicXml::tempoSound(TempoText const* const text)
{
    // Format tempo with maximum 2 decimal places, because in some MuseScore files tempo is stored
    // imprecisely and this could cause rounding errors (e.g. 92 BPM would be saved as 91.9998).
    BeatsPerMinute bpm = text->tempo().toBPM();
    if (text->isATempo() || text->isTempoPrimo()) {
        bpm = m_score->tempomap()->tempo(text->tick().ticks()).toBPM();
    }
    double bpmRounded = round(bpm.val * 100) / 100;
    m_xml.tag("sound", { { "tempo", bpmRounded } });
}

//---------------------------------------------------------
//   playText
//---------------------------------------------------------

void ExportMusicXml::playText(PlayTechAnnotation const* const annot, staff_idx_t staff)
{
    const int offset = calculateTimeDeltaInDivisions(annot->tick(), tick(), m_div);

    if (annot->plainText() == "") {
        // sometimes empty Texts are present, exporting would result
        // in invalid MusicXML (as an empty direction-type would be created)
        return;
    }

    directionTag(m_xml, m_attr, annot);
    wordsMetronome(m_xml, m_score->style(), annot, offset);

    const PlayingTechniqueType type = annot->techniqueType();
    if (type == PlayingTechniqueType::Pizzicato) {
        m_xml.tag("sound", { { "pizzicato", "yes" } });
    } else if ((type != PlayingTechniqueType::Pizzicato) && (m_currPlayTechnique == PlayingTechniqueType::Pizzicato)) {
        m_xml.tag("sound", { { "pizzicato", "no" } });
    } else if ((type != PlayingTechniqueType::Undefined) && (type != PlayingTechniqueType::Natural)) {
        m_xml.startElement("sound");
        m_xml.startElement("play");
        if (type == PlayingTechniqueType::Mute) {
            m_xml.tag("mute", "on");
        } else if (type == PlayingTechniqueType::Open) {
            m_xml.tag("mute", "off");
        } else {
            m_xml.tag("other-play", { { "type", TConv::toXml(type) } }, TConv::userName(type).translated());
        }
        m_xml.endElement();
        m_xml.endElement();
    }
    m_currPlayTechnique = type;

    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportMusicXml::words(TextBase const* const text, staff_idx_t staff)
{
    const int offset = calculateTimeDeltaInDivisions(text->tick(), tick(), m_div);
    /*
    LOGD("tick %s text->tick %s offset %d userOff.x=%f userOff.y=%f xmlText='%s' plainText='%s'",
           muPrintable(tick().print()),
           muPrintable(text->tick().print()),
           offset,
           text->offset().x(), text->offset().y(),
           muPrintable(text->xmlText()),
           muPrintable(text->plainText()));
    */

    if (text->plainText() == "") {
        // sometimes empty Texts are present, exporting would result
        // in invalid MusicXML (as an empty direction-type would be created)
        return;
    }

    directionTag(m_xml, m_attr, text);
    wordsMetronome(m_xml, m_score->style(), text, offset);

    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   systemText
//---------------------------------------------------------

void ExportMusicXml::systemText(StaffTextBase const* const text, staff_idx_t staff)
{
    const int offset = calculateTimeDeltaInDivisions(text->tick(), tick(), m_div);

    if (text->plainText() == "") {
        // sometimes empty Texts are present, exporting would result
        // in invalid MusicXML (as an empty direction-type would be created)
        return;
    }

    directionTag(m_xml, m_attr, text);
    wordsMetronome(m_xml, m_score->style(), text, offset);

    if (text->swing()) {
        m_xml.startElement("sound");
        m_xml.startElement("swing");
        if (!text->swingParameters().swingUnit) {
            m_xml.tag("straight");
        } else {
            const int swingPercentage = text->swingParameters().swingRatio;
            const int swingDivisor = std::gcd(text->swingParameters().swingRatio, 100);
            m_xml.tag("first",  100 / swingDivisor);
            m_xml.tag("second", swingPercentage / swingDivisor);
            if (text->swingParameters().swingUnit == Constants::DIVISION / 2) {
                m_xml.tag("swing-type", TConv::toXml(DurationType::V_EIGHTH));
            } else {
                m_xml.tag("swing-type", TConv::toXml(DurationType::V_16TH));
            }
        }
        m_xml.endElement();
        m_xml.endElement();
    }

    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   positioningAttributesForTboxText
//---------------------------------------------------------

String ExportMusicXml::positioningAttributesForTboxText(const PointF position, float spatium)
{
    if (!configuration()->exportLayout()) {
        return String();
    }

    PointF relative;         // use zero relative position
    return positionToString(position, relative, spatium);
}

//---------------------------------------------------------
//   tboxTextAsWords
//---------------------------------------------------------

void ExportMusicXml::tboxTextAsWords(TextBase const* const text, const staff_idx_t staff, const PointF relativePosition)
{
    if (text->plainText() == "") {
        // sometimes empty Texts are present, exporting would result
        // in invalid MusicXML (as an empty direction-type would be created)
        return;
    }

    // set the default words format
    const MStyle& style = m_score->style();
    const String mtf = style.styleSt(Sid::musicalTextFont);
    const CharFormat defFmt = formatForWords(style);

    m_xml.startElement("direction", { { "placement", (relativePosition.y() < 0) ? "above" : "below" } });
    m_xml.startElement("direction-type");
    String attr;
    attr += frame2xml(text);
    attr += ExportMusicXml::positioningAttributesForTboxText(relativePosition, text->spatium());
    attr += u" valign=\"top\"";
    MScoreTextToMusicXml mttm(u"words", attr, defFmt, mtf);
    mttm.writeTextFragments(text->fragmentList(), m_xml);
    m_xml.endElement();
    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   rehearsal
//---------------------------------------------------------

void ExportMusicXml::rehearsal(RehearsalMark const* const rmk, staff_idx_t staff)
{
    if (rmk->plainText() == "") {
        // sometimes empty Texts are present, exporting would result
        // in invalid MusicXML (as an empty direction-type would be created)
        return;
    }

    directionTag(m_xml, m_attr, rmk);
    m_xml.startElement("direction-type");
    String attr;
    if (rmk->circle()) {
        attr = u" enclosure=\"circle\"";
    } else if (!rmk->hasFrame()) {
        // special default case
        attr = u" enclosure=\"none\"";
    }
    attr += color2xml(rmk);
    attr += positioningAttributes(rmk);
    if (configuration()->exportLayout()) {
        switch (rmk->align().horizontal) {
        case AlignH::LEFT:
            // default in MusicXML
            break;
        case AlignH::HCENTER:
            attr += u" justify=\"center\"";
            break;
        case AlignH::RIGHT:
            attr += u" justify=\"right\"";
            break;
        }
    }
    // set the default words format
    const MStyle& style = m_score->style();
    const String mtf = style.styleSt(Sid::musicalTextFont);
    const CharFormat defFmt = formatForWords(style);
    // write formatted
    MScoreTextToMusicXml mttm(u"rehearsal", attr, defFmt, mtf);
    mttm.writeTextFragments(rmk->fragmentList(), m_xml);
    m_xml.endElement();
    const int offset = calculateTimeDeltaInDivisions(rmk->tick(), tick(), m_div);
    if (offset) {
        m_xml.tag("offset", offset);
    }
    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   harp pedal
//---------------------------------------------------------

void ExportMusicXml::harpPedals(HarpPedalDiagram const* const hpd, staff_idx_t staff)
{
    directionTag(m_xml, m_attr, hpd);
    m_xml.startElement("direction-type");
    XmlWriter::Attributes harpPedalAttrs;
    addColorAttr(hpd, harpPedalAttrs);
    if (hpd->isDiagram()) {
        m_xml.startElement("harp-pedals", harpPedalAttrs);
        const std::vector <String> pedalSteps = { u"D", u"C", u"B", u"E", u"F", u"G", u"A" };
        for (size_t idx = 0; idx < pedalSteps.size(); idx++) {
            m_xml.startElement("pedal-tuning");
            m_xml.tag("pedal-step", pedalSteps.at(idx));
            m_xml.tag("pedal-alter", static_cast<int>(hpd->getPedalState().at(idx)) - 1);
            m_xml.endElement();
        }
        m_xml.endElement();
    } else {
        m_xml.tag("words", harpPedalAttrs, hpd->plainText());
    }
    m_xml.endElement();
    const int offset = calculateTimeDeltaInDivisions(hpd->tick(), tick(), m_div);
    if (offset) {
        m_xml.tag("offset", offset);
    }
    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   findDashes -- get index of hairpin in dashes table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findDashes(const TextLineBase* hp) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (m_dashes[i] == hp) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   findHairpin -- get index of hairpin in hairpin table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findHairpin(const Hairpin* hp) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (m_hairpins[i] == hp) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   findInString
//---------------------------------------------------------

// find the longest first match of dynList's dynamic text in s
// used by the MusicXML export to correctly export dynamics embedded
// in spanner begin- or endtexts
// return match's position and length and the dynamic type

static size_t findDynamicInString(const String& s, size_t& length, String& type)
{
    length = 0;
    type.clear();
    size_t matchIndex = muse::nidx;
    const int n = static_cast<int>(DynamicType::LAST) - 1;

    // for all dynamics, find their text in s
    for (int i = 0; i < n; ++i) {
        DynamicType t = static_cast<DynamicType>(i);
        const String dynamicText = Dynamic::dynamicText(t);
        const size_t dynamicLength = dynamicText.size();
        // note: skip entries with empty text
        if (dynamicLength > 0) {
            const size_t index = s.indexOf(dynamicText);
            if (index != muse::nidx) {
                // found a match, accept it if
                // - it is the first one
                // - or it starts a the same index but is longer ("pp" versus "p")
                if (matchIndex == muse::nidx || (index == matchIndex && dynamicLength > length)) {
                    matchIndex = index;
                    length = dynamicLength;
                    type = String::fromAscii(TConv::toXml(t).ascii());
                }
            }
        }
    }

    return matchIndex;
}

//---------------------------------------------------------
//   writeHairpinText
//---------------------------------------------------------

static void writeHairpinText(XmlWriter& xml, const TextLineBase* const tlb, bool isStart = true)
{
    String text = isStart ? tlb->beginText() : tlb->endText();
    while (!text.empty()) {
        size_t dynamicLength = 0;
        String dynamicsType;
        size_t dynamicPosition = findDynamicInString(text, dynamicLength, dynamicsType);
        if (dynamicPosition == muse::nidx || dynamicPosition > 0) {
            // text remaining and either no dynamic of not at front of text
            xml.startElement("direction-type");
            String tag = u"words";
            tag += String(u" font-family=\"%1\"").arg(tlb->getProperty(isStart
                                                                       ? Pid::BEGIN_FONT_FACE
                                                                       : Pid::END_FONT_FACE).value<String>());
            tag += String(u" font-size=\"%1\"").arg(tlb->getProperty(isStart ? Pid::BEGIN_FONT_SIZE : Pid::END_FONT_SIZE).toReal());
            tag += fontStyleToXML(static_cast<FontStyle>(tlb->getProperty(isStart ? Pid::BEGIN_FONT_STYLE : Pid::END_FONT_STYLE).toInt()));
            tag += color2xml(tlb);
            tag += ExportMusicXml::positioningAttributes(tlb, isStart);
            xml.tagRaw(tag, dynamicPosition == muse::nidx ? text : text.left(dynamicPosition));
            xml.endElement();
            if (dynamicPosition == muse::nidx) {
                text.clear();
            } else if (dynamicPosition > 0) {
                text.remove(0, dynamicPosition);
                dynamicPosition = 0;
            }
        }
        if (dynamicPosition == 0) {
            // dynamic at front of text
            xml.startElement("direction-type");
            String tag = u"dynamics";
            tag += color2xml(tlb);
            tag += ExportMusicXml::positioningAttributes(tlb, isStart);
            xml.startElementRaw(tag);
            xml.tagRaw(dynamicsType);
            xml.endElement();
            xml.endElement();
            text.remove(0, dynamicLength);
        }
    }
}

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportMusicXml::hairpin(Hairpin const* const hp, staff_idx_t staff, const Fraction& tick)
{
    const bool isLineType = hp->isLineType();
    const bool isStart = hp->tick() == tick;
    int n;
    if (isLineType) {
        if (!hp->lineVisible()) {
            if ((isStart && hp->beginText().isEmpty()) || (!isStart && hp->endText().isEmpty())) {
                return;
            }
            // generate backup or forward to the start time of the element
            const Fraction tickToWrite = isStart ? hp->tick() : hp->tick2();
            moveToTickIfNeed(tickToWrite);
            directionTag(m_xml, m_attr, hp);
            writeHairpinText(m_xml, hp, isStart);
            directionETag(m_xml, staff);
            return;
        }
        n = findDashes(hp);
        if (n >= 0) {
            m_dashes[n] = nullptr;
        } else {
            n = findDashes(nullptr);
            if (n >= 0) {
                m_dashes[n] = hp;
            } else {
                LOGD("too many overlapping dashes (hp %p staff %zu tick %d)", hp, staff, tick.ticks());
                return;
            }
        }
    } else {
        n = findHairpin(hp);
        if (n >= 0) {
            m_hairpins[n] = nullptr;
        } else {
            n = findHairpin(nullptr);
            if (n >= 0) {
                m_hairpins[n] = hp;
            } else {
                LOGD("too many overlapping hairpins (hp %p staff %zu tick %d)", hp, staff, tick.ticks());
                return;
            }
        }
    }

    // generate backup or forward to the start time of the element
    const Fraction tickToWrite = isStart ? hp->tick() : hp->tick2();
    moveToTickIfNeed(tickToWrite);

    directionTag(m_xml, m_attr, hp);
    if (isStart) {
        writeHairpinText(m_xml, hp, isStart);
    }
    if (isLineType) {
        if (hp->lineVisible()) {
            if (isStart) {
                m_xml.startElement("direction-type");
                String tag = u"dashes type=\"start\"";
                tag += String(u" number=\"%1\"").arg(n + 1);
                tag += color2xml(hp);
                tag += positioningAttributes(hp, isStart);
                m_xml.tagRaw(tag);
                m_xml.endElement();
            } else {
                m_xml.startElement("direction-type");
                m_xml.tagRaw(String(u"dashes type=\"stop\" number=\"%1\"").arg(n + 1));
                m_xml.endElement();
            }
        }
    } else {
        m_xml.startElement("direction-type");
        String tag = u"wedge type=";
        if (isStart) {
            if (hp->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                tag += u"\"crescendo\"";
                if (hp->hairpinCircledTip()) {
                    tag += u" niente=\"yes\"";
                }
            } else {
                tag += u"\"diminuendo\"";
            }
            tag += color2xml(hp);
            tag += positioningAttributes(hp, isStart);
        } else {
            tag += u"\"stop\"";
            if (hp->hairpinCircledTip() && hp->hairpinType() == HairpinType::DIM_HAIRPIN) {
                tag += u" niente=\"yes\"";
            }
        }
        tag += String(u" number=\"%1\"").arg(n + 1);
        m_xml.tagRaw(tag);
        m_xml.endElement();
    }
    if (!isStart) {
        writeHairpinText(m_xml, hp, isStart);
    }
    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   findOttava -- get index of ottava in ottava table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findOttava(const Ottava* ot) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (m_ottavas[i] == ot) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   ottava
// <octave-shift type="down" size="8" relative-y="14"/>
// <octave-shift type="stop" size="8"/>
//---------------------------------------------------------

void ExportMusicXml::ottava(Ottava const* const ot, staff_idx_t staff, const Fraction& tick)
{
    int n = findOttava(ot);
    bool isStart = ot->tick() == tick;
    if (n >= 0) {
        m_ottavas[n] = 0;
    } else {
        n = findOttava(0);
        if (n >= 0) {
            m_ottavas[n] = ot;
        } else {
            LOGD("too many overlapping ottavas (ot %p staff %zu tick %d)", ot, staff, tick.ticks());
            return;
        }
    }

    String octaveShiftXml;
    const OttavaType st = ot->ottavaType();
    if (ot->tick() == tick) {
        String sz;
        String tp;
        switch (st) {
        case OttavaType::OTTAVA_8VA:
            sz = u"8";
            tp = u"down";
            break;
        case OttavaType::OTTAVA_15MA:
            sz = u"15";
            tp = u"down";
            break;
        case OttavaType::OTTAVA_8VB:
            sz = u"8";
            tp = u"up";
            break;
        case OttavaType::OTTAVA_15MB:
            sz = u"15";
            tp = u"up";
            break;
        default:
            LOGD("ottava subtype %d not understood", int(st));
        }
        if (!sz.empty() && !tp.empty()) {
            octaveShiftXml = String(u"octave-shift type=\"%1\" size=\"%2\" number=\"%3\"").arg(tp, sz).arg(n + 1);
        }
    } else {
        if (st == OttavaType::OTTAVA_8VA || st == OttavaType::OTTAVA_8VB) {
            octaveShiftXml = String(u"octave-shift type=\"stop\" size=\"8\" number=\"%1\"").arg(n + 1);
        } else if (st == OttavaType::OTTAVA_15MA || st == OttavaType::OTTAVA_15MB) {
            octaveShiftXml = String(u"octave-shift type=\"stop\" size=\"15\" number=\"%1\"").arg(n + 1);
        } else {
            LOGD("ottava subtype %d not understood", int(st));
        }
    }

    if (!octaveShiftXml.empty()) {
        // generate backup or forward to the start time of the element
        const Fraction tickToWrite = isStart ? ot->tick() : ot->tick2();
        moveToTickIfNeed(tickToWrite);

        directionTag(m_xml, m_attr, ot);
        m_xml.startElement("direction-type");
        octaveShiftXml += color2xml(ot);
        octaveShiftXml += positioningAttributes(ot, ot->tick() == tick);
        m_xml.tagRaw(octaveShiftXml);
        m_xml.endElement();
        directionETag(m_xml, staff);
    }
}

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

void ExportMusicXml::pedal(Pedal const* const pd, staff_idx_t staff, const Fraction& tick)
{
    // "change" type is handled only on the beginning of pedal lines
    if (pd->tick() != tick && pd->endHookType() == HookType::HOOK_45) {
        return;
    }
    bool isStart = pd->tick() == tick;

    // generate backup or forward to the start time of the element
    const Fraction tickToWrite = isStart ? pd->tick() : pd->tick2();
    moveToTickIfNeed(tickToWrite);

    directionTag(m_xml, m_attr, pd);
    m_xml.startElement("direction-type");
    String pedalType;
    String pedalXml;
    String signText;
    String lineText = pd->lineVisible() ? u" line=\"yes\"" : u" line=\"no\"";
    if (pd->tick() == tick) {
        switch (pd->beginHookType()) {
        case HookType::HOOK_45:
            pedalType = u"change";
            break;
        case HookType::NONE:
            pedalType = pd->lineVisible() ? u"resume" : u"start";
            break;
        default:
            pedalType = u"start";
        }
        signText = pd->beginText().isEmpty() ? u" sign=\"no\"" : u" sign=\"yes\"";
        if (pd->beginText() == u"<sym>keyboardPedalSost</sym>" || pd->beginText() == u"<sym>keyboardPedalS</sym>") {
            pedalType = u"sostenuto";
        }
    } else {
        if (!pd->endText().isEmpty() || pd->endHookType() == HookType::HOOK_90) {
            pedalType = u"stop";
        } else {
            pedalType = u"discontinue";
        }
        // "change" type is handled only on the beginning of pedal lines

        signText = pd->endText().isEmpty() ? u" sign=\"no\"" : u" sign=\"yes\"";
    }
    pedalXml = String(u"pedal type=\"%1\"").arg(pedalType);
    pedalXml += lineText;
    pedalXml += signText;
    pedalXml += color2xml(pd);
    pedalXml += positioningAttributes(pd, pd->tick() == tick);
    m_xml.tagRaw(pedalXml);
    m_xml.endElement();
    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   findBracket -- get index of bracket in bracket table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findBracket(const TextLineBase* tl) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (m_brackets[i] == tl) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   textLine
//---------------------------------------------------------

void ExportMusicXml::textLine(TextLineBase const* const tl, staff_idx_t staff, const Fraction& tick)
{
    using namespace muse::draw;

    bool isStart = tl->tick() == tick;
    if (!tl->lineVisible()) {
        if ((isStart && tl->beginText().isEmpty()) || (!isStart && tl->endText().isEmpty())) {
            return;
        }
        directionTag(m_xml, m_attr, tl);
        writeHairpinText(m_xml, tl, isStart);
        directionETag(m_xml, staff);
        return;
    }

    int n;
    // special case: a dashed line w/o hooks is written as dashes
    const bool isDashes = tl->lineStyle() == LineType::DASHED && (tl->beginHookType() == HookType::NONE)
                          && (tl->endHookType() == HookType::NONE);

    if (isDashes) {
        n = findDashes(tl);
        if (n >= 0) {
            m_dashes[n] = nullptr;
        } else {
            n = findBracket(nullptr);
            if (n >= 0) {
                m_dashes[n] = tl;
            } else {
                LOGD("too many overlapping dashes (tl %p staff %zu tick %d)", tl, staff, tick.ticks());
                return;
            }
        }
    } else {
        n = findBracket(tl);
        if (n >= 0) {
            m_brackets[n] = nullptr;
        } else {
            n = findBracket(nullptr);
            if (n >= 0) {
                m_brackets[n] = tl;
            } else {
                LOGD("too many overlapping textlines (tl %p staff %zu tick %d)", tl, staff, tick.ticks());
                return;
            }
        }
    }

    String rest;
    PointF p;

    String type;
    HookType hookType = HookType::NONE;
    double hookHeight = 0.0;
    if (tl->tick() == tick) {
        if (!isDashes) {
            String lineType;
            switch (tl->lineStyle()) {
            case LineType::SOLID:
                lineType = u"solid";
                break;
            case LineType::DASHED:
                lineType = u"dashed";
                break;
            case LineType::DOTTED:
                lineType = u"dotted";
                break;
            }
            rest += String(u" line-type=\"%1\"").arg(lineType);
        }
        hookType   = tl->beginHookType();
        hookHeight = tl->beginHookHeight().val();
        if (!tl->segmentsEmpty()) {
            p = tl->frontSegment()->offset();
        }
        type = u"start";
    } else {
        hookType   = tl->endHookType();
        hookHeight = tl->endHookHeight().val();
        if (!tl->segmentsEmpty()) {
            p = (toLineSegment(tl->backSegment()))->userOff2();
        }
        type = u"stop";
    }

    String lineEnd;
    switch (hookType) {
    case HookType::HOOK_90T:
        lineEnd = u"both";
        rest += String(u" end-length=\"%1\"").arg(std::abs(hookHeight * 20));
        break;
    case HookType::HOOK_90:
        lineEnd = (hookHeight < 0.0) ? u"up" : u"down";
        rest += String(u" end-length=\"%1\"").arg(std::abs(hookHeight * 10));
        break;
    case HookType::NONE:
        lineEnd = u"none";
        break;
    default:
        lineEnd = u"none";
        LOGD("HookType %d not supported", int(hookType));
    }

    rest += color2xml(tl);
    rest += positioningAttributes(tl, tl->tick() == tick);

    // generate backup or forward to the start time of the element
    const Fraction tickToWrite = isStart ? tl->tick() : tl->tick2();
    moveToTickIfNeed(tickToWrite);

    directionTag(m_xml, m_attr, tl);

    if (!tl->beginText().isEmpty() && tl->tick() == tick) {
        m_xml.startElement("direction-type");
        m_xml.tag("words", tl->beginText());
        m_xml.endElement();
    }

    if (tl->lineVisible()) {
        m_xml.startElement("direction-type");
        if (isDashes) {
            m_xml.tag("dashes", { { "type", type }, { "number", n + 1 } });
        } else {
            m_xml.tagRaw(String(u"bracket type=\"%1\" number=\"%2\" line-end=\"%3\"%4").arg(type, String::number(n + 1), lineEnd, rest));
        }
        m_xml.endElement();
    }

    if (!tl->endText().isEmpty() && tl->tick() != tick) {
        m_xml.startElement("direction-type");
        m_xml.tag("words", tl->endText());
        m_xml.endElement();
    }

    /*
    if (offs)
          xml.tag("offset", offs);
    */

    directionETag(m_xml, staff);
}

//---------------------------------------------------------
//   dynamic
//---------------------------------------------------------

// In MuseScore dynamics are essentially user-defined texts, therefore the ones
// supported by MusicXML need to be filtered out. Everything not recognized
// as MusicXML dynamics is written as other-dynamics.

template<typename T>
inline std::set<String>& operator<<(std::set<String>& s, const T& v)
{
    s.emplace(v);
    return s;
}

void ExportMusicXml::dynamic(Dynamic const* const dyn, staff_idx_t staff)
{
    static const std::set<String> validMusicXmlDynamics {
        u"f", u"ff", u"fff", u"ffff", u"fffff", u"ffffff",
        u"fp", u"fz",
        u"mf", u"mp",
        u"p", u"pp", u"ppp", u"pppp", u"ppppp", u"pppppp",
        u"rf", u"rfz",
        u"sf", u"sffz", u"sfp", u"sfpp", u"sfz"
    };

    directionTag(m_xml, m_attr, dyn);

    m_xml.startElement("direction-type");

    String tagName = u"dynamics";
    tagName += frame2xml(dyn);
    tagName += color2xml(dyn);
    tagName += positioningAttributes(dyn);
    m_xml.startElementRaw(tagName);
    const String dynTypeName = String::fromAscii(TConv::toXml(dyn->dynamicType()).ascii());
    bool hasCustomText = dyn->hasCustomText();

    if (muse::contains(validMusicXmlDynamics, dynTypeName) && !hasCustomText) {
        m_xml.tagRaw(dynTypeName);
    } else if (!dynTypeName.empty()) {
        static const std::map<ushort, Char> map = {
            { 0xE520, u'p' },
            { 0xE521, u'm' },
            { 0xE522, u'f' },
            { 0xE523, u'r' },
            { 0xE524, u's' },
            { 0xE525, u'z' },
            { 0xE526, u'n' }
        };

        String dynText = dynTypeName;
        if (dyn->dynamicType() == DynamicType::OTHER || hasCustomText) {
            dynText = dyn->plainText();
        }

        // collect consecutive runs of either dynamics glyphs
        // or other characters and write the runs.
        String text;
        bool inDynamicsSym = false;
        for (size_t i = 0; i < dynText.size(); ++i) {
            Char ch = dynText.at(i);
            const auto it = map.find(ch.unicode());
            if (it != map.end()) {
                // found a SMUFL single letter dynamics glyph
                if (!inDynamicsSym) {
                    if (!text.empty()) {
                        m_xml.tag("other-dynamics", text);
                        text.clear();
                    }
                    inDynamicsSym = true;
                }
                text += it->second;
            } else {
                // found a non-dynamics character
                if (inDynamicsSym) {
                    if (!text.empty()) {
                        if (muse::contains(validMusicXmlDynamics, text)) {
                            m_xml.tagRaw(text);
                        } else {
                            m_xml.tag("other-dynamics", text);
                        }
                        text.clear();
                    }
                    inDynamicsSym = false;
                }
                text += ch.unicode();
            }
        }
        if (!text.empty()) {
            if (inDynamicsSym && muse::contains(validMusicXmlDynamics, text)) {
                m_xml.tagRaw(text);
            } else {
                m_xml.tag("other-dynamics", text);
            }
        }
    }

    m_xml.endElement();

    m_xml.endElement();

    const int offset = calculateTimeDeltaInDivisions(dyn->tick(), tick(), m_div);
    if (offset) {
        m_xml.tag("offset", offset);
    }

    if (staff) {
        m_xml.tag("staff", static_cast<int>(staff));
    }

    if (dyn->velocity() > 0) {
        m_xml.tagRaw(String(u"sound dynamics=\"%1\"").arg(String::number(dyn->velocity() * 100.0 / 90.0, 2)));
    }

    m_xml.endElement();
}

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

void ExportMusicXml::lyrics(const std::vector<Lyrics*>& ll, const track_idx_t trk)
{
    for (const Lyrics* l : ll) {
        if (l && !l->xmlText().isEmpty()) {
            if ((l)->track() == trk) {
                String lyricXml = String(u"lyric number=\"%1\"").arg((l)->no() + 1);
                lyricXml += color2xml(l);
                lyricXml += positioningAttributes(l);
                if (!l->visible()) {
                    lyricXml += u" print-object=\"no\"";
                }
                if (l->placeAbove()) {
                    lyricXml += u" placement=\"above\"";
                }
                m_xml.startElementRaw(lyricXml);
                LyricsSyllabic syl = (l)->syllabic();
                String s;
                switch (syl) {
                case LyricsSyllabic::SINGLE: s = u"single";
                    break;
                case LyricsSyllabic::BEGIN:  s = u"begin";
                    break;
                case LyricsSyllabic::END:    s = u"end";
                    break;
                case LyricsSyllabic::MIDDLE: s = u"middle";
                    break;
                default:
                    LOGD("unknown syllabic %d", int(syl));
                }
                m_xml.tag("syllabic", s);
                String attr;         // TODO TBD
                // set the default words format
                const String mtf = m_score->style().styleSt(Sid::musicalTextFont);
                CharFormat defFmt;
                defFmt.setFontFamily(m_score->style().styleSt(l->isEven() ? Sid::lyricsEvenFontFace : Sid::lyricsOddFontFace));
                defFmt.setFontSize(m_score->style().styleD(l->isEven() ? Sid::lyricsEvenFontSize : Sid::lyricsOddFontSize));
                // write formatted
                MScoreTextToMusicXml mttm(u"text", attr, defFmt, mtf);
                mttm.writeTextFragments(l->fragmentList(), m_xml);
                if (l->ticks().isNotZero()) {
                    m_xml.tag("extend");
                }
                m_xml.endElement();
            }
        }
    }
}

//---------------------------------------------------------
//   directionJump -- write jump
//---------------------------------------------------------

// LVIFIX: TODO coda and segno should be numbered uniquely

static void directionJump(XmlWriter& xml, const Jump* const jp)
{
    JumpType jtp = jp->jumpType();
    String words;
    String type;
    String sound;
    bool isDaCapo = false;
    bool isDalSegno = false;
    if (jtp == JumpType::DC) {
        if (jp->xmlText().empty()) {
            words = u"D.C.";
        } else {
            words = jp->xmlText();
        }
        isDaCapo = true;
    } else if (jtp == JumpType::DC_AL_FINE) {
        if (jp->xmlText().empty()) {
            words = u"D.C. al Fine";
        } else {
            words = jp->xmlText();
        }
        isDaCapo = true;
    } else if (jtp == JumpType::DC_AL_CODA) {
        if (jp->xmlText().empty()) {
            words = u"D.C. al Coda";
        } else {
            words = jp->xmlText();
        }
        isDaCapo = true;
    } else if (jtp == JumpType::DS_AL_CODA) {
        if (jp->xmlText().empty()) {
            words = u"D.S. al Coda";
        } else {
            words = jp->xmlText();
        }
        isDalSegno = true;
    } else if (jtp == JumpType::DS_AL_FINE) {
        if (jp->xmlText().empty()) {
            words = u"D.S. al Fine";
        } else {
            words = jp->xmlText();
        }
        isDalSegno = true;
    } else if (jtp == JumpType::DS) {
        words = u"D.S.";
        isDalSegno = true;
    } else {
        words = jp->xmlText();

        if (jp->jumpTo() == "start") {
            isDaCapo = true;
        } else {
            isDalSegno = true;
        }
    }

    if (isDaCapo) {
        sound = u"dacapo=\"yes\"";
    } else if (isDalSegno) {
        if (jp->xmlText().empty()) {
            sound = u"dalsegno=\"1\"";
        } else {
            sound = u"dalsegno=\"" + jp->jumpTo() + u"\"";
        }
    }

    if (!sound.empty()) {
        xml.startElement("direction", { { "placement", (jp->placement() == PlacementV::BELOW) ? "below" : "above" } });
        xml.startElement("direction-type");
        String attrs = color2xml(jp);
        attrs += ExportMusicXml::positioningAttributes(jp);
        if (!type.empty()) {
            xml.tagRaw(type + attrs);
        }
        if (!words.empty()) {
            xml.tagRaw(u"words" + attrs, words);
        }
        xml.endElement();
        if (!sound.empty()) {
            xml.tagRaw(u"sound " + sound);
        }
        xml.endElement();
    }
}

//---------------------------------------------------------
//   getEffectiveMarkerType
//---------------------------------------------------------

static MarkerType getEffectiveMarkerType(const Marker* const m, const std::vector<const Jump*>& jumps)
{
    MarkerType mtp = m->markerType();

    if (mtp != MarkerType::USER) {
        return mtp;
    }

    // Try to guess marker type from its usage in jumps.
    const String label = m->label();

    for (const Jump* j : jumps) {
        MarkerType guessedMarkerType = mtp;

        if (j->jumpTo() == label) {
            guessedMarkerType = MarkerType::SEGNO;
        } else if (j->playUntil() == label) {
            guessedMarkerType = j->continueAt().isEmpty() ? MarkerType::FINE : MarkerType::TOCODA;
        } else if (j->continueAt() == label) {
            guessedMarkerType = MarkerType::CODA;
        }

        if (guessedMarkerType != mtp) {
            if (mtp != MarkerType::USER) {
                // Type guesses differ for different jump elements.
                LOGD("Cannot guess type for marker with label=\"%s\"", muPrintable(label));
                return MarkerType::USER;
            }
            mtp = guessedMarkerType;
        }
    }

    return mtp;
}

//---------------------------------------------------------
//   findCodaLabel
//---------------------------------------------------------

static String findCodaLabel(const std::vector<const Jump*>& jumps, const String& toCodaLabel)
{
    for (const Jump* j : jumps) {
        if (j->playUntil() == toCodaLabel) {
            return j->continueAt();
        }
    }

    return String();
}

//---------------------------------------------------------
//   directionMarker -- write marker
//---------------------------------------------------------

static void directionMarker(XmlWriter& xml, const Marker* const m, const std::vector<const Jump*>& jumps)
{
    const MarkerType mtp = getEffectiveMarkerType(m, jumps);
    String words;
    String type;
    String sound;

    switch (mtp) {
    case MarkerType::CODA:
    case MarkerType::VARCODA:
    case MarkerType::CODETTA:
        type = u"coda";
        if (m->label() == "") {
            sound = u"coda=\"1\"";
        } else {
            sound = u"coda=\"" + m->label() + u"\"";
        }
        break;
    case MarkerType::SEGNO:
    case MarkerType::VARSEGNO:
        type = u"segno";
        if (m->label() == "") {
            sound = u"segno=\"1\"";
        } else {
            sound = u"segno=\"" + m->label() + u"\"";
        }
        break;
    case MarkerType::FINE:
        words = u"Fine";
        sound = u"fine=\"yes\"";
        break;
    case MarkerType::TOCODA:
    case MarkerType::TOCODASYM:
    case MarkerType::DA_CODA:
    case MarkerType::DA_DBLCODA: {
        if (m->xmlText() == "") {
            words = u"To Coda";
        } else {
            words = m->xmlText();
        }
        const String codaLabel = findCodaLabel(jumps, m->label());
        if (codaLabel.empty()) {
            sound = u"tocoda=\"1\"";
        } else {
            sound = u"tocoda=\"" + codaLabel + u"\"";
        }
        break;
    }
    case MarkerType::USER:
        LOGD("marker type=%d not implemented", int(mtp));
        break;
    }

    if (!sound.empty()) {
        xml.startElement("direction", { { "placement", (m->placement() == PlacementV::BELOW) ? "below" : "above" } });
        xml.startElement("direction-type");
        String attrs = color2xml(m);
        attrs += ExportMusicXml::positioningAttributes(m);
        if (!type.empty()) {
            xml.tagRaw(type + attrs);
        }
        if (!words.empty()) {
            xml.tagRaw(u"words" + attrs, words);
        }
        xml.endElement();
        if (!sound.empty()) {
            xml.tagRaw(String(u"sound ") + sound);
        }
        xml.endElement();
    }
}

//---------------------------------------------------------
//  findTrackForAnnotations
//---------------------------------------------------------

// An annotation is attached to the staff, with track set
// to the lowest track in the staff. Find a track for it
// (the lowest track in this staff that has a chord or rest)

static track_idx_t findTrackForAnnotations(track_idx_t track, Segment* seg)
{
    if (!(seg->isChordRestType() || seg->isTimeTickType())) {
        return muse::nidx;
    }

    staff_idx_t staff = track / VOICES;
    track_idx_t strack = staff * VOICES;        // start track of staff containing track
    track_idx_t etrack = strack + VOICES;       // end track of staff containing track + 1

    for (track_idx_t i = strack; i < etrack; i++) {
        if (seg->element(i)) {
            return i;
        }
    }

    return muse::nidx;
}

//---------------------------------------------------------
//  repeatAtMeasureStart -- write repeats at begin of measure
//---------------------------------------------------------

void ExportMusicXml::repeatAtMeasureStart(Attributes& attr, const Measure* const m, track_idx_t strack, track_idx_t etrack,
                                          track_idx_t track)
{
    // loop over all segments
    for (EngravingItem* e : m->el()) {
        track_idx_t wtrack = muse::nidx;     // track to write jump
        if (strack <= e->track() && e->track() < etrack) {
            wtrack = findTrackForAnnotations(e->track(), m->first(SegmentType::ChordRest));
        }
        if (track != wtrack) {
            continue;
        }
        switch (e->type()) {
        case ElementType::MARKER:
        {
            // filter out the markers at measure Start
            const Marker* const mk = toMarker(e);
            const MarkerType mtp = getEffectiveMarkerType(mk, m_jumpElements);

            switch (mtp) {
            case MarkerType::SEGNO:
            case MarkerType::VARSEGNO:
            case MarkerType::CODA:
            case MarkerType::VARCODA:
            case MarkerType::CODETTA:
                LOGD(" -> handled");
                attr.doAttr(m_xml, false);
                directionMarker(m_xml, mk, m_jumpElements);
                break;
            case MarkerType::FINE:
            case MarkerType::TOCODA:
            case MarkerType::TOCODASYM:
            case MarkerType::DA_CODA:
            case MarkerType::DA_DBLCODA:
                // ignore
                break;
            case MarkerType::USER:
                LOGD("repeatAtMeasureStart: marker %d not implemented", int(mtp));
                break;
            }
        }
        break;
        default:
            LOGD("repeatAtMeasureStart: direction type %s at tick %d not implemented",
                 e->typeName(), m->tick().ticks());
            break;
        }
    }
}

//---------------------------------------------------------
//  repeatAtMeasureStop -- write repeats at end of measure
//---------------------------------------------------------

void ExportMusicXml::repeatAtMeasureStop(const Measure* const m, track_idx_t strack, track_idx_t etrack, track_idx_t track)
{
    for (EngravingItem* e : m->el()) {
        track_idx_t wtrack = muse::nidx;     // track to write jump
        if (strack <= e->track() && e->track() < etrack) {
            wtrack = findTrackForAnnotations(e->track(), m->first(SegmentType::ChordRest));
        }
        if (track != wtrack) {
            continue;
        }
        switch (e->type()) {
        case ElementType::MARKER:
        {
            // filter out the markers at measure stop
            const Marker* const mk = toMarker(e);
            const MarkerType mtp = getEffectiveMarkerType(mk, m_jumpElements);

            switch (mtp) {
            case MarkerType::FINE:
            case MarkerType::TOCODA:
            case MarkerType::TOCODASYM:
            case MarkerType::DA_CODA:
            case MarkerType::DA_DBLCODA:
                directionMarker(m_xml, mk, m_jumpElements);
                break;
            case MarkerType::SEGNO:
            case MarkerType::VARSEGNO:
            case MarkerType::CODA:
            case MarkerType::VARCODA:
            case MarkerType::CODETTA:
                // ignore
                break;
            case MarkerType::USER:
                LOGD("repeatAtMeasureStop: marker %d not implemented", int(mtp));
                break;
            }
        }
        break;
        case ElementType::JUMP:
            directionJump(m_xml, toJump(e));
            break;
        default:
            LOGD("repeatAtMeasureStop: direction type %s at tick %d not implemented",
                 e->typeName(), m->tick().ticks());
            break;
        }
    }
}

//---------------------------------------------------------
//  work -- write the <work> element
//  note that order must be work-number, work-title
//  also write <movement-number> and <movement-title>
//  data is taken from the score metadata instead of the Text elements
//---------------------------------------------------------

void ExportMusicXml::work(const MeasureBase* /*measure*/)
{
    const String workTitle  = m_score->metaTag(u"workTitle");
    const String workNumber = m_score->metaTag(u"workNumber");
    if (!(workTitle.isEmpty() && workNumber.isEmpty())) {
        m_xml.startElement("work");
        if (!workNumber.isEmpty()) {
            m_xml.tag("work-number", workNumber);
        }
        if (!workTitle.isEmpty()) {
            m_xml.tag("work-title", workTitle);
        }
        m_xml.endElement();
    }
    if (!m_score->metaTag(u"movementNumber").isEmpty()) {
        m_xml.tag("movement-number", m_score->metaTag(u"movementNumber"));
    }
    if (!m_score->metaTag(u"movementTitle").isEmpty()) {
        m_xml.tag("movement-title", m_score->metaTag(u"movementTitle"));
    }
}

//---------------------------------------------------------
//  measureRepeat -- write measure-repeat
//---------------------------------------------------------

static void measureRepeat(XmlWriter& xml, Attributes& attr, const Measure* const m, const int partIndex)
{
    Part* part = m->score()->parts().at(partIndex);
    const staff_idx_t scoreRelStaff = m->score()->staffIdx(part);
    for (size_t i = 0; i < part->nstaves(); ++i) {
        staff_idx_t staffIdx = scoreRelStaff + i;
        XmlWriter::Attributes styleAttrs;
        if (part->nstaves() > 1) {
            styleAttrs.emplace_back(std::make_pair("number", i + 1));
        }
        if (m->isMeasureRepeatGroup(staffIdx)
            && (!m->prevMeasure() || !m->prevMeasure()->isMeasureRepeatGroup(staffIdx)
                || (m->measureRepeatNumMeasures(staffIdx) != m->prevMeasure()->measureRepeatNumMeasures(staffIdx)))) {
            attr.doAttr(xml, true);
            addColorAttr(m->measureRepeatElement(staffIdx), styleAttrs);
            xml.startElement("measure-style", styleAttrs);
            const int numMeasures = m->measureRepeatNumMeasures(staffIdx);
            if (numMeasures > 1) {
                // slashes == numMeasures for everything MuseScore currently supports
                xml.tag("measure-repeat", { { "slashes", numMeasures }, { "type", "start" } }, numMeasures);
            } else {
                // no need to include slashes
                xml.tag("measure-repeat", { { "type", "start" } }, numMeasures);
            }
            xml.endElement();
        } else if (
            // no longer in measure repeats
            m->prevMeasure() && ((m->prevMeasure()->isMeasureRepeatGroup(staffIdx) && !m->isMeasureRepeatGroup(staffIdx))
                                 // or still in measure repeats, but now of different duration
                                 || (m->prevMeasure()->measureRepeatElement(staffIdx) && m->measureRepeatElement(staffIdx)
                                     && (m->measureRepeatNumMeasures(staffIdx) != m->prevMeasure()->measureRepeatNumMeasures(staffIdx))))) {
            attr.doAttr(xml, true);
            xml.startElement("measure-style", styleAttrs);
            xml.tag("measure-repeat", { { "type", "stop" } }, "");
            xml.endElement();
        }
    }
}

//---------------------------------------------------------
//  measureStyle -- write measure-style
//---------------------------------------------------------

// this is done at the first measure of a multimeasure rest
// note: for a normal measure, mmRest1 is the measure itself,
// for a multi-measure rest, it is the replacing measure

static void measureStyle(XmlWriter& xml, Attributes& attr, const Measure* const m, const int partIndex)
{
    const Measure* mmR1 = m->coveringMMRestOrThis();
    if (m != mmR1 && m == mmR1->mmRestFirst()) {
        attr.doAttr(xml, true);
        xml.startElement("measure-style");
        String multiRestTag = u"multiple-rest";
        if (m->score()->style().styleB(Sid::oldStyleMultiMeasureRests)) {
            if (mmR1->mmRestCount() <= m->score()->style().styleI(Sid::mmRestOldStyleMaxMeasures)) {
                multiRestTag += u" use-symbols=\"yes\"";
            } else {
                multiRestTag += u" use-symbols=\"no\"";
            }
        }
        xml.tagRaw(multiRestTag, mmR1->mmRestCount());
        xml.endElement();
    } else {
        // measure repeat can only possibly be present if mmrest was not
        measureRepeat(xml, attr, m, partIndex);
    }
}

//---------------------------------------------------------
//  commonAnnotations
//---------------------------------------------------------

static bool commonAnnotations(ExportMusicXml* exp, const EngravingItem* e, staff_idx_t sstaff)
{
    if (!exp->canWrite(e)) {
        // write only tempo
        if (e->isTempoText()) {
            exp->tempoSound(toTempoText(e));
        }
        return false;
    }

    bool instrChangeHandled = false;

    // note: the instrument change details are handled in ExportMusicXml::writeMeasureTracks,
    // optionally writing the associated staff text is done below
    if (e->isTempoText()) {
        exp->tempoText(toTempoText(e), sstaff);
    } else if (e->isPlayTechAnnotation()) {
        exp->playText(toPlayTechAnnotation(e), sstaff);
    } else if (e->isCapo() || e->isStringTunings() || e->isStaffText() || e->isTripletFeel() || e->isText()
               || e->isExpression() || (e->isInstrumentChange() && e->visible()) || e->isSticking()) {
        exp->words(toTextBase(e), sstaff);
    } else if (e->isDynamic()) {
        exp->dynamic(toDynamic(e), sstaff);
    } else if (e->isHarpPedalDiagram()) {
        exp->harpPedals(toHarpPedalDiagram(e), sstaff);
    } else if (e->isRehearsalMark()) {
        exp->rehearsal(toRehearsalMark(e), sstaff);
    } else if (e->isSystemText()) {
        exp->systemText(toStaffTextBase(e), sstaff);
    } else {
        return instrChangeHandled;
    }

    return true;
}

//---------------------------------------------------------
//  annotations
//---------------------------------------------------------

// Only handle common annotations, others are handled elsewhere

static void annotations(ExportMusicXml* exp, track_idx_t strack, track_idx_t etrack, track_idx_t track, staff_idx_t sstaff, Segment* seg)
{
    for (const EngravingItem* e : seg->annotations()) {
        track_idx_t wtrack = muse::nidx;       // track to write annotation

        if (strack <= e->track() && e->track() < etrack) {
            wtrack = findTrackForAnnotations(e->track(), seg);
        }

        if (track == wtrack) {
            if (commonAnnotations(exp, e, sstaff)) {
                // already handled
            }
        }
    }
}

//---------------------------------------------------------
//  harmonies
//---------------------------------------------------------

/*
 * Helper method to export harmonies and chord diagrams for a single segment.
 */

static void segmentHarmonies(ExportMusicXml* exp, track_idx_t track, Segment* seg, Fraction offset)
{
    const std::vector<EngravingItem*> diagrams = seg->findAnnotations(ElementType::FRET_DIAGRAM, track, track);
    std::vector<EngravingItem*> harmonies = seg->findAnnotations(ElementType::HARMONY, track, track);

    for (const EngravingItem* d : diagrams) {
        const FretDiagram* diagram = toFretDiagram(d);
        const Harmony* harmony = diagram->harmony();
        if (harmony) {
            exp->harmony(harmony, diagram, offset);
        } else if (!harmonies.empty()) {
            const EngravingItem* defaultHarmony = harmonies.back();
            exp->harmony(toHarmony(defaultHarmony), diagram, offset);
            harmonies.pop_back();
        } else {
            // Found a fret diagram with no harmony, ignore
            LOGD("segmentHarmonies() seg %p found fretboard diagram %p w/o harmony: cannot write", seg, diagram);
        }
    }

    for (const EngravingItem* h : harmonies) {
        exp->harmony(toHarmony(h), 0, offset);
    }
}

/*
 * Write harmonies and fret diagrams that are attached to chords or rests.
 *
 * There are fondamental differences between the ways Musescore and MusicXML handle harmonies (Chord symbols)
 * and fretboard diagrams.
 *
 * In MuseScore, the Harmony element is now a child of FretboardDiagram BUT in previous versions,
 * both elements were independant siblings so we have to handle both cases.
 * In MusicXML, fretboard diagram is always contained in a harmony element.
 *
 * In MuseScore, Harmony elements are not always linked to notes, and each Harmony will be contained
 * in a `ChordRest` Segment.
 * In MusicXML, those successive Harmony elements must be exported before the note with different offsets.
 *
 * Edge cases that we simply cannot handle:
 *  - as of MusicXML 3.1, there is no way to represent a diagram without an associated chord symbol,
 * so when we encounter such an object in MuseScore, we simply cannot export it.
 *  - If a ChordRest segment contans a FretboardDiagram with no harmonies and several different Harmony siblings,
 * we simply have to pick a random one to export.
 */

static void harmonies(ExportMusicXml* exp, track_idx_t track, Segment* seg)
{
    Fraction offset = { 0, 1 };
    segmentHarmonies(exp, track, seg, offset);

    // Edge case: find remaining `harmony` elements.
    // Suppose you have one single whole note in the measure but several chord symbols.
    // In MuseScore, each `Harmony` object will be stored in a `ChordRest` Segment that contains
    // no other Chords.
    // But in MusicXML, you are supposed to output all `harmony` elements before the first `note`,
    // with different `offset` parameters.
    //
    // That's why we need to explore the remaining segments to find
    // `Harmony` and `FretDiagram` elements in Segments without Chords and output them now.
    for (Segment* seg1 = seg->next(); seg1; seg1 = seg1->next()) {
        if (!seg1->isChordRestType()) {
            continue;
        }

        const EngravingItem* el1 = seg1->element(track);
        if (el1) { // found a ChordRest, next harmony will be attached to this one
            break;
        }
        offset = (seg1->tick() - seg->tick());
        segmentHarmonies(exp, track, seg1, offset);
    }
}

//---------------------------------------------------------
//   Write MusicXML
//
// Writes the portion within the <figure> tag.
//
// NOTE: Both MuseScore and MusicXML provide two ways of altering the (temporal) length of a
// figured bass object: extension lines and duration. The convention is that an EXTENSION is
// used if the figure lasts LONGER than the note (i.e., it "extends" to the following notes),
// whereas DURATION is used if the figure lasts SHORTER than the note (e.g., when notating a
// figure change under a note). However, MuseScore does not restrict durations in this way,
// allowing them to act as extensions themselves. As a result, a few more branches are
// required in the decision tree to handle everything correctly.
//---------------------------------------------------------

//---------------------------------------------------------
//   Convert Modifier to MusicXML prefix/suffix
//---------------------------------------------------------

static String Modifier2MusicXml(FiguredBassItem::Modifier prefix)
{
    switch (prefix) {
    case FiguredBassItem::Modifier::NONE:        return String();
    case FiguredBassItem::Modifier::DOUBLEFLAT:  return u"flat-flat";
    case FiguredBassItem::Modifier::FLAT:        return u"flat";
    case FiguredBassItem::Modifier::NATURAL:     return u"natural";
    case FiguredBassItem::Modifier::SHARP:       return u"sharp";
    case FiguredBassItem::Modifier::DOUBLESHARP: return u"double-sharp";
    case FiguredBassItem::Modifier::CROSS:       return u"cross";
    case FiguredBassItem::Modifier::BACKSLASH:   return u"backslash";
    case FiguredBassItem::Modifier::SLASH:       return u"slash";
    case FiguredBassItem::Modifier::NUMOF:       return String();         // prevent gcc "‘FBINumOfAccid’ not handled in switch" warning
    }
    return String();
}

static void writeMusicXml(const FiguredBassItem* item, XmlWriter& xml, bool isOriginalFigure, int crEndTick, int fbEndTick)
{
    xml.startElement("figure");

    // The first figure of each group is the "original" figure. Practically, it is one inserted manually
    // by the user, rather than automatically by the "duration" extend method.
    if (isOriginalFigure) {
        String strPrefix = Modifier2MusicXml(item->prefix());
        if (!strPrefix.empty()) {
            xml.tag("prefix", strPrefix);
        }
        if (item->digit() != FBIDigitNone) {
            xml.tag("figure-number", item->digit());
        }
        String strSuffix = Modifier2MusicXml(item->suffix());
        if (!strSuffix.empty()) {
            xml.tag("suffix", strSuffix);
        }

        // Check if the figure ends before or at the same time as the current note. Otherwise, the figure
        // extends to the next note, and so carries an extension type "start" by definition.
        if (fbEndTick <= crEndTick) {
            if (item->contLine() == FiguredBassItem::ContLine::SIMPLE) {
                xml.tag("extend", { { "type", "stop" } });
            } else if (item->contLine() == FiguredBassItem::ContLine::EXTENDED) {
                bool hasFigure = (!strPrefix.empty() || item->digit() != FBIDigitNone || !strSuffix.empty());
                if (hasFigure) {
                    xml.tag("extend", { { "type", "start" } });
                } else {
                    xml.tag("extend", { { "type", "continue" } });
                }
            }
        } else {
            xml.tag("extend", { { "type", "start" } });
        }
    }
    // If the figure is not "original", it must have been created using the "duration" feature of figured bass.
    // In other words, the original figure belongs to a previous note rather than the current note.
    else {
        if (crEndTick < fbEndTick) {
            xml.tag("extend", { { "type", "continue" } });
        } else {
            xml.tag("extend", { { "type", "stop" } });
        }
    }
    xml.endElement();
}

static void writeMusicXml(const FiguredBass* item, XmlWriter& xml, bool isOriginalFigure, int crEndTick, int fbEndTick, bool writeDuration,
                          int divisions)
{
    XmlWriter::Attributes attrs;
    if (item->parenthesesMode()) {
        attrs = { { "parentheses", "yes" } };
    }
    if (item->placeAbove()) {
        attrs.emplace_back(std::make_pair("placement", "above"));
    }
    addColorAttr(item, attrs);
    if (!item->visible()) {
        attrs.emplace_back(std::make_pair("print-object", "no"));
    }
    xml.startElement("figured-bass", attrs);
    for (FiguredBassItem* fbItem : item->items()) {
        writeMusicXml(fbItem, xml, isOriginalFigure, crEndTick, fbEndTick);
    }
    if (writeDuration) {
        xml.tag("duration", calculateDurationInDivisions(item->ticks(), divisions));
    }
    xml.endElement();
}

//---------------------------------------------------------
//  figuredBass
//---------------------------------------------------------

static void figuredBass(XmlWriter& xml, track_idx_t strack, track_idx_t etrack, track_idx_t track, const ChordRest* cr, FigBassMap& fbMap,
                        int divisions)
{
    Segment* seg = cr->segment();
    if (seg->segmentType() == SegmentType::ChordRest) {
        for (const EngravingItem* e : seg->annotations()) {
            track_idx_t wtrack = muse::nidx;       // track to write annotation

            if (strack <= e->track() && e->track() < etrack) {
                wtrack = findTrackForAnnotations(e->track(), seg);
            }

            if (track == wtrack) {
                if (e->type() == ElementType::FIGURED_BASS) {
                    const FiguredBass* fb = dynamic_cast<const FiguredBass*>(e);
                    if (fb->items().empty()) {
                        continue;
                    }
                    //LOGD("figuredbass() track %d seg %p fb %p seg %p tick %d ticks %d cr %p tick %d ticks %d",
                    //       track, seg, fb, fb->segment(), fb->segment()->tick(), fb->ticks(), cr, cr->tick(), cr->actualTicks());
                    bool extend = fb->ticks() > cr->actualTicks();
                    if (extend) {
                        //LOGD("figuredbass() extend to %d + %d = %d",
                        //       cr->tick(), fb->ticks(), cr->tick() + fb->ticks());
                        fbMap.insert({ strack, fb });
                    } else {
                        muse::remove(fbMap, strack);
                    }
                    const Fraction crEndTick = cr->tick() + cr->actualTicks();
                    const Fraction fbEndTick = fb->segment()->tick() + fb->ticks();
                    const bool writeDuration = fb->ticks() < cr->actualTicks();
                    writeMusicXml(fb, xml, true, crEndTick.ticks(), fbEndTick.ticks(),
                                  writeDuration, divisions);
                    // Check for changing figures under a single note (each figure stored in a separate segment)
                    for (Segment* segNext = seg->next(); segNext && segNext->element(track) == NULL; segNext = segNext->next()) {
                        for (EngravingItem* annot : segNext->annotations()) {
                            if (annot->type() == ElementType::FIGURED_BASS && annot->track() == track) {
                                fb = dynamic_cast<const FiguredBass*>(annot);
                                writeMusicXml(fb, xml, true, 0, 0, true, divisions);
                            }
                        }
                    }
                    // no extend can be pending
                    return;
                }
            }
        }
        // check for extend pending
        if (muse::contains(fbMap, strack)) {
            const FiguredBass* fb = fbMap.at(strack);
            Fraction crEndTick = cr->tick() + cr->actualTicks();
            Fraction fbEndTick = fb->segment()->tick() + fb->ticks();
            bool writeDuration = fb->ticks() < cr->actualTicks();
            if (cr->tick() < fbEndTick) {
                //LOGD("figuredbass() at tick %d extend only", cr->tick());
                writeMusicXml(fb, xml, false, crEndTick.ticks(), fbEndTick.ticks(), writeDuration, divisions);
            }
            if (fbEndTick <= crEndTick) {
                //LOGD("figuredbass() at tick %d extend done", cr->tick() + cr->actualTicks());
                muse::remove(fbMap, strack);
            }
        }
    }
}

//---------------------------------------------------------
//  spannerStart
//---------------------------------------------------------

// for each spanner start:
// find start track
// find stop track
// if stop track < start track
//   get data from list of already stopped spanners
// else
//   calculate data
// write start if in right track

static void spannerStart(ExportMusicXml* exp, track_idx_t strack, track_idx_t etrack, track_idx_t track, staff_idx_t sstaff, Segment* seg)
{
    if (seg->isChordRestType() || seg->isTimeTickType()) {
        Fraction stick = seg->tick();
        for (auto it = exp->score()->spanner().lower_bound(stick.ticks()); it != exp->score()->spanner().upper_bound(stick.ticks()); ++it) {
            Spanner* e = it->second;

            if (!exp->canWrite(e)) {
                continue;
            }

            track_idx_t wtrack = muse::nidx;       // track to write spanner
            if (strack <= e->track() && e->track() < etrack) {
                wtrack = findTrackForAnnotations(e->track(), seg);
            }

            if (track == wtrack) {
                switch (e->type()) {
                case ElementType::HAIRPIN:
                    exp->hairpin(toHairpin(e), sstaff, seg->tick());
                    break;
                case ElementType::OTTAVA:
                    exp->ottava(toOttava(e), sstaff, seg->tick());
                    break;
                case ElementType::PEDAL:
                    exp->pedal(toPedal(e), sstaff, seg->tick());
                    break;
                case ElementType::TEXTLINE:
                    exp->textLine(toTextLineBase(e), sstaff, seg->tick());
                    break;
                case ElementType::LET_RING:
                    exp->textLine(toLetRing(e), sstaff, seg->tick());
                    break;
                case ElementType::GRADUAL_TEMPO_CHANGE:
                    exp->textLine(toGradualTempoChange(e), sstaff, seg->tick());
                    break;
                case ElementType::PALM_MUTE:
                    exp->textLine(toPalmMute(e), sstaff, seg->tick());
                    break;
                case ElementType::WHAMMY_BAR:
                    exp->textLine(toWhammyBar(e), sstaff, seg->tick());
                    break;
                case ElementType::RASGUEADO:
                    exp->textLine(toRasgueado(e), sstaff, seg->tick());
                    break;
                case ElementType::HARMONIC_MARK:
                    exp->textLine(toHarmonicMark(e), sstaff, seg->tick());
                    break;
                case ElementType::PICK_SCRAPE:
                    exp->textLine(toPickScrape(e), sstaff, seg->tick());
                    break;
                case ElementType::TRILL:
                    // ignore (written as <note><notations><ornaments><wavy-line>)
                    break;
                case ElementType::SLUR:
                    // ignore (written as <note><notations><slur>)
                    break;
                default:
                    LOGD("spannerStart: direction type %d ('%s') at tick %d not implemented",
                         int(e->type()), e->typeName(), seg->tick().ticks());
                    break;
                }
            }
        }           // for
    }
}

//---------------------------------------------------------
//  spannerStop
//---------------------------------------------------------

// called after writing each chord or rest to check if a spanner must be stopped
// loop over all spanners and find spanners in strack ending at tick2
// note that more than one voice may contains notes ending at tick2,
// remember which spanners have already been stopped (the "stopped" set)

static void spannerStop(ExportMusicXml* exp, track_idx_t strack, track_idx_t etrack, const Fraction& tick2, staff_idx_t sstaff,
                        std::set<const Spanner*>& stopped)
{
    for (auto it : exp->score()->spanner()) {
        const Spanner* e = it.second;

        if (!exp->canWrite(e)) {
            continue;
        }

        if (e->tick2() != tick2 || e->track() < strack || e->track() >= etrack) {
            continue;
        }

        if (!muse::contains(stopped, e)) {
            stopped.insert(e);
            switch (e->type()) {
            case ElementType::HAIRPIN:
                exp->hairpin(toHairpin(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::OTTAVA:
                exp->ottava(toOttava(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::PEDAL:
                exp->pedal(toPedal(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::TEXTLINE:
                exp->textLine(item_cast<const TextLineBase*>(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::LET_RING:
                exp->textLine(toLetRing(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::GRADUAL_TEMPO_CHANGE:
                exp->textLine(toGradualTempoChange(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::PALM_MUTE:
                exp->textLine(toPalmMute(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::WHAMMY_BAR:
                exp->textLine(toWhammyBar(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::RASGUEADO:
                exp->textLine(toRasgueado(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::HARMONIC_MARK:
                exp->textLine(toHarmonicMark(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::PICK_SCRAPE:
                exp->textLine(toPickScrape(e), sstaff, Fraction(-1, 1));
                break;
            case ElementType::TRILL:
                // ignore (written as <note><notations><ornaments><wavy-line>
                break;
            case ElementType::SLUR:
                // ignore (written as <note><notations><slur>)
                break;
            default:
                LOGD("spannerStop: direction type %s at tick2 %d not implemented",
                     e->typeName(), tick2.ticks());
                break;
            }
        }
    }         // for
}

//---------------------------------------------------------
//  keysigTimesig
//---------------------------------------------------------

/**
 Output attributes at start of measure: key, time
 */

void ExportMusicXml::keysigTimesig(const Measure* m, const Part* p)
{
    track_idx_t strack = p->startTrack();
    track_idx_t etrack = p->endTrack();
    //LOGD("keysigTimesig m %p strack %d etrack %d", m, strack, etrack);

    // search all staves for non-generated key signatures
    std::map<staff_idx_t, KeySig*> keysigs;   // map staff to key signature
    for (Segment* seg = m->first(); seg; seg = seg->next()) {
        if (seg->tick() > m->tick()) {
            break;
        }
        for (track_idx_t t = strack; t < etrack; t += VOICES) {
            EngravingItem* el = seg->element(t);
            if (!el) {
                continue;
            }
            if (el->type() == ElementType::KEYSIG) {
                //LOGD(" found keysig %p track %d", el, el->track());
                staff_idx_t st = (t - strack) / VOICES;
                if (!el->generated()) {
                    keysigs[st] = static_cast<KeySig*>(el);
                }
            }
        }
    }

    //ClefType ct = rest->staff()->clef(rest->tick());

    // write the key signatues
    if (!keysigs.empty()) {
        // determine if all staves have a keysig and all keysigs are identical
        // in that case a single <key> is written, without number=... attribute
        size_t nstaves = p->nstaves();
        bool singleKey = true;
        // check if all staves have a keysig
        for (staff_idx_t i = 0; i < nstaves; i++) {
            if (!muse::contains(keysigs, i)) {
                singleKey = false;
            }
        }
        // check if all keysigs are identical
        if (singleKey) {
            for (staff_idx_t i = 1; i < nstaves; i++) {
                if (!(keysigs.at(i)->key() == keysigs.at(0)->key())) {
                    singleKey = false;
                }
            }
        }

        // write the keysigs
        //LOGD(" singleKey %d", singleKey);
        if (singleKey) {
            // keysig applies to all staves
            keysig(keysigs.at(0), p->staff(0)->clef(m->tick()), 0, keysigs.at(0)->visible());
        } else {
            // staff-specific keysigs
            for (staff_idx_t st : muse::keys(keysigs)) {
                keysig(keysigs.at(st), p->staff(st)->clef(m->tick()), st + 1, keysigs.at(st)->visible());
            }
        }
    } else {
        // always write a keysig at tick = 0
        if (m->tick().isZero()) {
            //KeySigEvent kse;
            //kse.setKey(Key::C);
            KeySig* ks = Factory::createKeySig(m_score->dummy()->segment());
            ks->setKey(Key::C);
            keysig(ks, p->staff(0)->clef(m->tick()));
            delete ks;
        }
    }

    TimeSig* tsig = 0;
    for (Segment* seg = m->first(); seg; seg = seg->next()) {
        if (seg->tick() > m->tick()) {
            break;
        }
        EngravingItem* el = seg->element(strack);
        if (el && el->type() == ElementType::TIMESIG) {
            tsig = (TimeSig*)el;
        }
    }
    if (tsig) {
        timesig(tsig);
    }
}

//---------------------------------------------------------
//  identification -- write the identification
//---------------------------------------------------------

void ExportMusicXml::identification(XmlWriter& xml, Score const* const score)
{
    xml.startElement("identification");

    // the creator types commonly found in MusicXML
    std::set<String> metaTagNames = { u"arranger", u"composer", u"lyricist", u"poet", u"translator" };
    for (const String& type : metaTagNames) {
        String creator = score->metaTag(type);
        if (!creator.isEmpty()) {
            xml.tag("creator", { { "type", type } }, creator);
        }
    }

    if (!score->metaTag(u"copyright").isEmpty()) {
        xml.tag("rights", score->metaTag(u"copyright"));
        metaTagNames.emplace(u"copyright");
    }

    xml.startElement("encoding");

    String encoder = score->metaTag(u"encoder");
    if (!encoder.empty()) {
        xml.tag("encoder", encoder);
    }

    if (MScore::debugMode) {
        xml.tag("software", String(u"MuseScore 0.7.0"));
        xml.tag("encoding-date", String(u"2007-09-10"));
    } else {
        xml.tag("software", String(u"MuseScore Studio ") + application()->version().toString());
        xml.tag("encoding-date", muse::Date::currentDate().toString(muse::DateFormat::ISODate));
    }

    // specify supported elements
    xml.tag("supports", { { "element", "accidental" }, { "type", "yes" } });
    xml.tag("supports", { { "element", "beam" }, { "type", "yes" } });
    // set support for print new-page and new-system to match user preference
    // for MusicXmlExportBreaks::MANUAL support is "no" because "yes" breaks Finale NotePad import
    IMusicXmlConfiguration::MusicXmlExportBreaksType breaksType = configuration()->exportBreaksType();
    if (configuration()->exportLayout() && breaksType == IMusicXmlConfiguration::MusicXmlExportBreaksType::All) {
        xml.tag("supports", { { "element", "print" }, { "attribute", "new-page" }, { "type", "yes" }, { "value", "yes" } });
        xml.tag("supports", { { "element", "print" }, { "attribute", "new-system" }, { "type", "yes" }, { "value", "yes" } });
    } else {
        xml.tag("supports", { { "element", "print" }, { "attribute", "new-page" }, { "type", "no" } });
        xml.tag("supports", { { "element", "print" }, { "attribute", "new-system" }, { "type", "no" } });
    }
    xml.tag("supports", { { "element", "stem" }, { "type", "yes" } });

    xml.endElement();

    if (!score->metaTag(u"source").isEmpty()) {
        xml.tag("source", score->metaTag(u"source"));
        metaTagNames.emplace(u"source");
    }

    if (!MScore::debugMode) {
        // do not write miscellaneous in debug mode
        metaTagNames.insert({ u"workTitle", u"workNumber", u"movementTitle", u"movementNumber", u"originalFormat" });
        xml.startElement("miscellaneous");
        for (const auto& metaTag : score->metaTags()) {
            auto search = metaTagNames.find(metaTag.first);
            if (search != metaTagNames.end()) {
                continue;
            } else if (!metaTag.second.isEmpty()) {
                xml.tag("miscellaneous-field", { { "name", metaTag.first } }, metaTag.second);
            }
        }
        xml.endElement();
    }

    xml.endElement();
}

//---------------------------------------------------------
//  findPartGroupNumber
//---------------------------------------------------------

static int findPartGroupNumber(int* partGroupEnd)
{
    // find part group number
    for (int number = 0; number < MAX_PART_GROUPS; ++number) {
        if (partGroupEnd[number] == -1) {
            return number;
        }
    }
    LOGD("no free part group number");
    return MAX_PART_GROUPS;
}

//---------------------------------------------------------
//  scoreInstrument
//---------------------------------------------------------

static void scoreInstrument(XmlWriter& xml, const int partNr, const int instrNr, const String& instrName,
                            const Instrument* instr = nullptr)
{
    xml.startElementRaw(String(u"score-instrument %1").arg(instrId(partNr, instrNr)));
    xml.tag("instrument-name", instrName);
    if (instr && !instr->musicXmlId().isEmpty() && !MScore::testMode) {
        xml.tag("instrument-sound", instr->musicXmlId());
    }
    xml.endElement();
}

//---------------------------------------------------------
//  midiInstrument
//---------------------------------------------------------

static void midiInstrument(XmlWriter& xml, const int partNr, const int instrNr,
                           const Instrument* instr, const Score* score, const int unpitched = 0)
{
    xml.startElementRaw(String(u"midi-instrument %1").arg(instrId(partNr, instrNr)));
    int midiChannel = score->masterScore()->midiChannel(instr->channel(0)->channel());
    if (midiChannel >= 0 && midiChannel < 16) {
        xml.tag("midi-channel", midiChannel + 1);
    }
    int midiProgram = instr->channel(0)->program();
    if (midiProgram >= 0 && midiProgram < 128) {
        xml.tag("midi-program", midiProgram + 1);
    }
    if (unpitched > 0) {
        xml.tag("midi-unpitched", unpitched);
    }
    xml.tag("volume", (instr->channel(0)->volume() / 127.0) * 100);    //percent
    xml.tag("pan", int(((instr->channel(0)->pan() - 63.5) / 63.5) * 90));   //-90 hard left, +90 hard right      xml.etag();
    xml.endElement();
}

//---------------------------------------------------------
//  initInstrMap
//---------------------------------------------------------

/**
 Initialize the Instrument* to number map for a Part
 Used to generate instrument numbers for a multi-instrument part
 */

static void initInstrMap(MusicXmlInstrumentMap& im, const InstrumentList& il, const Score* /*score*/)
{
    im.clear();
    for (const auto& pair : il) {
        const Instrument* instr = pair.second;
        if (!muse::contains(im, instr)) {
            im.insert({ instr, static_cast<int>(im.size()) });
        }
    }
}

//---------------------------------------------------------
//  initReverseInstrMap
//---------------------------------------------------------

typedef std::map<int, const Instrument*> MusicXmlReverseInstrumentMap;

/**
 Initialize the number t Instrument* map for a Part
 Used to iterate in sequence over instrument numbers for a multi-instrument part
 */

static void initReverseInstrMap(MusicXmlReverseInstrumentMap& rim, const MusicXmlInstrumentMap& im)
{
    rim.clear();
    for (const Instrument* i : muse::keys(im)) {
        int instNr = im.at(i);
        rim.insert({ instNr, i });
    }
}

//---------------------------------------------------------
//  hasPageBreak
//---------------------------------------------------------

static MeasureBase* lastMeasureBase(const System* const system)
{
    MeasureBase* mb = nullptr;
    if (system) {
        const auto& measures = system->measures();
        IF_ASSERT_FAILED(!(measures.empty())) {
            return nullptr;
        }
        mb = measures.back();
    }
    return mb;
}

//---------------------------------------------------------
//  hasPageBreak
//---------------------------------------------------------

static bool hasPageBreak(const System* const system)
{
    const MeasureBase* mb = nullptr;
    if (system) {
        const auto& measures = system->measures();
        IF_ASSERT_FAILED(!(measures.empty())) {
            return false;
        }
        mb = measures.back();
    }

    return mb && mb->pageBreak();
}

//---------------------------------------------------------
//  print
//---------------------------------------------------------

/**
 Handle the <print> element.
 When exporting layout and all breaks, a <print> with layout information
 is generated for the first measure in the score, in a system or on a page.
 When exporting layout but only manual or no breaks, a <print> with
 layout information is generated only for the first measure in the score,
 as it is assumed the system layout is broken by the importing application
 anyway and is thus useless.

 a page break is explicit (manual) if:
 - the last system on the previous page has a page break
 a system break is explicit (manual) if:
 - the previous system in the score has a system or layout break
 - if the previous system in the score does not have measures
   (i.e. only has (a) frame(s))
 */

void ExportMusicXml::print(const Measure* const m, const int partNr, const int firstStaffOfPart,
                           const size_t nrStavesInPart, const MeasurePrintContext& mpc)
{
    const MeasureBase* const prevSysMB = lastMeasureBase(mpc.prevSystem);

    const bool prevMeasLineBreak = prevSysMB ? prevSysMB->lineBreak() : false;
    const bool prevMeasSectionBreak = prevSysMB ? prevSysMB->sectionBreak() : false;
    const bool prevPageBreak = hasPageBreak(mpc.lastSystemPrevPage);

    String newSystemOrPage;               // new-[system|page]="yes" or empty
    if (!mpc.scoreStart) {
        IMusicXmlConfiguration::MusicXmlExportBreaksType exportBreaksType = configuration()->exportBreaksType();

        if (exportBreaksType == IMusicXmlConfiguration::MusicXmlExportBreaksType::All) {
            if (mpc.pageStart) {
                newSystemOrPage = u" new-page=\"yes\"";
            } else if (mpc.systemStart) {
                newSystemOrPage = u" new-system=\"yes\"";
            }
        } else if (exportBreaksType == IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual) {
            if (mpc.pageStart && prevPageBreak) {
                newSystemOrPage = u" new-page=\"yes\"";
            } else if (mpc.systemStart && (prevMeasLineBreak || prevMeasSectionBreak)) {
                newSystemOrPage = u" new-system=\"yes\"";
            }
        }
    }

    bool doBreak = mpc.scoreStart || (!newSystemOrPage.empty());
    bool doLayout = configuration()->exportLayout();

    if (doBreak) {
        if (doLayout) {
            m_xml.startElementRaw(String(u"print%1").arg(newSystemOrPage));
            const MStyle& style = score()->style();
            const double pageWidth  = getTenthsFromInches(style.styleD(Sid::pageWidth));
            const double lm = getTenthsFromInches(style.styleD(Sid::pageOddLeftMargin));
            const double rm = getTenthsFromInches(style.styleD(Sid::pageWidth)
                                                  - style.styleD(Sid::pagePrintableWidth)
                                                  - style.styleD(Sid::pageOddLeftMargin));
            const double tm = getTenthsFromInches(style.styleD(Sid::pageOddTopMargin));

            // System Layout

            // For a multi-measure rest positioning is valid only
            // in the replacing measure
            // note: for a normal measure, mmRest1 is the measure itself,
            // for a multi-measure rest, it is the replacing measure
            const Measure* mmR1 = m->coveringMMRestOrThis();
            const System* system = mmR1->system();

            // Put the system print suggestions only for the first part in a score...
            if (partNr == 0) {
                // Find the right margin of the system.
                double systemLM = getTenthsFromDots(mmR1->pagePos().x() - system->page()->pagePos().x()) - lm;
                double systemRM = pageWidth - rm - (getTenthsFromDots(system->ldata()->bbox().width()) + lm);

                m_xml.startElement("system-layout");
                m_xml.startElement("system-margins");
                m_xml.tag("left-margin", String::number(systemLM, 2));
                m_xml.tag("right-margin", String::number(systemRM, 2));
                m_xml.endElement();

                if (mpc.systemStart && !mpc.pageStart) {
                    // see System::layout2() for the factor 2 * score()->spatium()
                    const Measure* prevSystemMeasure = mpc.prevMeasure->coveringMMRestOrThis();
                    const double sysDist = getTenthsFromDots(mmR1->pagePos().y()
                                                             - prevSystemMeasure->pagePos().y()
                                                             - prevSystemMeasure->ldata()->bbox().height()
                                                             + 2 * score()->style().spatium()
                                                             );
                    m_xml.tag("system-distance", String::number(sysDist, 2));
                }

                if (mpc.pageStart || mpc.scoreStart) {
                    const double topSysDist = getTenthsFromDots(mmR1->pagePos().y()) - tm;
                    m_xml.tag("top-system-distance", String::number(topSysDist, 2));
                }

                m_xml.endElement();
            }

            // Staff layout elements.
            for (staff_idx_t staffIdx = (firstStaffOfPart == 0) ? 1 : 0; staffIdx < nrStavesInPart; staffIdx++) {
                // calculate distance between this and previous staff using the bounding boxes
                const staff_idx_t staffNr = firstStaffOfPart + staffIdx;
                const staff_idx_t prevStaffNr = system->prevVisibleStaff(staffNr);
                if (prevStaffNr == muse::nidx) {
                    continue;
                }
                if (!system->staff(staffNr)->show()) {
                    m_hiddenStaves.push_back(staffIdx);
                    continue;
                }
                const RectF& prevBbox = system->staff(prevStaffNr)->bbox();
                const double staffDist = system->staff(staffNr)->bbox().y() - prevBbox.y() - prevBbox.height();

                if (staffDist > 0) {
                    m_xml.startElement("staff-layout", { { "number", staffIdx + 1 } });
                    m_xml.tag("staff-distance", String::number(getTenthsFromDots(staffDist), 2));
                    m_xml.endElement();
                } else {
                    m_hiddenStaves.push_back(staffIdx);
                }
            }

            // Measure layout elements.
            if (m->prev() && m->prev()->isHBox()) {
                measureLayout(m->prev()->width());
            }

            m_xml.endElement();
        } else if (!newSystemOrPage.empty()) {
            m_xml.tagRaw(String(u"print%1").arg(newSystemOrPage));
        }
    } else if (m->prev() && m->prev()->isHBox()) {
        m_xml.startElement("print");
        measureLayout(m->prev()->width());
        m_xml.endElement();
    }
}

//---------------------------------------------------------
//  measureLayout
//---------------------------------------------------------

void ExportMusicXml::measureLayout(const double distance)
{
    m_xml.startElement("measure-layout");
    m_xml.tag("measure-distance", String::number(getTenthsFromDots(distance), 2));
    m_xml.endElement();
}

//---------------------------------------------------------
//  exportDefaultClef
//---------------------------------------------------------

/**
 In case no clef is found, export a default clef with type determined by staff type.
 Note that a multi-measure rest starting in the first measure should be handled correctly.
 */

void ExportMusicXml::exportDefaultClef(const Part* const part, const Measure* const m)
{
    const size_t staves = part->nstaves();

    if (m->tick() == Fraction(0, 1)) {
        const Segment* clefSeg = m->findSegment(SegmentType::HeaderClef, Fraction(0, 1));

        if (clefSeg) {
            for (size_t i = 0; i < staves; ++i) {
                // sstaff - xml staff number, counting from 1 for this
                // instrument
                // special number 0 -> don’t show staff number in
                // xml output (because there is only one staff)

                size_t sstaff = (staves > 1) ? i + 1 : 0;
                track_idx_t track = part->startTrack() + VOICES * i;

                if (clefSeg->element(track) == nullptr) {
                    ClefType ct { ClefType::G };
                    String stafftype;
                    switch (part->staff(i)->staffType(Fraction(0, 1))->group()) {
                    case StaffGroup::TAB:
                        ct = ClefType::TAB;
                        stafftype = u"tab";
                        break;
                    case StaffGroup::STANDARD:
                        ct = ClefType::G;
                        stafftype = u"std";
                        break;
                    case StaffGroup::PERCUSSION:
                        ct = ClefType::PERC;
                        stafftype = u"perc";
                        break;
                    }
                    LOGD("no clef found in first measure track %zu (stafftype %s)", track, muPrintable(stafftype));
                    clef(sstaff, ct, u" print-object=\"no\"");
                }
            }
        }
    }
}

//---------------------------------------------------------
//  findAndExportClef
//---------------------------------------------------------

/**
 Make sure clefs at end of measure get exported at start of next measure.
 */

void ExportMusicXml::findAndExportClef(const Measure* const m, const int staves, const track_idx_t strack, const track_idx_t etrack)
{
    Measure* prevMeasure = m->prevMeasure();
    Measure* mmR         = m->mmRest();         // the replacing measure in a multi-measure rest
    Fraction tick        = m->tick();
    Segment* cs1;
    Segment* cs2         = m->findSegment(SegmentType::Clef, tick);
    Segment* cs3;
    Segment* seg         = 0;

    if (prevMeasure) {
        cs1 = prevMeasure->findSegment(SegmentType::Clef, tick);
    } else {
        cs1 = m->findSegment(SegmentType::HeaderClef, tick);
    }

    if (mmR) {
        cs3 = mmR->findSegment(SegmentType::HeaderClef, tick);
        if (!cs3) {
            cs3 = mmR->findSegment(SegmentType::Clef, tick);
        }
    } else {
        cs3 = 0;
    }

    if (cs1 && cs2) {
        // should only happen at begin of new system
        // when previous system ends with a non-generated clef
        seg = cs1;
    } else if (cs1) {
        seg = cs1;
    } else if (cs3) {
        // happens when the first measure is a multi-measure rest
        // containing a generated clef
        seg = cs3;
    } else {
        seg = cs2;
    }
    clefDebug("exportxml: clef segments cs1=%p cs2=%p cs3=%p seg=%p", cs1, cs2, cs3, seg);

    // output attribute at start of measure: clef
    if (seg) {
        for (track_idx_t st = strack; st < etrack; st += VOICES) {
            // sstaff - xml staff number, counting from 1 for this
            // instrument
            // special number 0 -> don’t show staff number in
            // xml output (because there is only one staff)

            staff_idx_t sstaff = (staves > 1) ? st - strack + VOICES : 0;
            sstaff /= VOICES;

            Clef* cle = static_cast<Clef*>(seg->element(st));
            if (cle) {
                clefDebug("exportxml: clef at start measure ti=%d ct=%d gen=%d", tick, int(cle->clefType()), cle->generated());
                // output only clef changes, not generated clefs at line beginning
                // exception: at tick=0, export clef anyway
                if ((tick.isZero() || !cle->generated())
                    && ((seg->measure() != m) || ((seg->segmentType() == SegmentType::HeaderClef) && !cle->otherClef()))) {
                    clefDebug("exportxml: clef exported");
                    String clefAttr = color2xml(cle);
                    if (!cle->visible()) {
                        clefAttr += u" print-object=\"no\"";
                    }
                    clef(sstaff, cle->clefType(), clefAttr);
                } else {
                    clefDebug("exportxml: clef not exported");
                }
            }
        }
    }
}

//---------------------------------------------------------
//  findPitchesUsed
//---------------------------------------------------------

/**
 Find the set of pitches actually used in a part.
 */

typedef std::set<int> pitchSet;       // the set of pitches used

static void addChordPitchesToSet(const Chord* c, pitchSet& set)
{
    for (const Note* note : c->notes()) {
        LOGD("chord %p note %p pitch %d", c, note, note->pitch() + 1);
        set.emplace(note->pitch());
    }
}

static void findPitchesUsed(const Part* part, pitchSet& set)
{
    track_idx_t strack = part->startTrack();
    track_idx_t etrack = part->endTrack();

    // loop over all chords in the part
    for (const MeasureBase* mb = part->score()->measures()->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        const Measure* m = static_cast<const Measure*>(mb);
        for (track_idx_t st = strack; st < etrack; ++st) {
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                const EngravingItem* el = seg->element(st);
                if (!el) {
                    continue;
                }
                if (el->type() == ElementType::CHORD) {
                    // add grace and non-grace note pitches to the result set
                    const Chord* c = static_cast<const Chord*>(el);
                    if (c) {
                        for (const Chord* g : c->graceNotesBefore()) {
                            addChordPitchesToSet(g, set);
                        }
                        addChordPitchesToSet(c, set);
                        for (const Chord* g : c->graceNotesAfter()) {
                            addChordPitchesToSet(g, set);
                        }
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//  partList
//---------------------------------------------------------

/**
 Write the part list to \a xml.
 */

static void partList(XmlWriter& xml, Score* score, MusicXmlInstrumentMap& instrMap)
{
    xml.startElement("part-list");
    size_t staffCount = 0;                               // count sum of # staves in parts
    const auto& parts = score->parts();
    int partGroupEnd[MAX_PART_GROUPS];                // staff where part group ends (bracketSpan is in staves, not parts)
    for (int i = 0; i < MAX_PART_GROUPS; i++) {
        partGroupEnd[i] = -1;
    }
    for (size_t idx = 0; idx < parts.size(); ++idx) {
        const Part* part = parts.at(idx);
        bool bracketFound = false;
        // handle brackets
        for (size_t i = 0; i < part->nstaves(); i++) {
            Staff* st = part->staff(i);
            if (st) {
                for (size_t j = 0; j < st->bracketLevels() + 1; j++) {
                    if (st->bracketType(j) != BracketType::NO_BRACKET) {
                        bracketFound = true;
                        if (i == 0) {
                            // OK, found bracket in first staff of part
                            // filter out implicit brackets
                            if (!(st->bracketSpan(j) == part->nstaves()
                                  && st->bracketType(j) == BracketType::BRACE)) {
                                // filter out brackets starting in the last part
                                // as they cannot span multiple parts
                                if (idx < parts.size() - 1) {
                                    // add others
                                    int number = findPartGroupNumber(partGroupEnd);
                                    if (number < MAX_PART_GROUPS) {
                                        const BracketItem* bi = st->brackets().at(j);
                                        partGroupStart(xml, number + 1, bi, st->barLineSpan());
                                        partGroupEnd[number] = static_cast<int>(staffCount + st->bracketSpan(j));
                                    }
                                }
                            }
                        } else {
                            // bracket in other staff not supported in MusicXML
                            LOGD("bracket starting in staff %zu not supported", i + 1);
                        }
                    }
                }
            }
        }
        // handle bracket none
        if (!bracketFound && part->nstaves() > 1) {
            int number = findPartGroupNumber(partGroupEnd);
            if (number < MAX_PART_GROUPS) {
                const BracketItem* bi = Factory::createBracketItem(score->dummy());
                partGroupStart(xml, number + 1, bi, false);
                delete bi;
                partGroupEnd[number] = static_cast<int>(idx + part->nstaves());
            }
        }

        xml.startElementRaw(String(u"score-part id=\"P%1\"").arg(idx + 1));
        initInstrMap(instrMap, part->instruments(), score);
        static const std::wregex acc(L"[♭♯]");
        XmlWriter::Attributes attributes;
        // by default export the parts long name as part-name
        String partName = part->longName();
        // use the track name if no part long name
        if (partName.empty()) {
            partName = part->partName();
            if (!partName.empty()) {
                attributes.emplace_back(std::make_pair("print-object", "no"));
            }
        }
        xml.tag("part-name", attributes, MScoreTextToMusicXml::toPlainText(partName).replace(u"♭", u"b").replace(u"♯", u"#"));
        if (partName.contains(acc)) {
            xml.startElement("part-name-display");
            writeDisplayName(xml, partName);
            xml.endElement();
        }
        if (!part->shortName().isEmpty()) {
            xml.tag("part-abbreviation", MScoreTextToMusicXml::toPlainText(part->shortName()).replace(u"♭", u"b").replace(u"♯", u"#"));
            if (part->shortName().contains(acc)) {
                xml.startElement("part-abbreviation-display");
                writeDisplayName(xml, part->shortName());
                xml.endElement();
            }
        }

        if (part->instrument()->useDrumset()) {
            const Drumset* drumset = part->instrument()->drumset();
            pitchSet pitches;
            findPitchesUsed(part, pitches);
            for (int i = 0; i < 128; ++i) {
                DrumInstrument di = drumset->drum(i);
                if (di.notehead != NoteHeadGroup::HEAD_INVALID) {
                    scoreInstrument(xml, static_cast<int>(idx) + 1, i + 1, di.name);
                } else if (muse::contains(pitches, i)) {
                    scoreInstrument(xml, static_cast<int>(idx) + 1, i + 1, String(u"Instrument %1").arg(i + 1));
                }
            }
            int midiPort = part->midiPort() + 1;
            if (midiPort >= 1 && midiPort <= 16) {
                xml.tag("midi-device", { { "port", midiPort } }, "");
            }

            for (int i = 0; i < 128; ++i) {
                DrumInstrument di = drumset->drum(i);
                if (di.notehead != NoteHeadGroup::HEAD_INVALID || muse::contains(pitches, i)) {
                    midiInstrument(xml, static_cast<int>(idx) + 1, i + 1, part->instrument(), score, i + 1);
                }
            }
        } else {
            MusicXmlReverseInstrumentMap rim;
            initReverseInstrMap(rim, instrMap);
            for (int instNr : muse::keys(rim)) {
                const Instrument* instr = rim.at(instNr);
                scoreInstrument(xml, static_cast<int>(idx) + 1, instNr + 1,
                                MScoreTextToMusicXml::toPlainText(instr->trackName()),
                                instr);
            }
            for (auto ii = rim.cbegin(); ii != rim.cend(); ii++) {
                int instNr = ii->first;
                int midiPort = part->midiPort() + 1;
                if (ii->second->channel().size() > 0) {
                    midiPort = score->masterScore()->midiMapping(ii->second->channel(0)->channel())->port() + 1;
                }
                if (midiPort >= 1 && midiPort <= 16) {
                    xml.tagRaw(String(u"midi-device %1 port=\"%2\"").arg(instrId(static_cast<int>(idx) + 1, instNr + 1)).arg(midiPort), "");
                } else {
                    xml.tagRaw(String(u"midi-device %1").arg(instrId(static_cast<int>(idx) + 1, instNr + 1)), "");
                }
                midiInstrument(xml, static_cast<int>(idx) + 1, instNr + 1, rim.at(instNr), score);
            }
        }

        xml.endElement();
        staffCount += part->nstaves();
        for (int i = MAX_PART_GROUPS - 1; i >= 0; i--) {
            int end = partGroupEnd[i];
            if (end >= 0) {
                if (static_cast<int>(staffCount) >= end) {
                    xml.tag("part-group", { { "type", "stop" }, { "number", i + 1 } });
                    partGroupEnd[i] = -1;
                }
            }
        }
    }
    xml.endElement();
}

//---------------------------------------------------------
//  tickIsInMiddleOfMeasure
//---------------------------------------------------------

static bool tickIsInMiddleOfMeasure(const Fraction ti, const Measure* m)
{
    return ti != m->tick() && ti != m->endTick();
}

//---------------------------------------------------------
//  writeElement
//---------------------------------------------------------

/**
 Write \a el.
 */

void ExportMusicXml::writeElement(EngravingItem* el, const Measure* m, staff_idx_t sstaff, bool useDrumset)
{
    if (el->isClef()) {
        // output only clef changes, not generated clefs
        // at line beginning
        // also ignore clefs at the start of a measure,
        // these have already been output
        // also ignore clefs at the end of a measure
        // these will be output at the start of the next measure
        const Clef* cle = toClef(el);
        const Fraction ti = cle->segment()->tick();
        const String visible = (!cle->visible()) ? u" print-object=\"no\"" : String();
        clefDebug("exportxml: clef in measure ti=%d ct=%d gen=%d", ti, int(cle->clefType()), el->generated());
        if (el->generated()) {
            clefDebug("exportxml: generated clef not exported");
        } else if (!el->generated() && tickIsInMiddleOfMeasure(ti, m)) {
            clef(sstaff, cle->clefType(), color2xml(cle) + visible);
        } else if (!el->generated() && (ti == m->tick()) && (cle->segment()->segmentType() != SegmentType::HeaderClef)) {
            clef(sstaff, cle->clefType(), color2xml(cle) + visible + String(u" after-barline=\"yes\""));
        } else {
            clefDebug("exportxml: clef not exported");
        }
    } else if (el->isChord()) {
        Chord* const c = toChord(el);
        // ise grace after
        if (c) {
            const auto ll = c->lyrics();
            for (Chord* g : c->graceNotesBefore()) {
                chord(g, sstaff, ll, useDrumset);
            }
            chord(c, sstaff, ll, useDrumset);
            for (Chord* g : c->graceNotesAfter()) {
                chord(g, sstaff, ll, useDrumset);
            }
        }
    } else if (el->isRest()) {
        Rest* r = toRest(el);
        if (!(r->isGap())) {
            const auto ll = r->lyrics();
            rest(r, sstaff, ll);
        }
    } else if (el->isBarLine()) {
        const BarLine* barln = toBarLine(el);
        if (tickIsInMiddleOfMeasure(barln->tick(), m)) {
            barlineMiddle(barln);
        }
    } else if (el->isKeySig() || el->isTimeSig() || el->isBreath() || el->isTimeTickAnchor()) {
        // handled elsewhere
    } else {
        LOGD("ExportMusicXml::write unknown segment type %s", el->typeName());
    }
}

//---------------------------------------------------------
//  clampMusicXmlOctave
//---------------------------------------------------------

/**
 Clamps octave to min and max value as per MusicXML Schema
 */

static void clampMusicXmlOctave(int& octave)
{
    octave = std::clamp(octave, 0, 9);
}

//---------------------------------------------------------
//  writeStaffDetails
//---------------------------------------------------------

/**
 Write the staff details for \a part to \a xml.
 */

static void writeStaffDetails(XmlWriter& xml, const Part* part, const std::vector<size_t> hiddenStaves)
{
    const Instrument* instrument = part->instrument();
    const size_t staves = part->nstaves();

    // staff details
    for (size_t i = 0; i < staves; i++) {
        Staff* st = part->staff(i);
        const double mag = st->staffMag(Fraction(0, 1));
        bool hidden = false;
        if (!st->show()) {
            hidden = true;
        } else {
            for (size_t staffIdx : hiddenStaves) {
                if (i == staffIdx) {
                    hidden = true;
                }
            }
        }
        const Color lineColor = st->color(Fraction(0, 1));
        const bool invis = st->isLinesInvisible(Fraction(0, 1));
        const bool needsLineDetails = invis || lineColor != engravingConfiguration()->defaultColor();
        if (st->lines(Fraction(0, 1)) != 5 || st->isTabStaff(Fraction(0, 1)) || !muse::RealIsEqual(mag, 1.0)
            || hidden || needsLineDetails) {
            XmlWriter::Attributes attributes;
            if (staves > 1) {
                attributes.emplace_back(std::make_pair("number", i + 1));
            }
            if (hidden) {
                attributes.emplace_back(std::make_pair("print-object", "no"));
                if (st->cutaway()) {
                    attributes.emplace_back(std::make_pair("print-spacing", "yes"));
                }
            }

            xml.startElement("staff-details", attributes);

            if (i > 0 && st->links() && st->links()->contains(part->staff(i - 1))) {
                xml.tag("staff-type", "alternate");
            }

            xml.tag("staff-lines", st->lines(Fraction(0, 1)));
            if (needsLineDetails) {
                for (int lineIdx = 0; lineIdx < st->lines(Fraction(0, 1)); ++lineIdx) {
                    String ld = String(u"line-detail line=\"%1\"").arg(lineIdx + 1);
                    if (lineColor != engravingConfiguration()->defaultColor()) {
                        ld += String(u" color=\"%1\"").arg(String::fromStdString(lineColor.toString()));
                    }
                    if (invis) {
                        ld += u" print-object=\"no\"";
                    }
                    xml.tagRaw(ld);
                }
            }

            if (st->isTabStaff(Fraction(0, 1)) && instrument->stringData()) {
                std::vector<instrString> l = instrument->stringData()->stringList();
                for (size_t ii = 0; ii < l.size(); ii++) {
                    char16_t step  = u' ';
                    int alter  = 0;
                    int octave = 0;
                    midipitch2xml(l.at(ii).pitch, step, alter, octave);
                    xml.startElement("staff-tuning", { { "line", ii + 1 } });
                    xml.tag("tuning-step", String(Char(step)));
                    if (alter) {
                        xml.tag("tuning-alter", alter);
                    }
                    clampMusicXmlOctave(octave);
                    xml.tag("tuning-octave", octave);
                    xml.endElement();
                }
            }

            if (!muse::RealIsEqual(mag, 1.0)) {
                xml.tag("staff-size", mag * 100);
            }

            xml.endElement();
        }
    }
}

//---------------------------------------------------------
//  writeInstrumentChange
//---------------------------------------------------------

/**
 Write the instrument change.
 */

void ExportMusicXml::writeInstrumentChange(const InstrumentChange* instrChange)
{
    const Instrument* instr = instrChange->instrument();
    const Part* part = instrChange->part();
    const size_t partNr = muse::indexOf(m_score->parts(), part);
    const int instNr = muse::value(m_instrMap, instr, -1);
    const String longName = instr->nameAsPlainText();
    const String shortName = instr->abbreviatureAsPlainText();

    // Instrument changes could happen anywhere in the measure, so we close the initial attributes in any case
    m_attr.stop(m_xml);
    m_xml.startElement("print");
    if (!longName.isEmpty()) {
        m_xml.startElement("part-name-display");
        writeDisplayName(m_xml, longName);
        m_xml.endElement();
    }
    if (!shortName.isEmpty()) {
        m_xml.startElement("part-abbreviation-display");
        writeDisplayName(m_xml, shortName);
        m_xml.endElement();
    }
    m_xml.endElement();

    writeInstrumentDetails(instr, m_score->style().styleB(Sid::concertPitch));

    m_xml.startElement("sound");
    m_xml.startElement("instrument-change");
    scoreInstrument(m_xml, static_cast<int>(partNr) + 1, instNr + 1, instr->trackName(), instr);
    m_xml.endElement();
    m_xml.endElement();
}

//---------------------------------------------------------
//  writeInstrumentDetails
//---------------------------------------------------------

/**
 Write the instrument details for \a instrument.
 */

void ExportMusicXml::writeInstrumentDetails(const Instrument* instrument, const bool concertPitch)
{
    if (instrument->transpose().chromatic) {
        m_attr.doAttr(m_xml, true);
        if (concertPitch) {
            m_xml.startElement("for-part");
            m_xml.startElement("part-transpose");
        } else {
            m_xml.startElement("transpose");
        }
        m_xml.tag("diatonic",  instrument->transpose().diatonic % 7);
        m_xml.tag("chromatic", instrument->transpose().chromatic % 12);
        const int octaveChange = instrument->transpose().chromatic / 12;
        if (octaveChange) {
            m_xml.tag("octave-change", octaveChange);
        }
        m_xml.endElement();
        if (concertPitch) {
            m_xml.endElement();
        }
        m_attr.doAttr(m_xml, false);
    }
}

//---------------------------------------------------------
//  annotationsWithoutNote
//---------------------------------------------------------

/**
 Write the annotations that could not be attached to notes.
 */

static void annotationsWithoutNote(ExportMusicXml* exp, const track_idx_t strack, const int staves, const Measure* const measure)
{
    for (Segment* segment = measure->first(); segment; segment = segment->next()) {
        if (segment->segmentType() == SegmentType::ChordRest) {
            for (const EngravingItem* element : segment->annotations()) {
                if (!element->isFiguredBass() && !element->isHarmony()) {               // handled elsewhere
                    if (!exp->canWrite(element)) {
                        continue;
                    }
                    const track_idx_t wtrack = findTrackForAnnotations(element->track(), segment);           // track to write annotation
                    if (strack <= element->track() && element->track() < (strack + VOICES * staves) && wtrack == muse::nidx) {
                        commonAnnotations(exp, element, staves > 1 ? 1 : 0);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//  MeasureNumberStateHandler
//---------------------------------------------------------

MeasureNumberStateHandler::MeasureNumberStateHandler()
{
    init();
}

void MeasureNumberStateHandler::init()
{
    m_measureNo = 1;
    m_measureNoOffset = 0;
    m_irregularMeasureNo = 1;
    m_pickupMeasureNo = 1;
}

void MeasureNumberStateHandler::updateForMeasure(const Measure* const m)
{
    // restart measure numbering after a section break if startWithMeasureOne is set
    // check the previous MeasureBase instead of Measure to catch breaks in frames too
    const MeasureBase* previousMB = m->prev();
    if (previousMB) {
        previousMB = previousMB->findPotentialSectionBreak();
    }

    if (previousMB) {
        const LayoutBreak* layoutSectionBreak = previousMB->sectionBreakElement();
        if (layoutSectionBreak && layoutSectionBreak->startWithMeasureOne()) {
            init();
        }
    }

    // update measure numbers and cache result
    m_measureNoOffset = m->noOffset();
    m_measureNo += m_measureNoOffset;
    m_cachedAttributes = u" number=";
    if ((m_irregularMeasureNo + m_measureNo) == 2 && m->irregular()) {
        m_cachedAttributes += u"\"0\" implicit=\"yes\"";
        m_pickupMeasureNo++;
    } else if (m->irregular()) {
        m_cachedAttributes += String(u"\"X%1\" implicit=\"yes\"").arg(m_irregularMeasureNo++);
    } else {
        m_cachedAttributes += String(u"\"%1\"").arg(m_measureNo++);
    }
}

String MeasureNumberStateHandler::measureNumber() const
{
    return m_cachedAttributes;
}

bool MeasureNumberStateHandler::isFirstActualMeasure() const
{
    return (m_irregularMeasureNo + (m_measureNo - m_measureNoOffset) + m_pickupMeasureNo) == 4;
}

//---------------------------------------------------------
//  findLastSystemWithMeasures
//---------------------------------------------------------

static System* findLastSystemWithMeasures(const Page* const page)
{
    for (int i = static_cast<int>(page->systems().size()) - 1; i >= 0; --i) {
        System* const s = page->systems().at(i);
        const Measure* m = s->firstMeasure();
        if (m) {
            return s;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//  isFirstMeasureInSystem
//---------------------------------------------------------

static bool isFirstMeasureInSystem(const Measure* const measure)
{
    const System* system = measure->coveringMMRestOrThis()->system();
    const Measure* firstMeasureInSystem = system->firstMeasure();
    const Measure* realFirstMeasureInSystem = firstMeasureInSystem->isMMRest() ? firstMeasureInSystem->mmRestFirst() : firstMeasureInSystem;
    return measure == realFirstMeasureInSystem;
}

//---------------------------------------------------------
//  isFirstMeasureInLastSystem
//---------------------------------------------------------

static bool isFirstMeasureInLastSystem(const Measure* const measure)
{
    const System* system = measure->coveringMMRestOrThis()->system();
    const Page* page = system->page();

    /*
     Notes on multi-measure rest handling:
     Function mmRest1() returns either the measure itself (if not part of multi-measure rest)
     or the replacing multi-measure rest measure.
     Using this is required as a measure that is covered by a multi-measure rest has no system.
     Furthermore, the first measure in a system starting with a multi-measure rest is the a multi-
     measure rest itself instead of the first covered measure.
     */

    const System* lastSystem = findLastSystemWithMeasures(page);
    if (!lastSystem) {
        return false;           // degenerate case: no system with measures found
    }
    const Measure* firstMeasureInLastSystem = lastSystem->firstMeasure();
    const Measure* realFirstMeasureInLastSystem
        = firstMeasureInLastSystem->isMMRest() ? firstMeasureInLastSystem->mmRestFirst() : firstMeasureInLastSystem;
    return measure == realFirstMeasureInLastSystem;
}

//---------------------------------------------------------
//  systemHasMeasures
//---------------------------------------------------------

static bool systemHasMeasures(const System* const system)
{
    return system->firstMeasure();
}

//---------------------------------------------------------
//  findTextFramesToWriteAsWordsAbove
//---------------------------------------------------------

static std::vector<TBox*> findTextFramesToWriteAsWordsAbove(const Measure* const measure)
{
    const System* system = measure->coveringMMRestOrThis()->system();
    const Page* page = system->page();
    const size_t systemIndex = muse::indexOf(page->systems(), system);
    std::vector<TBox*> tboxes;
    if (isFirstMeasureInSystem(measure)) {
        for (int idx = static_cast<int>(systemIndex - 1); idx >= 0 && !systemHasMeasures(page->system(idx)); --idx) {
            const System* sys = page->system(idx);
            for (MeasureBase* mb : sys->measures()) {
                if (mb->isTBox()) {
                    TBox* const tbox = toTBox(mb);
                    tboxes.insert(tboxes.begin(), tbox);
                }
            }
        }
    }
    return tboxes;
}

//---------------------------------------------------------
//  findTextFramesToWriteAsWordsBelow
//---------------------------------------------------------

static std::vector<TBox*> findTextFramesToWriteAsWordsBelow(const Measure* const measure)
{
    const System* system = measure->coveringMMRestOrThis()->system();
    const Page* page = system->page();
    const size_t systemIndex = static_cast<int>(muse::indexOf(page->systems(), system));
    std::vector<TBox*> tboxes;
    if (isFirstMeasureInLastSystem(measure)) {
        for (size_t idx = systemIndex + 1; idx < page->systems().size() /* && !systemHasMeasures(page->system(idx))*/;
             ++idx) {
            const System* sys = page->system(static_cast<int>(idx));
            for (MeasureBase* mb : sys->measures()) {
                if (mb->isTBox()) {
                    TBox* const tbox = toTBox(mb);
                    tboxes.insert(tboxes.begin(), tbox);
                }
            }
        }
    }
    return tboxes;
}

//---------------------------------------------------------
//  writeMeasureTracks
//---------------------------------------------------------

/**
 Write data contained in the measure's tracks.
 */

void ExportMusicXml::writeMeasureTracks(const Measure* const m,
                                        const int partIndex,
                                        const staff_idx_t strack,
                                        const staff_idx_t partRelStaffNo,
                                        const bool useDrumset,
                                        const bool isLastStaffOfPart,
                                        FigBassMap& fbMap,
                                        std::set<const Spanner*>& spannersStopped)
{
    const auto tboxesAbove = findTextFramesToWriteAsWordsAbove(m);
    const auto tboxesBelow = findTextFramesToWriteAsWordsBelow(m);

    track_idx_t etrack = strack + VOICES;
    for (track_idx_t track = strack; track < etrack; ++track) {
        for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->isTimeTickType()) {
                // Prefer to start/stop spanners on a chordrest segment where one is available
                const Segment* crSeg = m_score->tick2leftSegment(seg->tick());
                if (crSeg && crSeg->tick() == seg->tick()) {
                    continue;
                }
                spannerStart(this, strack, etrack, track, partRelStaffNo, seg);

                const staff_idx_t spannerStaff = track2staff(track);
                const track_idx_t starttrack = staff2track(spannerStaff);
                const track_idx_t endtrack = staff2track(spannerStaff + 1);
                spannerStop(this, starttrack, endtrack, seg->tick(), partRelStaffNo, spannersStopped);

                // We check if there are additional annotations
                for (EngravingItem* annotation : seg->annotations()) {
                    if (annotation->track() != track || !annotation->isTextBase()) {
                        continue;
                    }
                    // Just to include them
                    annotations(this, strack, etrack, track, partRelStaffNo, seg);
                }
                continue;
            }
            EngravingItem* const el = seg->element(track);
            if (!el) {
                continue;
            }
            // must ignore start repeat to prevent spurious backup/forward
            if (el->isBarLine() && toBarLine(el)->barLineType() == BarLineType::START_REPEAT) {
                continue;
            }

            // generate backup or forward to the start time of the element
            moveToTickIfNeed(seg->tick());

            EngravingItem* ic = seg->findAnnotation(ElementType::INSTRUMENT_CHANGE, strack, etrack - 1);
            if (ic && (track == strack)) {
                const InstrumentChange* instrChange = toInstrumentChange(ic);
                writeInstrumentChange(instrChange);
            }

            // handle annotations and spanners (directions attached to this note or rest)
            if (el->isChordRest()) {
                m_attr.doAttr(m_xml, false);
                const bool isFirstPart = (partIndex == 0);
                const bool isLastPart = (partIndex == (static_cast<int>(m_score->parts().size()) - 1));
                if (!m_tboxesAboveWritten && isFirstPart) {
                    for (const TBox* tbox : tboxesAbove) {
                        // note: use mmRest1() to get at a possible multi-measure rest,
                        // as the covered measure would be positioned at 0,0.
                        tboxTextAsWords(tbox->text(), 0,
                                        muse::PointF(tbox->text()->canvasPos() - m->coveringMMRestOrThis()->canvasPos()));
                    }
                    m_tboxesAboveWritten = true;
                }
                if (!m_tboxesBelowWritten && isLastPart && isLastStaffOfPart) {
                    for (const TBox* tbox : tboxesBelow) {
                        const staff_idx_t lastStaffNr = track2staff(track);
                        const System* sys = m->coveringMMRestOrThis()->system();
                        PointF textPos = tbox->text()->canvasPos() - m->coveringMMRestOrThis()->canvasPos();
                        if (lastStaffNr < sys->staves().size()) {
                            // convert to position relative to last staff of system
                            textPos.setY(textPos.y() - (sys->staffCanvasYpage(lastStaffNr) - sys->staffCanvasYpage(0)));
                        }
                        tboxTextAsWords(tbox->text(), partRelStaffNo, textPos);
                    }
                    m_tboxesBelowWritten = true;
                }
                harmonies(this, track, seg);
                annotations(this, strack, etrack, track, partRelStaffNo, seg);
                figuredBass(m_xml, strack, etrack, track, static_cast<const ChordRest*>(el), fbMap, m_div);
                spannerStart(this, strack, etrack, track, partRelStaffNo, seg);
            }

            // write element el if necessary
            writeElement(el, m, partRelStaffNo, useDrumset);

            // handle annotations and spanners (directions attached to this note or rest)
            if (el->isChordRest()) {
                const staff_idx_t spannerStaff = track2staff(track);
                const track_idx_t starttrack = staff2track(spannerStaff);
                const track_idx_t endtrack = staff2track(spannerStaff + 1);
                spannerStop(this, starttrack, endtrack, m_tick, partRelStaffNo, spannersStopped);
            }
        }           // for (Segment* seg = ...
        m_attr.stop(m_xml);
    }         // for (int st = ...
}

//---------------------------------------------------------
//  writeMeasureStaves
//---------------------------------------------------------

/**
 Write each staff of a measure for a given part.
 */

void ExportMusicXml::writeMeasureStaves(const Measure* m,
                                        const int partIndex,
                                        const staff_idx_t startStaff,
                                        const size_t nstaves,
                                        const bool useDrumset,
                                        FigBassMap& fbMap,
                                        std::set<const Spanner*>& spannersStopped)
{
    const staff_idx_t endStaff = startStaff + nstaves;
    const Measure* const origM = m;
    m_tboxesAboveWritten = false;
    m_tboxesBelowWritten = false;
    m_measArpeggios.clear();

    for (staff_idx_t staffIdx = startStaff; staffIdx < endStaff; ++staffIdx) {
        // some staves may need to make m point somewhere else, so just in case, ensure start in same place
        IF_ASSERT_FAILED(m == origM) {
            return;
        }
        moveToTick(m->tick());

        staff_idx_t partRelStaffNo = (nstaves > 1 ? staffIdx - startStaff + 1 : 0); // xml staff number, counting from 1 for this instrument
        // special number 0 -> don’t show staff number in xml output
        // (because there is only one staff)

        // in presence of a MeasureRepeat, adjust m to point to the actual content being repeated
        if (m->isMeasureRepeatGroup(staffIdx)) {
            MeasureRepeat* mr = nullptr;
            while (m->isMeasureRepeatGroup(staffIdx) && m->prevMeasure()) { // keep going back until out of measure repeat groups
                mr = m->measureRepeatElement(staffIdx);
                IF_ASSERT_FAILED(mr) {
                    LOGE() << "Could not find MeasureRepeat on measure: " << m->index() << ", staff: " << staffIdx;
                    break;
                }

                for (int i = 0; i < mr->numMeasures() && m->prevMeasure(); ++i) {
                    m = m->prevMeasure();
                }
            }
        }
        // in case m was changed, also rewind _tick so as not to generate unnecessary backup/forward tags
        Fraction tickDelta = m_tick - m->tick();
        m_tick -= tickDelta;

        bool isLastStaffOfPart = (endStaff - 1 <= staffIdx); // for writing tboxes below

        writeMeasureTracks(m, partIndex, staff2track(staffIdx), partRelStaffNo, useDrumset, isLastStaffOfPart, fbMap, spannersStopped);

        // restore m and _tick before advancing to next staff in part
        m = origM;
        m_tick += tickDelta;
    }
}

//---------------------------------------------------------
//  writeMeasure
//---------------------------------------------------------

/**
 Write a measure.
 */

void ExportMusicXml::writeMeasure(const Measure* const m,
                                  const int partIndex,
                                  const int staffCount,
                                  MeasureNumberStateHandler& mnsh,
                                  FigBassMap& fbMap,
                                  const MeasurePrintContext& mpc,
                                  std::set<const Spanner*>& spannersStopped)
{
    const Part* part = m_score->parts().at(partIndex);
    const size_t staves = part->nstaves();
    const track_idx_t strack = part->startTrack();
    const track_idx_t etrack = part->endTrack();

    // pickup and other irregular measures need special care
    String measureTag = u"measure";
    mnsh.updateForMeasure(m);
    measureTag += mnsh.measureNumber();
    const bool isFirstActualMeasure = mnsh.isFirstActualMeasure();

    if (configuration()->exportLayout()) {
        measureTag += String(u" width=\"%1\"").arg(String::number(m->ldata()->bbox().width() / DPMM / m_millimeters * m_tenths, 2));
    }

    m_xml.startElementRaw(measureTag);

    print(m, partIndex, staffCount, staves, mpc);

    m_attr.start();

    findTrills(m, strack, etrack, m_trillStart, m_trillStop);

    // barline left must be the first element in a measure
    barlineLeft(m, strack);

    // output attributes with the first actual measure (pickup or regular)
    if (isFirstActualMeasure) {
        m_attr.doAttr(m_xml, true);
        m_xml.tag("divisions", m_div);
    }

    // output attributes at start of measure: key, time
    keysigTimesig(m, part);

    // output attributes with the first actual measure (pickup or regular) only
    if (isFirstActualMeasure) {
        if (staves > 1) {
            m_xml.tag("staves", static_cast<int>(staves));
        }

        if (m_instrMap.size() > 1) {
            m_xml.tag("instruments", m_instrMap.size());
        }
    }

    // make sure clefs at end of measure get exported at start of next measure
    findAndExportClef(m, static_cast<int>(staves), strack, etrack);

    // make sure a clef gets exported if none is found
    exportDefaultClef(part, m);

    // output attributes with the first actual measure (pickup or regular) only
    if (isFirstActualMeasure) {
        writeStaffDetails(m_xml, part, m_hiddenStaves);
        writeInstrumentDetails(part->instrument(), m_score->style().styleB(Sid::concertPitch));
    } else {
        for (size_t staffIdx : m_hiddenStaves) {
            m_attr.doAttr(m_xml, true);
            XmlWriter::Attributes attributes;
            if (staves > 1) {
                attributes.emplace_back(std::make_pair("number", staffIdx + 1));
            }
            attributes.emplace_back(std::make_pair("print-object", "no"));
            if (part->staff(staffIdx)->cutaway()) {
                attributes.emplace_back(std::make_pair("print-spacing", "yes"));
            }
            m_xml.tag("staff-details", attributes);
            m_attr.doAttr(m_xml, false);
        }
    }
    m_hiddenStaves.clear();

    // output attribute at start of measure: measure-style
    measureStyle(m_xml, m_attr, m, partIndex);

    // MuseScore limitation: repeats are always in the first part
    // and are implicitly placed at either measure start or stop
    if (partIndex == 0) {
        repeatAtMeasureStart(m_attr, m, strack, etrack, strack);
    }

    // write data in the staves
    writeMeasureStaves(m, partIndex, track2staff(strack), staves, part->instrument()->useDrumset(), fbMap, spannersStopped);

    // write the annotations that could not be attached to notes
    annotationsWithoutNote(this, strack, static_cast<int>(staves), m);

    // move to end of measure (in case of incomplete last voice)
       #ifdef DEBUG_TICK
    LOGD("end of measure");
       #endif
    moveToTick(m->endTick());
    if (partIndex == 0) {
        repeatAtMeasureStop(m, strack, etrack, strack);
    }
    // note: don't use "m->repeatFlags() & Repeat::END" here, because more
    // barline types need to be handled besides repeat end ("light-heavy")
    barlineRight(m, strack, etrack);
    m_xml.endElement();
}

//---------------------------------------------------------
//  measureWritten
//---------------------------------------------------------

void MeasurePrintContext::measureWritten(const Measure* m)
{
    scoreStart = false;
    pageStart = false;
    systemStart = false;
    prevMeasure = m;
}

//---------------------------------------------------------
//  writeParts
//---------------------------------------------------------

/**
 Write all parts.
 */

void ExportMusicXml::writeParts()
{
    int staffCount = 0;
    const auto& parts = m_score->parts();

    for (size_t partIndex = 0; partIndex < parts.size(); ++partIndex) {
        const Part* part = parts.at(partIndex);
        m_tick = { 0, 1 };
        m_xml.startElementRaw(String(u"part id=\"P%1\"").arg(partIndex + 1));

        m_trillStart.clear();
        m_trillStop.clear();
        initInstrMap(m_instrMap, part->instruments(), m_score);

        MeasureNumberStateHandler mnsh;
        FigBassMap fbMap;                     // pending figured bass extends

        // set of spanners already stopped in this part
        // required to prevent multiple spanner stops for the same spanner
        std::set<const Spanner*> spannersStopped;

        const auto& pages = m_score->pages();
        MeasurePrintContext mpc;

        for (size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
            const Page* page = pages.at(pageIndex);
            mpc.pageStart = true;
            const auto& systems = page->systems();

            for (int systemIndex = 0; systemIndex < static_cast<int>(systems.size()); ++systemIndex) {
                const System* system = systems.at(systemIndex);
                mpc.systemStart = true;

                for (const MeasureBase* mb : system->measures()) {
                    if (!mb->isMeasure()) {
                        continue;
                    }
                    const Measure* m = toMeasure(mb);

                    if (m->isMMRest()) {
                        // in case of a multimeasure rest (which is a single measure in MuseScore), write the measure range it replaces
                        const Measure* m2 = m->mmRestLast()->nextMeasure();
                        for (Measure* m1 = m->mmRestFirst(); m1 != m2; m1 = m1->nextMeasure()) {
                            if (m1->isMeasure()) {
                                writeMeasure(m1, static_cast<int>(partIndex), staffCount, mnsh, fbMap, mpc, spannersStopped);
                                mpc.measureWritten(m1);
                            }
                        }
                    } else {
                        // write the measure (or, if measure repeat, the "underlying" measure that it indicates for the musician to play)
                        writeMeasure(m, static_cast<int>(partIndex), staffCount, mnsh, fbMap, mpc, spannersStopped);
                        mpc.measureWritten(m);
                    }
                }
                mpc.prevSystem = system;
            }
            mpc.lastSystemPrevPage = mpc.prevSystem;
        }

        staffCount += static_cast<int>(part->nstaves());
        m_xml.endElement();
    }
}

//---------------------------------------------------------
//  findJumpElements
//---------------------------------------------------------

static std::vector<const Jump*> findJumpElements(const Score* score)
{
    std::vector<const Jump*> jumps;

    for (const MeasureBase* m = score->first(); m; m = m->next()) {
        for (const EngravingItem* e : m->el()) {
            if (e->isJump()) {
                jumps.push_back(toJump(e));
            }
        }
    }

    return jumps;
}

//---------------------------------------------------------
//  write
//---------------------------------------------------------

/**
 Write the score to \a dev in MusicXML format.
 */

void ExportMusicXml::write(muse::io::IODevice* dev)
{
    calcDivisions();

    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        m_brackets[i] = nullptr;
        m_dashes[i] = nullptr;
        m_hairpins[i] = nullptr;
        m_ottavas[i] = nullptr;
        m_trills[i] = nullptr;
    }

    m_jumpElements = findJumpElements(m_score);

    m_xml.setDevice(dev);
    m_xml.startDocument();
    m_xml.writeDoctype(
        u"score-partwise PUBLIC \"-//Recordare//DTD MusicXML 4.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\"");

    m_xml.startElement("score-partwise", { { "version", "4.0" } });

    work(m_score->measures()->first());
    identification(m_xml, m_score);

    if (configuration()->exportLayout()) {
        defaults(m_xml, m_score->style(), m_millimeters, m_tenths);
        credits(m_xml);
    } else if (m_score->style().styleB(Sid::concertPitch)) {
        m_xml.startElement("defaults");
        m_xml.tag("concert-score");
        m_xml.endElement();
    }

    partList(m_xml, m_score, m_instrMap);
    writeParts();

    m_xml.endElement();
}

//---------------------------------------------------------
//   saveXml
//    return false on error
//---------------------------------------------------------

/**
 Save Score as MusicXML file \a name.

 Return false on error.
 */

bool saveXml(Score* score, IODevice* device)
{
    muse::io::Buffer buf;
    buf.open(muse::io::IODevice::WriteOnly);
    ExportMusicXml em(score);
    em.write(&buf);
    device->write(buf.data());
    return true;
}

bool saveXml(Score* score, const String& name)
{
    File f(name);
    if (!f.open(IODevice::WriteOnly)) {
        return false;
    }

    bool res = saveXml(score, &f) && !f.hasError();
    f.close();
    return res;
}

//---------------------------------------------------------
//   saveMxl
//    return false on error
//---------------------------------------------------------

/**
 Save Score as compressed MusicXML file \a name.

 Return false on error.
 */

// META-INF/container.xml:
// <?xml version="1.0" encoding="UTF-8"?>
// <container>
//     <rootfiles>
//         <rootfile full-path="testHello.xml"/>
//     </rootfiles>
// </container>

static void writeMxlArchive(Score* score, muse::ZipWriter& zip, const String& filename)
{
    muse::io::Buffer cbuf;
    cbuf.open(muse::io::IODevice::ReadWrite);

    XmlWriter xml;
    xml.setDevice(&cbuf);
    xml.startDocument();
    xml.startElement("container");
    xml.startElement("rootfiles");
    xml.startElement("rootfile", { { "full-path", filename } });
    xml.endElement();
    xml.endElement();
    xml.endElement();
    cbuf.seek(0);

    zip.addFile("META-INF/container.xml", cbuf.data());

    muse::io::Buffer dbuf;
    dbuf.open(muse::io::IODevice::ReadWrite);
    ExportMusicXml em(score);
    em.write(&dbuf);
    dbuf.seek(0);
    zip.addFile(filename.toStdString(), dbuf.data());
}

bool saveMxl(Score* score, IODevice* device)
{
    muse::ZipWriter zip(device);

    //anonymized filename since we don't know the actual one here
    String fn = u"score.xml";
    writeMxlArchive(score, zip, fn);
    zip.close();

    return true;
}

bool saveMxl(Score* score, const String& name)
{
    muse::ZipWriter zip(name);

    FileInfo fi(name);
    String fn = fi.completeBaseName() + u".xml";
    writeMxlArchive(score, zip, fn);
    zip.close();

    return true;
}

double ExportMusicXml::getTenthsFromInches(double inches) const
{
    return inches * INCH / m_millimeters * m_tenths;
}

double ExportMusicXml::getTenthsFromDots(double dots) const
{
    return dots / DPMM / m_millimeters * m_tenths;
}

static void writeMusicXml(const FretDiagram* item, XmlWriter& xml)
{
    LOGD("FretDiagram::writeMusicXml() this %p harmony %p", item, item->harmony());
    XmlWriter::Attributes frameAttrs;
    addColorAttr(item, frameAttrs);
    xml.startElement("frame", frameAttrs);
    xml.tag("frame-strings", item->strings());
    xml.tag("frame-frets", item->frets());
    if (item->fretOffset() > 0) {
        xml.tag("first-fret", item->fretOffset() + 1);
    }

    for (int i = 0; i < item->strings(); ++i) {
        const int mxmlString = item->strings() - i;

        std::vector<int> bStarts;
        std::vector<int> bEnds;
        for (auto const& j : item->barres()) {
            FretItem::Barre b = j.second;
            const int fret = j.first;
            if (!b.exists()) {
                continue;
            }

            if (b.startString == i) {
                bStarts.push_back(fret);
            } else if (b.endString == i || (b.endString == -1 && mxmlString == 1)) {
                bEnds.push_back(fret);
            }
        }

        if (item->marker(i).exists() && item->marker(i).mtype == FretMarkerType::CIRCLE) {
            xml.startElement("frame-note");
            xml.tag("string", mxmlString);
            xml.tag("fret", "0");
            xml.endElement();
        }
        // Markers may exists alongside with dots
        // Write dots
        for (auto const& d : item->dot(i)) {
            if (!d.exists() || d.dtype == FretDotType::CROSS) {
                continue;
            }
            xml.startElement("frame-note");
            xml.tag("string", mxmlString);
            xml.tag("fret", d.fret + item->fretOffset());
            // TODO: write fingerings

            // Also write barre if it starts at this dot
            if (std::find(bStarts.begin(), bStarts.end(), d.fret) != bStarts.end()) {
                xml.tag("barre", { { "type", "start" } });
                bStarts.erase(std::remove(bStarts.begin(), bStarts.end(), d.fret), bStarts.end());
            }
            if (std::find(bEnds.begin(), bEnds.end(), d.fret) != bEnds.end()) {
                xml.tag("barre", { { "type", "stop" } });
                bEnds.erase(std::remove(bEnds.begin(), bEnds.end(), d.fret), bEnds.end());
            }
            xml.endElement();
        }

        // Write unwritten barres
        for (int j : bStarts) {
            xml.startElement("frame-note");
            xml.tag("string", mxmlString);
            xml.tag("fret", j);
            xml.tag("barre", { { "type", "start" } });
            xml.endElement();
        }

        for (int j : bEnds) {
            xml.startElement("frame-note");
            xml.tag("string", mxmlString);
            xml.tag("fret", j);
            xml.tag("barre", { { "type", "stop" } });
            xml.endElement();
        }
    }

    xml.endElement();
}

//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

void ExportMusicXml::harmony(Harmony const* const h, FretDiagram const* const fd, const Fraction& offset)
{
    // No supprt for polychords at the moment. Export the first chord from the list.
    HarmonyInfo* info = h->chords().empty() ? nullptr : h->chords().front();
    int rootTpc = info ? info->rootTpc() : Tpc::TPC_INVALID;
    int bassTpc = info ? info->bassTpc() : Tpc::TPC_INVALID;

    XmlWriter::Attributes harmonyAttrs;
    if (!h->isStyled(Pid::PLACEMENT)) {
        harmonyAttrs.emplace_back(std::make_pair("placement", (h->placement() == PlacementV::BELOW) ? "below" : "above"));
    }
    harmonyAttrs.emplace_back(std::make_pair("print-frame", h->hasFrame() ? "yes" : "no"));     // .append(relative));
    if (!h->visible()) {
        harmonyAttrs.emplace_back(std::make_pair("print-object", "no"));
    }
    addColorAttr(h, harmonyAttrs);
    m_xml.startElement("harmony", harmonyAttrs);
    if (h->harmonyType() == HarmonyType::STANDARD && tpcIsValid(rootTpc)) {
        m_xml.startElement("root");
        m_xml.tag("root-step", tpc2stepName(rootTpc));
        int alter = int(tpc2alter(rootTpc));
        if (alter) {
            m_xml.tag("root-alter", alter);
        }
        m_xml.endElement();

        const String xmlKind = harmonyXmlKind(info);

        if (!xmlKind.isEmpty()) {
            String s = u"kind";
            String kindText = harmonyXmlText(info);
            if (!harmonyXmlText(info).empty()) {
                s += u" text=\"" + kindText + u"\"";
            }
            if (harmonyXmlSymbols(info) == u"yes") {
                s += u" use-symbols=\"yes\"";
            }
            if (harmonyXmlParens(info) == u"yes") {
                s += u" parentheses-degrees=\"yes\"";
            }
            m_xml.tagRaw(s, xmlKind);

            if (bassTpc != Tpc::TPC_INVALID) {
                m_xml.startElement("bass");
                m_xml.tag("bass-step", tpc2stepName(bassTpc));
                alter = int(tpc2alter(bassTpc));
                if (alter) {
                    m_xml.tag("bass-alter", alter);
                }
                m_xml.endElement();
            }

            StringList l = harmonyXmlDegrees(info);
            if (!l.empty()) {
                for (const String& tag : l) {
                    String degreeText;
                    if (xmlKind.startsWith(u"suspended")
                        && tag.startsWith(u"add") && tag.at(3).isDigit()
                        && !kindText.isEmpty() && kindText.at(0).isDigit()) {
                        // hack to correct text for suspended chords whose kind text has degree information baked in
                        // (required by some other applications)
                        int tagDegree = tag.mid(3).toInt();
                        String kindTextExtension;
                        for (size_t i = 0; i < kindText.size() && kindText.at(i).isDigit(); ++i) {
                            kindTextExtension[i] = kindText[i];
                        }
                        int kindExtension = kindTextExtension.toInt();
                        if (tagDegree <= kindExtension && (tagDegree & 1) && (kindExtension & 1)) {
                            degreeText = u" text=\"\"";
                        }
                    }
                    m_xml.startElement("degree");
                    alter = 0;
                    int idx = 3;
                    if (tag[idx] == '#') {
                        alter = 1;
                        ++idx;
                    } else if (tag[idx] == 'b') {
                        alter = -1;
                        ++idx;
                    }
                    m_xml.tagRaw(String(u"degree-value%1").arg(degreeText), tag.mid(idx));
                    m_xml.tag("degree-alter", alter);               // finale insists on this even if 0
                    if (tag.startsWith(u"add")) {
                        m_xml.tagRaw(String(u"degree-type%1").arg(degreeText), "add");
                    } else if (tag.startsWith(u"sub")) {
                        m_xml.tag("degree-type", "subtract");
                    } else if (tag.startsWith(u"alt")) {
                        m_xml.tag("degree-type", "alter");
                    }
                    m_xml.endElement();
                }
            }
        } else {
            if (info->textName().empty()) {
                m_xml.tag("kind", "none");
            } else {
                m_xml.tag("kind", { { "text", info->textName() } }, "");
            }

            if (bassTpc != Tpc::TPC_INVALID) {
                m_xml.startElement("bass");
                m_xml.tag("bass-step", tpc2stepName(bassTpc));
                alter = int(tpc2alter(bassTpc));
                if (alter) {
                    m_xml.tag("bass-alter", alter);
                }
                m_xml.endElement();
            }
        }

        if (offset.isValid() && offset > Fraction(0, 1)) {
            m_xml.tag("offset", calculateDurationInDivisions(offset, m_div));
        } else {
            LOGD("invalid offset");
        }
        if (fd) {
            writeMusicXml(fd, m_xml);
        }
    } else {
        //
        // export an unrecognized Chord
        // which may contain arbitrary text
        //
        const String xmlKind = harmonyXmlKind(info);
        const String textName = info->textName();
        switch (h->harmonyType()) {
        case HarmonyType::NASHVILLE: {
            String alter;
            String functionText = harmonyXmlFunction(info, h);
            if (functionText.empty()) {
                // we just dump the text as deprecated function
                m_xml.tag("function", textName);
                m_xml.tag("kind", "none");
                break;
            } else if (!functionText.at(0).isDigit()) {
                alter = functionText.at(0);
                functionText = functionText.at(1);
            }
            m_xml.startElement("numeral");
            m_xml.tag("numeral-root", functionText);
            if (alter == u"b") {
                m_xml.tag("numeral-alter", "-1");
            } else if (alter == u"#") {
                m_xml.tag("numeral-alter", "1");
            }
            m_xml.endElement();
            if (!xmlKind.isEmpty()) {
                String s = u"kind";
                String kindText = harmonyXmlText(info);
                if (!harmonyXmlText(info).empty()) {
                    s += u" text=\"" + kindText + u"\"";
                }
                if (harmonyXmlSymbols(info) == "yes") {
                    s += u" use-symbols=\"yes\"";
                }
                if (harmonyXmlParens(info) == "yes") {
                    s += u" parentheses-degrees=\"yes\"";
                }
                m_xml.tagRaw(s, xmlKind);
            } else {
                // default is major
                m_xml.tag("kind", "major");
            }
        }
        break;
        case HarmonyType::ROMAN: {
            int alter = 0;
            static const std::wregex roman(L"(b|#)?([ivIV]+)");
            if (textName.contains(roman)) {
                StringList matches = textName.search(roman, { 1, 2 });
                m_xml.startElement("numeral");
                if (matches.at(0) == u"b") {
                    alter = -1;
                } else if (matches.at(0) == u"#") {
                    alter = 1;
                }
                const String numberStr = matches.at(1);
                size_t harmony = 1;
                if (numberStr.contains(u"v", CaseSensitivity::CaseInsensitive)) {
                    if (numberStr.startsWith(u"i", CaseSensitivity::CaseInsensitive)) {
                        harmony = 4;
                    } else {
                        harmony = 4 + numberStr.size();
                    }
                } else {
                    harmony = numberStr.size();
                }
                m_xml.tag("numeral-root", { { "text", numberStr } }, harmony);
                if (alter) {
                    m_xml.tag("numeral-alter", alter);
                }
                m_xml.endElement();
                // simple check for major or minor
                m_xml.tag("kind", numberStr.at(0).isUpper() ? "major" : "minor");
                // infer inversion from ending digits
                if (textName.endsWith(u"64")) {
                    m_xml.tag("inversion", 2);
                } else if (textName.endsWith(u"6")) {
                    m_xml.tag("inversion", 1);
                }
                break;
            }
        }
        // fallthrough
        case HarmonyType::STANDARD:
        default: {
            m_xml.startElement("root");
            m_xml.tag("root-step", { { "text", "" } }, "C");
            m_xml.endElement();                   // root
            m_xml.tag("kind", { { "text", textName } }, "none");
        }
        break;
        }
    }
    m_xml.endElement();
}

//---------------------------------------------------------
//  canWrite
//---------------------------------------------------------

/**
 Whether a tag corresponding to the given element \p e
 should be included to the exported MusicXML file.
 */

bool ExportMusicXml::canWrite(const EngravingItem* e)
{
    return e->visible() || configuration()->exportInvisibleElements();
}
}
