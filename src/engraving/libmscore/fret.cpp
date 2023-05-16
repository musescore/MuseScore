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

#include "fret.h"

#include "translation.h"

#include "draw/fontmetrics.h"
#include "draw/types/brush.h"
#include "draw/types/pen.h"

#include "layout/tlayout.h"

#include "chord.h"
#include "factory.h"
#include "harmony.h"
#include "measure.h"
#include "mscore.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stringdata.h"
#include "system.h"
#include "undo.h"

#include "log.h"

using namespace mu;
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
};

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::FretDiagram(Segment* parent)
    : EngravingItem(ElementType::FRET_DIAGRAM, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    m_font.setFamily(u"FreeSans", draw::Font::Type::Tablature);
    m_font.setPointSizeF(4.0 * mag());
    initElementStyle(&fretStyle);
}

FretDiagram::FretDiagram(const FretDiagram& f)
    : EngravingItem(f)
{
    _strings    = f._strings;
    _frets      = f._frets;
    _fretOffset = f._fretOffset;
    _maxFrets   = f._maxFrets;
    m_font        = f.m_font;
    _userMag    = f._userMag;
    _numPos     = f._numPos;
    _dots       = f._dots;
    _markers    = f._markers;
    _barres     = f._barres;
    _showNut    = f._showNut;
    _orientation= f._orientation;

    if (f._harmony) {
        Harmony* h = new Harmony(*f._harmony);
        add(h);
    }
}

FretDiagram::~FretDiagram()
{
    if (_harmony) {
        delete _harmony;
    }
}

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

EngravingItem* FretDiagram::linkedClone()
{
    FretDiagram* e = clone();
    e->setAutoplace(true);
    if (_harmony) {
        EngravingItem* newHarmony = _harmony->linkedClone();
        e->add(newHarmony);
    }
    score()->undo(new Link(e, this));
    return e;
}

//---------------------------------------------------------
//   fromString
///   Create diagram from string like "XO-123"
///   Always assume barre on the first visible fret
//---------------------------------------------------------

std::shared_ptr<FretDiagram> FretDiagram::createFromString(Score* score, const String& s)
{
    auto fd = Factory::makeFretDiagram(score->dummy()->segment());
    int strings = static_cast<int>(s.size());

    fd->setStrings(strings);
    fd->setFrets(4);
    fd->setPropertyFlags(Pid::FRET_STRINGS, PropertyFlags::UNSTYLED);
    fd->setPropertyFlags(Pid::FRET_FRETS,   PropertyFlags::UNSTYLED);
    int offset = 0;
    int barreString = -1;
    std::vector<std::pair<int, int> > dotsToAdd;

    for (int i = 0; i < strings; i++) {
        Char c = s.at(i);
        if (c == 'X' || c == 'O') {
            FretMarkerType mt = (c == 'X' ? FretMarkerType::CROSS : FretMarkerType::CIRCLE);
            fd->setMarker(i, mt);
        } else if (c == '-' && barreString == -1) {
            barreString = i;
        } else {
            int fret = c.digitValue();
            if (fret != -1) {
                dotsToAdd.push_back(std::make_pair(i, fret));
                if (fret - 3 > 0 && offset < fret - 3) {
                    offset = fret - 3;
                }
            }
        }
    }

    if (offset > 0) {
        fd->setFretOffset(offset);
    }

    for (std::pair<int, int> d : dotsToAdd) {
        fd->setDot(d.first, d.second - offset, true);
    }

    // This assumes that any barre goes to the end of the fret
    if (barreString >= 0) {
        fd->setBarre(barreString, -1, 1);
    }

    return fd;
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
    int difference = n - _strings;
    if (difference == 0 || n <= 0) {
        return;
    }

    // Move all dots, makers, barres to the RIGHT, so we add strings to the left
    // This is more useful - few instruments need strings added to the right.
    DotMap tempDots;
    MarkerMap tempMarkers;

    for (int string = 0; string < _strings; ++string) {
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

    _dots = tempDots;
    _markers = tempMarkers;

    for (int fret = 1; fret <= _frets; ++fret) {
        if (barre(fret).exists()) {
            if (_barres[fret].startString + difference <= 0) {
                removeBarre(fret);
                continue;
            }

            _barres[fret].startString = std::max(0, _barres[fret].startString + difference);
            _barres[fret].endString   = _barres[fret].endString == -1 ? -1 : _barres[fret].endString + difference;
        }
    }

    _strings = n;
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
        for (int string = 0; string < _strings; ++string) {
            setMarker(string, FretMarkerType::CROSS);
        }
        for (const Note* note : chord->notes()) {
            int string;
            int fret;
            if (stringData->convertPitch(note->pitch(), chord->staff(), &string, &fret)) {
                setDot(string, fret);
            }
        }
        _frets = stringData->frets();
    } else {
        _maxFrets = 6;
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    PointF translation = -PointF(m_stringDist * (_strings - 1), 0);
    if (_orientation == Orientation::HORIZONTAL) {
        painter->save();
        painter->rotate(-90);
        painter->translate(translation);
    }

    // Init pen and other values
    double _spatium = spatium() * _userMag;
    Pen pen(curColor());
    pen.setCapStyle(PenCapStyle::FlatCap);
    painter->setBrush(Brush(Color(painter->pen().color())));

    // x2 is the x val of the rightmost string
    double x2 = (_strings - 1) * m_stringDist;

    // Draw the nut
    pen.setWidthF(m_nutLw);
    painter->setPen(pen);
    painter->drawLine(LineF(-m_stringLw * .5, 0.0, x2 + m_stringLw * .5, 0.0));

    // Draw strings and frets
    pen.setWidthF(m_stringLw);
    painter->setPen(pen);

    // y2 is the y val of the bottom fretline
    double y2 = m_fretDist * (_frets + .5);
    for (int i = 0; i < _strings; ++i) {
        double x = m_stringDist * i;
        painter->drawLine(LineF(x, _fretOffset ? -_spatium * .2 : 0.0, x, y2));
    }
    for (int i = 1; i <= _frets; ++i) {
        double y = m_fretDist * i;
        painter->drawLine(LineF(0.0, y, x2, y));
    }

    // dotd is the diameter of a dot
    double dotd = _spatium * .49 * score()->styleD(Sid::fretDotSize);

    // Draw dots, sym pen is used to draw them (and markers)
    Pen symPen(pen);
    symPen.setCapStyle(PenCapStyle::RoundCap);
    double symPenWidth = m_stringLw * 1.2;
    symPen.setWidthF(symPenWidth);

    for (auto const& i : _dots) {
        for (auto const& d : i.second) {
            if (!d.exists()) {
                continue;
            }

            int string = i.first;
            int fret = d.fret - 1;

            // Calculate coords of the top left corner of the dot
            double x = m_stringDist * string - dotd * .5;
            double y = m_fretDist * fret + m_fretDist * .5 - dotd * .5;

            // Draw different symbols
            painter->setPen(symPen);
            switch (d.dtype) {
            case FretDotType::CROSS:
                // Give the cross a slightly larger width
                symPen.setWidthF(symPenWidth * 1.5);
                painter->setPen(symPen);
                painter->drawLine(LineF(x, y, x + dotd, y + dotd));
                painter->drawLine(LineF(x + dotd, y, x, y + dotd));
                symPen.setWidthF(symPenWidth);
                break;
            case FretDotType::SQUARE:
                painter->setBrush(BrushStyle::NoBrush);
                painter->drawRect(RectF(x, y, dotd, dotd));
                break;
            case FretDotType::TRIANGLE:
                painter->drawLine(LineF(x, y + dotd, x + .5 * dotd, y));
                painter->drawLine(LineF(x + .5 * dotd, y, x + dotd, y + dotd));
                painter->drawLine(LineF(x + dotd, y + dotd, x, y + dotd));
                break;
            case FretDotType::NORMAL:
            default:
                painter->setBrush(symPen.color());
                painter->setNoPen();
                painter->drawEllipse(RectF(x, y, dotd, dotd));
                break;
            }
        }
    }

    // Draw markers
    symPen.setWidthF(symPenWidth * 1.2);
    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(symPen);
    for (auto const& i : _markers) {
        int string = i.first;
        FretItem::Marker marker = i.second;
        if (!marker.exists()) {
            continue;
        }

        double x = m_stringDist * string - m_markerSize * .5;
        double y = -m_fretDist - m_markerSize * .5;
        if (marker.mtype == FretMarkerType::CIRCLE) {
            painter->drawEllipse(RectF(x, y, m_markerSize, m_markerSize));
        } else if (marker.mtype == FretMarkerType::CROSS) {
            painter->drawLine(PointF(x, y), PointF(x + m_markerSize, y + m_markerSize));
            painter->drawLine(PointF(x, y + m_markerSize), PointF(x + m_markerSize, y));
        }
    }

    // Draw barres
    for (auto const& i : _barres) {
        int fret        = i.first;
        int startString = i.second.startString;
        int endString   = i.second.endString;

        double x1    = m_stringDist * startString;
        double newX2 = endString == -1 ? x2 : m_stringDist * endString;
        double y     = m_fretDist * (fret - 1) + m_fretDist * .5;
        pen.setWidthF(dotd * score()->styleD(Sid::barreLineWidth));
        pen.setCapStyle(PenCapStyle::RoundCap);
        painter->setPen(pen);
        painter->drawLine(LineF(x1, y, newX2, y));
    }

    // Draw fret offset number
    if (_fretOffset > 0) {
        double fretNumMag = score()->styleD(Sid::fretNumMag);
        mu::draw::Font scaledFont(m_font);
        scaledFont.setPointSizeF(m_font.pointSizeF() * _userMag * (spatium() / SPATIUM20) * MScore::pixelRatio * fretNumMag);
        painter->setFont(scaledFont);
        String text = String::number(_fretOffset + 1);

        if (_orientation == Orientation::VERTICAL) {
            if (_numPos == 0) {
                painter->drawText(RectF(-m_stringDist * .4, .0, .0, m_fretDist),
                                  draw::AlignVCenter | draw::AlignRight | draw::TextDontClip, text);
            } else {
                painter->drawText(RectF(x2 + (m_stringDist * .4), .0, .0, m_fretDist),
                                  draw::AlignVCenter | draw::AlignLeft | draw::TextDontClip,
                                  String::number(_fretOffset + 1));
            }
        } else if (_orientation == Orientation::HORIZONTAL) {
            painter->save();
            painter->translate(-translation);
            painter->rotate(90);
            if (_numPos == 0) {
                painter->drawText(RectF(.0, m_stringDist * (_strings - 1), .0, .0),
                                  draw::AlignLeft | draw::TextDontClip, text);
            } else {
                painter->drawText(RectF(.0, .0, .0, .0), draw::AlignBottom | draw::AlignLeft | draw::TextDontClip, text);
            }
            painter->restore();
        }
        painter->setFont(m_font);
    }

    // NOTE:JT possible future todo - draw fingerings

    if (_orientation == Orientation::HORIZONTAL) {
        painter->restore();
    }
}

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
=======
>>>>>>> cd79de8b507ce5e52931bbfbce650f3fc04e0ae2
double FretDiagram::centerX() const
{
    // Keep in sync with how bbox is calculated in layout().
    return (bbox().right() - m_markerSize * .5) * .5;
}

double FretDiagram::rightX() const
{
    // Keep in sync with how bbox is calculated in layout().
    return bbox().right() - m_markerSize * .5;
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
    } else if (string >= 0 && string < _strings) {
        // Special case - with add, if there is a dot in the position, remove it
        // If not, add it.
        if (add) {
            if (dot(string, fret)[0].exists()) {
                removeDot(string, fret);
                return;             // We are done here, all we needed to do was remove a single dot
            }
        } else {
            _dots[string].clear();
        }

        _dots[string].push_back(FretItem::Dot(fret, dtype));
        if (!add) {
            setMarker(string, FretMarkerType::NONE);
        }
    }
}

//---------------------------------------------------------
//   setMarker
//    Removal of dots and barres if "Multiple dots" is inactive
//    is handled in FretCanvas::mousePressEvent()
//---------------------------------------------------------

void FretDiagram::setMarker(int string, FretMarkerType mtype)
{
    if (string >= 0 && string < _strings) {
        _markers[string] = FretItem::Marker(mtype);
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
    } else if (startString >= 0 && endString >= -1 && startString < _strings && endString < _strings) {
        _barres[fret] = FretItem::Barre(startString, endString);
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
        if (string < _strings - 1) {
            _barres[fret] = FretItem::Barre(string, -1);
            removeDotsMarkers(string, -1, fret);
        }
    } else if (b.endString == -1 && b.startString < string) {
        _barres[fret].endString = string;
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
    _barres.erase(f);
}

//---------------------------------------------------------
//   removeBarres
//    Remove barres crossing a certain point. Fret of 0 means any point along
//    the string.
//---------------------------------------------------------

void FretDiagram::removeBarres(int string, int fret /*= 0*/)
{
    auto iter = _barres.begin();
    while (iter != _barres.end()) {
        int bfret = iter->first;
        FretItem::Barre b = iter->second;

        if (b.exists() && b.startString <= string && (b.endString >= string || b.endString == -1)) {
            if (fret > 0 && fret != bfret) {
                ++iter;
            } else {
                iter = _barres.erase(iter);
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
    auto it = _markers.find(s);
    _markers.erase(it);
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

        _dots[s] = tempDots;
    } else {
        _dots[s].clear();
    }

    if (_dots[s].size() == 0) {
        auto it = _dots.find(s);
        _dots.erase(it);
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

    int end = es == -1 ? _strings : es;
    for (int string = ss; string <= end; ++string) {
        removeDot(string, fret);

        if (marker(string).exists()) {
            removeMarker(string);
        }
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void FretDiagram::clear()
{
    _barres.clear();
    _dots.clear();
    _markers.clear();
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

//---------------------------------------------------------
//   dot
//    take fret value of zero to mean all dots
//---------------------------------------------------------

std::vector<FretItem::Dot> FretDiagram::dot(int s, int f /*= 0*/) const
{
    if (_dots.find(s) != _dots.end()) {
        if (f != 0) {
            for (auto const& d : _dots.at(s)) {
                if (d.fret == f) {
                    return std::vector<FretItem::Dot> { FretItem::Dot(d) };
                }
            }
        } else {
            return _dots.at(s);
        }
    }
    return std::vector<FretItem::Dot> { FretItem::Dot(0) };
}

//---------------------------------------------------------
//   marker
//---------------------------------------------------------

FretItem::Marker FretDiagram::marker(int s) const
{
    if (_markers.find(s) != _markers.end()) {
        return _markers.at(s);
    }
    return FretItem::Marker(FretMarkerType::NONE);
}

//---------------------------------------------------------
//   barre
//---------------------------------------------------------

FretItem::Barre FretDiagram::barre(int f) const
{
    if (_barres.find(f) != _barres.end()) {
        return _barres.at(f);
    }
    return FretItem::Barre(-1, -1);
}

//---------------------------------------------------------
//   setHarmony
///   if this is being done by the user, use undoSetHarmony instead
//---------------------------------------------------------

void FretDiagram::setHarmony(String harmonyText)
{
    if (!_harmony) {
        Harmony* h = new Harmony(this->score()->dummy()->segment());
        add(h);
    }

    _harmony->setHarmony(harmonyText);
    _harmony->setXmlText(_harmony->harmonyName());
    triggerLayout();
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void FretDiagram::add(EngravingItem* e)
{
    e->setParent(this);
    if (e->isHarmony()) {
        _harmony = toHarmony(e);
        _harmony->setTrack(track());
        if (_harmony->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED) {
            _harmony->resetProperty(Pid::OFFSET);
        }

        _harmony->setProperty(Pid::ALIGN, Align(AlignH::HCENTER, AlignV::TOP));
        _harmony->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
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
    if (e == _harmony) {
        _harmony = nullptr;
        e->removed();
    } else {
        LOGW("FretDiagram: cannot remove <%s>\n", e->typeName());
    }
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
    if (_harmony && !score()->isPaletteScore()) {
        func(data, _harmony);
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
    case Pid::FRET_NUM_POS:
        return _numPos;
    case Pid::ORIENTATION:
        return _orientation;
        break;
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
    case Pid::FRET_NUM_POS:
        _numPos = v.toInt();
        break;
    case Pid::ORIENTATION:
        _orientation = v.value<Orientation>();
        break;
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
    }

    for (const StyledProperty& p : *styledProperties()) {
        if (p.pid == pid) {
            if (propertyType(pid) == P_TYPE::MILLIMETRE) {
                return score()->styleMM(p.sid);
            }
            return score()->styleV(p.sid);
        }
    }
    return EngravingItem::propertyDefault(pid);
}

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void FretDiagram::endEditDrag(EditData& editData)
{
    EngravingItem::endEditDrag(editData);

    triggerLayout();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String FretDiagram::accessibleInfo() const
{
    String chordName;
    if (_harmony) {
        chordName = mtrc("engraving", "with chord symbol %1").arg(_harmony->harmonyName());
    } else {
        chordName = mtrc("engraving", "without chord symbol");
    }
    return String(u"%1 %2").arg(translatedTypeUserName(), chordName);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String FretDiagram::screenReaderInfo() const
{
    String detailedInfo;
    for (int i = 0; i < _strings; i++) {
        String stringIdent = mtrc("engraving", "string %1").arg(i + 1);

        const FretItem::Marker& m = marker(i);
        String markerName;
        switch (m.mtype) {
        case FretMarkerType::CIRCLE:
            markerName = mtrc("engraving", "circle marker");
            break;
        case FretMarkerType::CROSS:
            markerName = mtrc("engraving", "cross marker");
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
            fretsWithDots.push_back(d.fret + _fretOffset);
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
                    fretInfo = mtrc("engraving", "%1 and %2").arg(fretInfo).arg(fretsWithDots[j]);
                } else {
                    fretInfo = String(u"%1 %2").arg(fretInfo).arg(fretsWithDots[j]);
                }
            }
        }

        //: Omit the "%n " for the singular translation (and the "(s)" too)
        String dotsInfo = mtrc("engraving", "%n dot(s) on fret(s) %1", "", dotsCount).arg(fretInfo);

        detailedInfo = String(u"%1 %2 %3 %4").arg(detailedInfo, stringIdent, markerName, dotsInfo);
    }

    String barreInfo;
    for (auto const& iter : _barres) {
        const FretItem::Barre& b = iter.second;
        if (!b.exists()) {
            continue;
        }

        String fretInfo = mtrc("engraving", "fret %1").arg(iter.first);

        String newBarreInfo;
        if (b.startString == 0 && (b.endString == -1 || b.endString == _strings - 1)) {
            newBarreInfo = mtrc("engraving", "barré %1").arg(fretInfo);
        } else {
            String startPart = mtrc("engraving", "beginning string %1").arg(b.startString + 1);
            String endPart;
            if (b.endString != -1) {
                endPart = mtrc("engraving", "and ending string %1").arg(b.endString + 1);
            }

            newBarreInfo = mtrc("engraving", "partial barré %1 %2 %3").arg(fretInfo, startPart, endPart);
        }

        barreInfo = String(u"%1 %2").arg(barreInfo, newBarreInfo);
    }

    detailedInfo = String(u"%1 %2").arg(detailedInfo, barreInfo);

    if (detailedInfo.trimmed().size() == 0) {
        detailedInfo = mtrc("engraving", "no content");
    }

    String chordName = _harmony
                       ? mtrc("engraving", "with chord symbol %1").arg(_harmony->generateScreenReaderInfo())
                       : mtrc("engraving", "without chord symbol");

    String basicInfo = String(u"%1 %2").arg(translatedTypeUserName(), chordName);

    String generalInfo = mtrc("engraving", "%n string(s) total", "", _strings);

    String res = String(u"%1 %2 %3").arg(basicInfo, generalInfo, detailedInfo);

    return res;
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
    _diagram = fd;
    _dots = _diagram->_dots;
    _markers = _diagram->_markers;
    _barres = _diagram->_barres;
}

//---------------------------------------------------------
//   updateDiagram
//---------------------------------------------------------

void FretUndoData::updateDiagram()
{
    if (!_diagram) {
        ASSERT_X("Tried to undo fret diagram change without ever setting diagram!");
        return;
    }

    // Reset every fret diagram property of the changed diagram
    // FretUndoData is a friend of FretDiagram so has access to these private members
    _diagram->_barres = _barres;
    _diagram->_markers = _markers;
    _diagram->_dots = _dots;
}
}
