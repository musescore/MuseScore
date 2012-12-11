//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: fret.cpp 5568 2012-04-22 10:08:43Z wschweer $
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
#include "tablature.h"
#include "chord.h"
#include "note.h"
#include "segment.h"
#include "mscore.h"
#include "harmony.h"

static const int DEFAULT_STRINGS = 6;
static const int DEFAULT_FRETS = 5;

//    parent() is Segment or Box
//

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::FretDiagram(Score* score)
   : Element(score)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_ON_STAFF | ELEMENT_SELECTABLE);
      _strings    = DEFAULT_STRINGS;
      _frets      = DEFAULT_FRETS;
      _maxFrets   = 24;
      maxStrings  = 0;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;
      font.setFamily("FreeSans");
      int size = lrint(4.0 * MScore::DPI * mag()/ PPI);
      font.setPixelSize(size);
      _harmony = 0;
      }

FretDiagram::FretDiagram(const FretDiagram& f)
   : Element(f)
      {
      _strings    = f._strings;
      _frets      = f._frets;
      _fretOffset = f._fretOffset;
      _maxFrets   = f._maxFrets;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      font        = f.font;

      if (f._dots) {
            _dots = new char[_strings];
            memcpy(_dots, f._dots, _strings);
            }
      if (f._marker) {
            _marker = new char[_strings];
            memcpy(_marker, f._marker, _strings);
            }
      if (f._fingering) {
            _fingering = new char[_strings];
            memcpy(_fingering, f._fingering, _strings);
            }
      if (f._harmony)
            _harmony = new Harmony(*f._harmony);
      else
            _harmony = 0;
      }

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::~FretDiagram()
      {
      delete _dots;
      delete _marker;
      delete _fingering;
      }

#if 1
//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF FretDiagram::pagePos() const
      {
      if (parent() == 0)
            return pos();
      if (parent()->type() == SEGMENT) {
            Measure* m = static_cast<Segment*>(parent())->measure();
            System* system = m->system();
            qreal yp = y();
            if (system)
                  yp += system->staffY(staffIdx());
            return QPointF(pageX(), yp);
            }
      else
            return Element::pagePos();
      }
#endif

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF FretDiagram::dragAnchor() const
      {
      if (parent()->type() == SEGMENT) {
            Segment* s     = static_cast<Segment*>(parent());
            Measure* m     = s->measure();
            System* system = m->system();
            qreal yp      = system->staff(staffIdx())->y() + system->y();
            qreal xp      = m->tick2pos(s->tick()) + m->pagePos().x();
            QPointF p1(xp, yp);

            qreal x  = 0.0;
            qreal y  = 0.0;
#if 0 // TODOxx
            qreal tw = width();
            qreal th = height();
            if (_align & ALIGN_BOTTOM)
                  y = th;
            else if (_align & ALIGN_VCENTER)
                  y = (th * .5);
            else if (_align & ALIGN_BASELINE)
                  y = baseLine();
            if (_align & ALIGN_RIGHT)
                  x = tw;
            else if (_align & ALIGN_HCENTER)
                  x = (tw * .5);
#endif
            return QLineF(p1, abbox().topLeft() + QPointF(x, y));
            }
      return QLineF(parent()->pagePos(), abbox().topLeft());
      }

//---------------------------------------------------------
//   setStrings
//---------------------------------------------------------

void FretDiagram::setStrings(int n)
      {
      if (n <= maxStrings) {
            _strings = n;
            return;
            }
      maxStrings = n;
      if (_dots) {
            char* ndots = new char[n];
            memcpy(ndots, _dots, _strings);
            for (int i = _strings; i < n; ++i)
                  ndots[i] = 0;
            delete _dots;
            _dots = ndots;
            }
      if (_marker) {
            char* nmarker = new char[n];
            memcpy(nmarker, _marker, _strings);
            for (int i = _strings; i < n; ++i)
                  nmarker[i] = 0;
            delete _marker;
            _marker = nmarker;
            }
      if (_fingering) {
            char* nfingering = new char[n];
            memcpy(nfingering, _fingering, _strings);
            for (int i = _strings; i < n; ++i)
                  nfingering[i] = 0;
            delete _fingering;
            _fingering = nfingering;
            }
      _strings = n;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void FretDiagram::init(Tablature* tab, Chord* chord)
      {
      if (tab == 0)
            setStrings(6);
      else
            setStrings(tab->strings());
      if (tab) {
            for (int string = 0; string < _strings; ++string)
                  _marker[string] = 'X';
            foreach(const Note* note, chord->notes()) {
                  int string;
                  int fret;
                  if (tab->convertPitch(note->ppitch(), &string, &fret))
                        setDot(string, fret);
                  }
            _maxFrets = tab->frets();
            }
      else
            _maxFrets = 6;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(QPainter* painter) const
      {
      qreal _spatium = spatium();
      QPen pen(curColor());
      pen.setWidthF(lw2);
      pen.setCapStyle(Qt::FlatCap);
      painter->setPen(pen);
      painter->setBrush(QBrush(QColor(painter->pen().color())));
      qreal x2 = (_strings-1) * stringDist;
      painter->drawLine(QLineF(-lw1*.5, 0.0, x2+lw1*.5, 0.0));

      pen.setWidthF(lw1);
      painter->setPen(pen);
      qreal y2 = (_frets+1) * fretDist - fretDist*.5;
      for (int i = 0; i < _strings; ++i) {
            qreal x = stringDist * i;
            painter->drawLine(QLineF(x, _fretOffset ? -_spatium*.2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            qreal y = fretDist * i;
            painter->drawLine(QLineF(0.0, y, x2, y));
            }
      painter->setFont(font);
      QFontMetricsF fm(font);
      for (int i = 0; i < _strings; ++i) {
            if (_dots && _dots[i]) {
                  qreal dotd = stringDist * .6;
                  int fret = _dots[i] - 1;
                  qreal x = stringDist * i - dotd * .5;
                  qreal y = fretDist * fret + fretDist * .5 - dotd * .5;
                  painter->drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            if (_marker && _marker[i]) {
                  qreal x = stringDist * i;
                  qreal y = -fretDist * .3 - fm.ascent();
                  painter->drawText(QRectF(x, y, .0,.0),
                     Qt::AlignHCenter|Qt::TextDontClip, QChar(_marker[i]));
                  }
            }
      if (_fretOffset > 0) {
            painter->drawText(QRectF(-stringDist, fretDist*.5, .0, .0),
               Qt::AlignVCenter | Qt::TextDontClip,
               QString("%1").arg(_fretOffset+1));
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
      {
      qreal _spatium = spatium();
      lw1             = _spatium * 0.08;
      lw2             = _fretOffset ? lw1 : _spatium * 0.2;
      stringDist      = _spatium * .7;
      fretDist        = _spatium * .8;

      qreal w = stringDist * (_strings-1);
      qreal h = _frets * fretDist + fretDist * .5;
      qreal y = 0.0;
      qreal dotd = stringDist * .6;
      qreal x = -((dotd+lw1) * .5);
      w += dotd + lw1;
      if (_marker) {
            QFontMetricsF fm(font);
            y = -(fretDist * .1 + fm.height());
            h -= y;
            }
      setbbox(QRectF(x, y, w, h));

      setPos(-_spatium, -h - _spatium);
      adjustReadPos();

      if (_harmony)
            _harmony->layout();

      if (parent() == 0 || parent()->type() != SEGMENT)
            return;
      Measure* m     = static_cast<Segment*>(parent())->measure();
      int idx        = staffIdx();
      MStaff* mstaff = m->mstaff(idx);
      System* system = m->system();
      qreal yp       = pos().y() + system->staff(idx)->y() + system->y();
      mstaff->distanceUp = qMax(mstaff->distanceUp, h + _spatium * 2 - yp);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FretDiagram::write(Xml& xml) const
      {
      xml.stag("FretDiagram");
      Element::writeProperties(xml);

      if (_strings != DEFAULT_STRINGS)
            xml.tag("strings", _strings);
      if (_frets != DEFAULT_FRETS)
            xml.tag("frets", _frets);
      if (_fretOffset)
            xml.tag("fretOffset", _fretOffset);
      for (int i = 0; i < _strings; ++i) {
            if ((_dots && _dots[i]) || (_marker && _marker[i]) || (_fingering && _fingering[i])) {
                  xml.stag(QString("string no=\"%1\"").arg(i));
                  if (_dots && _dots[i])
                        xml.tag("dot", _dots[i]);
                  if (_marker && _marker[i])
                        xml.tag("marker", _marker[i]);
                  if (_fingering && _fingering[i])
                        xml.tag("fingering", _fingering[i]);
                  xml.etag();
                  }
            }
      if (_harmony)
            _harmony->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FretDiagram::read(const QDomElement& de)
      {
      delete _dots;
      delete _marker;
      delete _fingering;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "strings")
                  _strings = val;
            else if (tag == "frets")
                  _frets = val;
            else if (tag == "fretOffset")
                  _fretOffset = val;
            else if (tag == "string") {
                  int no = e.attribute("no").toInt();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        int val = ee.text().toInt();
                        if (tag == "dot")
                              setDot(no, val);
                        else if (tag == "marker")
                              setMarker(no, val);
                        else if (tag == "fingering")
                              setFingering(no, val);
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "Harmony") {
                  Harmony* h = new Harmony(score());
                  h->read(e);
                  add(h);
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setDot
//---------------------------------------------------------

void FretDiagram::setDot(int string, int fret)
      {
      if (_dots == 0) {
            _dots = new char[_strings];
            memset(_dots, 0, _strings);
            }
      if (0 <= string && string < _strings) {
            _dots[string] = fret;
            setMarker(string, 0);
            }
      }

//---------------------------------------------------------
//   setMarker
//---------------------------------------------------------

void FretDiagram::setMarker(int string, int marker)
      {
      if (_marker == 0) {
            _marker = new char[_strings];
            memset(_marker, 0, _strings);
            }
      if (0 <= string && string < _strings)
            _marker[string] = marker;
      }

//---------------------------------------------------------
//   setFingering
//---------------------------------------------------------

void FretDiagram::setFingering(int string, int finger)
      {
      if (_fingering == 0) {
            _fingering = new char[_strings];
            memset(_fingering, 0, _strings);
            }
      _fingering[string] = finger;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void FretDiagram::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == HARMONY) {
            _harmony = static_cast<Harmony*>(e);
            _harmony->setTrack(track());
            }
      else
            qWarning("FretDiagram: cannot add <%s>\n", e->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void FretDiagram::remove(Element* e)
      {
      if (e == _harmony)
            _harmony = 0;
      else
            qWarning("FretDiagram: cannot remove <%s>\n", e->name());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool FretDiagram::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == HARMONY;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* FretDiagram::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() == HARMONY) {
            // TODO: make undoable
            Harmony* h = static_cast<Harmony*>(e);
            h->setParent(this);
            score()->undoAddElement(h);
            }
      else {
            qWarning("FretDiagram: cannot drop <%s>\n", e->name());
            delete e;
            e = 0;
            }
      return e;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void FretDiagram::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      Q_UNUSED(all);
      func(data, this);
      if (_harmony)
            func(data, _harmony);
      }


//---------------------------------------------------------
//   Read MusicXML
//
// Set the FretDiagram state based on the MusicXML <figure> node de.
//---------------------------------------------------------

void FretDiagram::readMusicXML(const QDomElement& de)
      {
      qDebug("FretDiagram::readMusicXML");

      // TODO: is this required ?
      delete _dots;
      delete _marker;
      delete _fingering;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;
      // end TODO: is this required ?

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "frame-frets") {
                  if (val > 0)
                        setFrets(val);
                  else
                        qDebug("FretDiagram::readMusicXML: illegal frame-fret %d", val);
                  }
            else if (tag == "frame-note") {
                  int fret   = -1;
                  int string = -1;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        int val = ee.text().toInt();
                        if (tag == "fret")
                              fret = val;
                        else if (tag == "string")
                              string = val;
                        else
                              domError(ee);
                        }
                  qDebug("FretDiagram::readMusicXML string %d fret %d", string, fret);
                  if (string > 0) {
                        if (fret == 0)
                              setMarker(strings() - string, 79 /* ??? */);
                        else if (fret > 0)
                              setDot(strings() - string, fret);
                        }
                  }
            else if (tag == "frame-strings") {
                  if (val > 0) {
                        setStrings(val);
                        for (int i = 0; i < val; ++i)
                              setMarker(i, 88 /* ??? */);
                        }
                  else
                        qDebug("FretDiagram::readMusicXML: illegal frame-strings %d", val);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void FretDiagram::writeMusicXML(Xml& /*xml*/) const
      {
      }
