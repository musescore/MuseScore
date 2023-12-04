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

#include "exportxml.h"

#include <math.h>
#include <QString>
#include <QBuffer>
#include <QDate>
#include <QRegularExpression>

#include "containers.h"
#include "io/iodevice.h"
#include "io/buffer.h"
#include "io/fileinfo.h"
#include "global/deprecated/qzipwriter_p.h"

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
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmonicmark.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/letring.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/page.h"
#include "engraving/dom/palmmute.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/pickscrape.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rasgueado.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textlinebase.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolo.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/undo.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/whammybar.h"

#include "musicxml.h"
#include "musicxmlfonthandler.h"
#include "musicxmlsupport.h"

#include "modularity/ioc.h"
#include "../../imusicxmlconfiguration.h"
#include "engraving/iengravingconfiguration.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::iex::musicxml;
using namespace mu::engraving;

namespace mu::engraving {
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

//---------------------------------------------------------
//   typedefs
//---------------------------------------------------------

typedef QMap<track_idx_t, const FiguredBass*> FigBassMap;

//---------------------------------------------------------
//   attributes -- prints <attributes> tag when necessary
//---------------------------------------------------------

class Attributes
{
    bool inAttributes;

public:
    Attributes() { start(); }
    void doAttr(XmlWriter& xml, bool attr);
    void start();
    void stop(XmlWriter& xml);
};

//---------------------------------------------------------
//   doAttr - when necessary change state and print <attributes> tag
//---------------------------------------------------------

void Attributes::doAttr(XmlWriter& xml, bool attr)
{
    if (!inAttributes && attr) {
        xml.startElement("attributes");
        inAttributes = true;
    } else if (inAttributes && !attr) {
        xml.endElement();
        inAttributes = false;
    }
}

//---------------------------------------------------------
//   start -- initialize
//---------------------------------------------------------

void Attributes::start()
{
    inAttributes = false;
}

//---------------------------------------------------------
//   stop -- print </attributes> tag when necessary
//---------------------------------------------------------

void Attributes::stop(XmlWriter& xml)
{
    if (inAttributes) {
        xml.endElement();
        inAttributes = false;
    }
}

//---------------------------------------------------------
//   notations -- prints <notations> tag when necessary
//---------------------------------------------------------

class Notations
{
    bool notationsPrinted = false;
    bool prevElementVisible = true;

public:
    void tag(XmlWriter& xml, const EngravingItem* e);
    void etag(XmlWriter& xml);
};

//---------------------------------------------------------
//   articulations -- prints <articulations> tag when necessary
//---------------------------------------------------------

class Articulations
{
    bool articulationsPrinted;

public:
    Articulations() { articulationsPrinted = false; }
    void tag(XmlWriter& xml);
    void etag(XmlWriter& xml);
};

//---------------------------------------------------------
//   ornaments -- prints <ornaments> tag when necessary
//---------------------------------------------------------

class Ornaments
{
    bool ornamentsPrinted;

public:
    Ornaments() { ornamentsPrinted = false; }
    void tag(XmlWriter& xml);
    void etag(XmlWriter& xml);
};

//---------------------------------------------------------
//   technical -- prints <technical> tag when necessary
//---------------------------------------------------------

class Technical
{
    bool technicalPrinted;

public:
    Technical() { technicalPrinted = false; }
    void tag(XmlWriter& xml);
    void etag(XmlWriter& xml);
};

//---------------------------------------------------------
//   slur handler -- prints <slur> tags
//---------------------------------------------------------

class SlurHandler
{
    const Slur* slur[MAX_NUMBER_LEVEL];
    bool started[MAX_NUMBER_LEVEL];
    int findSlur(const Slur* s) const;

public:
    SlurHandler();
    void doSlurs(const ChordRest* chordRest, Notations& notations, XmlWriter& xml);

private:
    void doSlurStart(const Slur* s, Notations& notations, XmlWriter& xml);
    void doSlurStop(const Slur* s, Notations& notations, XmlWriter& xml);
};

//---------------------------------------------------------
//   glissando handler -- prints <glissando> tags
//---------------------------------------------------------

class GlissandoHandler
{
    const Note* glissNote[MAX_NUMBER_LEVEL];
    const Note* slideNote[MAX_NUMBER_LEVEL];
    int findNote(const Note* note, int type) const;

public:
    GlissandoHandler();
    void doGlissandoStart(Glissando* gliss, Notations& notations, XmlWriter& xml);
    void doGlissandoStop(Glissando* gliss, Notations& notations, XmlWriter& xml);
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
    QString measureNumber() const;
    bool isFirstActualMeasure() const;
private:
    void init();
    int _measureNo;                             // number of next regular measure
    int _irregularMeasureNo;                    // number of next irregular measure
    int _pickupMeasureNo;                       // number of next pickup measure
    QString _cachedAttributes;                  // attributes calculated by updateForMeasure()
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

typedef QHash<const ChordRest* const, const Trill*> TrillHash;
typedef QMap<const Instrument*, int> MxmlInstrumentMap;

class ExportMusicXml
{
    INJECT_STATIC(mu::iex::musicxml::IMusicXmlConfiguration, configuration)

    Score* _score;
    XmlWriter _xml;
    SlurHandler sh;
    GlissandoHandler gh;
    Fraction _tick;
    Attributes _attr;
    TextLineBase const* brackets[MAX_NUMBER_LEVEL];
    TextLineBase const* dashes[MAX_NUMBER_LEVEL];
    Hairpin const* hairpins[MAX_NUMBER_LEVEL];
    Ottava const* ottavas[MAX_NUMBER_LEVEL];
    Trill const* trills[MAX_NUMBER_LEVEL];
    std::vector<const Jump*> _jumpElements;
    int div;
    double millimeters;
    int tenths;
    bool _tboxesAboveWritten;
    bool _tboxesBelowWritten;
    TrillHash _trillStart;
    TrillHash _trillStop;
    MxmlInstrumentMap instrMap;

    int findBracket(const TextLineBase* tl) const;
    int findDashes(const TextLineBase* tl) const;
    int findHairpin(const Hairpin* tl) const;
    int findOttava(const Ottava* tl) const;
    int findTrill(const Trill* tl) const;
    void chord(Chord* chord, staff_idx_t staff, const std::vector<Lyrics*>& ll, bool useDrumset);
    void rest(Rest* chord, staff_idx_t staff, const std::vector<Lyrics*>& ll);
    void clef(staff_idx_t staff, const ClefType ct, const QString& extraAttributes = "");
    void timesig(TimeSig* tsig);
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
    void wavyLineStartStop(const ChordRest* const cr, Notations& notations, Ornaments& ornaments, TrillHash& trillStart,
                           TrillHash& trillStop);
    void print(const Measure* const m, const int partNr, const int firstStaffOfPart, const int nrStavesInPart,
               const MeasurePrintContext& mpc);
    void findAndExportClef(const Measure* const m, const int staves, const track_idx_t strack, const track_idx_t etrack);
    void exportDefaultClef(const Part* const part, const Measure* const m);
    void writeElement(EngravingItem* el, const Measure* m, staff_idx_t sstaff, bool useDrumset);
    void writeMeasureTracks(const Measure* const m, const int partIndex, const staff_idx_t strack, const staff_idx_t partRelStaffNo,
                            const bool useDrumset, const bool isLastStaffOfPart, FigBassMap& fbMap, QSet<const Spanner*>& spannersStopped);
    void writeMeasureStaves(const Measure* m, const int partIndex, const staff_idx_t startStaff, const size_t nstaves,
                            const bool useDrumset, FigBassMap& fbMap, QSet<const Spanner*>& spannersStopped);
    void writeMeasure(const Measure* const m, const int idx, const int staffCount, MeasureNumberStateHandler& mnsh, FigBassMap& fbMap,
                      const MeasurePrintContext& mpc, QSet<const Spanner*>& spannersStopped);
    void repeatAtMeasureStart(Attributes& attr, const Measure* const m, track_idx_t strack, track_idx_t etrack, track_idx_t track);
    void repeatAtMeasureStop(const Measure* const m, track_idx_t strack, track_idx_t etrack, track_idx_t track);
    void writeParts();

    static QString fermataPosition(const Fermata* const fermata);
    static QString elementPosition(const ExportMusicXml* const expMxml, const EngravingItem* const elm);
    static QString positioningAttributes(EngravingItem const* const el, bool isSpanStart = true);
    static QString positioningAttributesForTboxText(const QPointF position, float spatium);
    static void identification(XmlWriter& xml, Score const* const score);

public:
    ExportMusicXml(Score* s)
    {
        _score = s;
        _tick = { 0, 1 };
        div = 1;
        tenths = 40;
        millimeters = _score->style().spatium() * tenths / (10 * DPMM);
    }

    void write(mu::io::IODevice* dev);
    void credits(XmlWriter& xml);
    void moveToTick(const Fraction& t);
    void words(TextBase const* const text, staff_idx_t staff);
    void tboxTextAsWords(TextBase const* const text, const staff_idx_t staff, QPointF position);
    void rehearsal(RehearsalMark const* const rmk, staff_idx_t staff);
    void hairpin(Hairpin const* const hp, staff_idx_t staff, const Fraction& tick);
    void ottava(Ottava const* const ot, staff_idx_t staff, const Fraction& tick);
    void pedal(Pedal const* const pd, staff_idx_t staff, const Fraction& tick);
    void textLine(TextLineBase const* const tl, staff_idx_t staff, const Fraction& tick);
    void dynamic(Dynamic const* const dyn, staff_idx_t staff);
    void symbol(Symbol const* const sym, staff_idx_t staff);
    void tempoText(TempoText const* const text, staff_idx_t staff);
    void harmony(Harmony const* const, FretDiagram const* const fd, int offset = 0);
    Score* score() const { return _score; }
    double getTenthsFromInches(double) const;
    double getTenthsFromDots(double) const;
    Fraction tick() const { return _tick; }
    void writeInstrumentDetails(const Instrument* instrument);

    static bool canWrite(const EngravingItem* e);
};

//---------------------------------------------------------
//   positionToQString
//---------------------------------------------------------

static QString positionToQString(const QPointF def, const QPointF rel, const float spatium)
{
    // minimum value to export
    const float positionElipson = 0.1f;

    // convert into tenths for MusicXML
    const float defaultX =  10 * def.x() / spatium;
    const float defaultY =  -10 * def.y() / spatium;
    const float relativeX =  10 * rel.x() / spatium;
    const float relativeY =  -10 * rel.y() / spatium;

    // generate string representation
    QString res;
    if (fabsf(defaultX) > positionElipson) {
        res += QString(" default-x=\"%1\"").arg(QString::number(defaultX, 'f', 2));
    }
    if (fabsf(defaultY) > positionElipson) {
        res += QString(" default-y=\"%1\"").arg(QString::number(defaultY, 'f', 2));
    }
    if (fabsf(relativeX) > positionElipson) {
        res += QString(" relative-x=\"%1\"").arg(QString::number(relativeX, 'f', 2));
    }
    if (fabsf(relativeY) > positionElipson) {
        res += QString(" relative-y=\"%1\"").arg(QString::number(relativeY, 'f', 2));
    }

    return res;
}

//---------------------------------------------------------
//   positioningAttributes
//   According to the specs (common.dtd), all direction-type and note elements must be relative to the measure
//   while all other elements are relative to their position or the nearest note.
//---------------------------------------------------------

QString ExportMusicXml::positioningAttributes(EngravingItem const* const el, bool isSpanStart)
{
    if (!configuration()->musicxmlExportLayout()) {
        return "";
    }

    //LOGD("single el %p _pos x,y %f %f _userOff x,y %f %f spatium %f",
    //       el, el->ldata()->pos.x(), el->ldata()->pos.y(), el->offset().x(), el->offset().y(), el->spatium());

    QPointF def;
    QPointF rel;
    float spatium = el->spatium();

    const SLine* span = nullptr;
    if (el->isSLine()) {
        span = static_cast<const SLine*>(el);
    }

    if (span && !span->segmentsEmpty()) {
        if (isSpanStart) {
            const auto seg = span->frontSegment();
            const auto offset = seg->offset();
            const auto p = seg->pos();
            rel.setX(offset.x());
            def.setY(p.y());

            //LOGD("sline start seg %p seg->pos x,y %f %f seg->userOff x,y %f %f spatium %f",
            //       seg, p.x(), p.y(), seg->offset().x(), seg->offset().y(), seg->spatium());
        } else {
            const auto seg = span->backSegment();
            const auto userOff = seg->offset();       // This is the offset accessible from the inspector
            const auto userOff2 = seg->userOff2();       // Offset of the actual dragged anchor, which doesn't affect the inspector offset
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
        def = el->ldata()->pos().toQPointF();       // Note: for some elements, Finale Notepad seems to work slightly better w/o default-x
        rel = el->offset().toQPointF();
    }

    return positionToQString(def, rel, spatium);
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Notations::tag(XmlWriter& xml, const EngravingItem* e)
{
    if (notationsPrinted && prevElementVisible != e->visible()) {
        etag(xml);
    }

    if (!notationsPrinted) {
        if (e->visible()) {
            xml.startElement("notations");
        } else {
            xml.startElement("notations", { { "print-object", "no" } });
        }
    }

    notationsPrinted = true;
    prevElementVisible = e->visible();
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Notations::etag(XmlWriter& xml)
{
    if (notationsPrinted) {
        xml.endElement();
    }
    notationsPrinted = false;
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Articulations::tag(XmlWriter& xml)
{
    if (!articulationsPrinted) {
        xml.startElement("articulations");
    }
    articulationsPrinted = true;
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Articulations::etag(XmlWriter& xml)
{
    if (articulationsPrinted) {
        xml.endElement();
    }
    articulationsPrinted = false;
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Ornaments::tag(XmlWriter& xml)
{
    if (!ornamentsPrinted) {
        xml.startElement("ornaments");
    }
    ornamentsPrinted = true;
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Ornaments::etag(XmlWriter& xml)
{
    if (ornamentsPrinted) {
        xml.endElement();
    }
    ornamentsPrinted = false;
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Technical::tag(XmlWriter& xml)
{
    if (!technicalPrinted) {
        xml.startElement("technical");
    }
    technicalPrinted = true;
}

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Technical::etag(XmlWriter& xml)
{
    if (technicalPrinted) {
        xml.endElement();
    }
    technicalPrinted = false;
}

static std::shared_ptr<mu::engraving::IEngravingConfiguration> engravingConfiguration()
{
    return mu::modularity::ioc()->resolve<mu::engraving::IEngravingConfiguration>("iex_musicxml");
}

//---------------------------------------------------------
//   color2xml
//---------------------------------------------------------

/**
 Return \a el color.
 */

static QString color2xml(const EngravingItem* el)
{
    if (el->color() != engravingConfiguration()->defaultColor()) {
        return QString(" color=\"%1\"").arg(QString::fromStdString(el->color().toString()).toUpper());
    } else if (el->isSLine() && ((SLine*)el)->lineColor() != engravingConfiguration()->defaultColor()) {
        return QString(" color=\"%1\"").arg(QString::fromStdString(((SLine*)el)->lineColor().toString()).toUpper());
    } else {
        return "";
    }
}

static void addColorAttr(const EngravingItem* el, XmlWriter::Attributes& attrs)
{
    if (el->color() != engravingConfiguration()->defaultColor()) {
        attrs.push_back({ "color", QString::fromStdString(el->color().toString()).toUpper() });
    }
}

//---------------------------------------------------------
//   fontStyleToXML
//---------------------------------------------------------

static QString fontStyleToXML(const FontStyle style, bool allowUnderline = true)
{
    QString res;
    if (style & FontStyle::Bold) {
        res += " font-weight=\"bold\"";
    }
    if (style & FontStyle::Italic) {
        res += " font-style=\"italic\"";
    }
    if (allowUnderline && style & FontStyle::Underline) {
        res += " underline=\"1\"";
    }
    // at places where underline is not wanted (e.g. fingering, pluck), strike is not wanted too
    if (allowUnderline && style & FontStyle::Strike) {
        res += " line-through=\"1\"";
    }
    return res;
}

//---------------------------------------------------------
//   slurHandler
//---------------------------------------------------------

SlurHandler::SlurHandler()
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        slur[i] = 0;
        started[i] = false;
    }
}

static QString slurTieLineStyle(const SlurTie* s)
{
    QString lineType;
    QString rest;
    switch (s->styleType()) {
    case SlurStyleType::Dashed:
    case SlurStyleType::WideDashed:
        lineType = "dashed";
        break;
    case SlurStyleType::Dotted:
        lineType = "dotted";
        break;
    case SlurStyleType::Solid:
    default:
        lineType = "";
    }
    if (!lineType.isEmpty()) {
        rest = QString(" line-type=\"%1\"").arg(lineType);
    }
    if (s->slurDirection() != engraving::DirectionV::AUTO) {
        rest += QString(" orientation=\"%1\"").arg(s->up() ? "over" : "under");
        rest += QString(" placement=\"%1\"").arg(s->up() ? "above" : "below");
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
        if (slur[i] == s) {
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

    const auto c1 = static_cast<const Chord*>(e1);
    const auto c2 = static_cast<const Chord*>(e2);

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
            auto sp = it.second;
            if (sp->generated() || sp->type() != ElementType::SLUR || !ExportMusicXml::canWrite(sp)) {
                continue;
            }
            if (chordRest == sp->startElement() || chordRest == sp->endElement()) {
                const auto s = static_cast<const Slur*>(sp);
                const auto firstChordRest = findFirstChordRest(s);
                if (firstChordRest) {
                    if (i == 0) {
                        // first time: do slur stops
                        if (firstChordRest != chordRest) {
                            doSlurStop(s, notations, xml);
                        }
                    } else {
                        // second time: do slur starts
                        if (firstChordRest == chordRest) {
                            doSlurStart(s, notations, xml);
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

void SlurHandler::doSlurStart(const Slur* s, Notations& notations, XmlWriter& xml)
{
    // check if on slur list (i.e. stop already seen)
    int i = findSlur(s);
    // compose tag
    QString tagName = "slur";
    tagName += QString(" type=\"start\"");
    tagName += slurTieLineStyle(s);
    tagName += ExportMusicXml::positioningAttributes(s, true);

    if (i >= 0) {
        // remove from list and print start
        slur[i] = 0;
        started[i] = false;
        notations.tag(xml, s);
        tagName += QString(" number=\"%1\"").arg(i + 1);
        xml.tagRaw(tagName);
    } else {
        // find free slot to store it
        i = findSlur(0);
        if (i >= 0) {
            slur[i] = s;
            started[i] = true;
            notations.tag(xml, s);
            tagName += QString(" number=\"%1\"").arg(i + 1);
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

void SlurHandler::doSlurStop(const Slur* s, Notations& notations, XmlWriter& xml)
{
    // check if on slur list
    int i = findSlur(s);
    if (i < 0) {
        // if not, find free slot to store it
        i = findSlur(0);
        if (i >= 0) {
            slur[i] = s;
            started[i] = false;
            notations.tag(xml, s);
            QString tagName = QString("slur type=\"stop\" number=\"%1\"").arg(i + 1);
            tagName += ExportMusicXml::positioningAttributes(s, false);
            xml.tagRaw(tagName);
        } else {
            LOGD("no free slur slot");
        }
    } else {
        // found (already started), stop it and remove from list
        slur[i] = 0;
        started[i] = false;
        notations.tag(xml, s);
        QString tagName = QString("slur type=\"stop\" number=\"%1\"").arg(i + 1);
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
    GlissandoType st = gli->glissandoType();
    QString tagName;
    switch (st) {
    case GlissandoType::STRAIGHT:
        tagName = "slide line-type=\"solid\"";
        break;
    case GlissandoType::WAVY:
        tagName = "glissando line-type=\"wavy\"";
        break;
    default:
        LOGD("unknown glissando subtype %d", int(st));
        return;
        break;
    }
    tagName += QString(" number=\"%1\" type=\"%2\"").arg(number).arg(start ? "start" : "stop");
    if (start) {
        tagName += color2xml(gli);
        tagName += ExportMusicXml::positioningAttributes(gli, start);
    }
    notations.tag(xml, gli);
    if (start && gli->showText() && gli->text() != "") {
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
        glissNote[i] = 0;
        slideNote[i] = 0;
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
        if (type == 0 && slideNote[i] == note) {
            return i;
        }
        if (type == 1 && glissNote[i] == note) {
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
            slideNote[i] = 0;
        }
        if (type == GlissandoType::WAVY) {
            glissNote[i] = 0;
        }
    }
    // find free slot to store it
    i = findNote(0, int(type));
    if (i >= 0) {
        if (type == GlissandoType::STRAIGHT) {
            slideNote[i] = note;
        }
        if (type == GlissandoType::WAVY) {
            glissNote[i] = note;
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
        if (type == GlissandoType::STRAIGHT && slideNote[i] == note) {
            slideNote[i] = 0;
            glissando(gliss, i + 1, false, notations, xml);
            return;
        }
        if (type == GlissandoType::WAVY && glissNote[i] == note) {
            glissNote[i] = 0;
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
    auto stick = measure->tick();
    auto etick = measure->tick() + measure->ticks();
    for (auto it = measure->score()->spanner().lower_bound(stick.ticks());
         it != measure->score()->spanner().upper_bound(etick.ticks()); ++it) {
        auto e = it->second;
        //LOGD("1 trill %p type %d track %d tick %s", e, e->type(), e->track(), qPrintable(e->tick().print()));
        if (e->isTrill() && ExportMusicXml::canWrite(e) && strack <= e->track() && e->track() < etrack
            && e->tick() >= measure->tick() && e->tick() < (measure->tick() + measure->ticks())) {
            //LOGD("2 trill %p", e);
            // a trill is found starting in this segment, trill end time is known
            // determine notes to write trill start and stop

            const auto tr = toTrill(e);
            auto elem1 = tr->startElement();
            auto elem2 = tr->endElement();

            if (elem1 && elem1->isChordRest() && elem2 && elem2->isChordRest()) {
                trillStart.insert(toChordRest(elem1), tr);
                trillStop.insert(toChordRest(elem2), tr);
            }
        }
    }
}

//---------------------------------------------------------
// helpers for ::calcDivisions
//---------------------------------------------------------

typedef QList<int> IntVector;
static IntVector integers;
static IntVector primes;

// check if all integers can be divided by d

static bool canDivideBy(int d)
{
    bool res = true;
    for (int i = 0; i < integers.count(); i++) {
        if ((integers[i] <= 1) || ((integers[i] % d) != 0)) {
            res = false;
        }
    }
    return res;
}

// divide all integers by d

static void divideBy(int d)
{
    for (int i = 0; i < integers.count(); i++) {
        integers[i] /= d;
    }
}

static void addInteger(int len)
{
    if (len > 0 && !integers.contains(len)) {
        integers.append(len);
    }
}

//---------------------------------------------------------
//   calcDivMoveToTick
//---------------------------------------------------------

void ExportMusicXml::calcDivMoveToTick(const Fraction& t)
{
    if (t < _tick) {
#ifdef DEBUG_TICK
        LOGD("backup %d", (tick - t).ticks());
#endif
        addInteger((_tick - t).ticks());
    } else if (t > _tick) {
#ifdef DEBUG_TICK
        LOGD("forward %d", (t - tick).ticks());
#endif
        addInteger((t - _tick).ticks());
    }
    _tick = t;
}

//---------------------------------------------------------
// isTwoNoteTremolo - determine is chord is part of two note tremolo
//---------------------------------------------------------

static bool isTwoNoteTremolo(Chord* chord)
{
    return chord->tremolo() && chord->tremolo()->twoNotes();
}

//---------------------------------------------------------
//  calcDivisions
//---------------------------------------------------------

// Loop over all voices in all staves and determine a suitable value for divisions.

// Length of time in MusicXML is expressed in "units", which should allow expressing all time values
// as an integral number of units. Divisions contains the number of units in a quarter note.
// MuseScore uses division (480) midi ticks to represent a quarter note, which expresses all note values
// plus triplets and quintuplets as integer values. Solution is to collect all time values required,
// and divide them by the highest common denominator, which is implemented as a series of
// divisions by prime factors. Initialize the list with division to make sure a quarter note can always
// be written as an integral number of units.

/**
 */

void ExportMusicXml::calcDivisions()
{
    // init
    integers.clear();
    primes.clear();
    integers.append(Constants::DIVISION);
    primes.append(2);
    primes.append(3);
    primes.append(5);

    const std::vector<Part*>& il = _score->parts();

    for (size_t idx = 0; idx < il.size(); ++idx) {
        Part* part = il.at(idx);
        _tick = { 0, 1 };

        size_t staves = part->nstaves();
        track_idx_t strack = _score->staffIdx(part) * VOICES;
        track_idx_t etrack = strack + staves * VOICES;

        for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
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
                            addInteger(fb->ticks().ticks());
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

                    if (_tick != seg->tick()) {
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
                        LOGD("chordrest tick %d duration %d", _tick.ticks(), l.ticks());
#endif
                        addInteger(l.ticks());
                        _tick += l;
                    }
                }
            }
            // move to end of measure (in case of incomplete last voice)
            calcDivMoveToTick(m->endTick());
        }
    }

    // do it: divide by all primes as often as possible
    for (int u = 0; u < primes.count(); u++) {
        while (canDivideBy(primes[u])) {
            divideBy(primes[u]);
        }
    }

    div = Constants::DIVISION / integers[0];
#ifdef DEBUG_TICK
    LOGD("divisions=%d div=%d", integers[0], div);
#endif
}

//---------------------------------------------------------
//   writePageFormat
//---------------------------------------------------------

static void writePageFormat(const MStyle& s, XmlWriter& xml, double conversion)
{
    xml.startElement("page-layout");

    xml.tag("page-height", s.styleD(Sid::pageHeight) * conversion);
    xml.tag("page-width", s.styleD(Sid::pageWidth) * conversion);

    QString type("both");
    if (s.styleB(Sid::pageTwosided)) {
        type = "even";
        xml.startElement("page-margins", { { "type", type } });
        xml.tag("left-margin",   s.styleD(Sid::pageEvenLeftMargin) * conversion);
        xml.tag("right-margin",  s.styleD(Sid::pageOddLeftMargin) * conversion);
        xml.tag("top-margin",    s.styleD(Sid::pageEvenTopMargin) * conversion);
        xml.tag("bottom-margin", s.styleD(Sid::pageEvenBottomMargin) * conversion);
        xml.endElement();
        type = "odd";
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
        xml.tag("line-width", { { "type", "slur middle" } }, s.styleS(Sid::SlurMidWidth).val() * 10);
        xml.tag("line-width", { { "type", "slur tip" } }, s.styleS(Sid::SlurEndWidth).val() * 10);
        xml.tag("line-width", { { "type", "staff" } }, s.styleS(Sid::staffLineWidth).val() * 10);
        xml.tag("line-width", { { "type", "stem" } }, s.styleS(Sid::stemWidth).val() * 10);
        xml.tag("line-width", { { "type", "tie middle" } }, s.styleS(Sid::SlurMidWidth).val() * 10);
        xml.tag("line-width", { { "type", "tie tip" } }, s.styleS(Sid::SlurEndWidth).val() * 10);
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

    xml.tag("music-font", { { "font-family", s.styleSt(Sid::MusicalSymbolFont) } });
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
                        const double x, const double y, const QString& just, const QString& val,
                        const std::list<TextFragment>& words, const QString& creditType)
{
    // prevent incorrect MusicXML for empty text
    if (words.empty()) {
        return;
    }

    const QString mtf = s.styleSt(Sid::MusicalTextFont);
    const CharFormat defFmt = formatForWords(s);

    // export formatted
    xml.startElement("credit", { { "page", pageNr } });
    if (creditType != "") {
        xml.tag("credit-type", creditType);
    }
    QString attr = QString(" default-x=\"%1\"").arg(x);
    attr += QString(" default-y=\"%1\"").arg(y);
    attr += " justify=\"" + just + "\"";
    attr += " valign=\"" + val + "\"";
    MScoreTextToMXML mttm("credit-words", attr, defFmt, mtf);
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

static QString tidToCreditType(const TextStyleType tid)
{
    QString res;
    switch (tid) {
    case TextStyleType::COMPOSER:
        res = "composer";
        break;
    case TextStyleType::LYRICIST:
        res = "lyricist";
        break;
    case TextStyleType::SUBTITLE:
        res = "subtitle";
        break;
    case TextStyleType::TITLE:
        res = "title";
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
    QString just;
    QString val;

    if (al == AlignH::RIGHT) {
        just = "right";
        tx   = w - rm;
    } else if (al == AlignH::HCENTER) {
        just = "center";
        // tx already set correctly
    } else {
        just = "left";
        tx   = lm;
    }

    if (al == AlignV::BOTTOM) {
        val = "bottom";
        ty -= ph;
    } else if (al == AlignV::VCENTER) {
        val = "middle";
        ty -= ph / 2;
    } else if (al == AlignV::BASELINE) {
        val = "baseline";
        ty -= ph / 2;
    } else {
        val = "top";
        // ty already set correctly
    }

    const QString creditType= tidToCreditType(text->textStyleType());

    creditWords(xml, s, pageNr, tx, ty, just, val, text->fragmentList(), creditType);
}

//---------------------------------------------------------
//   credits
//---------------------------------------------------------

void ExportMusicXml::credits(XmlWriter& xml)
{
    // find the vboxes in every page and write their elements as credit-words
    for (const auto page : _score->pages()) {
        const auto pageIdx = _score->pageIdx(page);
        for (const auto system : page->systems()) {
            for (const auto mb : system->measures()) {
                if (mb->isVBox()) {
                    for (const EngravingItem* element : mb->el()) {
                        if (element->isText()) {
                            const Text* text = toText(element);
                            textAsCreditWords(this, xml, _score->style(), static_cast<int>(pageIdx) + 1, text);
                        }
                    }
                }
            }
        }
    }

    // put copyright at the bottom center of every page
    // note: as the copyright metatag contains plain text, special XML characters must be escaped
    // determine page formatting
    const QString rights = _score->metaTag(u"copyright");
    if (!rights.isEmpty()) {
        const MStyle& style = _score->style();
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
        f.changeFormat(FormatId::FontFamily, style.styleSt(Sid::footerFontFace).toQString());
        f.changeFormat(FormatId::FontSize, style.styleD(Sid::footerFontSize));
        std::list<TextFragment> list;
        list.push_back(f);
        for (page_idx_t pageIdx = 0; pageIdx < _score->npages(); ++pageIdx) {
            creditWords(xml, style, pageIdx + 1, w / 2, bm, "center", "bottom", list, "rights");
        }
    }
}

//---------------------------------------------------------
//   midipitch2xml
//---------------------------------------------------------

static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char noteTab[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

static void midipitch2xml(int pitch, char& c, int& alter, int& octave)
{
    // 60 = C 4
    c      = noteTab[pitch % 12];
    alter  = alterTab[pitch % 12];
    octave = pitch / 12 - 1;
    //LOGD("midipitch2xml(pitch %d) step %c, alter %d, octave %d", pitch, c, alter, octave);
}

//---------------------------------------------------------
//   tabpitch2xml
//---------------------------------------------------------

static void tabpitch2xml(const int pitch, const int tpc, QString& s, int& alter, int& octave)
{
    s      = tpc2stepName(tpc);
    alter  = tpc2alterByKey(tpc, Key::C);
    octave = (pitch - alter) / 12 - 1;
    if (alter < -2 || 2 < alter) {
        LOGD("tabpitch2xml(pitch %d, tpc %d) problem:  step %s, alter %d, octave %d",
             pitch, tpc, qPrintable(s), alter, octave);
    }
    /*
    else
          LOGD("tabpitch2xml(pitch %d, tpc %d) step %s, alter %d, octave %d",
                 pitch, tpc, qPrintable(s), alter, octave);
     */
}

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

// TODO validation

static void pitch2xml(const Note* note, QString& s, int& alter, int& octave)
{
    const auto st = note->staff();
    const auto tick = note->tick();
    const auto instr = st->part()->instrument(tick);
    const auto intval = instr->transpose();

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
    //       note->pitch(), note->tpc(), ottava, clef, qPrintable(s), alter, octave);
}

// unpitch2xml -- calculate display-step and display-octave for an unpitched note
// note:
// even though this produces the correct step/octave according to Recordare's tutorial
// Finale Notepad 2012 does not import a three line staff with percussion clef correctly
// Same goes for Sibelius 6 in case of three or five line staff with percussion clef

static void unpitch2xml(const Note* note, QString& s, int& octave)
{
    static char table1[]  = "FEDCBAG";

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
    s               = table1[stepIdx];
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

static QString tick2xml(const Fraction& ticks, int* dots)
{
    TDuration t(ticks);
    *dots = t.dots();
    if (ticks == Fraction(0, 1)) {
        t.setType(DurationType::V_MEASURE);
        *dots = 0;
    }
    return TConv::toXml(t.type()).ascii();
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
    QString number = "";
    QString type = "";
    for (int i : v->endings()) {
        if (!number.isEmpty()) {
            number += ", ";
        }
        number += QString("%1").arg(i);
    }
    if (left) {
        type = "start";
    } else {
        Volta::Type st = v->voltaType();
        switch (st) {
        case Volta::Type::OPEN:
            type = "discontinue";
            break;
        case Volta::Type::CLOSED:
            type = "stop";
            break;
        default:
            LOGD("unknown volta subtype %d", int(st));
            return;
        }
    }
    QString voltaXml = QString("ending number=\"%1\" type=\"%2\"").arg(number, type);
    voltaXml += ExportMusicXml::positioningAttributes(v, left);
    if (left) {
        if (!v->visible()) {
            voltaXml += " print-object=\"no\"";
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
    _attr.doAttr(_xml, false);
    _xml.startElement("barline", { { "location", "left" } });
    if (rs) {
        _xml.tag("bar-style", QString("heavy-light"));
    }
    if (volta) {
        ending(_xml, volta, true);
    }
    if (rs) {
        _xml.tag("repeat", { { "direction", "forward" } });
    }
    _xml.endElement();
}

//---------------------------------------------------------
//   shortBarlineStyle -- recognize normal but shorter barline styles
//---------------------------------------------------------

static QString shortBarlineStyle(const BarLine* bl)
{
    if (bl->barLineType() == BarLineType::NORMAL && !bl->spanStaff()) {
        if (bl->spanTo() < 0) {
            // lowest point of barline above lowest staff line
            if (bl->spanFrom() < 0) {
                return "tick";               // highest point of barline above highest staff line
            } else {
                return "short";               // highest point of barline below highest staff line
            }
        }
    }

    return "";
}

//---------------------------------------------------------
//   normalBarlineStyle -- recognize other barline styles
//---------------------------------------------------------

static QString normalBarlineStyle(const BarLine* bl)
{
    const auto bst = bl->barLineType();

    switch (bst) {
    case BarLineType::NORMAL:
        return "regular";
    case BarLineType::DOUBLE:
        return "light-light";
    case BarLineType::REVERSE_END:
        return "heavy-light";
    case BarLineType::BROKEN:
        return "dashed";
    case BarLineType::DOTTED:
        return "dotted";
    case BarLineType::END:
    case BarLineType::END_REPEAT:
    case BarLineType::END_START_REPEAT:
        return "light-heavy";
    case BarLineType::HEAVY:
        return "heavy";
    case BarLineType::DOUBLE_HEAVY:
        return "heavy-heavy";
    default:
        LOGD("bar subtype %d not supported", int(bst));
    }

    return "";
}

//---------------------------------------------------------
//   barlineMiddle -- handle barline middle
//---------------------------------------------------------

void ExportMusicXml::barlineMiddle(const BarLine* bl)
{
    auto vis = bl->visible();
    auto shortStyle = shortBarlineStyle(bl);
    auto normalStyle = normalBarlineStyle(bl);
    QString barStyle;
    if (!vis) {
        barStyle = "none";
    } else if (shortStyle != "") {
        barStyle = shortStyle;
    } else {
        barStyle = normalStyle;
    }

    if (barStyle != "") {
        _xml.startElement("barline", { { "location", "middle" } });
        _xml.tag("bar-style", barStyle);
        _xml.endElement();
    }
}

//---------------------------------------------------------
//   fermataPosition -  return fermata y position as MusicXML string
//---------------------------------------------------------

QString ExportMusicXml::fermataPosition(const Fermata* const fermata)
{
    QString res;

    if (configuration()->musicxmlExportLayout()) {
        constexpr qreal SPATIUM2TENTHS = 10;
        constexpr qreal EPSILON = 0.01;
        const auto spatium = fermata->spatium();
        const auto defY = -1 * SPATIUM2TENTHS * fermata->ldata()->pos().y() / spatium;
        const auto relY = -1 * SPATIUM2TENTHS * fermata->offset().y() / spatium;

        if (qAbs(defY) >= EPSILON) {
            res += QString(" default-y=\"%1\"").arg(QString::number(defY, 'f', 2));
        }
        if (qAbs(relY) >= EPSILON) {
            res += QString(" relative-y=\"%1\"").arg(QString::number(relY, 'f', 2));
        }
    }

    return res;
}

//---------------------------------------------------------
//   fermata - write a fermata
//---------------------------------------------------------

static void fermata(const Fermata* const a, XmlWriter& xml)
{
    QString tagName = "fermata";
    tagName += QString(" type=\"%1\"").arg(a->placement() == PlacementV::ABOVE ? "upright" : "inverted");
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
        for (const auto anno : seg->annotations()) {
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
        for (const auto anno : seg->annotations()) {
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
    QString color = "";

    bool needBarStyle = (bst != BarLineType::NORMAL && bst != BarLineType::START_REPEAT) || !visible;
    Volta* volta = findVolta(m, false, strack);
    // detect short and tick barlines
    QString special = "";
    const BarLine* bl = m->endBarLine();
    if (bl) {
        color = color2xml(bl);
        if (bst == BarLineType::NORMAL && !bl->spanStaff()) {
            if (bl->spanFrom() == BARLINE_SPAN_TICK1_FROM && bl->spanTo() == BARLINE_SPAN_TICK1_TO) {
                special = "tick";
            }
            if (bl->spanFrom() == BARLINE_SPAN_TICK2_FROM && bl->spanTo() == BARLINE_SPAN_TICK2_TO) {
                special = "tick";
            }
            if (bl->spanFrom() == BARLINE_SPAN_SHORT1_FROM && bl->spanTo() == BARLINE_SPAN_SHORT1_TO) {
                special = "short";
            }
            if (bl->spanFrom() == BARLINE_SPAN_SHORT2_FROM && bl->spanTo() == BARLINE_SPAN_SHORT2_FROM) {
                special = "short";
            }
        }
    }

    // check fermata
    // no need to take mmrest into account, MS does not create mmrests for measure with fermatas
    const auto hasFermata = barlineHasFermata(m->endBarLine(), strack, etrack);

    if (!needBarStyle && !volta && special.isEmpty() && !hasFermata && color.isEmpty()) {
        return;
    }

    _xml.startElement("barline", { { "location", "right" } });
    QString tagName = "bar-style";
    tagName += color;
    if (needBarStyle) {
        if (!visible) {
            _xml.tagRaw(tagName, "none");
        } else {
            switch (bst) {
            case BarLineType::DOUBLE:
                _xml.tagRaw(tagName, "light-light");
                break;
            case BarLineType::REVERSE_END:
                _xml.tagRaw(tagName, "heavy-light");
                break;
            case BarLineType::BROKEN:
                _xml.tagRaw(tagName, "dashed");
                break;
            case BarLineType::DOTTED:
                _xml.tagRaw(tagName, "dotted");
                break;
            case BarLineType::END:
            case BarLineType::END_REPEAT:
            case BarLineType::END_START_REPEAT:
                _xml.tagRaw(tagName, "light-heavy");
                break;
            case BarLineType::HEAVY:
                _xml.tagRaw(tagName, "heavy");
                break;
            case BarLineType::DOUBLE_HEAVY:
                _xml.tagRaw(tagName, "heavy-heavy");
                break;
            default:
                LOGD("ExportMusicXml::bar(): bar subtype %d not supported", int(bst));
                break;
            }
        }
    } else if (!special.isEmpty()) {
        _xml.tagRaw(tagName, special);
    } else if (!color.isEmpty()) {
        _xml.tagRaw(tagName, "regular");
    }

    writeBarlineFermata(m->endBarLine(), _xml, strack, etrack);

    if (volta) {
        ending(_xml, volta, false);
    }

    if (bst == BarLineType::END_REPEAT || bst == BarLineType::END_START_REPEAT) {
        if (m->repeatCount() > 2) {
            _xml.tag("repeat", { { "direction", "backward" }, { "times", m->repeatCount() } });
        } else {
            _xml.tag("repeat", { { "direction", "backward" } });
        }
    }

    _xml.endElement();
}

//---------------------------------------------------------
//   calculateTimeDeltaInDivisions
//---------------------------------------------------------

static int calculateTimeDeltaInDivisions(const Fraction& t1, const Fraction& t2, const int divisions)
{
    return (t1 - t2).ticks() / divisions;
}

//---------------------------------------------------------
//   moveToTick
//---------------------------------------------------------

void ExportMusicXml::moveToTick(const Fraction& t)
{
    //LOGD("ExportMusicXml::moveToTick(t=%s) _tick=%s", qPrintable(t.print()), qPrintable(_tick.print()));
    if (t < _tick) {
#ifdef DEBUG_TICK
        LOGD(" -> backup");
#endif
        _attr.doAttr(_xml, false);
        _xml.startElement("backup");
        _xml.tag("duration", calculateTimeDeltaInDivisions(_tick, t, div));
        _xml.endElement();
    } else if (t > _tick) {
#ifdef DEBUG_TICK
        LOGD(" -> forward");
#endif
        _attr.doAttr(_xml, false);
        _xml.startElement("forward");
        _xml.tag("duration", calculateTimeDeltaInDivisions(t, _tick, div));
        _xml.endElement();
    }
    _tick = t;
}

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

void ExportMusicXml::timesig(TimeSig* tsig)
{
    TimeSigType st = tsig->timeSigType();
    Fraction ts = tsig->sig();
    int z = ts.numerator();
    int n = ts.denominator();
    QString ns = tsig->numeratorString();

    _attr.doAttr(_xml, true);
    XmlWriter::Attributes attrs;
    if (st == TimeSigType::FOUR_FOUR) {
        attrs = { { "symbol", "common" } };
    } else if (st == TimeSigType::ALLA_BREVE) {
        attrs = { { "symbol", "cut" } };
    }
    if (!tsig->visible()) {
        attrs.push_back({ "print-object", "no" });
    }

    addColorAttr(tsig, attrs);

    _xml.startElement("time", attrs);

    QRegularExpression regex(QRegularExpression::anchoredPattern("^\\d+(\\+\\d+)+$")); // matches a compound numerator
    if (regex.match(ns).hasMatch()) {
        // if compound numerator, exported as is
        _xml.tag("beats", ns);
    } else {
        // else fall back and use the numerator as integer
        _xml.tag("beats", z);
    }
    _xml.tag("beat-type", n);
    _xml.endElement();
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
    static char table2[]  = "CDEFGAB";
    int po = ClefInfo::pitchOffset(ct);   // actually 7 * oct + step for topmost staff line
    //LOGD("keysig st %d key %d custom %d ct %hhd st %d", staff, ks->key(), ks->isCustom(), ct, staff);
    //LOGD(" pitch offset clef %d stp %d oct %d ", po, po % 7, po / 7);

    XmlWriter::Attributes attrs;
    if (staff) {
        attrs.push_back({ "number", staff });
    }
    if (!visible) {
        attrs.push_back({ "print-object", "no" });
    }
    addColorAttr(ks, attrs);

    _attr.doAttr(_xml, true);
    _xml.startElement("key", attrs);

    //! NOTE It looks like there is some kind of problem here,
    //! layout data should not be used to write to a file or export
    const KeySig::LayoutData* ldata = ks->ldata();
    const std::vector<KeySym>& keysyms = ldata->keySymbols;
    if (ks->isCustom() && !ks->isAtonal() && keysyms.size() > 0) {
        // non-traditional key signature
        // MusicXML order is left-to-right order, while KeySims in keySymbols()
        // are in insertion order -> sorting required

        // first put the KeySyms in a map
        QMap<qreal, KeySym> map;
        for (const KeySym& ksym : keysyms) {
            map.insert(ksym.xPos, ksym);
        }
        // then write them (automatically sorted on key)
        for (const KeySym& ksym : map) {
            int step = (po - ksym.line) % 7;
            //LOGD(" keysym sym %d -> line %d step %d", ksym.sym, ksym.line, step);
            _xml.tag("key-step", QString(QChar(table2[step])));
            _xml.tag("key-alter", accSymId2alter(ksym.sym));
            XmlWriter::Attributes accidentalAttrs;
            QString s = accSymId2MxmlString(ksym.sym);
            if (s == "other") {
                accidentalAttrs = { { "smufl", accSymId2SmuflMxmlString(ksym.sym) } };
            }
            _xml.tag("key-accidental", accidentalAttrs, s);
        }
    } else {
        // traditional key signature
        _xml.tag("fifths", static_cast<int>(ks->key()));
        switch (ks->mode()) {
        case KeyMode::NONE:       _xml.tag("mode", "none");
            break;
        case KeyMode::MAJOR:      _xml.tag("mode", "major");
            break;
        case KeyMode::MINOR:      _xml.tag("mode", "minor");
            break;
        case KeyMode::DORIAN:     _xml.tag("mode", "dorian");
            break;
        case KeyMode::PHRYGIAN:   _xml.tag("mode", "phrygian");
            break;
        case KeyMode::LYDIAN:     _xml.tag("mode", "lydian");
            break;
        case KeyMode::MIXOLYDIAN: _xml.tag("mode", "mixolydian");
            break;
        case KeyMode::AEOLIAN:    _xml.tag("mode", "aeolian");
            break;
        case KeyMode::IONIAN:     _xml.tag("mode", "ionian");
            break;
        case KeyMode::LOCRIAN:    _xml.tag("mode", "locrian");
            break;
        case KeyMode::UNKNOWN:              // fall thru
        default:
            if (ks->isCustom()) {
                _xml.tag("mode", "none");
            }
        }
    }
    _xml.endElement();
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

void ExportMusicXml::clef(staff_idx_t staff, const ClefType ct, const QString& extraAttributes)
{
    clefDebug("ExportMusicXml::clef(staff %zu, clef %hhd)", staff, ct);

    QString tagName = "clef";
    if (staff) {
        tagName += QString(" number=\"%1\"").arg(static_cast<int>(staff));
    }
    tagName += extraAttributes;
    _attr.doAttr(_xml, true);
    _xml.startElementRaw(tagName);

    MusicXmlClefInfo info = findClefInfoByType(ct);

    int line = ClefInfo::line(ct);
    _xml.tag("sign", info.sign);
    _xml.tag("line", line);
    if (info.octChng) {
        _xml.tag("clef-octave-change", info.octChng);
    }
    _xml.endElement();
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
    for (const auto el : t->elements()) {
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
    const auto t = el->tuplet();
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
    const auto t = el->tuplet();
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

static void tupletTypeAndDots(const QString& type, const int dots, XmlWriter& xml)
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
    const auto s = tick2xml(t->baseLen().ticks(), &dots);
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
    QString tupletTag = "tuplet type=\"start\"";
    if (!isSimpleTuplet(t)) {
        tupletTag += QString(" number=\"%1\"").arg(number);
    }
    tupletTag += " bracket=";
    tupletTag += t->hasBracket() ? "\"yes\"" : "\"no\"";
    if (t->numberType() == TupletNumberType::SHOW_RELATION) {
        tupletTag += " show-number=\"both\"";
    } else if (t->numberType() == TupletNumberType::NO_TEXT) {
        tupletTag += " show-number=\"none\"";
    }
    if (t->direction() != engraving::DirectionV::AUTO) {
        if (t->direction() == engraving::DirectionV::UP) {
            tupletTag += " placement=\"above\"";
        } else if (t->direction() == engraving::DirectionV::DOWN) {
            tupletTag += " placement=\"below\"";
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
        tupletAttrs.push_back({ "number", number });
    }
    xml.tag("tuplet", tupletAttrs);
}

//---------------------------------------------------------
//   tupletStartStop
//---------------------------------------------------------

static void tupletStartStop(ChordRest* cr, Notations& notations, XmlWriter& xml)
{
    const auto nesting = tupletNesting(cr);
    bool doActualAndNormal = (nesting > 1);
    if (cr->isChord() && isTwoNoteTremolo(toChord(cr))) {
        doActualAndNormal = true;
    }
    for (int level = nesting - 1; level >= 0; --level) {
        const auto startTuplet = startTupletAtLevel(cr, level + 1);
        if (startTuplet) {
            tupletStart(startTuplet, nesting - level, doActualAndNormal, notations, xml);
        }
        const auto stopTuplet = stopTupletAtLevel(cr, level + 1);
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
        if (trills[i] == tr) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   writeAccidental
//---------------------------------------------------------

static void writeAccidental(XmlWriter& xml, const QString& tagName, const Accidental* const acc)
{
    if (acc) {
        QString s = accidentalType2MxmlString(acc->accidentalType());
        if (s != "") {
            XmlWriter::Attributes attrs;
            if (s == "other") {
                attrs = { { "smufl", accidentalType2SmuflMxmlString(acc->accidentalType()) } };
            }
            QString tag = tagName;
            if (acc->bracket() == AccidentalBracket::BRACKET) {
                attrs.emplace_back(std::make_pair("bracket", "yes"));
            } else if (acc->bracket() == AccidentalBracket::PARENTHESIS) {
                attrs.emplace_back(std::make_pair("parentheses", "yes"));
            }
            if (tagName == "accidental-mark") {
                if (acc->placeAbove()) {
                    attrs.emplace_back(std::make_pair("placement", "above"));
                } else if (acc->placeBelow()) {
                    attrs.emplace_back(std::make_pair("placement", "below"));
                }
            }
            addColorAttr(acc, attrs);
            xml.tag(AsciiStringView(tag.toStdString()), attrs, s);
        }
    }
}

//---------------------------------------------------------
//   wavyLineStart
//---------------------------------------------------------

static void wavyLineStart(const Trill* tr, const int number, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
{
    // mscore only supports wavy-line with trill-mark
    notations.tag(xml, tr);
    ornaments.tag(xml);
    xml.tagRaw("trill-mark" + color2xml(tr));
    writeAccidental(xml, "accidental-mark", tr->accidental());
    QString tagName = "wavy-line type=\"start\"";
    tagName += QString(" number=\"%1\"").arg(number + 1);
    tagName += color2xml(tr);
    tagName += ExportMusicXml::positioningAttributes(tr, true);
    xml.tagRaw(tagName);
}

//---------------------------------------------------------
//   wavyLineStop
//---------------------------------------------------------

static void wavyLineStop(const Trill* tr, const int number, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
{
    notations.tag(xml, tr);
    ornaments.tag(xml);
    QString trillXml = QString("wavy-line type=\"stop\" number=\"%1\"").arg(number + 1);
    trillXml += ExportMusicXml::positioningAttributes(tr, false);
    xml.tagRaw(trillXml);
}

//---------------------------------------------------------
//   wavyLineStartStop
//---------------------------------------------------------

void ExportMusicXml::wavyLineStartStop(const ChordRest* const cr, Notations& notations, Ornaments& ornaments,
                                       TrillHash& trillStart, TrillHash& trillStop)
{
    if (trillStart.contains(cr) && trillStop.contains(cr)) {
        const auto tr = trillStart.value(cr);
        auto n = findTrill(0);
        if (n >= 0) {
            wavyLineStart(tr, n, notations, ornaments, _xml);
            wavyLineStop(tr, n, notations, ornaments, _xml);
        } else {
            LOGD("too many overlapping trills (cr %p staff %zu tick %d)",
                 cr, cr->staffIdx(), cr->tick().ticks());
        }
    } else {
        if (trillStop.contains(cr)) {
            const auto tr = trillStop.value(cr);
            auto n = findTrill(tr);
            if (n >= 0) {
                // trill stop after trill start
                trills[n] = 0;
            } else {
                // trill stop before trill start
                n = findTrill(0);
                if (n >= 0) {
                    trills[n] = tr;
                } else {
                    LOGD("too many overlapping trills (cr %p staff %zu tick %d)",
                         cr, cr->staffIdx(), cr->tick().ticks());
                }
            }
            if (n >= 0) {
                wavyLineStop(tr, n, notations, ornaments, _xml);
            }
            trillStop.remove(cr);
        }
        if (trillStart.contains(cr)) {
            const auto tr = trillStart.value(cr);
            auto n = findTrill(tr);
            if (n >= 0) {
                LOGD("wavyLineStartStop error");
            } else {
                n = findTrill(0);
                if (n >= 0) {
                    trills[n] = tr;
                    wavyLineStart(tr, n, notations, ornaments, _xml);
                } else {
                    LOGD("too many overlapping trills (cr %p staff %zu tick %d)",
                         cr, cr->staffIdx(), cr->tick().ticks());
                }
                trillStart.remove(cr);
            }
        }
    }
}

//---------------------------------------------------------
//   tremoloSingleStartStop
//---------------------------------------------------------

static void tremoloSingleStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
{
    Tremolo* tr = chord->tremolo();

    if (tr && ExportMusicXml::canWrite(tr)) {
        int count = 0;
        TremoloType st = tr->tremoloType();
        QString type = "";

        if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
            type = "single";
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
        } else if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
            type = "start";
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
        } else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
            type = "stop";
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

        if (type != "" && count > 0) {
            notations.tag(xml, tr);
            ornaments.tag(xml);
            XmlWriter::Attributes attrs = { { "type", type } };
            if (type == "single" || type == "start") {
                addColorAttr(tr, attrs);
            }
            xml.tag("tremolo", attrs, count);
        }
    }
}

//---------------------------------------------------------
//   fermatas
//---------------------------------------------------------

static void fermatas(const QVector<EngravingItem*>& cra, XmlWriter& xml, Notations& notations)
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

static std::vector<QString> symIdToArtic(const SymId sid)
{
    switch (sid) {
    case SymId::articAccentAbove:
    case SymId::articAccentBelow:
        return { "accent" };
        break;

    case SymId::articStaccatoAbove:
    case SymId::articStaccatoBelow:
        return { "staccato" };
        break;

    case SymId::articStaccatissimoAbove:
    case SymId::articStaccatissimoBelow:
        return { "staccatissimo" };
        break;

    case SymId::articTenutoAbove:
    case SymId::articTenutoBelow:
        return { "tenuto" };
        break;

    case SymId::articMarcatoAbove:
    case SymId::articMarcatoBelow:
        return { "strong-accent" };
        break;

    case SymId::articTenutoStaccatoAbove:
    case SymId::articTenutoStaccatoBelow:
        return { "detached-legato" };
        break;

    case SymId::articSoftAccentAbove:
    case SymId::articSoftAccentBelow:
        return { "soft-accent" };
        break;

    case SymId::articSoftAccentStaccatoAbove:
    case SymId::articSoftAccentStaccatoBelow:
        return { "soft-accent", "staccato" };
        break;

    case SymId::articSoftAccentTenutoAbove:
    case SymId::articSoftAccentTenutoBelow:
        return { "soft-accent", "tenuto" };
        break;

    case SymId::articSoftAccentTenutoStaccatoAbove:
    case SymId::articSoftAccentTenutoStaccatoBelow:
        return { "soft-accent", "detached-legato" };
        break;

    case SymId::articStressAbove:
    case SymId::articStressBelow:
        return { "stress" };
        break;

    case SymId::articUnstressAbove:
    case SymId::articUnstressBelow:
        return { "unstress" };
        break;

    case SymId::articAccentStaccatoAbove:
    case SymId::articAccentStaccatoBelow:
        return { "accent", "staccato" };
        break;

    case SymId::articMarcatoStaccatoAbove:
    case SymId::articMarcatoStaccatoBelow:
        return { "strong-accent", "staccato" };
        break;

    case SymId::articMarcatoTenutoAbove:
    case SymId::articMarcatoTenutoBelow:
        return { "strong-accent", "tenuto" };
        break;

    case SymId::articTenutoAccentAbove:
    case SymId::articTenutoAccentBelow:
        return { "tenuto", "accent" };
        break;

    default:
        ;                 // nothing
        break;
    }

    return {};
}

//---------------------------------------------------------
//   symIdToOrnam
//---------------------------------------------------------

static QString symIdToOrnam(const SymId sid)
{
    switch (sid) {
    case SymId::ornamentTurnInverted:
        return "inverted-turn";
        break;
    case SymId::ornamentTurnSlash:
        return "turn slash=\"yes\"";
        break;
    case SymId::ornamentTurn:
        return "turn";
        break;
    case SymId::ornamentTrill:
    case SymId::ornamentShake3:
    case SymId::ornamentShakeMuffat1:
        return "trill-mark";
        break;
    case SymId::ornamentMordent:
        return "mordent";
        break;
    case SymId::ornamentShortTrill:
        // return "short-trill";
        return "inverted-mordent";
        break;
    case SymId::ornamentTremblement:
        return "inverted-mordent long=\"yes\"";
        break;
    case SymId::ornamentPrallMordent:
        return "mordent long=\"yes\"";
        break;
    case SymId::ornamentUpPrall:
        return "inverted-mordent long=\"yes\" approach=\"below\"";
        break;
    case SymId::ornamentPrecompMordentUpperPrefix:
        return "inverted-mordent long=\"yes\" approach=\"above\"";
        break;
    case SymId::ornamentUpMordent:
        return "mordent long=\"yes\" approach=\"below\"";
        break;
    case SymId::ornamentDownMordent:
        return "mordent long=\"yes\" approach=\"above\"";
        break;
    case SymId::ornamentPrallDown:
        return "inverted-mordent long=\"yes\" departure=\"below\"";
        break;
    case SymId::ornamentPrallUp:
        return "inverted-mordent long=\"yes\" departure=\"above\"";
        break;
    case SymId::ornamentLinePrall:
        // MusicXML 3.0 does not distinguish between downprall and lineprall
        return "inverted-mordent long=\"yes\" approach=\"above\"";
        break;
    case SymId::ornamentPrecompSlide:
        return "schleifer";
        break;
    case SymId::ornamentTremblementCouperin:
        return "other-ornament smufl=\"ornamentTremblementCouperin\"";
        break;
    case SymId::ornamentPinceCouperin:
        return "other-ornament smufl=\"ornamentPinceCouperin\"";
        break;

    default:
        ;           // nothing
        break;
    }

    return "";
}

//---------------------------------------------------------
//   symIdToTechn
//---------------------------------------------------------

static QString symIdToTechn(const SymId sid)
{
    switch (sid) {
    case SymId::stringsUpBow:
        return "up-bow";
        break;
    case SymId::stringsDownBow:
        return "down-bow";
        break;
    case SymId::stringsHarmonic:
        return "harmonic";
        break;
    case SymId::stringsThumbPosition:
    case SymId::stringsThumbPositionTurned:
        return "thumb-position";
        break;
    case SymId::doubleTongueAbove:
    case SymId::doubleTongueBelow:
        return "double-tongue";
        break;
    case SymId::tripleTongueAbove:
    case SymId::tripleTongueBelow:
        return "triple-tongue";
        break;
    case SymId::brassMuteClosed:
        return "stopped";
        break;
    case SymId::pluckedSnapPizzicatoAbove:
    case SymId::pluckedSnapPizzicatoBelow:
        return "snap-pizzicato";
        break;
    case SymId::keyboardPedalHeel1:
    case SymId::keyboardPedalHeel2:
    case SymId::keyboardPedalHeel3:
        return "heel";
        break;
    case SymId::keyboardPedalToe1:
    case SymId::keyboardPedalToe2:
        return "toe";
        break;
    case SymId::pluckedWithFingernails:
        return "fingernails";
        break;
    case SymId::brassBend:
        return "brass-bend";
        break;
    case SymId::brassFlip:
        return "brass-flip";
        break;
    case SymId::brassSmear:
        return "smear";
        break;
    case SymId::brassMuteOpen:
        // return "open-string";
        return "open";
        break;
    case SymId::brassMuteHalfClosed:
        return "half-muted";
        break;
    case SymId::brassHarmonMuteClosed:
        return "harmon-mute";
        break;
    case SymId::guitarGolpe:
        return "golpe";
        break;
    default:
        ;           // nothing
        break;
    }

    return "";
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
            QString subtype;
            switch (cl->chordLineType()) {
            case ChordLineType::FALL:
                subtype = "falloff";
                break;
            case ChordLineType::DOIT:
                subtype = "doit";
                break;
            case ChordLineType::PLOP:
                subtype = "plop";
                break;
            case ChordLineType::SCOOP:
                subtype = "scoop";
                break;
            default:
                LOGD("unknown ChordLine subtype %d", int(cl->chordLineType()));
            }
            if (subtype != "") {
                notations.tag(xml, e);
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
        notations.tag(xml, breath);
        articulations.tag(xml);
        if (breath->isCaesura()) {
            xml.tag("caesura");
        } else {
            QString breathMarkType;
            switch (breath->symId()) {
            case SymId::breathMarkTick:
                breathMarkType = "tick";
                break;
            case SymId::breathMarkUpbow:
                breathMarkType = "upbow";
                break;
            case SymId::breathMarkSalzedo:
                breathMarkType = "salzedo";
                break;
            default:
                breathMarkType = "comma";
            }

            xml.tag("breath-mark", breathMarkType);
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
        QVector<EngravingItem*> fl;
        for (EngravingItem* e : chord->segment()->annotations()) {
            if (e->track() == chord->track() && e->isFermata()) {
                fl.push_back(e);
            }
        }
        fermatas(fl, _xml, notations);
    }

    const std::vector<Articulation*> na = chord->articulations();
    // first the attributes whose elements are children of <articulations>
    Articulations articulations;
    for (const Articulation* a : na) {
        if (!ExportMusicXml::canWrite(a)) {
            continue;
        }

        auto sid = a->symId();
        auto mxmlArtics = symIdToArtic(sid);

        for (QString mxmlArtic : mxmlArtics) {
            if (mxmlArtic == "strong-accent") {
                if (a->up()) {
                    mxmlArtic += " type=\"up\"";
                } else {
                    mxmlArtic += " type=\"down\"";
                }
            } else if (a->anchor() != ArticulationAnchor::AUTO) {
                if (a->anchor() == ArticulationAnchor::TOP) {
                    mxmlArtic += " placement=\"above\"";
                } else {
                    mxmlArtic += " placement=\"below\"";
                }
            }
            mxmlArtic += color2xml(a);

            notations.tag(_xml, a);
            articulations.tag(_xml);
            _xml.tagRaw(mxmlArtic);
        }
    }

    writeBreathMark(chord->hasBreathMark(), _xml, notations, articulations);
    writeChordLines(chord, _xml, notations, articulations);

    articulations.etag(_xml);

    // then the attributes whose elements are children of <ornaments>
    Ornaments ornaments;
    for (const Articulation* a : na) {
        if (!ExportMusicXml::canWrite(a)) {
            continue;
        }

        auto sid = a->symId();
        auto mxmlOrnam = symIdToOrnam(sid);

        if (mxmlOrnam != "") {
            mxmlOrnam += color2xml(a);

            notations.tag(_xml, a);
            ornaments.tag(_xml);
            _xml.tagRaw(mxmlOrnam);
            // accidental-mark is missing
        }
    }

    tremoloSingleStartStop(chord, notations, ornaments, _xml);
    wavyLineStartStop(chord, notations, ornaments, trillStart, trillStop);
    ornaments.etag(_xml);

    // and finally the attributes whose elements are children of <technical>
    for (const Articulation* a : na) {
        if (!ExportMusicXml::canWrite(a)) {
            continue;
        }

        auto sid = a->symId();
        QString placement;
        QString direction;

        QString attr;
        if (!a->isStyled(Pid::ARTICULATION_ANCHOR) && a->anchor() != ArticulationAnchor::AUTO) {
            placement = (a->anchor() == ArticulationAnchor::BOTTOM) ? "below" : "above";
        } else if (!a->isStyled(Pid::DIRECTION) && a->direction() != DirectionV::AUTO) {
            direction = (a->direction() == DirectionV::DOWN) ? "down" : "up";
        }
        /* For future use if/when implemented font details for articulation
        if (!a->isStyled(Pid::FONT_FACE))
              attr += QString(" font-family=\"%1\"").arg(a->getProperty(Pid::FONT_FACE).toString());
        if (!a->isStyled(Pid::FONT_SIZE))
              attr += QString(" font-size=\"%1\"").arg(a->getProperty(Pid::FONT_SIZE).toReal());
        if (!a->isStyled(Pid::FONT_STYLE))
              attr += fontStyleToXML(static_cast<FontStyle>(a->getProperty(Pid::FONT_STYLE).toInt()), false);
        */

        auto mxmlTechn = symIdToTechn(sid);
        if (mxmlTechn != "") {
            notations.tag(_xml, a);
            technical.tag(_xml);
            mxmlTechn += color2xml(a);
            mxmlTechn += ExportMusicXml::positioningAttributes(a);
            if (sid == SymId::stringsHarmonic) {
                if (placement != "") {
                    attr += QString(" placement=\"%1\"").arg(placement);
                }
                _xml.startElementRaw(mxmlTechn + attr);
                _xml.tag("natural");
                _xml.endElement();
            } else {
                if (placement != "") {
                    attr += QString(" placement=\"%1\"").arg(placement);
                }
                _xml.tagRaw(mxmlTechn + attr);
            }
        }
    }

    // check if all articulations were handled
    for (const Articulation* a : na) {
        if (!ExportMusicXml::canWrite(a)) {
            continue;
        }

        auto sid = a->symId();
        if (symIdToArtic(sid).empty()
            && symIdToOrnam(sid) == ""
            && symIdToTechn(sid) == ""
            && !isLaissezVibrer(sid)) {
            LOGD("unknown chord attribute %d %s", static_cast<int>(sid), qPrintable(a->translatedTypeUserName()));
        }
    }
}

//---------------------------------------------------------
//   arpeggiate
//---------------------------------------------------------

// <notations>
//   <arpeggiate direction="up"/>
//   </notations>

static void arpeggiate(Arpeggio* arp, bool front, bool back, XmlWriter& xml, Notations& notations)
{
    if (!ExportMusicXml::canWrite(arp)) {
        return;
    }

    QString tagName;
    switch (arp->arpeggioType()) {
    case ArpeggioType::NORMAL:
        notations.tag(xml, arp);
        tagName = "arpeggiate";
        break;
    case ArpeggioType::UP:                  // fall through
    case ArpeggioType::UP_STRAIGHT:         // not supported by MusicXML, export as normal arpeggio
        notations.tag(xml, arp);
        tagName = "arpeggiate direction=\"up\"";
        break;
    case ArpeggioType::DOWN:                  // fall through
    case ArpeggioType::DOWN_STRAIGHT:         // not supported by MusicXML, export as normal arpeggio
        notations.tag(xml, arp);
        tagName = "arpeggiate direction=\"down\"";
        break;
    case ArpeggioType::BRACKET:
        if (front) {
            notations.tag(xml, arp);
            tagName = "non-arpeggiate type=\"bottom\"";
        }
        if (back) {
            notations.tag(xml, arp);
            tagName = "non-arpeggiate type=\"top\"";
        }
        break;
    default:
        LOGD("unknown arpeggio subtype %d", int(arp->arpeggioType()));
        break;
    }

    if (tagName != "") {
        tagName += color2xml(arp);
        tagName += ExportMusicXml::positioningAttributes(arp);
        xml.tagRaw(tagName);
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
    LOGD("determineTupletNormalTicks t %p baselen %s", t, qPrintable(t->baseLen().ticks().print()));
    for (int i = 0; i < t->elements().size(); ++i)
          LOGD("determineTupletNormalTicks t %p i %d ticks %s", t, i, qPrintable(t->elements().at(i)->ticks().print()));
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

static QString beamFanAttribute(const Beam* const b)
{
    const qreal epsilon = 0.1;

    QString fan;
    if ((b->growRight() - b->growLeft() > epsilon)) {
        fan = "accel";
    }

    if ((b->growLeft() - b->growRight() > epsilon)) {
        fan = "rit";
    }

    if (fan != "") {
        return QString(" fan=\"%1\"").arg(fan);
    }

    return "";
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
    const size_t idx = mu::indexOf(elements, cr);
    if (idx == mu::nidx) {
        LOGD("Beam::writeMusicXml(): cannot find ChordRest");
        return;
    }
    int blp = -1;   // beam level previous chord
    int blc = -1;   // beam level current chord
    int bln = -1;   // beam level next chord
    BeamMode bmc = BeamMode::AUTO; // beam mode current chord
    BeamMode bmn = BeamMode::AUTO; // beam mode next chord
    // find beam level previous chord
    for (size_t i = idx - 1; blp == -1 && i != mu::nidx; --i) {
        const auto crst = elements[i];
        if (crst->isChord()) {
            blp = toChord(crst)->beams();
        }
    }
    // find beam level current chord
    if (cr->isChord()) {
        blc = toChord(cr)->beams();
        bmc = toChord(cr)->beamMode();
    }
    // find beam level next chord
    for (size_t i = idx + 1; bln == -1 && i < elements.size(); ++i) {
        const auto crst = elements[i];
        if (crst->isChord()) {
            bln = toChord(crst)->beams();
            bmn = toChord(crst)->beamMode();
        }
    }
    // find beam type and write
    for (int i = 1; i <= blc; ++i) {
        QString text;
        // TODO: correctly handle Beam::Mode::AUTO
        // when equivalent to BEGIN32 or BEGIN64
        if ((blp < i && bln >= i)
            || (bmc == BeamMode::BEGIN16 && i > 1)
            || (bmc == BeamMode::BEGIN32 && i > 2)) {
            text = "begin";
        } else if (blp < i && bln < i) {
            if (bln > 0) {
                text = "forward hook";
            } else if (blp > 0) {
                text = "backward hook";
            }
        } else if ((blp >= i && bln < i)
                   || (bmn == BeamMode::BEGIN16 && i > 1)
                   || (bmn == BeamMode::BEGIN32 && i > 2)) {
            text = "end";
        } else if (blp >= i && bln >= i) {
            text = "continue";
        }
        if (text != "") {
            QString tag = "beam";
            tag += QString(" number=\"%1\"").arg(i);
            if (text == "begin") {
                tag += beamFanAttribute(b);
            }
            xml.tagRaw(tag, text);
        }
    }
}

//---------------------------------------------------------
//   instrId
//---------------------------------------------------------

static QString instrId(int partNr, int instrNr)
{
    return QString("id=\"P%1-I%2\"").arg(partNr).arg(instrNr);
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
    QString noteheadTagname = QString("notehead");
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
        noteheadTagname += " parentheses=\"yes\"";
    }
    if (note->headType() == NoteHeadType::HEAD_QUARTER) {
        noteheadTagname += " filled=\"yes\"";
    } else if ((note->headType() == NoteHeadType::HEAD_HALF) || (note->headType() == NoteHeadType::HEAD_WHOLE)) {
        noteheadTagname += " filled=\"no\"";
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
        noteheadTagname += QString(" smufl=\"%1\"").arg(noteheadName.ascii());
        xml.tagRaw(noteheadTagname, "other");
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
            QString t = MScoreTextToMXML::toPlainText(f->xmlText());
            QString attr;
            if (!f->isStyled(Pid::PLACEMENT) || f->placement() == PlacementV::BELOW) {
                attr = QString(" placement=\"%1\"").arg((f->placement() == PlacementV::BELOW) ? "below" : "above");
            }
            if (!f->isStyled(Pid::FONT_FACE)) {
                attr += QString(" font-family=\"%1\"").arg(f->getProperty(Pid::FONT_FACE).value<String>());
            }
            if (!f->isStyled(Pid::FONT_SIZE)) {
                attr += QString(" font-size=\"%1\"").arg(f->getProperty(Pid::FONT_SIZE).toReal());
            }
            if (!f->isStyled(Pid::FONT_STYLE)) {
                attr += fontStyleToXML(static_cast<FontStyle>(f->getProperty(Pid::FONT_STYLE).toInt()), false);
            }
            attr += color2xml(f);

            if (f->textStyleType() == TextStyleType::RH_GUITAR_FINGERING) {
                xml.tagRaw("pluck" + attr, t);
            } else if (f->textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
                xml.tagRaw("fingering" + attr, t);
            } else if (f->textStyleType() == TextStyleType::FINGERING) {
                // for generic fingering, try to detect plucking
                // (backwards compatibility with MuseScore 1.x)
                // p, i, m, a, c represent the plucking finger
                if (t == "p" || t == "i" || t == "m" || t == "a" || t == "c") {
                    xml.tagRaw("pluck" + attr, t);
                } else {
                    xml.tagRaw("fingering" + attr, t);
                }
            } else if (f->textStyleType() == TextStyleType::STRING_NUMBER) {
                bool ok;
                int i = t.toInt(&ok);
                if (ok) {
                    if (i == 0) {
                        xml.tagRaw("open-string" + attr);
                    } else if (i > 0) {
                        xml.tagRaw("string" + attr, t);
                    }
                }
                if (!ok || i < 0) {
                    LOGD("invalid string number '%s'", qPrintable(t));
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
//   writeTypeAndDots
//---------------------------------------------------------

static void writeTypeAndDots(XmlWriter& xml, const Note* const note)
{
    int dots = 0;
    const auto ratio = timeModification(note->chord()->tuplet());

    const auto strActFraction = stretchCorrActFraction(note);
    const Fraction tt  = strActFraction * ratio * tremoloCorrection(note);
    const QString s { tick2xml(tt, &dots) };
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
    for (int ni = dots; ni > 0; ni--) {
        xml.tag("dot");
    }
}

//---------------------------------------------------------
//   writeTimeModification
//---------------------------------------------------------

static void writeTimeModification(XmlWriter& xml, const Tuplet* const tuplet, const int tremolo = 1)
{
    const auto ratio = timeModification(tuplet, tremolo);
    if (ratio != Fraction(1, 1)) {
        const auto actNotes = ratio.numerator();
        const auto nrmNotes = ratio.denominator();

        const auto nrmTicks = determineTupletNormalTicks(tuplet);
        xml.startElement("time-modification");
        xml.tag("actual-notes", actNotes);
        xml.tag("normal-notes", nrmNotes);
        //LOGD("nrmTicks %d", nrmTicks);
        if (nrmTicks > 0) {
            int nrmDots { 0 };
            const QString nrmType { tick2xml(Fraction::fromTicks(nrmTicks), &nrmDots) };
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
    QString step;
    int alter = 0;
    int octave = 0;
    const auto chord = note->chord();
    if (chord->staff() && chord->staff()->isTabStaff(Fraction(0, 1))) {
        tabpitch2xml(note->pitch(), note->tpc(), step, alter, octave);
    } else {
        if (!useDrumset) {
            pitch2xml(note, step, alter, octave);
        } else {
            unpitch2xml(note, step, octave);
        }
    }
    xml.startElement(useDrumset ? "unpitched" : "pitch");
    xml.tag(useDrumset ? "display-step" : "step", step);
    // Check for microtonal accidentals and overwrite "alter" tag
    auto acc = note->accidental();
    double alter2 = 0.0;
    if (acc) {
        switch (acc->accidentalType()) {
        case AccidentalType::MIRRORED_FLAT:  alter2 = -0.5;
            break;
        case AccidentalType::SHARP_SLASH:    alter2 = 0.5;
            break;
        case AccidentalType::MIRRORED_FLAT2: alter2 = -1.5;
            break;
        case AccidentalType::SHARP_SLASH4:   alter2 = 1.5;
            break;
        default:                                             break;
        }
    }
    if (alter && !alter2) {
        xml.tag("alter", alter);
    }
    if (!alter && alter2) {
        xml.tag("alter", alter2);
    }
    // TODO what if both alter and alter2 are present? For Example: playing with transposing instruments
    xml.tag(useDrumset ? "display-octave" : "octave", octave);
    xml.endElement();
}

//---------------------------------------------------------
//   elementPosition
//---------------------------------------------------------

QString ExportMusicXml::elementPosition(const ExportMusicXml* const expMxml, const EngravingItem* const elm)
{
    QString res;

    if (configuration()->musicxmlExportLayout()) {
        const double pageHeight  = expMxml->getTenthsFromInches(expMxml->score()->style().styleD(Sid::pageHeight));

        const auto meas = elm->findMeasure();
        IF_ASSERT_FAILED(meas) {
            return res;
        }

        double measureX = expMxml->getTenthsFromDots(meas->pagePos().x());
        double measureY = pageHeight - expMxml->getTenthsFromDots(meas->pagePos().y());
        double noteX = expMxml->getTenthsFromDots(elm->pagePos().x());
        double noteY = pageHeight - expMxml->getTenthsFromDots(elm->pagePos().y());

        res += QString(" default-x=\"%1\"").arg(QString::number(noteX - measureX, 'f', 2));
        res += QString(" default-y=\"%1\"").arg(QString::number(noteY - measureY, 'f', 2));
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
    size_t partNr = mu::indexOf(_score->parts(), part);
    int instNr = instrMap.value(part->instrument(_tick), -1);
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
    if (!grace) {
        _tick += chord->actualTicks();
    }
#ifdef DEBUG_TICK
    LOGD("ExportMusicXml::chord() oldtick=%d", tick);
    LOGD("notetype=%d grace=%d", gracen, grace);
    LOGD(" newtick=%d", tick);
#endif

    for (Note* note : nl) {
        _attr.doAttr(_xml, false);
        QString noteTag = QString("note");

        noteTag += elementPosition(this, note);

        int velo = note->userVelocity();
        if (velo != 0) {
            noteTag += QString(" dynamics=\"%1\"").arg(QString::number(velo * 100.0 / 90.0, 'f', 2));
        }

        _xml.startElementRaw(noteTag);

        if (grace) {
            if (note->noteType() == NoteType::ACCIACCATURA) {
                _xml.tag("grace", { { "slash", "yes" } });
            } else {
                _xml.tag("grace");
            }
        }
        if (isCueNote(note)) {
            _xml.tag("cue");
        }
        if (note != nl.front()) {
            _xml.tag("chord");
        }

        writePitch(_xml, note, useDrumset);

        // duration
        if (!grace) {
            _xml.tag("duration", stretchCorrActFraction(note).ticks() / div);
        }

        if (!isCueNote(note)) {
            if (note->tieBack()) {
                _xml.tag("tie", { { "type", "stop" } });
            }
            if (note->tieFor()) {
                _xml.tag("tie", { { "type", "start" } });
            }
        }

        // instrument for multi-instrument or unpitched parts
        if (!useDrumset) {
            if (instrMap.size() > 1 && instNr >= 0) {
                _xml.tagRaw(QString("instrument %1").arg(instrId(static_cast<int>(partNr) + 1, instNr + 1)));
            }
        } else {
            _xml.tagRaw(QString("instrument %1").arg(instrId(static_cast<int>(partNr) + 1, note->pitch() + 1)));
        }

        // voice
        // for a single-staff part, staff is 0, which needs to be corrected
        // to calculate the correct voice number
        voice_idx_t voice = (static_cast<int>(staff) - 1) * VOICES + note->chord()->voice() + 1;
        if (staff == 0) {
            voice += VOICES;
        }

        _xml.tag("voice", static_cast<int>(voice));

        writeTypeAndDots(_xml, note);
        writeAccidental(_xml, "accidental", note->accidental());
        writeTimeModification(_xml, note->chord()->tuplet(), tremoloCorrection(note));

        // no stem for whole notes and beyond
        if (chord->noStem() || chord->measure()->stemless(chord->staffIdx())) {
            _xml.tag("stem", QString("none"));
        } else if (note->chord()->stem()) {
            _xml.tag("stem", QString(note->chord()->up() ? "up" : "down"));
        }

        writeNotehead(_xml, note);

        // LVIFIX: check move() handling
        if (staff) {
            _xml.tag("staff", static_cast<int>(staff + note->chord()->staffMove()));
        }

        if (note == nl.front() && chord->beam()) {
            writeBeam(_xml, chord, chord->beam());
        }

        Notations notations;
        Technical technical;

        const Tie* tieBack = note->tieBack();
        if (tieBack && ExportMusicXml::canWrite(tieBack)) {
            notations.tag(_xml, tieBack);
            _xml.tag("tied", { { "type", "stop" } });
        }
        const Tie* tieFor = note->tieFor();
        if (tieFor && ExportMusicXml::canWrite(tieFor)) {
            notations.tag(_xml, tieFor);
            QString rest = slurTieLineStyle(tieFor);
            _xml.tagRaw(QString("tied type=\"start\"%1").arg(rest));
        }
        const Articulation* laissezVibrer = findLaissezVibrer(chord);
        if (laissezVibrer && ExportMusicXml::canWrite(laissezVibrer)) {
            notations.tag(_xml, laissezVibrer);
            _xml.tag("tied", { { "type", "let-ring" } });
        }

        if (note == nl.front()) {
            if (!grace) {
                tupletStartStop(chord, notations, _xml);
            }

            sh.doSlurs(chord, notations, _xml);

            chordAttributes(chord, notations, technical, _trillStart, _trillStop);
        }

        writeFingering(_xml, notations, technical, note);
        writeNotationSymbols(_xml, notations, note->el(), true);

        // write tablature string / fret
        if (chord->staff() && chord->staff()->isTabStaff(Fraction(0, 1))) {
            if (note->fret() >= 0 && note->string() >= 0) {
                notations.tag(_xml, note);
                technical.tag(_xml);
                _xml.tag("string", note->string() + 1);
                _xml.tag("fret", note->fret());
            }
        }

        technical.etag(_xml);
        if (chord->arpeggio()) {
            arpeggiate(chord->arpeggio(), note == nl.front(), note == nl.back(), _xml, notations);
        }
        for (Spanner* spanner : note->spannerFor()) {
            if (spanner->type() == ElementType::GLISSANDO && ExportMusicXml::canWrite(spanner)) {
                gh.doGlissandoStart(static_cast<Glissando*>(spanner), notations, _xml);
            }
        }
        for (Spanner* spanner : note->spannerBack()) {
            if (spanner->type() == ElementType::GLISSANDO && ExportMusicXml::canWrite(spanner)) {
                gh.doGlissandoStop(static_cast<Glissando*>(spanner), notations, _xml);
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
        notations.etag(_xml);
        // write lyrics (only for first note)
        if (!grace && (note == nl.front())) {
            lyrics(ll, chord->track());
        }
        _xml.endElement();
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
    static char table2[]  = "CDEFGAB";
#ifdef DEBUG_TICK
    LOGD("ExportMusicXml::rest() oldtick=%d", tick);
#endif
    _attr.doAttr(_xml, false);

    QString noteTag = QString("note");
    noteTag += color2xml(rest);
    noteTag += elementPosition(this, rest);
    if (!rest->visible()) {
        noteTag += " print-object=\"no\"";
    }
    _xml.startElementRaw(noteTag);

    int yOffsSt   = 0;
    int oct       = 0;
    int stp       = 0;
    ClefType clef = rest->staff()->clef(rest->tick());
    int po        = ClefInfo::pitchOffset(clef);

    // Determine y position, but leave at zero in case of tablature staff
    // as no display-step or display-octave should be written for a tablature staff,

    if (clef != ClefType::TAB && clef != ClefType::TAB_SERIF && clef != ClefType::TAB4 && clef != ClefType::TAB4_SERIF) {
        double yOffsSp = rest->offset().y() / rest->spatium();                  // y offset in spatium (negative = up)
        yOffsSt = -2 * int(yOffsSp > 0.0 ? yOffsSp + 0.5 : yOffsSp - 0.5);     // same rounded to int (positive = up)

        po -= 4;        // pitch middle staff line (two lines times two steps lower than top line)
        po += yOffsSt;     // rest "pitch"
        oct = po / 7;     // octave
        stp = po % 7;     // step
    }

    QString restTag { "rest" };
    const TDuration d = rest->durationType();
    if (d.type() == DurationType::V_MEASURE) {
        restTag += " measure=\"yes\"";
    }
    // Either <rest/>
    // or <rest><display-step>F</display-step><display-octave>5</display-octave></rest>
    if (yOffsSt == 0) {
        _xml.tagRaw(restTag);
    } else {
        _xml.startElementRaw(restTag);
        _xml.tag("display-step", QString(QChar(table2[stp])));
        _xml.tag("display-octave", oct - 1);
        _xml.endElement();
    }

    Fraction tickLen = rest->actualTicks();
    if (d.type() == DurationType::V_MEASURE) {
        // to avoid forward since rest->ticklen=0 in this case.
        tickLen = rest->measure()->ticks();
    }
    _tick += tickLen;
#ifdef DEBUG_TICK
    LOGD(" tickLen=%d newtick=%d", tickLen, tick);
#endif

    _xml.tag("duration", tickLen.ticks() / div);

    // for a single-staff part, staff is 0, which needs to be corrected
    // to calculate the correct voice number
    voice_idx_t voice = (static_cast<int>(staff) - 1) * VOICES + rest->voice() + 1;
    if (staff == 0) {
        voice += VOICES;
    }
    _xml.tag("voice", static_cast<int>(voice));

    // do not output a "type" element for whole measure rest
    if (d.type() != DurationType::V_MEASURE) {
        AsciiStringView s = TConv::toXml(d.type());
        int dots  = rest->dots();
        if (rest->isSmall()) {
            _xml.tag("type", { { "size", "cue" } }, s);
        } else {
            _xml.tag("type", s);
        }
        for (int i = dots; i > 0; i--) {
            _xml.tag("dot");
        }
    }

    writeTimeModification(_xml, rest->tuplet());

    if (staff) {
        _xml.tag("staff", static_cast<int>(staff));
    }

    Notations notations;
    QVector<EngravingItem*> fl;
    for (EngravingItem* e : rest->segment()->annotations()) {
        if (e->isFermata() && e->track() == rest->track()) {
            fl.push_back(e);
        }
    }
    fermatas(fl, _xml, notations);

    Articulations articulations;
    writeBreathMark(rest->hasBreathMark(), _xml, notations, articulations);
    articulations.etag(_xml);

    Ornaments ornaments;
    wavyLineStartStop(rest, notations, ornaments, _trillStart, _trillStop);
    ornaments.etag(_xml);

    writeNotationSymbols(_xml, notations, rest->el(), false);

    sh.doSlurs(rest, notations, _xml);

    tupletStartStop(rest, notations, _xml);
    notations.etag(_xml);

    lyrics(ll, rest->track());

    _xml.endElement();
}

//---------------------------------------------------------
//   directionTag
//---------------------------------------------------------

static void directionTag(XmlWriter& xml, Attributes& attr, EngravingItem const* const el = 0)
{
    attr.doAttr(xml, false);
    QString tagname = QString("direction");
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
        } else if (el->type() == ElementType::DYNAMIC
                   || el->type() == ElementType::INSTRUMENT_CHANGE
                   || el->type() == ElementType::REHEARSAL_MARK
                   || el->type() == ElementType::STAFF_TEXT
                   || el->type() == ElementType::PLAYTECH_ANNOTATION
                   || el->type() == ElementType::CAPO
                   || el->type() == ElementType::STRING_TUNINGS
                   || el->type() == ElementType::SYMBOL
                   || el->type() == ElementType::TEXT) {
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
            QRectF bb = sys->staff(el->staffIdx())->ldata()->bbox;
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
                    tagname += " placement=\"above\"";
                } else {
                    tagname += " placement=\"below\"";
                }
            } else if (el->isDynamic()) {
                tagname += " placement=\"";
                tagname += el->placement() == PlacementV::ABOVE ? "above" : "below";
                tagname += "\"";
            } else {
                /*
                LOGD("directionTag()  staff ely=%g elh=%g bby=%g bbh=%g",
                       el->y(), el->height(),
                       bb.y(), bb.height());
                 */
                // if (el->y() + el->height() / 2 < /*bb.y() +*/ bb.height() / 2)
                if (el->placement() == PlacementV::ABOVE) {
                    tagname += " placement=\"above\"";
                } else {
                    tagname += " placement=\"below\"";
                }
            }
        }           // if (pel && ...
    }
    xml.startElementRaw(tagname);
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

static void partGroupStart(XmlWriter& xml, int number, BracketType bracket)
{
    xml.startElement("part-group", { { "type", "start" }, { "number", number } });
    QString br = "";
    switch (bracket) {
    case BracketType::NO_BRACKET:
        br = "none";
        break;
    case BracketType::NORMAL:
        br = "bracket";
        break;
    case BracketType::BRACE:
        br = "brace";
        break;
    case BracketType::LINE:
        br = "line";
        break;
    case BracketType::SQUARE:
        br = "square";
        break;
    default:
        LOGD("bracket subtype %d not understood", int(bracket));
    }
    if (br != "") {
        xml.tag("group-symbol", br);
    }
    xml.endElement();
}

//---------------------------------------------------------
//   findMetronome
//---------------------------------------------------------

static bool findMetronome(const std::list<TextFragment>& list,
                          std::list<TextFragment>& wordsLeft,  // words left of metronome
                          bool& hasParen,      // parenthesis
                          QString& metroLeft,  // left part of metronome
                          QString& metroRight, // right part of metronome
                          std::list<TextFragment>& wordsRight // words right of metronome
                          )
{
    QString words = MScoreTextToMXML::toPlainTextPlusSymbols(list);
    //LOGD("findMetronome('%s')", qPrintable(words));
    hasParen   = false;
    metroLeft  = "";
    metroRight = "";
    int metroPos = -1;     // metronome start position
    int metroLen = 0;      // metronome length

    int indEq  = words.indexOf('=');
    if (indEq <= 0) {
        return false;
    }

    int len1 = 0;
    TDuration dur;

    // find first note, limiting search to the part left of the first '=',
    // to prevent matching the second note in a "note1 = note2" metronome
    int pos1 = TempoText::findTempoDuration(words.left(indEq), len1, dur);
    QRegularExpression equationRegEx("\\s*=\\s*");
    QRegularExpressionMatch equationMatch;
    int pos2 = words.indexOf(equationRegEx, pos1 + len1, &equationMatch);
    if (pos1 != -1 && pos2 == pos1 + len1) {
        int len2 = equationMatch.capturedLength();
        if (words.length() > pos2 + len2) {
            QString s1 = words.mid(0, pos1);           // string to the left of metronome
            QString s2 = words.mid(pos1, len1);        // first note
            QString s3 = words.mid(pos2, len2);        // equals sign
            QString s4 = words.mid(pos2 + len2);       // string to the right of equals sign
            /*
            LOGD("found note and equals: '%s'%s'%s'%s'",
                   qPrintable(s1),
                   qPrintable(s2),
                   qPrintable(s3),
                   qPrintable(s4)
                   );
             */

            // now determine what is to the right of the equals sign
            // must have either a (dotted) note or a number at start of s4
            int len3 = 0;
            // One or more digits, optionally followed by a single dot or comma and one or more digits
            QRegularExpression numberRegEx("\\d+([,\\.]{1}\\d+)?");
            int pos3 = TempoText::findTempoDuration(s4, len3, dur);
            if (pos3 == -1) {
                // did not find note, try to find a number
                QRegularExpressionMatch numberMatch;
                pos3 = s4.indexOf(numberRegEx, 0, &numberMatch);
                if (pos3 == 0) {
                    len3 = numberMatch.capturedLength();
                }
            }
            if (pos3 == -1) {
                // neither found
                return false;
            }

            QString s5 = s4.mid(0, len3);       // number or second note
            QString s6 = s4.mid(len3);          // string to the right of metronome
            /*
            LOGD("found right part: '%s'%s'",
                   qPrintable(s5),
                   qPrintable(s6)
                   );
             */

            // determine if metronome has parentheses
            // left part of string must end with parenthesis plus optional spaces
            // right part of string must have parenthesis (but not in first pos)
            int lparen = s1.indexOf("(");
            int rparen = s6.indexOf(")");
            hasParen = (lparen == s1.length() - 1 && rparen == 0);

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

            /*
            LOGD("-> found '%s'%s' hasParen %d metro pos %d len %d",
                   qPrintable(metroLeft),
                   qPrintable(metroRight),
                   hasParen, metroPos, metroLen
                   );
             */
            std::list<TextFragment> mid;       // not used
            MScoreTextToMXML::split(list, metroPos, metroLen, wordsLeft, mid, wordsRight);
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
    QString metroLeft;                      // left part of metronome
    QString metroRight;                     // right part of metronome
    std::list<TextFragment> wordsRight;         // words right of metronome

    // set the default words format
    const QString mtf = s.styleSt(Sid::MusicalTextFont);
    const CharFormat defFmt = formatForWords(s);

    if (findMetronome(list, wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {
        if (wordsLeft.size() > 0) {
            xml.startElement("direction-type");
            QString attr = ExportMusicXml::positioningAttributes(text);
            MScoreTextToMXML mttm("words", attr, defFmt, mtf);
            mttm.writeTextFragments(wordsLeft, xml);
            xml.endElement();
        }

        xml.startElement("direction-type");
        QString tagName = QString("metronome parentheses=\"%1\"").arg(hasParen ? "yes" : "no");
        tagName += color2xml(text);
        tagName += ExportMusicXml::positioningAttributes(text);
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
            QString attr = ExportMusicXml::positioningAttributes(text);
            MScoreTextToMXML mttm("words", attr, defFmt, mtf);
            mttm.writeTextFragments(wordsRight, xml);
            xml.endElement();
        }
    } else {
        xml.startElement("direction-type");
        QString attr;
        if (text->hasFrame()) {
            if (text->circle()) {
                attr = " enclosure=\"circle\"";
            } else {
                attr = " enclosure=\"rectangle\"";
            }
        }
        attr += color2xml(text);
        attr += ExportMusicXml::positioningAttributes(text);
        MScoreTextToMXML mttm("words", attr, defFmt, mtf);
        //LOGD("words('%s')", qPrintable(text->text()));
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
    const auto offset = calculateTimeDeltaInDivisions(text->tick(), tick(), div);
    /*
    LOGD("tick %s text->tick %s offset %d xmlText='%s')",
           qPrintable(tick().print()),
           qPrintable(text->tick().print()),
           offset,
           qPrintable(text->xmlText()));
    */
    _attr.doAttr(_xml, false);
    _xml.startElement("direction", { { "placement", (text->placement() == PlacementV::BELOW) ? "below" : "above" } });
    wordsMetronome(_xml, _score->style(), text, offset);

    if (staff) {
        _xml.tag("staff", static_cast<int>(staff));
    }
    // Format tempo with maximum 2 decimal places, because in some MuseScore files tempo is stored
    // imprecisely and this could cause rounding errors (e.g. 92 BPM would be saved as 91.9998).
    BeatsPerMinute bpm = text->tempo().toBPM();
    qreal bpmRounded = round(bpm.val * 100) / 100;
    _xml.tag("sound", { { "tempo", bpmRounded } });
    _xml.endElement();
}

//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportMusicXml::words(TextBase const* const text, staff_idx_t staff)
{
    const auto offset = calculateTimeDeltaInDivisions(text->tick(), tick(), div);
    /*
    LOGD("tick %s text->tick %s offset %d userOff.x=%f userOff.y=%f xmlText='%s' plainText='%s'",
           qPrintable(tick().print()),
           qPrintable(text->tick().print()),
           offset,
           text->offset().x(), text->offset().y(),
           qPrintable(text->xmlText()),
           qPrintable(text->plainText()));
    */

    if (text->plainText() == "") {
        // sometimes empty Texts are present, exporting would result
        // in invalid MusicXML (as an empty direction-type would be created)
        return;
    }

    directionTag(_xml, _attr, text);
    wordsMetronome(_xml, _score->style(), text, offset);
    directionETag(_xml, staff);
}

//---------------------------------------------------------
//   positioningAttributesForTboxText
//---------------------------------------------------------

QString ExportMusicXml::positioningAttributesForTboxText(const QPointF position, float spatium)
{
    if (!configuration()->musicxmlExportLayout()) {
        return "";
    }

    QPointF relative;         // use zero relative position
    return positionToQString(position, relative, spatium);
}

//---------------------------------------------------------
//   tboxTextAsWords
//---------------------------------------------------------

void ExportMusicXml::tboxTextAsWords(TextBase const* const text, const staff_idx_t staff, const QPointF relativePosition)
{
    if (text->plainText() == "") {
        // sometimes empty Texts are present, exporting would result
        // in invalid MusicXML (as an empty direction-type would be created)
        return;
    }

    // set the default words format
    const MStyle& style = _score->style();
    const QString mtf = style.styleSt(Sid::MusicalTextFont);
    const CharFormat defFmt = formatForWords(style);

    _xml.startElement("direction", { { "placement", (relativePosition.y() < 0) ? "above" : "below" } });
    _xml.startElement("direction-type");
    QString attr;
    if (text->hasFrame()) {
        if (text->circle()) {
            attr = " enclosure=\"circle\"";
        } else {
            attr = " enclosure=\"rectangle\"";
        }
    }
    attr += ExportMusicXml::positioningAttributesForTboxText(relativePosition, text->spatium());
    attr += " valign=\"top\"";
    MScoreTextToMXML mttm("words", attr, defFmt, mtf);
    mttm.writeTextFragments(text->fragmentList(), _xml);
    _xml.endElement();
    directionETag(_xml, staff);
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

    directionTag(_xml, _attr, rmk);
    _xml.startElement("direction-type");
    QString attr;
    if (rmk->circle()) {
        attr = " enclosure=\"circle\"";
    } else if (!rmk->hasFrame()) {
        attr = " enclosure=\"none\"";
    }
    attr += color2xml(rmk);
    attr += positioningAttributes(rmk);
    // set the default words format
    const MStyle& style = _score->style();
    const QString mtf = style.styleSt(Sid::MusicalTextFont);
    const CharFormat defFmt = formatForWords(style);
    // write formatted
    MScoreTextToMXML mttm("rehearsal", attr, defFmt, mtf);
    mttm.writeTextFragments(rmk->fragmentList(), _xml);
    _xml.endElement();
    const auto offset = calculateTimeDeltaInDivisions(rmk->tick(), tick(), div);
    if (offset) {
        _xml.tag("offset", offset);
    }
    directionETag(_xml, staff);
}

//---------------------------------------------------------
//   findDashes -- get index of hairpin in dashes table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findDashes(const TextLineBase* hp) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (dashes[i] == hp) {
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
        if (hairpins[i] == hp) {
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

static int findDynamicInString(const QString& s, int& length, QString& type)
{
    length = 0;
    type = "";
    int matchIndex { -1 };
    const int n = static_cast<int>(DynamicType::LAST) - 1;

    // for all dynamics, find their text in s
    for (int i = 0; i < n; ++i) {
        DynamicType t = static_cast<DynamicType>(i);
        const QString dynamicText = Dynamic::dynamicText(t);
        const int dynamicLength = dynamicText.length();
        // note: skip entries with empty text
        if (dynamicLength > 0) {
            const auto index = s.indexOf(dynamicText);
            if (index >= 0) {
                // found a match, accept it if
                // - it is the first one
                // - or it starts a the same index but is longer ("pp" versus "p")
                if (matchIndex == -1 || (index == matchIndex && dynamicLength > length)) {
                    matchIndex = index;
                    length = dynamicLength;
                    type = TConv::toXml(t).ascii();
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
    auto text = isStart ? tlb->beginText() : tlb->endText();
    while (text != "") {
        int dynamicLength { 0 };
        QString dynamicsType;
        auto dynamicPosition = findDynamicInString(text, dynamicLength, dynamicsType);
        if (dynamicPosition == -1 || dynamicPosition > 0) {
            // text remaining and either no dynamic of not at front of text
            xml.startElement("direction-type");
            QString tag = "words";
            tag
                += QString(" font-family=\"%1\"").arg(tlb->getProperty(isStart ? Pid::BEGIN_FONT_FACE
                                                                       : Pid::END_FONT_FACE).value<String>());
            tag += QString(" font-size=\"%1\"").arg(tlb->getProperty(isStart ? Pid::BEGIN_FONT_SIZE : Pid::END_FONT_SIZE).toReal());
            tag += fontStyleToXML(static_cast<FontStyle>(tlb->getProperty(isStart ? Pid::BEGIN_FONT_STYLE : Pid::END_FONT_STYLE).toInt()));
            tag += color2xml(tlb);
            tag += ExportMusicXml::positioningAttributes(tlb, isStart);
            xml.tagRaw(tag, dynamicPosition == -1 ? text : text.left(dynamicPosition));
            xml.endElement();
            if (dynamicPosition == -1) {
                text = "";
            } else if (dynamicPosition > 0) {
                text.remove(0, dynamicPosition);
                dynamicPosition = 0;
            }
        }
        if (dynamicPosition == 0) {
            // dynamic at front of text
            xml.startElement("direction-type");
            QString tag = "dynamics";
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
    const auto isLineType = hp->isLineType();
    int n;
    if (isLineType) {
        if (!hp->lineVisible() && ((hp->beginText().isEmpty() && hp->tick() == tick)
                                   || (hp->endText().isEmpty() && hp->tick() != tick))) {
            return;
        }
        n = findDashes(hp);
        if (n >= 0) {
            dashes[n] = nullptr;
        } else {
            n = findDashes(nullptr);
            if (n >= 0) {
                dashes[n] = hp;
            } else {
                LOGD("too many overlapping dashes (hp %p staff %zu tick %d)", hp, staff, tick.ticks());
                return;
            }
        }
    } else {
        n = findHairpin(hp);
        if (n >= 0) {
            hairpins[n] = nullptr;
        } else {
            n = findHairpin(nullptr);
            if (n >= 0) {
                hairpins[n] = hp;
            } else {
                LOGD("too many overlapping hairpins (hp %p staff %zu tick %d)", hp, staff, tick.ticks());
                return;
            }
        }
    }

    directionTag(_xml, _attr, hp);
    if (hp->tick() == tick) {
        writeHairpinText(_xml, hp, hp->tick() == tick);
    }
    if (isLineType) {
        if (hp->lineVisible()) {
            if (hp->tick() == tick) {
                _xml.startElement("direction-type");
                QString tag = "dashes type=\"start\"";
                tag += QString(" number=\"%1\"").arg(n + 1);
                tag += color2xml(hp);
                tag += positioningAttributes(hp, hp->tick() == tick);
                _xml.tagRaw(tag);
                _xml.endElement();
            } else {
                _xml.startElement("direction-type");
                _xml.tagRaw(QString("dashes type=\"stop\" number=\"%1\"").arg(n + 1));
                _xml.endElement();
            }
        }
    } else {
        _xml.startElement("direction-type");
        QString tag = "wedge type=";
        if (hp->tick() == tick) {
            if (hp->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                tag += "\"crescendo\"";
                if (hp->hairpinCircledTip()) {
                    tag += " niente=\"yes\"";
                }
            } else {
                tag += "\"diminuendo\"";
            }
            tag += color2xml(hp);
        } else {
            tag += "\"stop\"";
            if (hp->hairpinCircledTip() && hp->hairpinType() == HairpinType::DECRESC_HAIRPIN) {
                tag += " niente=\"yes\"";
            }
        }
        tag += QString(" number=\"%1\"").arg(n + 1);
        tag += positioningAttributes(hp, hp->tick() == tick);
        _xml.tagRaw(tag);
        _xml.endElement();
    }
    if (hp->tick() != tick) {
        writeHairpinText(_xml, hp, hp->tick() == tick);
    }
    directionETag(_xml, staff);
}

//---------------------------------------------------------
//   findOttava -- get index of ottava in ottava table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findOttava(const Ottava* ot) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (ottavas[i] == ot) {
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
    auto n = findOttava(ot);
    if (n >= 0) {
        ottavas[n] = 0;
    } else {
        n = findOttava(0);
        if (n >= 0) {
            ottavas[n] = ot;
        } else {
            LOGD("too many overlapping ottavas (ot %p staff %zu tick %d)", ot, staff, tick.ticks());
            return;
        }
    }

    QString octaveShiftXml;
    const auto st = ot->ottavaType();
    if (ot->tick() == tick) {
        const char* sz = 0;
        const char* tp = 0;
        switch (st) {
        case OttavaType::OTTAVA_8VA:
            sz = "8";
            tp = "down";
            break;
        case OttavaType::OTTAVA_15MA:
            sz = "15";
            tp = "down";
            break;
        case OttavaType::OTTAVA_8VB:
            sz = "8";
            tp = "up";
            break;
        case OttavaType::OTTAVA_15MB:
            sz = "15";
            tp = "up";
            break;
        default:
            LOGD("ottava subtype %d not understood", int(st));
        }
        if (sz && tp) {
            octaveShiftXml = QString("octave-shift type=\"%1\" size=\"%2\" number=\"%3\"").arg(tp, sz).arg(n + 1);
        }
    } else {
        if (st == OttavaType::OTTAVA_8VA || st == OttavaType::OTTAVA_8VB) {
            octaveShiftXml = QString("octave-shift type=\"stop\" size=\"8\" number=\"%1\"").arg(n + 1);
        } else if (st == OttavaType::OTTAVA_15MA || st == OttavaType::OTTAVA_15MB) {
            octaveShiftXml = QString("octave-shift type=\"stop\" size=\"15\" number=\"%1\"").arg(n + 1);
        } else {
            LOGD("ottava subtype %d not understood", int(st));
        }
    }

    if (octaveShiftXml != "") {
        directionTag(_xml, _attr, ot);
        _xml.startElement("direction-type");
        octaveShiftXml += color2xml(ot);
        octaveShiftXml += positioningAttributes(ot, ot->tick() == tick);
        _xml.tagRaw(octaveShiftXml);
        _xml.endElement();
        directionETag(_xml, staff);
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

    directionTag(_xml, _attr, pd);
    _xml.startElement("direction-type");
    QString pedalType;
    QString pedalXml;
    QString signText;
    QString lineText = pd->lineVisible() ? " line=\"yes\"" : " line=\"no\"";
    if (pd->tick() == tick) {
        switch (pd->beginHookType()) {
        case HookType::HOOK_45:
            pedalType = "change";
            break;
        case HookType::NONE:
            pedalType = pd->lineVisible() ? "resume" : "start";
            break;
        default:
            pedalType = "start";
        }
        signText = pd->beginText().isEmpty() ? " sign=\"no\"" : " sign=\"yes\"";
        if (pd->beginText() == u"<sym>keyboardPedalSost</sym>" || pd->beginText() == u"<sym>keyboardPedalS</sym>") {
            pedalType = "sostenuto";
        }
    } else {
        if (!pd->endText().isEmpty() || pd->endHookType() == HookType::HOOK_90) {
            pedalType = "stop";
        } else {
            pedalType = "discontinue";
        }
        // "change" type is handled only on the beginning of pedal lines

        signText = pd->endText().isEmpty() ? " sign=\"no\"" : " sign=\"yes\"";
    }
    pedalXml = QString("pedal type=\"%1\"").arg(pedalType);
    pedalXml += lineText;
    pedalXml += signText;
    pedalXml += color2xml(pd);
    pedalXml += positioningAttributes(pd, pd->tick() == tick);
    _xml.tagRaw(pedalXml);
    _xml.endElement();
    directionETag(_xml, staff);
}

//---------------------------------------------------------
//   findBracket -- get index of bracket in bracket table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findBracket(const TextLineBase* tl) const
{
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        if (brackets[i] == tl) {
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
    using namespace mu::draw;

    if (!tl->lineVisible() && ((tl->beginText().isEmpty() && tl->tick() == tick)
                               || (tl->endText().isEmpty() && tl->tick() != tick))) {
        return;
    }

    int n;
    // special case: a dashed line w/o hooks is written as dashes
    const auto isDashes = tl->lineStyle() == LineType::DASHED && (tl->beginHookType() == HookType::NONE)
                          && (tl->endHookType() == HookType::NONE);

    if (isDashes) {
        n = findDashes(tl);
        if (n >= 0) {
            dashes[n] = nullptr;
        } else {
            n = findBracket(nullptr);
            if (n >= 0) {
                dashes[n] = tl;
            } else {
                LOGD("too many overlapping dashes (tl %p staff %zu tick %d)", tl, staff, tick.ticks());
                return;
            }
        }
    } else {
        n = findBracket(tl);
        if (n >= 0) {
            brackets[n] = nullptr;
        } else {
            n = findBracket(nullptr);
            if (n >= 0) {
                brackets[n] = tl;
            } else {
                LOGD("too many overlapping textlines (tl %p staff %zu tick %d)", tl, staff, tick.ticks());
                return;
            }
        }
    }

    QString rest;
    QPointF p;

    QString type;
    HookType hookType = HookType::NONE;
    double hookHeight = 0.0;
    if (tl->tick() == tick) {
        if (!isDashes) {
            QString lineType;
            switch (tl->lineStyle()) {
            case LineType::SOLID:
                lineType = "solid";
                break;
            case LineType::DASHED:
                lineType = "dashed";
                break;
            case LineType::DOTTED:
                lineType = "dotted";
                break;
            }
            rest += QString(" line-type=\"%1\"").arg(lineType);
        }
        hookType   = tl->beginHookType();
        hookHeight = tl->beginHookHeight().val();
        if (!tl->segmentsEmpty()) {
            p = tl->frontSegment()->offset().toQPointF();
        }
        // offs = tl->mxmlOff();
        type = "start";
    } else {
        hookType   = tl->endHookType();
        hookHeight = tl->endHookHeight().val();
        if (!tl->segmentsEmpty()) {
            p = (toLineSegment(tl->backSegment()))->userOff2().toQPointF();
        }
        // offs = tl->mxmlOff2();
        type = "stop";
    }

    QString lineEnd;
    switch (hookType) {
    case HookType::HOOK_90T:
        lineEnd = "both";
        rest += QString(" end-length=\"%1\"").arg(std::abs(hookHeight * 20));
        break;
    case HookType::HOOK_90:
        lineEnd = (hookHeight < 0.0) ? "up" : "down";
        rest += QString(" end-length=\"%1\"").arg(std::abs(hookHeight * 10));
        break;
    case HookType::NONE:
        lineEnd = "none";
        break;
    default:
        lineEnd = "none";
        LOGD("HookType %d not supported", int(hookType));
    }

    rest += color2xml(tl);
    rest += positioningAttributes(tl, tl->tick() == tick);

    directionTag(_xml, _attr, tl);

    if (!tl->beginText().isEmpty() && tl->tick() == tick) {
        _xml.startElement("direction-type");
        _xml.tag("words", tl->beginText());
        _xml.endElement();
    }

    if (tl->lineVisible()) {
        _xml.startElement("direction-type");
        if (isDashes) {
            _xml.tag("dashes", { { "type", type }, { "number", n + 1 } });
        } else {
            _xml.tagRaw(QString("bracket type=\"%1\" number=\"%2\" line-end=\"%3\"%4").arg(type, QString::number(n + 1), lineEnd, rest));
        }
        _xml.endElement();
    }

    if (!tl->endText().isEmpty() && tl->tick() != tick) {
        _xml.startElement("direction-type");
        _xml.tag("words", tl->endText());
        _xml.endElement();
    }

    /*
    if (offs)
          xml.tag("offset", offs);
    */

    directionETag(_xml, staff);
}

//---------------------------------------------------------
//   dynamic
//---------------------------------------------------------

// In MuseScore dynamics are essentially user-defined texts, therefore the ones
// supported by MusicXML need to be filtered out. Everything not recognized
// as MusicXML dynamics is written as other-dynamics.

void ExportMusicXml::dynamic(Dynamic const* const dyn, staff_idx_t staff)
{
    QSet<QString> set;   // the valid MusicXML dynamics
    set << "f" << "ff" << "fff" << "ffff" << "fffff" << "ffffff"
        << "fp" << "fz"
        << "mf" << "mp"
        << "p" << "pp" << "ppp" << "pppp" << "ppppp" << "pppppp"
        << "rf" << "rfz"
        << "sf" << "sffz" << "sfp" << "sfpp" << "sfz";

    directionTag(_xml, _attr, dyn);

    _xml.startElement("direction-type");

    QString tagName = "dynamics";
    tagName += color2xml(dyn);
    tagName += positioningAttributes(dyn);
    _xml.startElementRaw(tagName);
    const QString dynTypeName = TConv::toXml(dyn->dynamicType()).ascii();
    bool hasCustomText = dyn->hasCustomText();

    if (set.contains(dynTypeName) && !hasCustomText) {
        _xml.tagRaw(dynTypeName);
    } else if (dynTypeName != "") {
        std::map<ushort, QChar> map;
        map[0xE520] = 'p';
        map[0xE521] = 'm';
        map[0xE522] = 'f';
        map[0xE523] = 'r';
        map[0xE524] = 's';
        map[0xE525] = 'z';
        map[0xE526] = 'n';

        QString dynText = dynTypeName;
        if (dyn->dynamicType() == DynamicType::OTHER || hasCustomText) {
            dynText = dyn->plainText();
        }

        // collect consecutive runs of either dynamics glyphs
        // or other characters and write the runs.
        QString text;
        bool inDynamicsSym = false;
        for (const auto& ch : qAsConst(dynText)) {
            const auto it = map.find(ch.unicode());
            if (it != map.end()) {
                // found a SMUFL single letter dynamics glyph
                if (!inDynamicsSym) {
                    if (text != "") {
                        _xml.tag("other-dynamics", text);
                        text = "";
                    }
                    inDynamicsSym = true;
                }
                text += it->second;
            } else {
                // found a non-dynamics character
                if (inDynamicsSym) {
                    if (text != "") {
                        if (set.contains(text)) {
                            _xml.tagRaw(text);
                        } else {
                            _xml.tag("other-dynamics", text);
                        }
                        text = "";
                    }
                    inDynamicsSym = false;
                }
                text += ch;
            }
        }
        if (text != "") {
            if (inDynamicsSym && set.contains(text)) {
                _xml.tagRaw(text);
            } else {
                _xml.tag("other-dynamics", text);
            }
        }
    }

    _xml.endElement();

    _xml.endElement();

    const auto offset = calculateTimeDeltaInDivisions(dyn->tick(), tick(), div);
    if (offset) {
        _xml.tag("offset", offset);
    }

    if (staff) {
        _xml.tag("staff", static_cast<int>(staff));
    }

    if (dyn->velocity() > 0) {
        _xml.tagRaw(QString("sound dynamics=\"%1\"").arg(QString::number(dyn->velocity() * 100.0 / 90.0, 'f', 2)));
    }

    _xml.endElement();
}

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

// TODO: remove dependency on symbol name and replace by a more stable interface
// changes in sym.cpp r2494 broke MusicXML export of pedals (again)

void ExportMusicXml::symbol(Symbol const* const sym, staff_idx_t staff)
{
    AsciiStringView name = SymNames::nameForSymId(sym->sym());
    QString mxmlName = "";
    if (name == "keyboardPedalPed") {
        mxmlName = "pedal type=\"start\"";
    } else if (name == "keyboardPedalUp") {
        mxmlName = "pedal type=\"stop\"";
    } else {
        mxmlName = QString("other-direction smufl=\"%1\"").arg(name.ascii());
    }
    directionTag(_xml, _attr, sym);
    mxmlName += color2xml(sym);
    mxmlName += positioningAttributes(sym);
    _xml.startElement("direction-type");
    _xml.tagRaw(mxmlName);
    _xml.endElement();
    const auto offset = calculateTimeDeltaInDivisions(sym->tick(), tick(), div);
    if (offset) {
        _xml.tag("offset", offset);
    }
    directionETag(_xml, staff);
}

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

void ExportMusicXml::lyrics(const std::vector<Lyrics*>& ll, const track_idx_t trk)
{
    for (const Lyrics* l : ll) {
        if (l && !l->xmlText().isEmpty()) {
            if ((l)->track() == trk) {
                QString lyricXml = QString("lyric number=\"%1\"").arg((l)->no() + 1);
                lyricXml += color2xml(l);
                lyricXml += positioningAttributes(l);
                _xml.startElementRaw(lyricXml);
                LyricsSyllabic syl = (l)->syllabic();
                QString s = "";
                switch (syl) {
                case LyricsSyllabic::SINGLE: s = "single";
                    break;
                case LyricsSyllabic::BEGIN:  s = "begin";
                    break;
                case LyricsSyllabic::END:    s = "end";
                    break;
                case LyricsSyllabic::MIDDLE: s = "middle";
                    break;
                default:
                    LOGD("unknown syllabic %d", int(syl));
                }
                _xml.tag("syllabic", s);
                QString attr;         // TODO TBD
                // set the default words format
                const QString mtf       = _score->style().styleSt(Sid::MusicalTextFont);
                CharFormat defFmt;
                defFmt.setFontFamily(_score->style().styleSt(Sid::lyricsEvenFontFace));
                defFmt.setFontSize(_score->style().styleD(Sid::lyricsOddFontSize));
                // write formatted
                MScoreTextToMXML mttm("text", attr, defFmt, mtf);
                mttm.writeTextFragments(l->fragmentList(), _xml);
                if (l->ticks().isNotZero()) {
                    _xml.tag("extend");
                }
                _xml.endElement();
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
        if (jp->xmlText() == "") {
            words = u"D.C.";
        } else {
            words = jp->xmlText();
        }
        isDaCapo = true;
    } else if (jtp == JumpType::DC_AL_FINE) {
        if (jp->xmlText() == "") {
            words = u"D.C. al Fine";
        } else {
            words = jp->xmlText();
        }
        isDaCapo = true;
    } else if (jtp == JumpType::DC_AL_CODA) {
        if (jp->xmlText() == "") {
            words = u"D.C. al Coda";
        } else {
            words = jp->xmlText();
        }
        isDaCapo = true;
    } else if (jtp == JumpType::DS_AL_CODA) {
        if (jp->xmlText() == "") {
            words = u"D.S. al Coda";
        } else {
            words = jp->xmlText();
        }
        isDalSegno = true;
    } else if (jtp == JumpType::DS_AL_FINE) {
        if (jp->xmlText() == "") {
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
        if (jp->jumpTo() == "") {
            sound = u"dalsegno=\"1\"";
        } else {
            sound = u"dalsegno=\"" + jp->jumpTo() + u"\"";
        }
    }

    if (sound != "") {
        xml.startElement("direction", { { "placement", (jp->placement() == PlacementV::BELOW) ? "below" : "above" } });
        xml.startElement("direction-type");
        String positioning = ExportMusicXml::positioningAttributes(jp);
        if (type != "") {
            xml.tagRaw(type + positioning);
        }
        if (words != "") {
            xml.tagRaw(u"words" + positioning, words);
        }
        xml.endElement();
        if (sound != "") {
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
    const QString label = m->label();

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
                LOGD("Cannot guess type for marker with label=\"%s\"", qPrintable(label));
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

static QString findCodaLabel(const std::vector<const Jump*>& jumps, const QString& toCodaLabel)
{
    for (const Jump* j : jumps) {
        if (j->playUntil() == toCodaLabel) {
            return j->continueAt();
        }
    }

    return QString();
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
            sound = "segno=\"1\"";
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
            words = "To Coda";
        } else {
            words = m->xmlText();
        }
        const String codaLabel = findCodaLabel(jumps, m->label());
        if (codaLabel == "") {
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

    if (sound != u"") {
        xml.startElement("direction", { { "placement", (m->placement() == PlacementV::BELOW) ? "below" : "above" } });
        xml.startElement("direction-type");
        String positioning = ExportMusicXml::positioningAttributes(m);
        if (type != u"") {
            xml.tagRaw(type + positioning);
        }
        if (words != u"") {
            xml.tagRaw(u"words" + positioning, words);
        }
        xml.endElement();
        if (sound != u"") {
            xml.tagRaw(String("sound ") + sound);
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
    if (seg->segmentType() != SegmentType::ChordRest) {
        return mu::nidx;
    }

    staff_idx_t staff = track / VOICES;
    track_idx_t strack = staff * VOICES;        // start track of staff containing track
    track_idx_t etrack = strack + VOICES;       // end track of staff containing track + 1

    for (track_idx_t i = strack; i < etrack; i++) {
        if (seg->element(i)) {
            return i;
        }
    }

    return mu::nidx;
}

//---------------------------------------------------------
//  repeatAtMeasureStart -- write repeats at begin of measure
//---------------------------------------------------------

void ExportMusicXml::repeatAtMeasureStart(Attributes& attr, const Measure* const m, track_idx_t strack, track_idx_t etrack,
                                          track_idx_t track)
{
    // loop over all segments
    for (EngravingItem* e : m->el()) {
        track_idx_t wtrack = mu::nidx;     // track to write jump
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
            const MarkerType mtp = getEffectiveMarkerType(mk, _jumpElements);

            switch (mtp) {
            case MarkerType::SEGNO:
            case MarkerType::VARSEGNO:
            case MarkerType::CODA:
            case MarkerType::VARCODA:
            case MarkerType::CODETTA:
                LOGD(" -> handled");
                attr.doAttr(_xml, false);
                directionMarker(_xml, mk, _jumpElements);
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
        track_idx_t wtrack = mu::nidx;     // track to write jump
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
            const MarkerType mtp = getEffectiveMarkerType(mk, _jumpElements);

            switch (mtp) {
            case MarkerType::FINE:
            case MarkerType::TOCODA:
            case MarkerType::TOCODASYM:
            case MarkerType::DA_CODA:
            case MarkerType::DA_DBLCODA:
                directionMarker(_xml, mk, _jumpElements);
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
            directionJump(_xml, toJump(e));
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
    QString workTitle  = _score->metaTag(u"workTitle");
    QString workNumber = _score->metaTag(u"workNumber");
    if (!(workTitle.isEmpty() && workNumber.isEmpty())) {
        _xml.startElement("work");
        if (!workNumber.isEmpty()) {
            _xml.tag("work-number", workNumber);
        }
        if (!workTitle.isEmpty()) {
            _xml.tag("work-title", workTitle);
        }
        _xml.endElement();
    }
    if (!_score->metaTag(u"movementNumber").isEmpty()) {
        _xml.tag("movement-number", _score->metaTag(u"movementNumber"));
    }
    if (!_score->metaTag(u"movementTitle").isEmpty()) {
        _xml.tag("movement-title", _score->metaTag(u"movementTitle"));
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
        if (m->isMeasureRepeatGroup(staffIdx)
            && (!m->prevMeasure() || !m->prevMeasure()->isMeasureRepeatGroup(staffIdx)
                || (m->measureRepeatNumMeasures(staffIdx) != m->prevMeasure()->measureRepeatNumMeasures(staffIdx)))) {
            attr.doAttr(xml, true);
            xml.startElement("measure-style", { { "number", i + 1 } });
            int numMeasures = m->measureRepeatNumMeasures(staffIdx);
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
            xml.startElement("measure-style", { { "number", i + 1 } });
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
        xml.tag("multiple-rest", mmR1->mmRestCount());
        xml.endElement();
    } else {
        // measure repeat can only possibly be present if mmrest was not
        measureRepeat(xml, attr, m, partIndex);
    }
}

//---------------------------------------------------------
//  findFretDiagram
//---------------------------------------------------------

static const FretDiagram* findFretDiagram(track_idx_t strack, track_idx_t etrack, track_idx_t track, Segment* seg)
{
    if (seg->segmentType() == SegmentType::ChordRest) {
        for (const EngravingItem* e : seg->annotations()) {
            track_idx_t wtrack = mu::nidx;       // track to write annotation

            if (strack <= e->track() && e->track() < etrack) {
                wtrack = findTrackForAnnotations(e->track(), seg);
            }

            if (track == wtrack && e->type() == ElementType::FRET_DIAGRAM) {
                return static_cast<const FretDiagram*>(e);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//  commonAnnotations
//---------------------------------------------------------

static bool commonAnnotations(ExportMusicXml* exp, const EngravingItem* e, staff_idx_t sstaff)
{
    bool instrChangeHandled  = false;

    // note: write the changed instrument details (transposition) here,
    // optionally writing the associated staff text is done below
    if (e->isInstrumentChange()) {
        const auto instrChange = toInstrumentChange(e);
        exp->writeInstrumentDetails(instrChange->instrument());
        instrChangeHandled = true;
    }

    if (e->isSymbol()) {
        exp->symbol(toSymbol(e), sstaff);
    } else if (e->isTempoText()) {
        exp->tempoText(toTempoText(e), sstaff);
    } else if (e->isPlayTechAnnotation() || e->isCapo() || e->isStringTunings() || e->isStaffText() || e->isSystemText()
               || e->isTripletFeel() || e->isText()
               || e->isExpression() || (e->isInstrumentChange() && e->visible())) {
        exp->words(toTextBase(e), sstaff);
    } else if (e->isDynamic()) {
        exp->dynamic(toDynamic(e), sstaff);
    } else if (e->isRehearsalMark()) {
        exp->rehearsal(toRehearsalMark(e), sstaff);
    } else {
        return instrChangeHandled;
    }

    return true;
}

//---------------------------------------------------------
//  annotations
//---------------------------------------------------------

/*
 * Write annotations that are attached to chords or rests
 */

// In MuseScore, EngravingItem::FRET_DIAGRAM and EngravingItem::HARMONY are separate annotations,
// in MusicXML they are combined in the harmony element. This means they have to be matched.
// TODO: replace/repair current algorithm (which can only handle one FRET_DIAGRAM and one HARMONY)

static void annotations(ExportMusicXml* exp, track_idx_t strack, track_idx_t etrack, track_idx_t track, staff_idx_t sstaff, Segment* seg)
{
    if (seg->segmentType() == SegmentType::ChordRest) {
        const FretDiagram* fd = findFretDiagram(strack, etrack, track, seg);
        // if (fd) LOGD("annotations seg %p found fretboard diagram %p", seg, fd);

        for (const EngravingItem* e : seg->annotations()) {
            if (!exp->canWrite(e)) {
                continue;
            }

            track_idx_t wtrack = mu::nidx;       // track to write annotation

            if (strack <= e->track() && e->track() < etrack) {
                wtrack = findTrackForAnnotations(e->track(), seg);
            }

            if (track == wtrack) {
                if (commonAnnotations(exp, e, sstaff)) {
                    // already handled
                } else if (e->isHarmony()) {
                    // LOGD("annotations seg %p found harmony %p", seg, e);
                    exp->harmony(toHarmony(e), fd);
                    fd = nullptr;           // make sure to write only once ...
                } else if (e->isFermata() || e->isFiguredBass() || e->isFretDiagram() || e->isJump()) {
                    // handled separately by chordAttributes(), figuredBass(), findFretDiagram() or ignored
                } else {
                    LOGD("direction type %s at tick %d not implemented",
                         e->typeName(), seg->tick().ticks());
                }
            }
        }
        if (fd) {
            // found fd but no harmony, cannot write (MusicXML would be invalid)
            LOGD("seg %p found fretboard diagram %p w/o harmony: cannot write",
                 seg, fd);
        }
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

static String Modifier2MusicXML(FiguredBassItem::Modifier prefix)
{
    switch (prefix) {
    case FiguredBassItem::Modifier::NONE:        return u"";
    case FiguredBassItem::Modifier::DOUBLEFLAT:  return u"flat-flat";
    case FiguredBassItem::Modifier::FLAT:        return u"flat";
    case FiguredBassItem::Modifier::NATURAL:     return u"natural";
    case FiguredBassItem::Modifier::SHARP:       return u"sharp";
    case FiguredBassItem::Modifier::DOUBLESHARP: return u"double-sharp";
    case FiguredBassItem::Modifier::CROSS:       return u"cross";
    case FiguredBassItem::Modifier::BACKSLASH:   return u"backslash";
    case FiguredBassItem::Modifier::SLASH:       return u"slash";
    case FiguredBassItem::Modifier::NUMOF:       return u"";         // prevent gcc "‘FBINumOfAccid’ not handled in switch" warning
    }
    return u"";
}

static void writeMusicXML(const FiguredBassItem* item, XmlWriter& xml, bool isOriginalFigure, int crEndTick, int fbEndTick)
{
    xml.startElement("figure");

    // The first figure of each group is the "original" figure. Practically, it is one inserted manually
    // by the user, rather than automatically by the "duration" extend method.
    if (isOriginalFigure) {
        String strPrefix = Modifier2MusicXML(item->prefix());
        if (strPrefix != "") {
            xml.tag("prefix", strPrefix);
        }
        if (item->digit() != FBIDigitNone) {
            xml.tag("figure-number", item->digit());
        }
        String strSuffix = Modifier2MusicXML(item->suffix());
        if (strSuffix != "") {
            xml.tag("suffix", strSuffix);
        }

        // Check if the figure ends before or at the same time as the current note. Otherwise, the figure
        // extends to the next note, and so carries an extension type "start" by definition.
        if (fbEndTick <= crEndTick) {
            if (item->contLine() == FiguredBassItem::ContLine::SIMPLE) {
                xml.tag("extend", { { "type", "stop" } });
            } else if (item->contLine() == FiguredBassItem::ContLine::EXTENDED) {
                bool hasFigure = (strPrefix != "" || item->digit() != FBIDigitNone || strSuffix != "");
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

static void writeMusicXML(const FiguredBass* item, XmlWriter& xml, bool isOriginalFigure, int crEndTick, int fbEndTick, bool writeDuration,
                          int divisions)
{
    XmlWriter::Attributes attrs;
    if (item->hasParentheses()) {
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
        writeMusicXML(fbItem, xml, isOriginalFigure, crEndTick, fbEndTick);
    }
    if (writeDuration) {
        xml.tag("duration", item->ticks().ticks() / divisions);
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
            track_idx_t wtrack = mu::nidx;       // track to write annotation

            if (strack <= e->track() && e->track() < etrack) {
                wtrack = findTrackForAnnotations(e->track(), seg);
            }

            if (track == wtrack) {
                if (e->type() == ElementType::FIGURED_BASS) {
                    const FiguredBass* fb = dynamic_cast<const FiguredBass*>(e);
                    //LOGD("figuredbass() track %d seg %p fb %p seg %p tick %d ticks %d cr %p tick %d ticks %d",
                    //       track, seg, fb, fb->segment(), fb->segment()->tick(), fb->ticks(), cr, cr->tick(), cr->actualTicks());
                    bool extend = fb->ticks() > cr->actualTicks();
                    if (extend) {
                        //LOGD("figuredbass() extend to %d + %d = %d",
                        //       cr->tick(), fb->ticks(), cr->tick() + fb->ticks());
                        fbMap.insert(strack, fb);
                    } else {
                        fbMap.remove(strack);
                    }
                    const Fraction crEndTick = cr->tick() + cr->actualTicks();
                    const Fraction fbEndTick = fb->segment()->tick() + fb->ticks();
                    const bool writeDuration = fb->ticks() < cr->actualTicks();
                    writeMusicXML(fb, xml, true, crEndTick.ticks(), fbEndTick.ticks(),
                                  writeDuration, divisions);

                    // Check for changing figures under a single note (each figure stored in a separate segment)
                    for (Segment* segNext = seg->next(); segNext && segNext->element(track) == NULL; segNext = segNext->next()) {
                        for (EngravingItem* annot : segNext->annotations()) {
                            if (annot->type() == ElementType::FIGURED_BASS && annot->track() == track) {
                                fb = dynamic_cast<const FiguredBass*>(annot);
                                writeMusicXML(fb, xml, true, 0, 0, true, divisions);
                            }
                        }
                    }
                    // no extend can be pending
                    return;
                }
            }
        }
        // check for extend pending
        if (fbMap.contains(strack)) {
            const FiguredBass* fb = fbMap.value(strack);
            Fraction crEndTick = cr->tick() + cr->actualTicks();
            Fraction fbEndTick = fb->segment()->tick() + fb->ticks();
            bool writeDuration = fb->ticks() < cr->actualTicks();
            if (cr->tick() < fbEndTick) {
                //LOGD("figuredbass() at tick %d extend only", cr->tick());
                writeMusicXML(fb, xml, false, crEndTick.ticks(), fbEndTick.ticks(), writeDuration, divisions);
            }
            if (fbEndTick <= crEndTick) {
                //LOGD("figuredbass() at tick %d extend done", cr->tick() + cr->actualTicks());
                fbMap.remove(strack);
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
    if (seg->segmentType() == SegmentType::ChordRest) {
        Fraction stick = seg->tick();
        for (auto it = exp->score()->spanner().lower_bound(stick.ticks()); it != exp->score()->spanner().upper_bound(stick.ticks()); ++it) {
            Spanner* e = it->second;

            if (!exp->canWrite(e)) {
                continue;
            }

            track_idx_t wtrack = mu::nidx;       // track to write spanner
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
                        QSet<const Spanner*>& stopped)
{
    for (auto it : exp->score()->spanner()) {
        Spanner* e = it.second;

        if (!exp->canWrite(e)) {
            continue;
        }

        if (e->tick2() != tick2 || e->track() < strack || e->track() >= etrack) {
            continue;
        }

        if (!stopped.contains(e)) {
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
                exp->textLine(toTextLineBase(e), sstaff, Fraction(-1, 1));
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
            if (!mu::contains(keysigs, i)) {
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
            for (staff_idx_t st : mu::keys(keysigs)) {
                keysig(keysigs.at(st), p->staff(st)->clef(m->tick()), st + 1, keysigs.at(st)->visible());
            }
        }
    } else {
        // always write a keysig at tick = 0
        if (m->tick().isZero()) {
            //KeySigEvent kse;
            //kse.setKey(Key::C);
            KeySig* ks = Factory::createKeySig(_score->dummy()->segment());
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

    QStringList creators;
    // the creator types commonly found in MusicXML
    creators << "arranger" << "composer" << "lyricist" << "poet" << "translator";
    for (const QString& type : qAsConst(creators)) {
        QString creator = score->metaTag(type);
        if (!creator.isEmpty()) {
            xml.tag("creator", { { "type", type } }, creator);
        }
    }

    if (!score->metaTag(u"copyright").isEmpty()) {
        xml.tag("rights", score->metaTag(u"copyright"));
    }

    xml.startElement("encoding");

    if (MScore::debugMode) {
        xml.tag("software", QString("MuseScore 0.7.0"));
        xml.tag("encoding-date", QString("2007-09-10"));
    } else {
        xml.tag("software", QString("MuseScore ") + QString(MUSESCORE_VERSION));
        xml.tag("encoding-date", QDate::currentDate().toString(Qt::ISODate));
    }

    // specify supported elements
    xml.tag("supports", { { "element", "accidental" }, { "type", "yes" } });
    xml.tag("supports", { { "element", "beam" }, { "type", "yes" } });
    // set support for print new-page and new-system to match user preference
    // for MusicxmlExportBreaks::MANUAL support is "no" because "yes" breaks Finale NotePad import
    IMusicXmlConfiguration::MusicxmlExportBreaksType breaksType = configuration()->musicxmlExportBreaksType();
    if (configuration()->musicxmlExportLayout() && breaksType == IMusicXmlConfiguration::MusicxmlExportBreaksType::All) {
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

static void scoreInstrument(XmlWriter& xml, const int partNr, const int instrNr, const QString& instrName,
                            const Instrument* instr = nullptr)
{
    xml.startElementRaw(QString("score-instrument %1").arg(instrId(partNr, instrNr)));
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
    xml.startElementRaw(QString("midi-instrument %1").arg(instrId(partNr, instrNr)));
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

static void initInstrMap(MxmlInstrumentMap& im, const InstrumentList& il, const Score* /*score*/)
{
    im.clear();
    for (const auto& pair : il) {
        const Instrument* instr = pair.second;
        if (!im.contains(instr)) {
            im.insert(instr, im.size());
        }
    }
}

//---------------------------------------------------------
//  initReverseInstrMap
//---------------------------------------------------------

typedef QMap<int, const Instrument*> MxmlReverseInstrumentMap;

/**
 Initialize the number t Instrument* map for a Part
 Used to iterate in sequence over instrument numbers for a multi-instrument part
 */

static void initReverseInstrMap(MxmlReverseInstrumentMap& rim, const MxmlInstrumentMap& im)
{
    rim.clear();
    for (const Instrument* i : im.keys()) {
        int instNr = im.value(i);
        rim.insert(instNr, i);
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
                           const int nrStavesInPart, const MeasurePrintContext& mpc)
{
    const MeasureBase* const prevSysMB = lastMeasureBase(mpc.prevSystem);

    const bool prevMeasLineBreak = prevSysMB ? prevSysMB->lineBreak() : false;
    const bool prevMeasSectionBreak = prevSysMB ? prevSysMB->sectionBreak() : false;
    const bool prevPageBreak = hasPageBreak(mpc.lastSystemPrevPage);

    QString newSystemOrPage;               // new-[system|page]="yes" or empty
    if (!mpc.scoreStart) {
        IMusicXmlConfiguration::MusicxmlExportBreaksType exportBreaksType = configuration()->musicxmlExportBreaksType();

        if (exportBreaksType == IMusicXmlConfiguration::MusicxmlExportBreaksType::All) {
            if (mpc.pageStart) {
                newSystemOrPage = " new-page=\"yes\"";
            } else if (mpc.systemStart) {
                newSystemOrPage = " new-system=\"yes\"";
            }
        } else if (exportBreaksType == IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual) {
            if (mpc.pageStart && prevPageBreak) {
                newSystemOrPage = " new-page=\"yes\"";
            } else if (mpc.systemStart && (prevMeasLineBreak || prevMeasSectionBreak)) {
                newSystemOrPage = " new-system=\"yes\"";
            }
        }
    }

    bool doBreak = mpc.scoreStart || (newSystemOrPage != "");
    bool doLayout = configuration()->musicxmlExportLayout();

    if (doBreak) {
        if (doLayout) {
            _xml.startElementRaw(QString("print%1").arg(newSystemOrPage));
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

                _xml.startElement("system-layout");
                _xml.startElement("system-margins");
                _xml.tag("left-margin", QString("%1").arg(QString::number(systemLM, 'f', 2)));
                _xml.tag("right-margin", QString("%1").arg(QString::number(systemRM, 'f', 2)));
                _xml.endElement();

                if (mpc.systemStart && !mpc.pageStart) {
                    // see System::layout2() for the factor 2 * score()->spatium()
                    const Measure* prevSystemMeasure = mpc.prevMeasure->coveringMMRestOrThis();
                    const double sysDist = getTenthsFromDots(mmR1->pagePos().y()
                                                             - prevSystemMeasure->pagePos().y()
                                                             - prevSystemMeasure->ldata()->bbox().height()
                                                             + 2 * score()->style().spatium()
                                                             );
                    _xml.tag("system-distance", QString("%1").arg(QString::number(sysDist, 'f', 2)));
                }

                if (mpc.pageStart || mpc.scoreStart) {
                    const double topSysDist = getTenthsFromDots(mmR1->pagePos().y()) - tm;
                    _xml.tag("top-system-distance", QString("%1").arg(QString::number(topSysDist, 'f', 2)));
                }

                _xml.endElement();
            }

            // Staff layout elements.
            for (int staffIdx = (firstStaffOfPart == 0) ? 1 : 0; staffIdx < nrStavesInPart; staffIdx++) {
                // calculate distance between this and previous staff using the bounding boxes
                const auto staffNr = firstStaffOfPart + staffIdx;
                const auto prevBbox = system->staff(staffNr - 1)->bbox();
                const auto staffDist = system->staff(staffNr)->bbox().y() - prevBbox.y() - prevBbox.height();

                _xml.startElement("staff-layout", { { "number", staffIdx + 1 } });
                _xml.tag("staff-distance", QString("%1").arg(QString::number(getTenthsFromDots(staffDist), 'f', 2)));
                _xml.endElement();
            }

            _xml.endElement();
        } else if (newSystemOrPage != "") {
            _xml.tagRaw(QString("print%1").arg(newSystemOrPage));
        }
    }
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
    const auto staves = part->nstaves();

    if (m->tick() == Fraction(0, 1)) {
        const auto clefSeg = m->findSegment(SegmentType::HeaderClef, Fraction(0, 1));

        if (clefSeg) {
            for (size_t i = 0; i < staves; ++i) {
                // sstaff - xml staff number, counting from 1 for this
                // instrument
                // special number 0 -> don’t show staff number in
                // xml output (because there is only one staff)

                auto sstaff = (staves > 1) ? i + 1 : 0;
                auto track = part->startTrack() + VOICES * i;

                if (clefSeg->element(track) == nullptr) {
                    ClefType ct { ClefType::G };
                    QString stafftype;
                    switch (part->staff(i)->staffType(Fraction(0, 1))->group()) {
                    case StaffGroup::TAB:
                        ct = ClefType::TAB;
                        stafftype = "tab";
                        break;
                    case StaffGroup::STANDARD:
                        ct = ClefType::G;
                        stafftype = "std";
                        break;
                    case StaffGroup::PERCUSSION:
                        ct = ClefType::PERC;
                        stafftype = "perc";
                        break;
                    }
                    LOGD("no clef found in first measure track %zu (stafftype %s)", track, qPrintable(stafftype));
                    clef(sstaff, ct, " print-object=\"no\"");
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
                    QString clefAttr = color2xml(cle);
                    if (!cle->visible()) {
                        clefAttr += " print-object=\"no\"";
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

typedef QSet<int> pitchSet;       // the set of pitches used

static void addChordPitchesToSet(const Chord* c, pitchSet& set)
{
    for (const Note* note : c->notes()) {
        LOGD("chord %p note %p pitch %d", c, note, note->pitch() + 1);
        set.insert(note->pitch());
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

static void partList(XmlWriter& xml, Score* score, MxmlInstrumentMap& instrMap)
{
    xml.startElement("part-list");
    size_t staffCount = 0;                               // count sum of # staves in parts
    const auto& parts = score->parts();
    int partGroupEnd[MAX_PART_GROUPS];                // staff where part group ends (bracketSpan is in staves, not parts)
    for (int i = 0; i < MAX_PART_GROUPS; i++) {
        partGroupEnd[i] = -1;
    }
    for (size_t idx = 0; idx < parts.size(); ++idx) {
        const auto part = parts.at(idx);
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
                                        partGroupStart(xml, number + 1, st->bracketType(j));
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
                partGroupStart(xml, number + 1, BracketType::NO_BRACKET);
                partGroupEnd[number] = static_cast<int>(idx + part->nstaves());
            }
        }

        xml.startElementRaw(QString("score-part id=\"P%1\"").arg(idx + 1));
        initInstrMap(instrMap, part->instruments(), score);
        // by default export the parts long name as part-name
        if (part->longName() != "") {
            xml.tag("part-name", MScoreTextToMXML::toPlainText(part->longName()));
        } else {
            if (part->partName() != "") {
                // use the track name if no part long name
                // to prevent an empty track name on import
                xml.tag("part-name", { { "print-object", "no" } }, MScoreTextToMXML::toPlainText(part->partName()));
            } else {
                // part-name is required
                xml.tag("part-name", "");
            }
        }
        if (!part->shortName().isEmpty()) {
            xml.tag("part-abbreviation", MScoreTextToMXML::toPlainText(part->shortName()));
        }

        if (part->instrument()->useDrumset()) {
            const Drumset* drumset = part->instrument()->drumset();
            pitchSet pitches;
            findPitchesUsed(part, pitches);
            for (int i = 0; i < 128; ++i) {
                DrumInstrument di = drumset->drum(i);
                if (di.notehead != NoteHeadGroup::HEAD_INVALID) {
                    scoreInstrument(xml, static_cast<int>(idx) + 1, i + 1, di.name);
                } else if (pitches.contains(i)) {
                    scoreInstrument(xml, static_cast<int>(idx) + 1, i + 1, QString("Instrument %1").arg(i + 1));
                }
            }
            int midiPort = part->midiPort() + 1;
            if (midiPort >= 1 && midiPort <= 16) {
                xml.tag("midi-device", { { "port", midiPort } }, "");
            }

            for (int i = 0; i < 128; ++i) {
                DrumInstrument di = drumset->drum(i);
                if (di.notehead != NoteHeadGroup::HEAD_INVALID || pitches.contains(i)) {
                    midiInstrument(xml, static_cast<int>(idx) + 1, i + 1, part->instrument(), score, i + 1);
                }
            }
        } else {
            MxmlReverseInstrumentMap rim;
            initReverseInstrMap(rim, instrMap);
            for (int instNr : rim.keys()) {
                scoreInstrument(xml, static_cast<int>(idx) + 1, instNr + 1,
                                MScoreTextToMXML::toPlainText(rim.value(instNr)->trackName()),
                                rim.value(instNr));
            }
            for (auto ii = rim.constBegin(); ii != rim.constEnd(); ii++) {
                int instNr = ii.key();
                int midiPort = part->midiPort() + 1;
                if (ii.value()->channel().size() > 0) {
                    midiPort = score->masterScore()->midiMapping(ii.value()->channel(0)->channel())->port() + 1;
                }
                if (midiPort >= 1 && midiPort <= 16) {
                    xml.tagRaw(QString("midi-device %1 port=\"%2\"").arg(instrId(static_cast<int>(idx) + 1, instNr + 1)).arg(midiPort), "");
                } else {
                    xml.tagRaw(QString("midi-device %1").arg(instrId(static_cast<int>(idx) + 1, instNr + 1)), "");
                }
                midiInstrument(xml, static_cast<int>(idx) + 1, instNr + 1, rim.value(instNr), score);
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
        const auto cle = toClef(el);
        const auto ti = cle->segment()->tick();
        const QString visible = (!cle->visible()) ? " print-object=\"no\"" : "";
        clefDebug("exportxml: clef in measure ti=%d ct=%d gen=%d", ti, int(cle->clefType()), el->generated());
        if (el->generated()) {
            clefDebug("exportxml: generated clef not exported");
        } else if (!el->generated() && tickIsInMiddleOfMeasure(ti, m)) {
            clef(sstaff, cle->clefType(), color2xml(cle) + visible);
        } else if (!el->generated() && (ti == m->tick()) && (cle->segment()->segmentType() != SegmentType::HeaderClef)) {
            clef(sstaff, cle->clefType(), color2xml(cle) + visible + QString(" after-barline=\"yes\""));
        } else {
            clefDebug("exportxml: clef not exported");
        }
    } else if (el->isChord()) {
        const auto c = toChord(el);
        // ise grace after
        if (c) {
            const auto ll = c->lyrics();
            for (const auto g : c->graceNotesBefore()) {
                chord(g, sstaff, ll, useDrumset);
            }
            chord(c, sstaff, ll, useDrumset);
            for (const auto g : c->graceNotesAfter()) {
                chord(g, sstaff, ll, useDrumset);
            }
        }
    } else if (el->isRest()) {
        const auto r = toRest(el);
        if (!(r->isGap())) {
            const auto ll = r->lyrics();
            rest(r, sstaff, ll);
        }
    } else if (el->isBarLine()) {
        const auto barln = toBarLine(el);
        if (tickIsInMiddleOfMeasure(barln->tick(), m)) {
            barlineMiddle(barln);
        }
    } else if (el->isKeySig() || el->isTimeSig() || el->isBreath()) {
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

static void writeStaffDetails(XmlWriter& xml, const Part* part)
{
    const Instrument* instrument = part->instrument();
    size_t staves = part->nstaves();

    // staff details
    // TODO: decide how to handle linked regular / TAB staff
    //       currently exported as a two staff part ...
    for (size_t i = 0; i < staves; i++) {
        Staff* st = part->staff(i);
        if (st->lines(Fraction(0, 1)) != 5 || st->isTabStaff(Fraction(0, 1)) || !st->show()) {
            XmlWriter::Attributes attributes;
            if (staves > 1) {
                attributes.push_back({ "number", i + 1 });
            }
            if (!st->show()) {
                attributes.push_back({ "print-object", "no" });
            }
            xml.startElement("staff-details", attributes);

            xml.tag("staff-lines", st->lines(Fraction(0, 1)));
            if (st->isTabStaff(Fraction(0, 1)) && instrument->stringData()) {
                std::vector<instrString> l = instrument->stringData()->stringList();
                for (size_t ii = 0; ii < l.size(); ii++) {
                    char step  = ' ';
                    int alter  = 0;
                    int octave = 0;
                    midipitch2xml(l.at(ii).pitch, step, alter, octave);
                    xml.startElement("staff-tuning", { { "line", ii + 1 } });
                    xml.tag("tuning-step", QString("%1").arg(step));
                    if (alter) {
                        xml.tag("tuning-alter", alter);
                    }
                    clampMusicXmlOctave(octave);
                    xml.tag("tuning-octave", octave);
                    xml.endElement();
                }
            }
            xml.endElement();
        }
    }
}

//---------------------------------------------------------
//  writeInstrumentDetails
//---------------------------------------------------------

/**
 Write the instrument details for \a instrument.
 */

void ExportMusicXml::writeInstrumentDetails(const Instrument* instrument)
{
    if (instrument->transpose().chromatic) {
        _attr.doAttr(_xml, true);
        _xml.startElement("transpose");
        _xml.tag("diatonic",  instrument->transpose().diatonic % 7);
        _xml.tag("chromatic", instrument->transpose().chromatic % 12);
        int octaveChange = instrument->transpose().chromatic / 12;
        if (octaveChange != 0) {
            _xml.tag("octave-change", octaveChange);
        }
        _xml.endElement();
        _attr.doAttr(_xml, false);
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
    for (auto segment = measure->first(); segment; segment = segment->next()) {
        if (segment->segmentType() == SegmentType::ChordRest) {
            for (const auto element : segment->annotations()) {
                if (!element->isFiguredBass() && !element->isHarmony()) {               // handled elsewhere
                    if (!exp->canWrite(element)) {
                        continue;
                    }
                    const track_idx_t wtrack = findTrackForAnnotations(element->track(), segment);           // track to write annotation
                    if (strack <= element->track() && element->track() < (strack + VOICES * staves) && wtrack == mu::nidx) {
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
    _measureNo = 1;
    _irregularMeasureNo = 1;
    _pickupMeasureNo = 1;
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
        const auto layoutSectionBreak = previousMB->sectionBreakElement();
        if (layoutSectionBreak && layoutSectionBreak->startWithMeasureOne()) {
            init();
        }
    }

    // update measure numbers and cache result
    _measureNo += m->noOffset();
    _cachedAttributes = " number=";
    if ((_irregularMeasureNo + _measureNo) == 2 && m->irregular()) {
        _cachedAttributes += "\"0\" implicit=\"yes\"";
        _pickupMeasureNo++;
    } else if (m->irregular()) {
        _cachedAttributes += QString("\"X%1\" implicit=\"yes\"").arg(_irregularMeasureNo++);
    } else {
        _cachedAttributes += QString("\"%1\"").arg(_measureNo++);
    }
}

QString MeasureNumberStateHandler::measureNumber() const
{
    return _cachedAttributes;
}

bool MeasureNumberStateHandler::isFirstActualMeasure() const
{
    return (_irregularMeasureNo + _measureNo + _pickupMeasureNo) == 4;
}

//---------------------------------------------------------
//  findLastSystemWithMeasures
//---------------------------------------------------------

static System* findLastSystemWithMeasures(const Page* const page)
{
    for (int i = static_cast<int>(page->systems().size()) - 1; i >= 0; --i) {
        const auto s = page->systems().at(i);
        const auto m = s->firstMeasure();
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
    const auto system = measure->coveringMMRestOrThis()->system();
    const auto firstMeasureInSystem = system->firstMeasure();
    const auto realFirstMeasureInSystem = firstMeasureInSystem->isMMRest() ? firstMeasureInSystem->mmRestFirst() : firstMeasureInSystem;
    return measure == realFirstMeasureInSystem;
}

//---------------------------------------------------------
//  isFirstMeasureInLastSystem
//---------------------------------------------------------

static bool isFirstMeasureInLastSystem(const Measure* const measure)
{
    const auto system = measure->coveringMMRestOrThis()->system();
    const auto page = system->page();

    /*
     Notes on multi-measure rest handling:
     Function mmRest1() returns either the measure itself (if not part of multi-measure rest)
     or the replacing multi-measure rest measure.
     Using this is required as a measure that is covered by a multi-measure rest has no system.
     Furthermore, the first measure in a system starting with a multi-measure rest is the a multi-
     measure rest itself instead of the first covered measure.
     */

    const auto lastSystem = findLastSystemWithMeasures(page);
    if (!lastSystem) {
        return false;           // degenerate case: no system with measures found
    }
    const auto firstMeasureInLastSystem = lastSystem->firstMeasure();
    const auto realFirstMeasureInLastSystem
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
    const auto system = measure->coveringMMRestOrThis()->system();
    const auto page = system->page();
    const size_t systemIndex = mu::indexOf(page->systems(), system);
    std::vector<TBox*> tboxes;
    if (isFirstMeasureInSystem(measure)) {
        for (int idx = static_cast<int>(systemIndex - 1); idx >= 0 && !systemHasMeasures(page->system(idx)); --idx) {
            const auto sys = page->system(idx);
            for (const auto mb : sys->measures()) {
                if (mb->isTBox()) {
                    auto tbox = toTBox(mb);
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
    const auto system = measure->coveringMMRestOrThis()->system();
    const auto page = system->page();
    const size_t systemIndex = static_cast<int>(mu::indexOf(page->systems(), system));
    std::vector<TBox*> tboxes;
    if (isFirstMeasureInLastSystem(measure)) {
        for (size_t idx = systemIndex + 1; idx < page->systems().size() /* && !systemHasMeasures(page->system(idx))*/;
             ++idx) {
            const auto sys = page->system(static_cast<int>(idx));
            for (const auto mb : sys->measures()) {
                if (mb->isTBox()) {
                    auto tbox = toTBox(mb);
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
                                        QSet<const Spanner*>& spannersStopped)
{
    const auto tboxesAbove = findTextFramesToWriteAsWordsAbove(m);
    const auto tboxesBelow = findTextFramesToWriteAsWordsBelow(m);

    track_idx_t etrack = strack + VOICES;
    for (track_idx_t track = strack; track < etrack; ++track) {
        for (auto seg = m->first(); seg; seg = seg->next()) {
            const auto el = seg->element(track);
            if (!el) {
                continue;
            }
            // must ignore start repeat to prevent spurious backup/forward
            if (el->isBarLine() && toBarLine(el)->barLineType() == BarLineType::START_REPEAT) {
                continue;
            }

            // generate backup or forward to the start time of the element
            if (_tick != seg->tick()) {
                _attr.doAttr(_xml, false);
                moveToTick(seg->tick());
            }

            // handle annotations and spanners (directions attached to this note or rest)
            if (el->isChordRest()) {
                _attr.doAttr(_xml, false);
                const bool isFirstPart = (partIndex == 0);
                const bool isLastPart = (partIndex == (static_cast<int>(_score->parts().size()) - 1));
                if (!_tboxesAboveWritten && isFirstPart) {
                    for (const auto tbox : tboxesAbove) {
                        // note: use mmRest1() to get at a possible multi-measure rest,
                        // as the covered measure would be positioned at 0,0.
                        tboxTextAsWords(tbox->text(), 0, mu::PointF(
                                            tbox->text()->canvasPos() - m->coveringMMRestOrThis()->canvasPos()).toQPointF());
                    }
                    _tboxesAboveWritten = true;
                }
                if (!_tboxesBelowWritten && isLastPart && isLastStaffOfPart) {
                    for (const auto tbox : tboxesBelow) {
                        const auto lastStaffNr = track2staff(track);
                        const auto sys = m->coveringMMRestOrThis()->system();
                        auto textPos = tbox->text()->canvasPos() - m->coveringMMRestOrThis()->canvasPos();
                        if (lastStaffNr < sys->staves().size()) {
                            // convert to position relative to last staff of system
                            textPos.setY(textPos.y() - (sys->staffCanvasYpage(lastStaffNr) - sys->staffCanvasYpage(0)));
                        }
                        tboxTextAsWords(tbox->text(), partRelStaffNo, textPos.toQPointF());
                    }
                    _tboxesBelowWritten = true;
                }
                annotations(this, strack, etrack, track, partRelStaffNo, seg);
                // look for more harmony
                for (auto seg1 = seg->next(); seg1; seg1 = seg1->next()) {
                    if (seg1->isChordRestType()) {
                        const auto el1 = seg1->element(track);
                        if (el1) {           // found a ChordRest, next harmony will be attach to this one
                            break;
                        }
                        for (auto annot : seg1->annotations()) {
                            if (annot->isHarmony() && annot->track() == track) {
                                harmony(toHarmony(annot), 0, (seg1->tick() - seg->tick()).ticks() / div);
                            }
                        }
                    }
                }
                figuredBass(_xml, strack, etrack, track, static_cast<const ChordRest*>(el), fbMap, div);
                spannerStart(this, strack, etrack, track, partRelStaffNo, seg);
            }

            // write element el if necessary
            writeElement(el, m, partRelStaffNo, useDrumset);

            // handle annotations and spanners (directions attached to this note or rest)
            if (el->isChordRest()) {
                const staff_idx_t spannerStaff = track2staff(track);
                const track_idx_t starttrack = staff2track(spannerStaff);
                const track_idx_t endtrack = staff2track(spannerStaff + 1);
                spannerStop(this, starttrack, endtrack, _tick, partRelStaffNo, spannersStopped);
            }
        }           // for (Segment* seg = ...
        _attr.stop(_xml);
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
                                        QSet<const Spanner*>& spannersStopped)
{
    const staff_idx_t endStaff = startStaff + nstaves;
    const Measure* const origM = m;
    _tboxesAboveWritten = false;
    _tboxesBelowWritten = false;

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
                    LOGE() << String("Could not find MeasureRepeat on measure %1, staff %2").arg(m->index()).arg(staffIdx);
                    break;
                }

                for (int i = 0; i < mr->numMeasures() && m->prevMeasure(); ++i) {
                    m = m->prevMeasure();
                }
            }
        }
        // in case m was changed, also rewind _tick so as not to generate unnecessary backup/forward tags
        auto tickDelta = _tick - m->tick();
        _tick -= tickDelta;

        bool isLastStaffOfPart = (endStaff - 1 <= staffIdx); // for writing tboxes below

        writeMeasureTracks(m, partIndex, staff2track(staffIdx), partRelStaffNo, useDrumset, isLastStaffOfPart, fbMap, spannersStopped);

        // restore m and _tick before advancing to next staff in part
        m = origM;
        _tick += tickDelta;
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
                                  QSet<const Spanner*>& spannersStopped)
{
    const auto part = _score->parts().at(partIndex);
    const size_t staves = part->nstaves();
    const track_idx_t strack = part->startTrack();
    const track_idx_t etrack = part->endTrack();

    // pickup and other irregular measures need special care
    QString measureTag = "measure";
    mnsh.updateForMeasure(m);
    measureTag += mnsh.measureNumber();
    const bool isFirstActualMeasure = mnsh.isFirstActualMeasure();

    if (configuration()->musicxmlExportLayout()) {
        measureTag += QString(" width=\"%1\"").arg(QString::number(m->ldata()->bbox().width() / DPMM / millimeters * tenths, 'f', 2));
    }

    _xml.startElementRaw(measureTag);

    print(m, partIndex, staffCount, static_cast<int>(staves), mpc);

    _attr.start();

    findTrills(m, strack, etrack, _trillStart, _trillStop);

    // barline left must be the first element in a measure
    barlineLeft(m, strack);

    // output attributes with the first actual measure (pickup or regular)
    if (isFirstActualMeasure) {
        _attr.doAttr(_xml, true);
        _xml.tag("divisions", Constants::DIVISION / div);
    }

    // output attributes at start of measure: key, time
    keysigTimesig(m, part);

    // output attributes with the first actual measure (pickup or regular) only
    if (isFirstActualMeasure) {
        if (staves > 1) {
            _xml.tag("staves", static_cast<int>(staves));
        }
        if (instrMap.size() > 1) {
            _xml.tag("instruments", instrMap.size());
        }
    }

    // make sure clefs at end of measure get exported at start of next measure
    findAndExportClef(m, static_cast<int>(staves), strack, etrack);

    // make sure a clef gets exported if none is found
    exportDefaultClef(part, m);

    // output attributes with the first actual measure (pickup or regular) only
    if (isFirstActualMeasure) {
        writeStaffDetails(_xml, part);
        writeInstrumentDetails(part->instrument());
    }

    // output attribute at start of measure: measure-style
    measureStyle(_xml, _attr, m, partIndex);

    // MuseScore limitation: repeats are always in the first part
    // and are implicitly placed at either measure start or stop
    if (partIndex == 0) {
        repeatAtMeasureStart(_attr, m, strack, etrack, strack);
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
    _xml.endElement();
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
    const auto& parts = _score->parts();

    for (size_t partIndex = 0; partIndex < parts.size(); ++partIndex) {
        const auto part = parts.at(partIndex);
        _tick = { 0, 1 };
        _xml.startElementRaw(QString("part id=\"P%1\"").arg(partIndex + 1));

        _trillStart.clear();
        _trillStop.clear();
        initInstrMap(instrMap, part->instruments(), _score);

        MeasureNumberStateHandler mnsh;
        FigBassMap fbMap;                     // pending figured bass extends

        // set of spanners already stopped in this part
        // required to prevent multiple spanner stops for the same spanner
        QSet<const Spanner*> spannersStopped;

        const auto& pages = _score->pages();
        MeasurePrintContext mpc;

        for (size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
            const auto page = pages.at(pageIndex);
            mpc.pageStart = true;
            const auto& systems = page->systems();

            for (int systemIndex = 0; systemIndex < static_cast<int>(systems.size()); ++systemIndex) {
                const auto system = systems.at(systemIndex);
                mpc.systemStart = true;

                for (const auto mb : system->measures()) {
                    if (!mb->isMeasure()) {
                        continue;
                    }
                    const auto m = toMeasure(mb);

                    if (m->isMMRest()) {
                        // in case of a multimeasure rest (which is a single measure in MuseScore), write the measure range it replaces
                        const auto m2 = m->mmRestLast()->nextMeasure();
                        for (auto m1 = m->mmRestFirst(); m1 != m2; m1 = m1->nextMeasure()) {
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
        _xml.endElement();
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

void ExportMusicXml::write(mu::io::IODevice* dev)
{
    // must export in transposed pitch to prevent
    // losing the transposition information
    // if necessary, switch concert pitch mode off
    // before export and restore it after export
    bool concertPitch = score()->style().styleB(Sid::concertPitch);
    if (concertPitch) {
        score()->startCmd();
        score()->undo(new ChangeStyleVal(score(), Sid::concertPitch, false));
        score()->doLayout();        // this is only allowed in a cmd context to not corrupt the undo/redo stack
    }

    calcDivisions();

    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        brackets[i] = nullptr;
        dashes[i] = nullptr;
        hairpins[i] = nullptr;
        ottavas[i] = nullptr;
        trills[i] = nullptr;
    }

    _jumpElements = findJumpElements(_score);

    _xml.setDevice(dev);
    _xml.startDocument();
    _xml.writeDoctype(
        u"score-partwise PUBLIC \"-//Recordare//DTD MusicXML 4.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\"");

    _xml.startElement("score-partwise", { { "version", "4.0" } });

    work(_score->measures()->first());
    identification(_xml, _score);

    if (configuration()->musicxmlExportLayout()) {
        defaults(_xml, _score->style(), millimeters, tenths);
        credits(_xml);
    }

    partList(_xml, _score, instrMap);
    writeParts();

    _xml.endElement();

    if (concertPitch) {
        // restore concert pitch
        score()->endCmd(true);            // rollback
    }
}

//---------------------------------------------------------
//   saveXml
//    return false on error
//---------------------------------------------------------

/**
 Save Score as MusicXML file \a name.

 Return false on error.
 */

bool saveXml(Score* score, QIODevice* device)
{
    mu::io::Buffer buf;
    buf.open(mu::io::IODevice::WriteOnly);
    ExportMusicXml em(score);
    em.write(&buf);
    device->write(buf.data().toQByteArrayNoCopy());
    return true;
}

bool saveXml(Score* score, const QString& name)
{
    QFile f(name);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }

    bool res = saveXml(score, &f) && (f.error() == QFile::NoError);
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

static void writeMxlArchive(Score* score, MQZipWriter& zipwriter, const QString& filename)
{
    mu::io::Buffer cbuf;
    cbuf.open(mu::io::IODevice::ReadWrite);

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

    zipwriter.addFile("META-INF/container.xml", cbuf.data().toQByteArrayNoCopy());

    mu::io::Buffer dbuf;
    dbuf.open(mu::io::IODevice::ReadWrite);
    ExportMusicXml em(score);
    em.write(&dbuf);
    dbuf.seek(0);
    zipwriter.addFile(filename, dbuf.data().toQByteArrayNoCopy());
}

bool saveMxl(Score* score, QIODevice* device)
{
    MQZipWriter uz(device);

    //anonymized filename since we don't know the actual one here
    QString fn = "score.xml";
    writeMxlArchive(score, uz, fn);
    uz.close();

    return true;
}

bool saveMxl(Score* score, const QString& name)
{
    MQZipWriter uz(name);

    FileInfo fi(name);
    QString fn = fi.completeBaseName() + u".xml";
    writeMxlArchive(score, uz, fn);

    return true;
}

double ExportMusicXml::getTenthsFromInches(double inches) const
{
    return inches * INCH / millimeters * tenths;
}

double ExportMusicXml::getTenthsFromDots(double dots) const
{
    return dots / DPMM / millimeters * tenths;
}

static void writeMusicXML(const FretDiagram* item, XmlWriter& xml)
{
    LOGD("FretDiagram::writeMusicXML() this %p harmony %p", item, item->harmony());
    xml.startElement("frame");
    xml.tag("frame-strings", item->strings());
    xml.tag("frame-frets", item->frets());
    if (item->fretOffset() > 0) {
        xml.tag("first-fret", item->fretOffset() + 1);
    }

    for (int i = 0; i < item->strings(); ++i) {
        int mxmlString = item->strings() - i;

        std::vector<int> bStarts;
        std::vector<int> bEnds;
        for (auto const& j : item->barres()) {
            FretItem::Barre b = j.second;
            int fret = j.first;
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
            if (!d.exists()) {
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

void ExportMusicXml::harmony(Harmony const* const h, FretDiagram const* const fd, int offset)
{
    // this code was probably in place to allow chord symbols shifted *right* to export with offset
    // since this was at once time the only way to get a chord to appear over beat 3 in an empty 4/4 measure
    // but the value was calculated incorrectly (should be divided by spatium) and would be better off using offset anyhow
    // since we now support placement of chord symbols over "empty" beats directly,
    // and we don't generally export position info for other elements
    // it's just as well to not bother doing so here
    //double rx = h->offset().x()*10;
    //QString relative;
    //if (rx > 0) {
    //      relative = QString(" relative-x=\"%1\"").arg(QString::number(rx,'f',2));
    //      }
    int rootTpc = h->rootTpc();
    if (rootTpc != Tpc::TPC_INVALID) {
        XmlWriter::Attributes harmonyAttrs;
        bool frame = h->hasFrame();
        harmonyAttrs.push_back({ "print-frame", frame ? "yes" : "no" });     // .append(relative));
        addColorAttr(h, harmonyAttrs);
        _xml.startElement("harmony", harmonyAttrs);
        _xml.startElement("root");
        _xml.tag("root-step", tpc2stepName(rootTpc));
        int alter = int(tpc2alter(rootTpc));
        if (alter) {
            _xml.tag("root-alter", alter);
        }
        _xml.endElement();

        if (!h->xmlKind().isEmpty()) {
            QString s = "kind";
            QString kindText = h->musicXmlText();
            if (h->musicXmlText() != "") {
                s += " text=\"" + kindText + "\"";
            }
            if (h->xmlSymbols() == "yes") {
                s += " use-symbols=\"yes\"";
            }
            if (h->xmlParens() == "yes") {
                s += " parentheses-degrees=\"yes\"";
            }
            _xml.tagRaw(s, h->xmlKind());

            int baseTpc = h->baseTpc();
            if (baseTpc != Tpc::TPC_INVALID) {
                _xml.startElement("bass");
                _xml.tag("bass-step", tpc2stepName(baseTpc));
                alter = int(tpc2alter(baseTpc));
                if (alter) {
                    _xml.tag("bass-alter", alter);
                }
                _xml.endElement();
            }

            StringList l = h->xmlDegrees();
            if (!l.empty()) {
                for (const String& _tag : qAsConst(l)) {
                    QString tag = _tag.toQString();
                    QString degreeText;
                    if (h->xmlKind().startsWith(u"suspended")
                        && tag.startsWith("add") && tag[3].isDigit()
                        && !kindText.isEmpty() && kindText[0].isDigit()) {
                        // hack to correct text for suspended chords whose kind text has degree information baked in
                        // (required by some other applications)
                        int tagDegree = tag.midRef(3).toInt();
                        QString kindTextExtension;
                        for (int i = 0; i < kindText.length() && kindText[i].isDigit(); ++i) {
                            kindTextExtension[i] = kindText[i];
                        }
                        int kindExtension = kindTextExtension.toInt();
                        if (tagDegree <= kindExtension && (tagDegree & 1) && (kindExtension & 1)) {
                            degreeText = " text=\"\"";
                        }
                    }
                    _xml.startElement("degree");
                    alter = 0;
                    int idx = 3;
                    if (tag[idx] == '#') {
                        alter = 1;
                        ++idx;
                    } else if (tag[idx] == 'b') {
                        alter = -1;
                        ++idx;
                    }
                    _xml.tagRaw(QString("degree-value%1").arg(degreeText), tag.mid(idx));
                    _xml.tag("degree-alter", alter);               // finale insists on this even if 0
                    if (tag.startsWith("add")) {
                        _xml.tagRaw(QString("degree-type%1").arg(degreeText), "add");
                    } else if (tag.startsWith("sub")) {
                        _xml.tag("degree-type", "subtract");
                    } else if (tag.startsWith("alt")) {
                        _xml.tag("degree-type", "alter");
                    }
                    _xml.endElement();
                }
            }
        } else {
            if (h->extensionName().empty()) {
                _xml.tag("kind", "");
            } else {
                _xml.tag("kind", { { "text", h->extensionName() } }, "");
            }

            int baseTpc = h->baseTpc();
            if (baseTpc != Tpc::TPC_INVALID) {
                _xml.startElement("bass");
                _xml.tag("bass-step", tpc2stepName(baseTpc));
                alter = int(tpc2alter(baseTpc));
                if (alter) {
                    _xml.tag("bass-alter", alter);
                }
                _xml.endElement();
            }
        }

        if (offset > 0) {
            _xml.tag("offset", offset);
        }
        if (fd) {
            writeMusicXML(fd, _xml);
        }

        _xml.endElement();
    } else {
        //
        // export an unrecognized Chord
        // which may contain arbitrary text
        //
        _xml.startElement("harmony", { { "print-frame", h->hasFrame() ? "yes" : "no" } });

        const String textName = h->hTextName();
        switch (h->harmonyType()) {
        case HarmonyType::NASHVILLE: {
            _xml.tag("function", h->hFunction());
            _xml.tag("kind", { { "text", textName } }, "none");
        }
        break;
        case HarmonyType::ROMAN: {
            // TODO: parse?
            _xml.tag("function", h->hTextName());   // note: HTML escape done by tag()
            _xml.tag("kind", { { "text", "" } }, "none");
        }
        break;
        case HarmonyType::STANDARD:
        default: {
            _xml.startElement("root");
            _xml.tag("root-step", { { "text", "" } }, "C");
            _xml.endElement();                   // root
            _xml.tag("kind", { { "text", textName } }, "none");
        }
        break;
        }
        _xml.endElement();           // harmony
    }
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
    return e->visible() || configuration()->musicxmlExportInvisibleElements();
}
}
