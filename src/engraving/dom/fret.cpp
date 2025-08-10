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

#include "fret.h"

#include "translation.h"

#include "io/file.h"

#include "anchors.h"
#include "chord.h"
#include "factory.h"
#include "harmony.h"
#include "measure.h"
#include "note.h"
#include "pitchspelling.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stringdata.h"
#include "system.h"
#include "undo.h"

#include "rw/read460/tread.h"
#include "rw/read460/harmonytodiagramreader.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//    parent() is Segment or Box
//

//---------------------------------------------------------
//   fretStyle
//---------------------------------------------------------

static const ElementStyle fretStyle {
    { Sid::fretNumPos,                         Pid::FRET_NUM_POS },
    { Sid::fretMag,                            Pid::MAG },
    { Sid::fretPlacement,                      Pid::PLACEMENT },
    { Sid::fretStrings,                        Pid::FRET_STRINGS },
    { Sid::fretFrets,                          Pid::FRET_FRETS },
    { Sid::fretNut,                            Pid::FRET_NUT },
    { Sid::fretMinDistance,                    Pid::MIN_DISTANCE },
    { Sid::fretOrientation,                    Pid::ORIENTATION },
    { Sid::fretShowFingerings,                 Pid::FRET_SHOW_FINGERINGS },
};

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

struct HarmonyMapKey
{
    HarmonyMapKey() {}
    HarmonyMapKey(int k, int r, int b)
        : keys(k), rootTpc(r), bassTpc(b) {}

    int keys = 0;
    int rootTpc = Tpc::TPC_INVALID;
    int bassTpc = Tpc::TPC_INVALID;

    bool operator<(const HarmonyMapKey& other) const
    {
        return std::tie(keys, rootTpc, bassTpc) < std::tie(other.keys, other.rootTpc, other.bassTpc);
    }

    bool operator==(const HarmonyMapKey& other) const
    {
        return std::tie(keys, rootTpc, bassTpc) == std::tie(other.keys, other.rootTpc, other.bassTpc);
    }
};

static std::map<HarmonyMapKey /*key*/, String /*harmonyXml*/> s_harmonyToDiagramMap;
static std::unordered_map<String /*pattern*/, std::vector<String /*harmonyName*/> > s_diagramPatternToHarmoniesMap;

static const muse::io::path_t HARMONY_TO_DIAGRAM_FILE_PATH("://data/harmony_to_diagram.xml");

static const String blankPattern(int strings)
{
    std::vector<Char> blank(strings, Char('-'));
    String pattern(blank.data(), blank.size());
    return pattern;
}

static HarmonyMapKey createHarmonyMapKey(const String& harmony, const NoteSpellingType& spellingType, const ChordList* cl)
{
    String s = harmony;
    NoteCaseType noteCase;
    size_t idx;
    int rootTpc = convertNote(s, spellingType, noteCase, idx);

    int bassTpc = Tpc::TPC_INVALID;
    size_t slash = s.lastIndexOf(u'/');
    if (slash != muse::nidx) {
        String bs = s.mid(slash + 1).simplified();
        s = s.mid(idx, slash - idx).simplified();
        size_t idx2 = 0;
        bassTpc = convertNote(bs, spellingType, noteCase, idx2);

        if (!tpcIsValid(bassTpc)) {
            // if what follows after slash is not (just) a TPC
            // then reassemble chord and try to parse with the slash
            s = s + u"/" + bs;
        }
    } else {
        s = s.mid(idx);
    }

    ParsedChord chord;
    HarmonyMapKey mapKey;
    if (!chord.parse(s, cl)) {
        LOGE() << "Error parse " << harmony;
    }

    int keys = chord.keys();

    return HarmonyMapKey(keys, rootTpc, bassTpc);
}

FretDiagram::FretDiagram(Segment* parent)
    : EngravingItem(ElementType::FRET_DIAGRAM, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initDefaultValues();

    initElementStyle(&fretStyle);
}

FretDiagram::FretDiagram(const FretDiagram& f)
    : EngravingItem(f)
{
    m_strings    = f.m_strings;
    m_frets      = f.m_frets;
    m_fretOffset = f.m_fretOffset;
    m_maxFrets   = f.m_maxFrets;
    m_userMag    = f.m_userMag;
    m_dots       = f.m_dots;
    m_markers    = f.m_markers;
    m_barres     = f.m_barres;
    m_showNut    = f.m_showNut;
    m_orientation= f.m_orientation;
    m_showFingering = f.m_showFingering;
    m_fingering = f.m_fingering;

    if (f.m_harmony) {
        Harmony* h = new Harmony(*f.m_harmony);
        add(h);
    }
}

void FretDiagram::initDefaultValues()
{
    m_strings = 6;
    m_frets = 4;
    m_fretOffset = 0;
    m_maxFrets = 24;
    m_showNut = true;
    m_orientation = Orientation::VERTICAL;

    m_userMag = 1.0;

    m_showFingering = false;
    m_fingering = std::vector<int>(m_strings, 0);
}

FretDiagram::~FretDiagram()
{
    if (m_harmony) {
        delete m_harmony;
    }
}

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

EngravingItem* FretDiagram::linkedClone()
{
    FretDiagram* e = clone();
    e->setAutoplace(true);
    if (m_harmony) {
        EngravingItem* newHarmony = m_harmony->linkedClone();
        e->add(newHarmony);
    }
    score()->undo(new Link(e, this));
    return e;
}

Segment* FretDiagram::segment() const
{
    EngravingObject* parent = explicitParent();
    if (!parent || !parent->isSegment()) {
        return nullptr;
    }

    return toSegment(explicitParent());
}

void FretDiagram::updateDiagram(const String& harmonyName)
{
    if (s_harmonyToDiagramMap.empty()) {
        readHarmonyToDiagramFile(HARMONY_TO_DIAGRAM_FILE_PATH);
    }

    String _harmonyName = harmonyName;

    NoteSpellingType spellingType = style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();

    ParsedChord chord;
    HarmonyMapKey key = createHarmonyMapKey(_harmonyName, spellingType, score()->chordList());

    String diagramXml = muse::value(s_harmonyToDiagramMap, key);

    if (diagramXml.empty()) {
        return;
    }

    clear();

    read460::ReadContext ctx;
    XmlReader reader(diagramXml.toUtf8());

    read460::TRead::read(this, reader, ctx);

    triggerLayout();
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF FretDiagram::pagePos() const
{
    if (explicitParent() == 0) {
        return pos();
    }
    if (explicitParent()->isSegment()) {
        Measure* m = toSegment(explicitParent())->measure();
        System* system = m->system();
        double yp = y();
        if (system) {
            yp += system->staffYpage(staffIdx());
        }
        return PointF(pageX(), yp);
    } else {
        return EngravingItem::pagePos();
    }
}

double FretDiagram::mainWidth() const
{
    double mainWidth = 0.0;
    if (orientation() == Orientation::VERTICAL) {
        mainWidth = ldata()->stringDist * (strings() - 1);
    } else if (orientation() == Orientation::HORIZONTAL) {
        mainWidth = ldata()->fretDist * frets();
    }
    return mainWidth;
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> FretDiagram::dragAnchorLines() const
{
    return genericDragAnchorLines();
}

//---------------------------------------------------------
//   setStrings
//---------------------------------------------------------

void FretDiagram::setStrings(int n)
{
    int difference = n - m_strings;
    if (difference == 0 || n <= 0) {
        return;
    }

    // Move all dots, makers, barres to the RIGHT, so we add strings to the left
    // This is more useful - few instruments need strings added to the right.
    DotMap tempDots;
    MarkerMap tempMarkers;

    for (int string = 0; string < m_strings; ++string) {
        if (string + difference < 0) {
            continue;
        }

        for (auto const& d : dot(string)) {
            if (d.exists()) {
                tempDots[string + difference].push_back(FretItem::Dot(d));
            }
        }

        if (marker(string).exists()) {
            tempMarkers[string + difference] = marker(string);
        }
    }

    m_dots = tempDots;
    m_markers = tempMarkers;

    for (int fret = 1; fret <= m_frets; ++fret) {
        if (barre(fret).exists()) {
            if (m_barres[fret].startString + difference <= 0) {
                removeBarre(fret);
                continue;
            }

            m_barres[fret].startString = std::max(0, m_barres[fret].startString + difference);
            m_barres[fret].endString   = m_barres[fret].endString == -1 ? -1 : m_barres[fret].endString + difference;
        }
    }

    if (difference > 0) {
        for (int i = 0; i < difference; ++i) {
            m_fingering.push_back(0);
        }
    } else {
        m_fingering.erase(m_fingering.end() + difference, m_fingering.end());
    }

    m_strings = n;
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void FretDiagram::init(StringData* stringData, Chord* chord)
{
    if (!stringData) {
        setStrings(6);
    } else {
        setStrings(static_cast<int>(stringData->strings()));
    }
    if (stringData) {
        for (int string = 0; string < m_strings; ++string) {
            setMarker(string, FretMarkerType::CROSS);
        }
        for (const Note* note : chord->notes()) {
            int string;
            int fret;
            if (stringData->convertPitch(note->pitch(), chord->staff(), &string, &fret)) {
                setDot(string, fret);
            }
        }
        m_frets = stringData->frets();
    } else {
        m_maxFrets = 6;
    }
}

//---------------------------------------------------------
//   setDot
//    take a fret value of 0 to mean remove the dot, except with add
//    where we actually need to pass a fret val.
//---------------------------------------------------------

void FretDiagram::setDot(int string, int fret, bool add /*= false*/, FretDotType dtype /*= FretDotType::NORMAL*/)
{
    if (fret == 0) {
        removeDot(string, fret);
    } else if (string >= 0 && string < m_strings) {
        // Special case - with add, if there is a dot in the position, remove it
        // If not, add it.
        if (add) {
            if (dot(string, fret)[0].exists()) {
                removeDot(string, fret);
                return;             // We are done here, all we needed to do was remove a single dot
            }
        } else {
            m_dots[string].clear();
        }

        m_dots[string].push_back(FretItem::Dot(fret, dtype));
        if (!add) {
            setMarker(string, FretMarkerType::NONE);
        }
    }
}

void FretDiagram::addDotForDotStyleBarre(int string, int fret)
{
    if (m_dots[string].empty()) {
        m_dots[string].push_back(FretItem::Dot(fret, FretDotType::NORMAL));
    }
}

void FretDiagram::removeDotForDotStyleBarre(int string, int fret)
{
    removeDot(string, fret);
}

//---------------------------------------------------------
//   setMarker
//    Removal of dots and barres if "Multiple dots" is inactive
//    is handled in FretCanvas::mousePressEvent()
//---------------------------------------------------------

void FretDiagram::setMarker(int string, FretMarkerType mtype)
{
    if (string >= 0 && string < m_strings) {
        m_markers[string] = FretItem::Marker(mtype);
        if (mtype != FretMarkerType::NONE) {
            removeDot(string);
            removeBarres(string);
        }
    }
}

//---------------------------------------------------------
//   setBarre
//    We'll accept a value of -1 for the end string, to denote
//    that the barre goes as far right as possible.
//    Take a start string value of -1 to mean 'remove this barre'
//---------------------------------------------------------

void FretDiagram::setBarre(int startString, int endString, int fret)
{
    if (startString == -1) {
        removeBarre(fret);
    } else if (startString >= 0 && endString >= -1 && startString < m_strings && endString < m_strings) {
        m_barres[fret] = FretItem::Barre(startString, endString);
    }
}

//---------------------------------------------------------
//    This version is for clicks on a dot with shift.
//    If there is no barre at fret, then add one with the string as the start.
//    If there is a barre with a -1 end string, set the end string to string.
//    If there is a barre with a set start and end, remove it.
//    Add may be used in the future if we decide to add dots as default with barres.
//---------------------------------------------------------

void FretDiagram::setBarre(int string, int fret, bool add /*= false*/)
{
    UNUSED(add);

    FretItem::Barre b = barre(fret);
    if (!b.exists()) {
        if (string < m_strings - 1) {
            m_barres[fret] = FretItem::Barre(string, -1);
            removeDotsMarkers(string, -1, fret);
        }
    } else if (b.endString == -1 && b.startString < string) {
        m_barres[fret].endString = string;
    } else {
        removeDotsMarkers(b.startString, b.endString, fret);
        removeBarre(fret);
    }
}

//---------------------------------------------------------
//   undoSetFretDot
//---------------------------------------------------------

void FretDiagram::undoSetFretDot(int _string, int _fret, bool _add /*= true*/, FretDotType _dtype /*= FretDotType::NORMAl*/)
{
    for (EngravingObject* e : linkList()) {
        FretDiagram* fd = toFretDiagram(e);
        fd->score()->undo(new FretDot(fd, _string, _fret, _add, _dtype));
    }
}

//---------------------------------------------------------
//   undoSetFretMarker
//---------------------------------------------------------

void FretDiagram::undoSetFretMarker(int _string, FretMarkerType _mtype)
{
    for (EngravingObject* e : linkList()) {
        FretDiagram* fd = toFretDiagram(e);
        fd->score()->undo(new FretMarker(fd, _string, _mtype));
    }
}

//---------------------------------------------------------
//   undoSetFretBarre
//    add refers to using multiple dots per string when adding dots automatically
//---------------------------------------------------------

void FretDiagram::undoSetFretBarre(int _string, int _fret, bool _add /*= false*/)
{
    for (EngravingObject* e : linkList()) {
        FretDiagram* fd = toFretDiagram(e);
        fd->score()->undo(new FretBarre(fd, _string, _fret, _add));
    }
}

//---------------------------------------------------------
//   removeBarre
//    Remove a barre on a given fret.
//---------------------------------------------------------

void FretDiagram::removeBarre(int f)
{
    m_barres.erase(f);
}

//---------------------------------------------------------
//   removeBarres
//    Remove barres crossing a certain point. Fret of 0 means any point along
//    the string.
//---------------------------------------------------------

void FretDiagram::removeBarres(int string, int fret /*= 0*/)
{
    auto iter = m_barres.begin();
    while (iter != m_barres.end()) {
        int bfret = iter->first;
        FretItem::Barre b = iter->second;

        if (b.exists() && b.startString <= string && (b.endString >= string || b.endString == -1)) {
            if (fret > 0 && fret != bfret) {
                ++iter;
            } else {
                iter = m_barres.erase(iter);
            }
        } else {
            ++iter;
        }
    }
}

//---------------------------------------------------------
//   removeMarker
//---------------------------------------------------------

void FretDiagram::removeMarker(int s)
{
    auto it = m_markers.find(s);
    m_markers.erase(it);
}

//---------------------------------------------------------
//   removeDot
//    take a fret value of 0 to mean remove all dots
//---------------------------------------------------------

void FretDiagram::removeDot(int s, int f /*= 0*/)
{
    if (f > 0) {
        std::vector<FretItem::Dot> tempDots;
        for (auto const& d : dot(s)) {
            if (d.exists() && d.fret != f) {
                tempDots.push_back(FretItem::Dot(d));
            }
        }

        m_dots[s] = tempDots;
    } else {
        m_dots[s].clear();
    }

    if (m_dots[s].size() == 0) {
        auto it = m_dots.find(s);
        m_dots.erase(it);
    }
}

//---------------------------------------------------------
//   removeDotsMarkers
//    removes all markers between [ss, es] and dots between [ss, es],
//    where the dots have a fret of fret.
//---------------------------------------------------------

void FretDiagram::removeDotsMarkers(int ss, int es, int fret)
{
    if (ss == -1) {
        return;
    }

    int end = es == -1 ? m_strings : es;
    for (int string = ss; string <= end; ++string) {
        removeDot(string, fret);

        if (marker(string).exists()) {
            removeMarker(string);
        }
    }
}

//---------------------------------------------------------
//   Fill diagram from string like "XO-[1-O][1-X,2-O,3-S][3-T];B1[0-5];B..."
//   - Each character or bracketed block represents a string, from lowest to highest.
//   - 'X' = muted string (cross marker)
//   - 'O' = open string (circle marker)
//   - '-' = empty or unused string
//   - [fret-type,...] = one or more fretted dots on that string:
//       • fret is absolute (already includes offset)
//       • type is:
//           - O = circle
//           - X = cross
//           - S = square
//           - T = triangle
//   - Example: [1-X,2-O,3-S] means three dots on frets 1, 2, and 3 with different types
//   - Barre chords follow after ';', in the format: B{fret}[{start}-{end}]
//     e.g. B1[0-5] = barre on fret 1 from string 0 to 5
//---------------------------------------------------------

void FretDiagram::applyDiagramPattern(FretDiagram* diagram, const String& pattern)
{
    diagram->clear();

    const std::vector<String> parts = pattern.split(';');
    if (parts.empty()) {
        return;
    }

    const String& mainPart = parts[0];
    std::vector<String> stringTokens;

    for (size_t i = 0; i < mainPart.size();) {
        if (mainPart[i] == u'[') {
            size_t rb = mainPart.indexOf(u']', i);
            if (rb != muse::nidx) {
                stringTokens.push_back(mainPart.mid(i, rb - i + 1));
                i = rb + 1;
                continue;
            }
        }
        stringTokens.push_back(mainPart.mid(i, 1));
        ++i;
    }

    const int stringCount = static_cast<int>(stringTokens.size());
    diagram->setStrings(stringCount);
    diagram->setFrets(4);
    diagram->setPropertyFlags(Pid::FRET_STRINGS, PropertyFlags::UNSTYLED);
    diagram->setPropertyFlags(Pid::FRET_FRETS,   PropertyFlags::UNSTYLED);

    int offset = 0;

    for (int i = 0; i < stringCount; ++i) {
        const String& token = stringTokens[i];

        if (token == u"-") {
            continue;
        }

        if (token.startsWith(u"[") && token.endsWith(u"]")) {
            // Example: [1-O,2-X,3-S]
            String inner = token.mid(1, token.size() - 2);
            std::vector<String> pairs = inner.split(u',');

            for (const String& p : pairs) {
                size_t dash = p.indexOf(u'-');
                if (dash > 0) {
                    int fret = p.left(dash).toInt();
                    Char typeChar = p.mid(dash + 1, 1).at(0);
                    FretDotType dt;

                    switch (typeChar.unicode()) {
                    case 'X': dt = FretDotType::CROSS;
                        break;
                    case 'O': dt = FretDotType::NORMAL;
                        break;
                    case 'S': dt = FretDotType::SQUARE;
                        break;
                    case 'T': dt = FretDotType::TRIANGLE;
                        break;
                    default:  dt = FretDotType::NORMAL;
                        break;
                    }

                    diagram->setDot(i, fret - offset, true, dt);
                    if (fret - 3 > 0 && offset < fret - 3) {
                        offset = fret - 3;
                    }
                }
            }
        } else if (token == u"X") {
            diagram->setMarker(i, FretMarkerType::CROSS);
        } else if (token == u"O") {
            diagram->setMarker(i, FretMarkerType::CIRCLE);
        }
    }

    if (offset > 0) {
        diagram->setFretOffset(offset);
    }

    for (size_t i = 1; i < parts.size(); ++i) {
        const String& barrePart = parts[i];
        if (!barrePart.startsWith(u'B')) {
            continue;
        }

        size_t lb = barrePart.indexOf(u'[');
        size_t rb = barrePart.indexOf(u']');
        if (lb < 2 || rb < lb) {
            continue;
        }

        int fret = barrePart.mid(1, lb - 1).toInt();
        if (fret <= 0) {
            continue;
        }

        String range = barrePart.mid(lb + 1, rb - lb - 1);
        size_t dash = range.indexOf('-');
        if (dash > 0) {
            int start = range.left(dash).toInt();
            int end = range.mid(dash + 1).toInt();
            if (start >= 0 && end >= start && end < stringCount) {
                diagram->setBarre(start, end, fret - offset);
            }
        }
    }
}

String FretDiagram::patternFromDiagram(const FretDiagram* diagram)
{
    const int strings = diagram->strings();
    const int offset = diagram->fretOffset();

    StringList patternParts;
    const DotMap& dotsMap = diagram->dots();

    for (int i = 0; i < strings; ++i) {
        const FretItem::Marker marker = diagram->marker(i);

        if (marker.mtype == FretMarkerType::CROSS) {
            patternParts.push_back(u"X");
            continue;
        } else if (marker.mtype == FretMarkerType::CIRCLE) {
            patternParts.push_back(u"O");
            continue;
        }

        const auto it = dotsMap.find(i);
        if (it != dotsMap.end() && !it->second.empty()) {
            const auto& dotList = it->second;
            StringList dotDescriptions;
            for (const auto& dot : dotList) {
                int actualFret = dot.fret + offset;
                Char typeChar = u'O';

                switch (dot.dtype) {
                case FretDotType::NORMAL:   typeChar = u'O';
                    break;
                case FretDotType::CROSS:    typeChar = u'X';
                    break;
                case FretDotType::SQUARE:   typeChar = u'S';
                    break;
                case FretDotType::TRIANGLE: typeChar = u'T';
                    break;
                default: break;
                }

                dotDescriptions.push_back(String::number(actualFret) + u'-' + typeChar);
            }

            patternParts.push_back(u'[' + dotDescriptions.join(u",") + u']');
        } else {
            patternParts.push_back(u"-");
        }
    }

    String pattern = patternParts.join(u"");

    const BarreMap& barres = diagram->barres();
    StringList barreParts;
    for (const auto& [fret, b] : barres) {
        if (!b.exists()) {
            continue;
        }

        int adjustedFret = fret + offset;
        int start = b.startString;
        int end = (b.endString != -1) ? b.endString : strings - 1;

        barreParts.push_back(u'B' + String::number(adjustedFret)
                             + u'[' + String::number(start)
                             + u'-' + String::number(end) + u']');
    }

    if (!barreParts.empty()) {
        pattern += u';' + barreParts.join(u";");
    }

    return pattern;
}

void FretDiagram::applyAlignmentToHarmony()
{
    if (m_harmony->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED) {
        m_harmony->resetProperty(Pid::OFFSET);
    }

    m_harmony->setProperty(Pid::ALIGN, Align(AlignH::HCENTER, AlignV::TOP));
    m_harmony->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void FretDiagram::clear()
{
    m_barres.clear();
    m_dots.clear();
    m_markers.clear();
}

//---------------------------------------------------------
//   undoFretClear
//---------------------------------------------------------

void FretDiagram::undoFretClear()
{
    for (EngravingObject* e : linkList()) {
        FretDiagram* fd = toFretDiagram(e);
        fd->score()->undo(new FretClear(fd));
    }
}

int FretDiagram::numPos() const
{
    return style().styleI(Sid::fretNumPos);
}

//---------------------------------------------------------
//   dot
//    take fret value of zero to mean all dots
//---------------------------------------------------------

std::vector<FretItem::Dot> FretDiagram::dot(int s, int f /*= 0*/) const
{
    if (m_dots.find(s) != m_dots.end()) {
        if (f != 0) {
            for (auto const& d : m_dots.at(s)) {
                if (d.fret == f) {
                    return std::vector<FretItem::Dot> { FretItem::Dot(d) };
                }
            }
        } else {
            return m_dots.at(s);
        }
    }
    return std::vector<FretItem::Dot> { FretItem::Dot(0) };
}

//---------------------------------------------------------
//   marker
//---------------------------------------------------------

FretItem::Marker FretDiagram::marker(int s) const
{
    if (m_markers.find(s) != m_markers.end()) {
        return m_markers.at(s);
    }
    return FretItem::Marker(FretMarkerType::NONE);
}

//---------------------------------------------------------
//   barre
//---------------------------------------------------------

FretItem::Barre FretDiagram::barre(int f) const
{
    if (m_barres.find(f) != m_barres.end()) {
        return m_barres.at(f);
    }
    return FretItem::Barre(-1, -1);
}

Font FretDiagram::fretNumFont() const
{
    const MStyle& st = style();
    Font f(st.styleSt(Sid::fretDiagramFretNumberFontFace), Font::Type::Text);
    f.setPointSizeF(st.styleD(Sid::fretDiagramFretNumberFontSize) * userMag());
    FontStyle fStyle = st.styleV(Sid::fretDiagramFretNumberFontStyle).value<FontStyle>();
    f.setBold(fStyle & FontStyle::Bold);
    f.setItalic(fStyle & FontStyle::Italic);
    f.setUnderline(fStyle & FontStyle::Underline);
    f.setStrike(fStyle & FontStyle::Strike);
    return f;
}

Font FretDiagram::fingeringFont() const
{
    const MStyle& st = style();
    Font f(st.styleSt(Sid::fretDiagramFingeringFontFace), Font::Type::Text);
    f.setPointSizeF(st.styleD(Sid::fretDiagramFingeringFontSize) * userMag());
    FontStyle fStyle = st.styleV(Sid::fretDiagramFingeringFontStyle).value<FontStyle>();
    f.setBold(fStyle & FontStyle::Bold);
    f.setItalic(fStyle & FontStyle::Italic);
    f.setUnderline(fStyle & FontStyle::Underline);
    f.setStrike(fStyle & FontStyle::Strike);
    return f;
}

String FretDiagram::harmonyText() const
{
    return m_harmony ? m_harmony->plainText() : String();
}

//---------------------------------------------------------
//   setHarmony
///   if this is being done by the user, use undoSetHarmony instead
//---------------------------------------------------------

void FretDiagram::setHarmony(String harmonyText)
{
    if (!m_harmony) {
        Harmony* h = new Harmony(this->score()->dummy()->segment());
        add(h);
    }

    m_harmony->setHarmony(harmonyText);
    m_harmony->setXmlText(m_harmony->harmonyName());
    triggerLayout();
}

void FretDiagram::linkHarmony(Harmony* harmony)
{
    m_harmony = harmony;

    setParent(harmony->explicitParent());
    harmony->setParent(this);

    if (Segment* segment = this->segment()) {
        segment->removeAnnotation(harmony);
    }

    m_harmony->setTrack(track());

    applyAlignmentToHarmony();
}

void FretDiagram::unlinkHarmony()
{
    m_harmony->setTrack(track());

    segment()->add(m_harmony);

    m_harmony = nullptr;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void FretDiagram::add(EngravingItem* e)
{
    e->setParent(this);
    if (e->isHarmony()) {
        m_harmony = toHarmony(e);

        m_harmony->setTrack(track());

        if (m_harmony->harmonyName().empty()) {
            if (s_diagramPatternToHarmoniesMap.empty()) {
                readHarmonyToDiagramFile(HARMONY_TO_DIAGRAM_FILE_PATH);
            }

            String pattern = patternFromDiagram(this);
            if (!pattern.empty()) {
                std::vector<String> matchedHarmonies = muse::value(s_diagramPatternToHarmoniesMap, pattern);
                if (!matchedHarmonies.empty()) {
                    m_harmony->setHarmony(matchedHarmonies.front());
                }
            }
        }

        applyAlignmentToHarmony();

        e->added();
    } else {
        LOGW("FretDiagram: cannot add <%s>\n", e->typeName());
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void FretDiagram::remove(EngravingItem* e)
{
    if (e == m_harmony) {
        m_harmony = nullptr;
        e->removed();
    } else {
        LOGW("FretDiagram: cannot remove <%s>\n", e->typeName());
    }
}

RectF FretDiagram::drag(EditData& ed)
{
    RectF result = EngravingItem::drag(ed);

    MoveElementAnchors::moveElementAnchorsOnDrag(this, ed);

    return result;
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool FretDiagram::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::HARMONY;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* FretDiagram::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    if (e->isHarmony()) {
        Harmony* h = toHarmony(e);
        h->setParent(explicitParent());
        h->setTrack(track());
        score()->undoAddElement(h);
    } else {
        LOGW("FretDiagram: cannot drop <%s>\n", e->typeName());
        delete e;
        e = 0;
    }
    return e;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void FretDiagram::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    UNUSED(all);

    func(data, this);

    // don't display harmony in palette
    if (m_harmony && !score()->isPaletteScore()) {
        func(data, m_harmony);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue FretDiagram::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MAG:
        return userMag();
    case Pid::FRET_STRINGS:
        return strings();
    case Pid::FRET_FRETS:
        return frets();
    case Pid::FRET_NUT:
        return showNut();
    case Pid::FRET_OFFSET:
        return fretOffset();
    case Pid::ORIENTATION:
        return m_orientation;
    case Pid::FRET_SHOW_FINGERINGS:
        return m_showFingering;
    case Pid::FRET_FINGERING:
        return m_fingering;
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool FretDiagram::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::MAG:
        setUserMag(v.toDouble());
        break;
    case Pid::FRET_STRINGS:
        setStrings(v.toInt());
        break;
    case Pid::FRET_FRETS:
        setFrets(v.toInt());
        break;
    case Pid::FRET_NUT:
        setShowNut(v.toBool());
        break;
    case Pid::FRET_OFFSET:
        setFretOffset(v.toInt());
        break;
    case Pid::ORIENTATION:
        m_orientation = v.value<Orientation>();
        break;
    case Pid::FRET_SHOW_FINGERINGS:
        setShowFingering(v.toBool());
        break;
    case Pid::FRET_FINGERING:
        setFingering(v.value<std::vector<int> >());
        break;
    case Pid::EXCLUDE_VERTICAL_ALIGN:
    {
        bool val = v.toBool();
        setExcludeVerticalAlign(val);
        Harmony* h = harmony();
        if (h && h->excludeVerticalAlign() != val) {
            h->setExcludeVerticalAlign(val);
        }
        break;
    }
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue FretDiagram::propertyDefault(Pid pid) const
{
    // We shouldn't style the fret offset
    if (pid == Pid::FRET_OFFSET) {
        return PropertyValue(0);
    } else if (pid == Pid::FRET_FINGERING) {
        return std::vector<int>(m_strings, 0);
    }

    for (const StyledProperty& p : *styledProperties()) {
        if (p.pid == pid) {
            if (propertyType(pid) == P_TYPE::MILLIMETRE) {
                return style().styleMM(p.sid);
            }
            return style().styleV(p.sid);
        }
    }
    return EngravingItem::propertyDefault(pid);
}

void FretDiagram::setTrack(track_idx_t val)
{
    EngravingItem::setTrack(val);

    if (m_harmony) {
        m_harmony->setTrack(val);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String FretDiagram::accessibleInfo() const
{
    String chordName;
    if (m_harmony) {
        chordName = muse::mtrc("engraving", "with chord symbol %1").arg(m_harmony->harmonyName());
    } else {
        chordName = muse::mtrc("engraving", "without chord symbol");
    }
    return String(u"%1 %2").arg(translatedTypeUserName(), chordName);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String FretDiagram::screenReaderInfo() const
{
    String detailedInfo;
    for (int i = 0; i < m_strings; i++) {
        String stringIdent = muse::mtrc("engraving", "string %1").arg(i + 1);

        const FretItem::Marker& m = marker(i);
        String markerName;
        switch (m.mtype) {
        case FretMarkerType::CIRCLE:
            markerName = muse::mtrc("engraving", "circle marker");
            break;
        case FretMarkerType::CROSS:
            markerName = muse::mtrc("engraving", "cross marker");
            break;
        case FretMarkerType::NONE:
        default:
            break;
        }

        int dotsCount = 0;
        std::vector<int> fretsWithDots;
        for (auto const& d : dot(i)) {
            if (!d.exists()) {
                continue;
            }
            fretsWithDots.push_back(d.fret + m_fretOffset);
            dotsCount += 1;
            // TODO consider: do we need to announce what type of dot a dot is?
            // i.e. triangle, square, normal dot. It's mostly just information
            // that clutters the screenreader output and makes it harder to
            // understand, so leaving it out for now.
        }

        if (dotsCount == 0 && markerName.size() == 0) {
            continue;
        }

        String fretInfo;
        if (dotsCount == 1) {
            fretInfo = String::number(fretsWithDots.front());
        } else if (dotsCount > 1) {
            int max = int(fretsWithDots.size());
            for (int j = 0; j < max; j++) {
                if (j == max - 1) {
                    fretInfo = muse::mtrc("engraving", "%1 and %2").arg(fretInfo).arg(fretsWithDots[j]);
                } else {
                    fretInfo = String(u"%1 %2").arg(fretInfo).arg(fretsWithDots[j]);
                }
            }
        }

        //: Omit the "%n " for the singular translation (and the "(s)" too)
        String dotsInfo = muse::mtrc("engraving", "%n dot(s) on fret(s) %1", "", dotsCount).arg(fretInfo);

        detailedInfo = String(u"%1 %2 %3 %4").arg(detailedInfo, stringIdent, markerName, dotsInfo);
    }

    String barreInfo;
    for (auto const& iter : m_barres) {
        const FretItem::Barre& b = iter.second;
        if (!b.exists()) {
            continue;
        }

        String fretInfo = muse::mtrc("engraving", "fret %1").arg(iter.first);

        String newBarreInfo;
        if (b.startString == 0 && (b.endString == -1 || b.endString == m_strings - 1)) {
            newBarreInfo = muse::mtrc("engraving", "barré %1").arg(fretInfo);
        } else {
            String startPart = muse::mtrc("engraving", "beginning string %1").arg(b.startString + 1);
            String endPart;
            if (b.endString != -1) {
                endPart = muse::mtrc("engraving", "and ending string %1").arg(b.endString + 1);
            }

            newBarreInfo = muse::mtrc("engraving", "partial barré %1 %2 %3").arg(fretInfo, startPart, endPart);
        }

        barreInfo = String(u"%1 %2").arg(barreInfo, newBarreInfo);
    }

    detailedInfo = String(u"%1 %2").arg(detailedInfo, barreInfo);

    if (detailedInfo.trimmed().size() == 0) {
        detailedInfo = muse::mtrc("engraving", "no content");
    }

    String chordName = m_harmony
                       ? muse::mtrc("engraving", "with chord symbol %1").arg(m_harmony->generateScreenReaderInfo())
                       : muse::mtrc("engraving", "without chord symbol");

    String basicInfo = String(u"%1 %2").arg(translatedTypeUserName(), chordName);

    String generalInfo = muse::mtrc("engraving", "%n string(s) total", "", m_strings);

    String res = String(u"%1 %2 %3").arg(basicInfo, generalInfo, detailedInfo);

    return res;
}

void FretDiagram::setFingering(std::vector<int> v)
{
    m_fingering = std::move(v);
}

FretDiagram* FretDiagram::makeFromHarmonyOrFretDiagram(const EngravingItem* harmonyOrFretDiagram)
{
    IF_ASSERT_FAILED(!harmonyOrFretDiagram || !harmonyOrFretDiagram->isHarmony() || !harmonyOrFretDiagram->isFretDiagram()) {
        return nullptr;
    }

    FretDiagram* fretDiagram = nullptr;

    if (harmonyOrFretDiagram->isHarmony() && !harmonyOrFretDiagram->parentItem()->isFretDiagram()) {
        Harmony* harmony = toHarmony(harmonyOrFretDiagram)->clone();

        fretDiagram = Factory::createFretDiagram(harmonyOrFretDiagram->score()->dummy()->segment());

        fretDiagram->setTrack(harmony->track());
        fretDiagram->updateDiagram(harmony->plainText());

        fretDiagram->linkHarmony(harmony);
    } else if (harmonyOrFretDiagram->isHarmony() && harmonyOrFretDiagram->parentItem()->isFretDiagram()) {
        fretDiagram = toFretDiagram(harmonyOrFretDiagram->parentItem())->clone();
    } else if (harmonyOrFretDiagram->isFretDiagram()) {
        fretDiagram = toFretDiagram(harmonyOrFretDiagram)->clone();
        if (!fretDiagram->harmony()) {
            //! generate from diagram and add harmony
            fretDiagram->add(Factory::createHarmony(harmonyOrFretDiagram->score()->dummy()->segment()));
        }
    }

    return fretDiagram;
}

bool FretDiagram::isInFretBox() const
{
    EngravingObject* parent = explicitParent();
    return parent ? parent->isFBox() : false;
}

bool FretDiagram::isCustom(const String& harmonyNameForCompare) const
{
    if (harmonyNameForCompare.empty()) {
        return false;
    }

    static const std::list<Pid> props {
        Pid::FRET_STRINGS,
        Pid::FRET_FRETS,
        Pid::FRET_OFFSET,
        Pid::FRET_NUT,
        Pid::FRET_FINGERING
    };

    for (const Pid& pid : props) {
        if (pid == Pid::FRET_FINGERING) {
            if (!custom(Pid::FRET_SHOW_FINGERINGS)) {
                break;
            }
        }

        if (custom(pid)) {
            return true;
        }
    }

    String pattern = patternFromDiagram(this);
    if (pattern == blankPattern(strings())) {
        return false;
    }

    if (s_diagramPatternToHarmoniesMap.empty()) {
        readHarmonyToDiagramFile(HARMONY_TO_DIAGRAM_FILE_PATH);
    }

    std::vector<String> patternHarmonies = muse::value(s_diagramPatternToHarmoniesMap, pattern);
    if (patternHarmonies.empty()) {
        return true;
    }

    return !muse::contains(patternHarmonies, harmonyNameForCompare.toLower());
}

void FretDiagram::readHarmonyToDiagramFile(const muse::io::path_t& filePath) const
{
    TRACEFUNC;

    muse::io::File file(filePath);
    if (!file.open()) {
        LOGE() << file.errorString();
        return;
    }

    XmlReader reader(&file);

    std::unordered_map<String,
                       read460::HarmonyToDiagramReader::FretDiagramInfo> harmonyToDiagramMap
        = read460::HarmonyToDiagramReader::read(reader);

    const ChordList* chordList = score()->chordList();
    const NoteSpellingType spellingType = NoteSpellingType::STANDARD;

    for (auto& [key, value] : harmonyToDiagramMap) {
        ParsedChord chord;
        HarmonyMapKey mapKey = createHarmonyMapKey(key, spellingType, chordList);

        s_harmonyToDiagramMap.insert({ mapKey, value.xml });
        s_diagramPatternToHarmoniesMap[value.pattern].push_back(key);
    }
}

//---------------------------------------------------------
//   markerToChar
//---------------------------------------------------------

Char FretItem::markerToChar(FretMarkerType t)
{
    switch (t) {
    case FretMarkerType::CIRCLE: return Char(u'O');
    case FretMarkerType::CROSS: return Char(u'X');
    case FretMarkerType::NONE:
    default:
        return Char();
    }
}

//---------------------------------------------------------
//   markerTypeToName
//---------------------------------------------------------

const std::vector<FretItem::MarkerTypeNameItem> FretItem::markerTypeNameMap = {
    { FretMarkerType::CIRCLE,     "circle" },
    { FretMarkerType::CROSS,      "cross" },
    { FretMarkerType::NONE,       "none" }
};

String FretItem::markerTypeToName(FretMarkerType t)
{
    for (auto i : FretItem::markerTypeNameMap) {
        if (i.mtype == t) {
            return String::fromAscii(i.name);
        }
    }

    ASSERT_X("Unrecognised FretMarkerType!");
    return String();         // prevent compiler warnings
}

//---------------------------------------------------------
//   nameToMarkerType
//---------------------------------------------------------

FretMarkerType FretItem::nameToMarkerType(String n)
{
    for (auto i : FretItem::markerTypeNameMap) {
        if (String::fromAscii(i.name) == n) {
            return i.mtype;
        }
    }
    LOGW("Unrecognised marker name!");
    return FretMarkerType::NONE;         // default
}

//---------------------------------------------------------
//   dotTypeToName
//---------------------------------------------------------

const std::vector<FretItem::DotTypeNameItem> FretItem::dotTypeNameMap = {
    { FretDotType::NORMAL,        "normal" },
    { FretDotType::CROSS,         "cross" },
    { FretDotType::SQUARE,        "square" },
    { FretDotType::TRIANGLE,      "triangle" },
};

String FretItem::dotTypeToName(FretDotType t)
{
    for (auto i : FretItem::dotTypeNameMap) {
        if (i.dtype == t) {
            return String::fromAscii(i.name);
        }
    }

    ASSERT_X("Unrecognised FretDotType!");
    return String();         // prevent compiler warnings
}

//---------------------------------------------------------
//   nameToDotType
//---------------------------------------------------------

FretDotType FretItem::nameToDotType(String n)
{
    for (auto i : FretItem::dotTypeNameMap) {
        if (String::fromAscii(i.name) == n) {
            return i.dtype;
        }
    }
    LOGW("Unrecognised dot name!");
    return FretDotType::NORMAL;         // default
}

//---------------------------------------------------------
//   updateStored
//---------------------------------------------------------

FretUndoData::FretUndoData(FretDiagram* fd)
{
    // We need to store the old barres and markers, since predicting how
    // adding dots, markers, barres etc. will change things is too difficult.
    // Update linked fret diagrams:
    m_diagram = fd;
    m_dots = m_diagram->m_dots;
    m_markers = m_diagram->m_markers;
    m_barres = m_diagram->m_barres;

    m_strings = m_diagram->m_strings;
    m_frets = m_diagram->m_frets;
    m_fretOffset = m_diagram->m_fretOffset;
    m_maxFrets = m_diagram->m_maxFrets;
    m_showNut = m_diagram->m_showNut;
    m_orientation = m_diagram->m_orientation;

    m_userMag = m_diagram->m_userMag;

    m_showFingering = m_diagram->m_showFingering;
}

//---------------------------------------------------------
//   updateDiagram
//---------------------------------------------------------

void FretUndoData::updateDiagram()
{
    if (!m_diagram) {
        ASSERT_X("Tried to undo fret diagram change without ever setting diagram!");
        return;
    }

    // Reset every fret diagram property of the changed diagram
    // FretUndoData is a friend of FretDiagram so has access to these private members
    m_diagram->m_barres = m_barres;
    m_diagram->m_markers = m_markers;
    m_diagram->m_dots = m_dots;

    m_diagram->m_strings = m_strings;
    m_diagram->m_frets = m_frets;
    m_diagram->m_fretOffset = m_fretOffset;
    m_diagram->m_maxFrets = m_maxFrets;
    m_diagram->m_showNut = m_showNut;
    m_diagram->m_orientation = m_orientation;

    m_diagram->m_userMag = m_userMag;

    m_diagram->m_showFingering = m_showFingering;
}
}
