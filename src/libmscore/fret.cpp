//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "fret.h"
#include "measure.h"
#include "system.h"
#include "score.h"
#include "stringdata.h"
#include "chord.h"
#include "note.h"
#include "segment.h"
#include "mscore.h"
#include "harmony.h"
#include "staff.h"
#include "undo.h"

namespace Ms {
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

FretDiagram::FretDiagram(Score* score)
    : Element(score, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    font.setFamily("FreeSans");
    font.setPointSize(4.0 * mag());
    initElementStyle(&fretStyle);
}

FretDiagram::FretDiagram(const FretDiagram& f)
    : Element(f)
{
    _strings    = f._strings;
    _frets      = f._frets;
    _fretOffset = f._fretOffset;
    _maxFrets   = f._maxFrets;
    font        = f.font;
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

Element* FretDiagram::linkedClone()
{
    FretDiagram* e = clone();
    e->setAutoplace(true);
    if (_harmony) {
        Element* newHarmony = _harmony->linkedClone();
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

std::shared_ptr<FretDiagram> FretDiagram::createFromString(Score* score, const QString& s)
{
    auto fd = makeElement<FretDiagram>(score);
    int strings = s.size();

    fd->setStrings(strings);
    fd->setFrets(4);
    fd->setPropertyFlags(Pid::FRET_STRINGS, PropertyFlags::UNSTYLED);
    fd->setPropertyFlags(Pid::FRET_FRETS,   PropertyFlags::UNSTYLED);
    int offset = 0;
    int barreString = -1;
    std::vector<std::pair<int, int> > dotsToAdd;

    for (int i = 0; i < strings; i++) {
        QChar c = s.at(i);
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

QPointF FretDiagram::pagePos() const
{
    if (parent() == 0) {
        return pos();
    }
    if (parent()->isSegment()) {
        Measure* m = toSegment(parent())->measure();
        System* system = m->system();
        qreal yp = y();
        if (system) {
            yp += system->staffYpage(staffIdx());
        }
        return QPointF(pageX(), yp);
    } else {
        return Element::pagePos();
    }
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

QVector<QLineF> FretDiagram::dragAnchorLines() const
{
    return genericDragAnchorLines();
#if 0 // TODOxx
    if (parent()->isSegment()) {
        Segment* s     = toSegment(parent());
        Measure* m     = s->measure();
        System* system = m->system();
        qreal yp      = system->staff(staffIdx())->y() + system->y();
        qreal xp      = m->tick2pos(s->tick()) + m->pagePos().x();
        QPointF p1(xp, yp);

        qreal x  = 0.0;
        qreal y  = 0.0;
        qreal tw = width();
        qreal th = height();
        if (_align & Align::BOTTOM) {
            y = th;
        } else if (_align & Align::VCENTER) {
            y = (th * .5);
        } else if (_align & Align::BASELINE) {
            y = baseLine();
        }
        if (_align & Align::RIGHT) {
            x = tw;
        } else if (_align & Align::HCENTER) {
            x = (tw * .5);
        }
        return QLineF(p1, abbox().topLeft() + QPointF(x, y));
    }
    return QLineF(parent()->pagePos(), abbox().topLeft());
#endif
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

            _barres[fret].startString = qMax(0, _barres[fret].startString + difference);
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
        setStrings(stringData->strings());
    }
    if (stringData) {
        for (int string = 0; string < _strings; ++string) {
            setMarker(string, FretMarkerType::CROSS);
        }
        for (const Note* note : chord->notes()) {
            int string;
            int fret;
            if (stringData->convertPitch(note->pitch(), chord->staff(), chord->segment()->tick(), &string, &fret)) {
                setDot(string, fret);
            }
        }
        _maxFrets = stringData->frets();
    } else {
        _maxFrets = 6;
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    QPointF translation = -QPointF(stringDist * (_strings - 1), 0);
    if (_orientation == Orientation::HORIZONTAL) {
        painter->save();
        painter->rotate(-90);
        painter->translate(translation);
    }

    // Init pen and other values
    qreal _spatium = spatium() * _userMag;
    QPen pen(curColor());
    pen.setCapStyle(Qt::FlatCap);
    painter->setBrush(QBrush(QColor(painter->pen().color())));

    // x2 is the x val of the rightmost string
    qreal x2 = (_strings - 1) * stringDist;

    // Draw the nut
    pen.setWidthF(nutLw);
    painter->setPen(pen);
    painter->drawLine(QLineF(-stringLw * .5, 0.0, x2 + stringLw * .5, 0.0));

    // Draw strings and frets
    pen.setWidthF(stringLw);
    painter->setPen(pen);

    // y2 is the y val of the bottom fretline
    qreal y2 = fretDist * (_frets + .5);
    for (int i = 0; i < _strings; ++i) {
        qreal x = stringDist * i;
        painter->drawLine(QLineF(x, _fretOffset ? -_spatium * .2 : 0.0, x, y2));
    }
    for (int i = 1; i <= _frets; ++i) {
        qreal y = fretDist * i;
        painter->drawLine(QLineF(0.0, y, x2, y));
    }

    // dotd is the diameter of a dot
    qreal dotd = _spatium * .49 * score()->styleD(Sid::fretDotSize);

    // Draw dots, sym pen is used to draw them (and markers)
    QPen symPen(pen);
    symPen.setCapStyle(Qt::RoundCap);
    qreal symPenWidth = stringLw * 1.2;
    symPen.setWidthF(symPenWidth);

    for (auto const& i : _dots) {
        for (auto const& d : i.second) {
            if (!d.exists()) {
                continue;
            }

            int string = i.first;
            int fret = d.fret - 1;

            // Calculate coords of the top left corner of the dot
            qreal x = stringDist * string - dotd * .5;
            qreal y = fretDist * fret + fretDist * .5 - dotd * .5;

            // Draw different symbols
            painter->setPen(symPen);
            switch (d.dtype) {
            case FretDotType::CROSS:
                // Give the cross a slightly larger width
                symPen.setWidthF(symPenWidth * 1.5);
                painter->setPen(symPen);
                painter->drawLine(QLineF(x, y, x + dotd, y + dotd));
                painter->drawLine(QLineF(x + dotd, y, x, y + dotd));
                symPen.setWidthF(symPenWidth);
                break;
            case FretDotType::SQUARE:
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(QRectF(x, y, dotd, dotd));
                break;
            case FretDotType::TRIANGLE:
                painter->drawLine(QLineF(x, y + dotd, x + .5 * dotd, y));
                painter->drawLine(QLineF(x + .5 * dotd, y, x + dotd, y + dotd));
                painter->drawLine(QLineF(x + dotd, y + dotd, x, y + dotd));
                break;
            case FretDotType::NORMAL:
            default:
                painter->setBrush(symPen.color());
                painter->setNoPen();
                painter->drawEllipse(QRectF(x, y, dotd, dotd));
                break;
            }
        }
    }

    // Draw markers
    symPen.setWidthF(symPenWidth * 1.2);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(symPen);
    for (auto const& i : _markers) {
        int string = i.first;
        FretItem::Marker marker = i.second;
        if (!marker.exists()) {
            continue;
        }

        qreal x = stringDist * string - markerSize * .5;
        qreal y = -fretDist - markerSize * .5;
        if (marker.mtype == FretMarkerType::CIRCLE) {
            painter->drawEllipse(QRectF(x, y, markerSize, markerSize));
        } else if (marker.mtype == FretMarkerType::CROSS) {
            painter->drawLine(QPointF(x, y), QPointF(x + markerSize, y + markerSize));
            painter->drawLine(QPointF(x, y + markerSize), QPointF(x + markerSize, y));
        }
    }

    // Draw barres
    for (auto const& i : _barres) {
        int fret        = i.first;
        int startString = i.second.startString;
        int endString   = i.second.endString;

        qreal x1    = stringDist * startString;
        qreal newX2 = endString == -1 ? x2 : stringDist * endString;
        qreal y     = fretDist * (fret - 1) + fretDist * .5;
        pen.setWidthF(dotd * score()->styleD(Sid::barreLineWidth));
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->drawLine(QLineF(x1, y, newX2, y));
    }

    // Draw fret offset number
    if (_fretOffset > 0) {
        qreal fretNumMag = score()->styleD(Sid::fretNumMag);
        QFont scaledFont(font);
        scaledFont.setPointSizeF(font.pointSize() * _userMag * (spatium() / SPATIUM20) * MScore::pixelRatio* fretNumMag);
        painter->setFont(scaledFont);
        QString text = QString("%1").arg(_fretOffset + 1);

        if (_orientation == Orientation::VERTICAL) {
            if (_numPos == 0) {
                painter->drawText(QRectF(-stringDist * .4, .0, .0, fretDist),
                                  Qt::AlignVCenter | Qt::AlignRight | Qt::TextDontClip, text);
            } else {
                painter->drawText(QRectF(x2 + (stringDist * .4), .0, .0, fretDist),
                                  Qt::AlignVCenter | Qt::AlignLeft | Qt::TextDontClip,
                                  QString("%1").arg(_fretOffset + 1));
            }
        } else if (_orientation == Orientation::HORIZONTAL) {
            painter->save();
            painter->translate(-translation);
            painter->rotate(90);
            if (_numPos == 0) {
                painter->drawText(QRectF(.0, stringDist * (_strings - 1), .0, .0),
                                  Qt::AlignLeft | Qt::TextDontClip, text);
            } else {
                painter->drawText(QRectF(.0, .0, .0, .0),
                                  Qt::AlignBottom | Qt::AlignLeft | Qt::TextDontClip, text);
            }
            painter->restore();
        }
        painter->setFont(font);
    }

    // NOTE:JT possible future todo - draw fingerings

    if (_orientation == Orientation::HORIZONTAL) {
        painter->restore();
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
{
    qreal _spatium  = spatium() * _userMag;
    stringLw        = _spatium * 0.08;
    nutLw           = (_fretOffset || !_showNut) ? stringLw : _spatium * 0.2;
    stringDist      = score()->styleP(Sid::fretStringSpacing) * _userMag;
    fretDist        = score()->styleP(Sid::fretFretSpacing) * _userMag;
    markerSize      = stringDist * .8;

    qreal w    = stringDist * (_strings - 1) + markerSize;
    qreal h    = (_frets + 1) * fretDist + markerSize;
    qreal y    = -(markerSize * .5 + fretDist);
    qreal x    = -(markerSize * .5);

    // Allocate space for fret offset number
    if (_fretOffset > 0) {
        QFont scaledFont(font);
        scaledFont.setPointSize(font.pointSize() * _userMag);
        QFontMetricsF fm(scaledFont, MScore::paintDevice());

        qreal fretNumMag = score()->styleD(Sid::fretNumMag);
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * fretNumMag);
        QFontMetricsF fm2(scaledFont, MScore::paintDevice());
        qreal numw = fm2.width(QString("%1").arg(_fretOffset + 1));
        qreal xdiff = numw + stringDist * .4;
        w += xdiff;
        x += (_numPos == 0) == (_orientation == Orientation::VERTICAL) ? -xdiff : 0;
    }

    if (_orientation == Orientation::HORIZONTAL) {
        qreal tempW = w,
              tempX = x;
        w = h;
        h = tempW;
        x = y;
        y = tempX;
    }

    bbox().setRect(x, y, w, h);

    if (!parent() || !parent()->isSegment()) {
        setPos(QPointF());
        return;
    }

    // We need to get the width of the notehead/rest in order to position the fret diagram correctly
    Segment* pSeg = toSegment(parent());
    qreal noteheadWidth = 0;
    if (pSeg->isChordRestType()) {
        int idx = staff()->idx();
        for (Element* e = pSeg->firstElementOfSegment(pSeg, idx); e; e = pSeg->nextElementOfSegment(pSeg, e, idx)) {
            if (e->isRest()) {
                Rest* r = toRest(e);
                noteheadWidth = symWidth(r->sym());
                break;
            } else if (e->isNote()) {
                Note* n = toNote(e);
                noteheadWidth = n->headWidth();
                break;
            }
        }
    }

    qreal mainWidth = 0.0;
    if (_orientation == Orientation::VERTICAL) {
        mainWidth = stringDist * (_strings - 1);
    } else if (_orientation == Orientation::HORIZONTAL) {
        mainWidth = fretDist * (_frets + 0.5);
    }
    setPos((noteheadWidth - mainWidth) / 2, -(h + styleP(Sid::fretY)));

    autoplaceSegmentElement();

    // don't display harmony in palette
    if (!parent()) {
        return;
    }

    if (_harmony) {
        _harmony->layout();
    }
    if (_harmony && _harmony->autoplace() && _harmony->parent()) {
        Segment* s = toSegment(parent());
        Measure* m = s->measure();
        int si     = staffIdx();

        SysStaff* ss = m->system()->staff(si);
        QRectF r     = _harmony->bbox().translated(m->pos() + s->pos() + pos() + _harmony->pos() + QPointF(_harmony->xShapeOffset(), 0.0));

        qreal minDistance = _harmony->minDistance().val() * spatium();
        SkylineLine sk(false);
        sk.add(r.x(), r.bottom(), r.width());
        qreal d = sk.minDistance(ss->skyline().north());
        if (d > -minDistance) {
            qreal yd = d + minDistance;
            yd *= -1.0;
            _harmony->rypos() += yd;
            r.translate(QPointF(0.0, yd));
        }
        if (_harmony->addToSkyline()) {
            ss->skyline().add(r);
        }
    }
}

//---------------------------------------------------------
//   centerX
///   used by harmony for layout. Keep in sync with layout, same dotd and x as above
//    also used in Element::canvasPos().
//---------------------------------------------------------

qreal FretDiagram::centerX() const
{
    qreal dotd = spatium() * _userMag * .49 * score()->styleD(Sid::fretDotSize);
    qreal x    = -((dotd + stringLw) * .5);
    return bbox().right() * .5 + x;
}

//---------------------------------------------------------
//   write
//    NOTICE: if you are looking to change how fret diagrams are
//    written, edit the writeNew function. writeOld is purely compatibility.
//---------------------------------------------------------

static const std::array<Pid, 8> pids { {
    Pid::MIN_DISTANCE,
    Pid::FRET_OFFSET,
    Pid::FRET_FRETS,
    Pid::FRET_STRINGS,
    Pid::FRET_NUT,
    Pid::MAG,
    Pid::FRET_NUM_POS,
    Pid::ORIENTATION
} };

void FretDiagram::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.stag(this);

    // Write properties first and only once
    for (Pid p : pids) {
        writeProperty(xml, p);
    }
    Element::writeProperties(xml);

    if (_harmony) {
        _harmony->write(xml);
    }

    // Lowercase f indicates new writing format
    // TODO: in the next score format version (4) use only write new + props and discard
    // the compatibility writing.
    xml.stag("fretDiagram");
    writeNew(xml);
    xml.etag();

    writeOld(xml);
    xml.etag();
}

//---------------------------------------------------------
//   writeOld
//    This is the old method of writing. This is for backwards
//    compatibility with < 3.1 versions.
//---------------------------------------------------------

void FretDiagram::writeOld(XmlWriter& xml) const
{
    int lowestDotFret = -1;
    int furthestLeftLowestDot = -1;

    // Do some checks for details needed for checking whether to add barres
    for (int i = 0; i < _strings; ++i) {
        std::vector<FretItem::Dot> allDots = dot(i);

        bool dotExists = false;
        for (auto const& d : allDots) {
            if (d.exists()) {
                dotExists = true;
                break;
            }
        }

        if (!dotExists) {
            continue;
        }

        for (auto const& d : allDots) {
            if (d.exists()) {
                if (d.fret < lowestDotFret || lowestDotFret == -1) {
                    lowestDotFret = d.fret;
                    furthestLeftLowestDot = i;
                } else if (d.fret == lowestDotFret && (i < furthestLeftLowestDot || furthestLeftLowestDot == -1)) {
                    furthestLeftLowestDot = i;
                }
            }
        }
    }

    // The old system writes a barre as a bool, which causes no problems in any way, not at all.
    // So, only write that if the barre is on the lowest fret with a dot,
    // and there are no other dots on its fret, and it goes all the way to the right.
    int barreStartString = -1;
    int barreFret = -1;
    for (auto const& i : _barres) {
        FretItem::Barre b = i.second;
        if (b.exists()) {
            int fret = i.first;
            if (fret <= lowestDotFret && b.endString == -1 && !(fret == lowestDotFret && b.startString > furthestLeftLowestDot)) {
                barreStartString = b.startString;
                barreFret = fret;
                break;
            }
        }
    }

    for (int i = 0; i < _strings; ++i) {
        FretItem::Marker m = marker(i);
        std::vector<FretItem::Dot> allDots = dot(i);

        bool dotExists = false;
        for (auto const& d : allDots) {
            if (d.exists()) {
                dotExists = true;
                break;
            }
        }

        if (!dotExists && !m.exists() && i != barreStartString) {
            continue;
        }

        xml.stag(QString("string no=\"%1\"").arg(i));

        if (m.exists()) {
            xml.tag("marker", FretItem::markerToChar(m.mtype).unicode());
        }

        for (auto const& d : allDots) {
            if (d.exists() && !(i == barreStartString && d.fret == barreFret)) {
                xml.tag("dot", d.fret);
            }
        }

        // Add dot so barre will display in pre-3.1
        if (barreStartString == i) {
            xml.tag("dot", barreFret);
        }

        xml.etag();
    }

    if (barreFret > 0) {
        xml.tag("barre", 1);
    }
}

//---------------------------------------------------------
//   writeNew
//    This is the important one for 3.1+
//---------------------------------------------------------

void FretDiagram::writeNew(XmlWriter& xml) const
{
    for (int i = 0; i < _strings; ++i) {
        FretItem::Marker m = marker(i);
        std::vector<FretItem::Dot> allDots = dot(i);

        bool dotExists = false;
        for (auto const& d : allDots) {
            if (d.exists()) {
                dotExists = true;
                break;
            }
        }

        // Only write a string if we have anything to write
        if (!dotExists && !m.exists()) {
            continue;
        }

        // Start the string writing
        xml.stag(QString("string no=\"%1\"").arg(i));

        // Write marker
        if (m.exists()) {
            xml.tag("marker", FretItem::markerTypeToName(m.mtype));
        }

        // Write any dots
        for (auto const& d : allDots) {
            if (d.exists()) {
                // TODO: write fingering
                xml.tag(QString("dot fret=\"%1\"").arg(d.fret), FretItem::dotTypeToName(d.dtype));
            }
        }

        xml.etag();
    }

    for (int f = 1; f <= _frets; ++f) {
        FretItem::Barre b = barre(f);
        if (!b.exists()) {
            continue;
        }

        xml.tag(QString("barre start=\"%1\" end=\"%2\"").arg(b.startString).arg(b.endString), f);
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FretDiagram::read(XmlReader& e)
{
    // Read the old format first
    bool hasBarre = false;
    bool haveReadNew = false;

    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        // Check for new format fret diagram
        if (haveReadNew) {
            e.skipCurrentElement();
            continue;
        }
        if (tag == "fretDiagram") {
            readNew(e);
            haveReadNew = true;
        }
        // Check for new properties
        else if (tag == "showNut") {
            readProperty(e, Pid::FRET_NUT);
        } else if (tag == "orientation") {
            readProperty(e, Pid::ORIENTATION);
        }
        // Then read the rest if there is no new format diagram (compatibility read)
        else if (tag == "strings") {
            readProperty(e, Pid::FRET_STRINGS);
        } else if (tag == "frets") {
            readProperty(e, Pid::FRET_FRETS);
        } else if (tag == "fretOffset") {
            readProperty(e, Pid::FRET_OFFSET);
        } else if (tag == "string") {
            int no = e.intAttribute("no");
            while (e.readNextStartElement()) {
                const QStringRef& t(e.name());
                if (t == "dot") {
                    setDot(no, e.readInt());
                } else if (t == "marker") {
                    setMarker(no, QChar(e.readInt()) == 'X' ? FretMarkerType::CROSS : FretMarkerType::CIRCLE);
                }
                /*else if (t == "fingering")
                      setFingering(no, e.readInt());*/
                else {
                    e.unknown();
                }
            }
        } else if (tag == "barre") {
            hasBarre = e.readBool();
        } else if (tag == "mag") {
            readProperty(e, Pid::MAG);
        } else if (tag == "Harmony") {
            Harmony* h = new Harmony(score());
            h->read(e);
            add(h);
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
    }

    // Old handling of barres
    if (hasBarre) {
        for (int s = 0; s < _strings; ++s) {
            for (auto& d : dot(s)) {
                if (d.exists()) {
                    setBarre(s, -1, d.fret);
                    return;
                }
            }
        }
    }
}

//---------------------------------------------------------
//   readNew
//    read the new 'fretDiagram' tag
//---------------------------------------------------------

void FretDiagram::readNew(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        if (tag == "string") {
            int no = e.intAttribute("no");
            while (e.readNextStartElement()) {
                const QStringRef& t(e.name());
                if (t == "dot") {
                    int fret = e.intAttribute("fret", 0);
                    FretDotType dtype = FretItem::nameToDotType(e.readElementText());
                    setDot(no, fret, true, dtype);
                } else if (t == "marker") {
                    FretMarkerType mtype = FretItem::nameToMarkerType(e.readElementText());
                    setMarker(no, mtype);
                } else if (t == "fingering") {
                    e.readElementText();
                    /*setFingering(no, e.readInt()); NOTE:JT todo */
                } else {
                    e.unknown();
                }
            }
        } else if (tag == "barre") {
            int start = e.intAttribute("start", -1);
            int end = e.intAttribute("end", -1);
            int fret = e.readInt();

            setBarre(start, end, fret);
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
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
//   setFingering
//    NOTE:JT: todo possible future feature
//---------------------------------------------------------

#if 0
void FretDiagram::setFingering(int string, int finger)
{
    if (_dots.find(string) != _dots.end()) {
        _dots[string].fingering = finger;
        qDebug("set finger: s %d finger %d", string, finger);
    }
}

#endif

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
    Q_UNUSED(add);

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
    for (ScoreElement* e : linkList()) {
        FretDiagram* fd = toFretDiagram(e);
        fd->score()->undo(new FretDot(fd, _string, _fret, _add, _dtype));
    }
}

//---------------------------------------------------------
//   undoSetFretMarker
//---------------------------------------------------------

void FretDiagram::undoSetFretMarker(int _string, FretMarkerType _mtype)
{
    for (ScoreElement* e : linkList()) {
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
    for (ScoreElement* e : linkList()) {
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
    for (ScoreElement* e : linkList()) {
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

void FretDiagram::setHarmony(QString harmonyText)
{
    if (!_harmony) {
        Harmony* h = new Harmony(score());
        add(h);
    }

    _harmony->setHarmony(harmonyText);
    _harmony->setXmlText(_harmony->harmonyName());
    triggerLayout();
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void FretDiagram::add(Element* e)
{
    e->setParent(this);
    if (e->isHarmony()) {
        _harmony = toHarmony(e);
        _harmony->setTrack(track());
        if (_harmony->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED) {
            _harmony->resetProperty(Pid::OFFSET);
        }

        _harmony->setProperty(Pid::ALIGN, int(Align::HCENTER | Align::TOP));
        _harmony->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
    } else {
        qWarning("FretDiagram: cannot add <%s>\n", e->name());
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void FretDiagram::remove(Element* e)
{
    if (e == _harmony) {
        _harmony = 0;
    } else {
        qWarning("FretDiagram: cannot remove <%s>\n", e->name());
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

Element* FretDiagram::drop(EditData& data)
{
    Element* e = data.dropElement;
    if (e->isHarmony()) {
        Harmony* h = toHarmony(e);
        h->setParent(parent());
        h->setTrack(track());
        score()->undoAddElement(h);
    } else {
        qWarning("FretDiagram: cannot drop <%s>\n", e->name());
        delete e;
        e = 0;
    }
    return e;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void FretDiagram::scanElements(void* data, void (* func)(void*, Element*), bool all)
{
    Q_UNUSED(all);
    ScoreElement::scanElements(data, func, all);
    func(data, this);
}

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void FretDiagram::writeMusicXML(XmlWriter& xml) const
{
    qDebug("FretDiagram::writeMusicXML() this %p harmony %p", this, _harmony);
    xml.stag("frame");
    xml.tag("frame-strings", _strings);
    xml.tag("frame-frets", frets());

    for (int i = 0; i < _strings; ++i) {
        int mxmlString = _strings - i;

        std::vector<int> bStarts;
        std::vector<int> bEnds;
        for (auto const& j : _barres) {
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

        if (marker(i).exists() && marker(i).mtype == FretMarkerType::CIRCLE) {
            xml.stag("frame-note");
            xml.tag("string", mxmlString);
            xml.tag("fret", "0");
            xml.etag();
        } else {
            // Write dots
            for (auto const& d : dot(i)) {
                if (!d.exists()) {
                    continue;
                }
                xml.stag("frame-note");
                xml.tag("string", mxmlString);
                xml.tag("fret", d.fret);
                // TODO: write fingerings

                // Also write barre if it starts at this dot
                if (std::find(bStarts.begin(), bStarts.end(), d.fret) != bStarts.end()) {
                    xml.tagE("barre type=\"start\"");
                    bStarts.erase(std::remove(bStarts.begin(), bStarts.end(), d.fret), bStarts.end());
                }
                if (std::find(bEnds.begin(), bEnds.end(), d.fret) != bEnds.end()) {
                    xml.tagE("barre type=\"stop\"");
                    bEnds.erase(std::remove(bEnds.begin(), bEnds.end(), d.fret), bEnds.end());
                }
                xml.etag();
            }
        }

        // Write unwritten barres
        for (int j : bStarts) {
            xml.stag("frame-note");
            xml.tag("string", mxmlString);
            xml.tag("fret", j);
            xml.tagE("barre type=\"start\"");
            xml.etag();
        }

        for (int j : bEnds) {
            xml.stag("frame-note");
            xml.tag("string", mxmlString);
            xml.tag("fret", j);
            xml.tagE("barre type=\"stop\"");
            xml.etag();
        }
    }

    xml.etag();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant FretDiagram::getProperty(Pid propertyId) const
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
        return int(_orientation);
        break;
    default:
        return Element::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool FretDiagram::setProperty(Pid propertyId, const QVariant& v)
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
        _orientation = Orientation(v.toInt());
        break;
    default:
        return Element::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant FretDiagram::propertyDefault(Pid pid) const
{
    // We shouldn't style the fret offset
    if (pid == Pid::FRET_OFFSET) {
        return QVariant(0);
    }

    for (const StyledProperty& p : *styledProperties()) {
        if (p.pid == pid) {
            if (propertyType(pid) == P_TYPE::SP_REAL) {
                return score()->styleP(p.sid);
            }
            return score()->styleV(p.sid);
        }
    }
    return Element::propertyDefault(pid);
}

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void FretDiagram::endEditDrag(EditData& editData)
{
    Element::endEditDrag(editData);

    triggerLayout();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString FretDiagram::accessibleInfo() const
{
    QString chordName = _harmony ? QObject::tr("with chord symbol %1").arg(_harmony->harmonyName()) : QObject::tr("without chord symbol");
    return QString("%1 %2").arg(userName(), chordName);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

QString FretDiagram::screenReaderInfo() const
{
    QString detailedInfo;
    for (int i = 0; i < _strings; i++) {
        QString stringIdent = QObject::tr("string %1").arg(i + 1);

        const FretItem::Marker& m = marker(i);
        QString markerName;
        switch (m.mtype) {
        case FretMarkerType::CIRCLE:
            markerName = QObject::tr("circle marker");
            break;
        case FretMarkerType::CROSS:
            markerName = QObject::tr("cross marker");
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

        if (dotsCount == 0 && markerName.length() == 0) {
            continue;
        }

        QString fretInfo;
        if (dotsCount == 1) {
            fretInfo = QString("%1").arg(fretsWithDots.front());
        } else if (dotsCount > 1) {
            int max = int(fretsWithDots.size());
            for (int j = 0; j < max; j++) {
                if (j == max - 1) {
                    fretInfo = QObject::tr("%1 and %2").arg(fretInfo).arg(fretsWithDots[j]);
                } else {
                    fretInfo = QString("%1 %2").arg(fretInfo).arg(fretsWithDots[j]);
                }
            }
        }

        //: Omit the "%n " for the singular translation (and the "(s)" too)
        QString dotsInfo = QObject::tr("%n dot(s) on fret(s) %1", "", dotsCount).arg(fretInfo);

        detailedInfo = QString("%1 %2 %3 %4").arg(detailedInfo, stringIdent, markerName, dotsInfo);
    }

    QString barreInfo;
    for (auto const& iter : _barres) {
        const FretItem::Barre& b = iter.second;
        if (!b.exists()) {
            continue;
        }

        QString fretInfo = QObject::tr("fret %1").arg(iter.first);

        QString newBarreInfo;
        if (b.startString == 0 && (b.endString == -1 || b.endString == _strings - 1)) {
            newBarreInfo = QObject::tr("barré %1").arg(fretInfo);
        } else {
            QString startPart = QObject::tr("beginning string %1").arg(b.startString + 1);
            QString endPart;
            if (b.endString != -1) {
                endPart = QObject::tr("and ending string %1").arg(b.endString + 1);
            }

            newBarreInfo = QObject::tr("partial barré %1 %2 %3").arg(fretInfo, startPart, endPart);
        }

        barreInfo = QString("%1 %2").arg(barreInfo, newBarreInfo);
    }

    detailedInfo = QString("%1 %2").arg(detailedInfo, barreInfo);

    if (detailedInfo.trimmed().length() == 0) {
        detailedInfo = QObject::tr("no content");
    }

    QString chordName = _harmony ? QObject::tr("with chord symbol %1").arg(_harmony->generateScreenReaderInfo()) : QObject::tr(
        "without chord symbol");
    QString basicInfo = QString("%1 %2").arg(userName(), chordName);

    QString generalInfo = QObject::tr("%n string(s) total", "", _strings);

    QString res = QString("%1 %2 %3").arg(basicInfo, generalInfo, detailedInfo);

    return res;
}

//---------------------------------------------------------
//   markerToChar
//---------------------------------------------------------

QChar FretItem::markerToChar(FretMarkerType t)
{
    switch (t) {
    case FretMarkerType::CIRCLE:
        return QChar('O');
    case FretMarkerType::CROSS:
        return QChar('X');
    case FretMarkerType::NONE:
    default:
        return QChar();
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

QString FretItem::markerTypeToName(FretMarkerType t)
{
    for (auto i : FretItem::markerTypeNameMap) {
        if (i.mtype == t) {
            return i.name;
        }
    }
    qFatal("Unrecognised FretMarkerType!");
    return QString();         // prevent compiler warnings
}

//---------------------------------------------------------
//   nameToMarkerType
//---------------------------------------------------------

FretMarkerType FretItem::nameToMarkerType(QString n)
{
    for (auto i : FretItem::markerTypeNameMap) {
        if (i.name == n) {
            return i.mtype;
        }
    }
    qWarning("Unrecognised marker name!");
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

QString FretItem::dotTypeToName(FretDotType t)
{
    for (auto i : FretItem::dotTypeNameMap) {
        if (i.dtype == t) {
            return i.name;
        }
    }
    qFatal("Unrecognised FretDotType!");
    return QString();         // prevent compiler warnings
}

//---------------------------------------------------------
//   nameToDotType
//---------------------------------------------------------

FretDotType FretItem::nameToDotType(QString n)
{
    for (auto i : FretItem::dotTypeNameMap) {
        if (i.name == n) {
            return i.dtype;
        }
    }
    qWarning("Unrecognised dot name!");
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
        qFatal("Tried to undo fret diagram change without ever setting diagram!");
        return;
    }

    // Reset every fret diagram property of the changed diagram
    // FretUndoData is a friend of FretDiagram so has access to these private members
    _diagram->_barres = _barres;
    _diagram->_markers = _markers;
    _diagram->_dots = _dots;
}
}
