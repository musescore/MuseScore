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

namespace Ms {

//    parent() is Segment or Box
//

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::FretDiagram(Score* score)
   : Element(score)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SELECTABLE);
      font.setFamily("FreeSans");
      font.setPixelSize(4.0 * mag());
      }

FretDiagram::FretDiagram(const FretDiagram& f)
   : Element(f)
      {
      _strings    = f._strings;
      _frets      = f._frets;
      _fretOffset = f._fretOffset;
      _maxFrets   = f._maxFrets;
      maxStrings  = f.maxStrings;
      font        = f.font;
      _barre      = f._barre;
      _userMag    = f._userMag;

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
      delete[] _dots;
      delete[] _marker;
      delete[] _fingering;
      }

//---------------------------------------------------------
//   fromString
///  Create diagram from string like "XO-123"
///  Always assume barre on the first visible fret
//---------------------------------------------------------

FretDiagram* FretDiagram::fromString(Score* score, const QString &s)
      {
      FretDiagram* fd = new FretDiagram(score);
      fd->setStrings(s.size());
      fd->setFrets(4);
      int offset = 0;
      int barreString = -1;
      for (int i = 0; i < s.size(); i++) {
            QChar c = s.at(i);
            if (c == 'X' or c == 'O')
                  fd->setMarker(i, c.unicode());
            else if (c == '-' && barreString == -1) {
                  fd->setBarre(1);
                  barreString = i;
                  }
            else {
                  int fret = c.digitValue();
                  if (fret != -1) {
                        fd->setDot(i, fret);
                        if (fret - 3 > 0 && offset < fret - 3)
                            offset = fret - 3;
                        }
                  }
            }
      if (offset > 0) {
            fd->setOffset(offset);
            for (int i = 0; i < fd->strings(); i++)
                  if (fd->dot(i))
                        fd->setDot(i, fd->dot(i) - offset);
            }
      if (barreString >= 0)
            fd->setDot(barreString, 1);
      return fd;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF FretDiagram::pagePos() const
      {
      if (parent() == 0)
            return pos();
      if (parent()->type() == Element::Type::SEGMENT) {
            Measure* m = static_cast<Segment*>(parent())->measure();
            System* system = m->system();
            qreal yp = y();
            if (system)
                  yp += system->staffYpage(staffIdx());
            return QPointF(pageX(), yp);
            }
      else
            return Element::pagePos();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF FretDiagram::dragAnchor() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp;
      if (parent()->type() == Element::Type::SEGMENT) {
            System* system = static_cast<Segment*>(parent())->measure()->system();
            yp = system->staffCanvasYpage(staffIdx());
            }
      else
            yp = parent()->canvasPos().y();
      QPointF p1(xp, yp);
      return QLineF(p1, canvasPos());
#if 0 // TODOxx
      if (parent()->type() == Element::Type::SEGMENT) {
            Segment* s     = static_cast<Segment*>(parent());
            Measure* m     = s->measure();
            System* system = m->system();
            qreal yp      = system->staff(staffIdx())->y() + system->y();
            qreal xp      = m->tick2pos(s->tick()) + m->pagePos().x();
            QPointF p1(xp, yp);

            qreal x  = 0.0;
            qreal y  = 0.0;
            qreal tw = width();
            qreal th = height();
            if (_align & AlignmentFlags::BOTTOM)
                  y = th;
            else if (_align & AlignmentFlags::VCENTER)
                  y = (th * .5);
            else if (_align & AlignmentFlags::BASELINE)
                  y = baseLine();
            if (_align & AlignmentFlags::RIGHT)
                  x = tw;
            else if (_align & AlignmentFlags::HCENTER)
                  x = (tw * .5);
            return QLineF(p1, abbox().topLeft() + QPointF(x, y));
            }
      return QLineF(parent()->pagePos(), abbox().topLeft());
#endif
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void FretDiagram::setOffset(int offset)
      {
      _fretOffset = offset;
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
            memcpy(ndots, _dots, n);
            for (int i = _strings; i < n; ++i)
                  ndots[i] = 0;
            delete[] _dots;
            _dots = ndots;
            }
      if (_marker) {
            char* nmarker = new char[n];
            memcpy(nmarker, _marker, n);
            for (int i = _strings; i < n; ++i)
                  nmarker[i] = 'O';
            delete[] _marker;
            _marker = nmarker;
            }
      if (_fingering) {
            char* nfingering = new char[n];
            memcpy(nfingering, _fingering, n);
            for (int i = _strings; i < n; ++i)
                  nfingering[i] = 0;
            delete[] _fingering;
            _fingering = nfingering;
            }
      _strings = n;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void FretDiagram::init(StringData* stringData, Chord* chord)
      {
      if (!stringData)
            setStrings(6);
      else
            setStrings(stringData->strings());
      if (stringData) {
            for (int string = 0; string < _strings; ++string)
                  _marker[string] = 'X';
            foreach(const Note* note, chord->notes()) {
                  int string;
                  int fret;
                  if (stringData->convertPitch(note->pitch(), chord->staff(), chord->segment()->tick(), &string, &fret))
                        setDot(string, fret);
                  }
            _maxFrets = stringData->frets();
            }
      else
            _maxFrets = 6;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(QPainter* painter) const
      {
      qreal _spatium = spatium() * _userMag * score()->styleD(StyleIdx::fretMag);
      QPen pen(curColor());
      pen.setWidthF(lw2);
      pen.setCapStyle(Qt::FlatCap);
      painter->setPen(pen);
      painter->setBrush(QBrush(QColor(painter->pen().color())));
      qreal x2 = (_strings-1) * stringDist;
      painter->drawLine(QLineF(-lw1 * .5, 0.0, x2 + lw1 * .5, 0.0));

      pen.setWidthF(lw1);
      painter->setPen(pen);
      qreal y2 = (_frets+1) * fretDist - fretDist * .5;
      for (int i = 0; i < _strings; ++i) {
            qreal x = stringDist * i;
            painter->drawLine(QLineF(x, _fretOffset ? -_spatium * .2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            qreal y = fretDist * i;
            painter->drawLine(QLineF(0.0, y, x2, y));
            }
      QFont scaledFont(font);
      scaledFont.setPointSizeF(font.pointSize() * _userMag);
      painter->setFont(scaledFont);
      QFontMetricsF fm(scaledFont, MScore::paintDevice());
      qreal dotd = stringDist * .6;

      for (int i = 0; i < _strings; ++i) {
            if (_dots && _dots[i] && _dots[i] != _barre) {
                  int fret = _dots[i] - 1;
                  qreal x = stringDist * i - dotd * .5;
                  qreal y = fretDist * fret + fretDist * .5 - dotd * .5;
                  painter->drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            if (_marker && _marker[i]) {
                  qreal x = stringDist * i;
                  qreal y = -fretDist * .3 - fm.ascent();
                  painter->drawText(QRectF(x, y, .0, .0),
                     Qt::AlignHCenter|Qt::TextDontClip, QChar(_marker[i]));
                  }
            }
      if (_barre) {
            int string = -1;
            for (int i = 0; i < _strings; ++i) {
                  if (_dots[i] == _barre) {
                        string = i;
                        break;
                        }
                  }
            if (string != -1) {
                  qreal x1   = stringDist * string;
                  qreal x2   = stringDist * (_strings-1);
                  qreal y    = fretDist * (_barre-1) + fretDist * .5;
                  pen.setWidthF((dotd + lw2 * .5) * score()->styleD(StyleIdx::barreLineWidth));
                  pen.setCapStyle(Qt::RoundCap);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x1, y, x2, y));
                  }
            }
      if (_fretOffset > 0) {
            qreal fretNumMag = score()->styleD(StyleIdx::fretNumMag);
            QFont scaledFont(font);
            scaledFont.setPointSizeF(font.pointSize() * fretNumMag * _userMag);
            painter->setFont(scaledFont);
            if (score()->styleI(StyleIdx::fretNumPos) == 0)
                  painter->drawText(QRectF(-stringDist *.4, .0, .0, fretDist),
                     Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
                     QString("%1").arg(_fretOffset+1));
            else
                  painter->drawText(QRectF(x2 + (stringDist * 0.4), .0, .0, fretDist),
                     Qt::AlignVCenter|Qt::AlignLeft|Qt::TextDontClip,
                     QString("%1").arg(_fretOffset+1));
            painter->setFont(font);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
      {
      qreal _spatium = spatium() * _userMag * score()->styleD(StyleIdx::fretMag);
      lw1             = _spatium * 0.08;
      lw2             = _fretOffset ? lw1 : _spatium * 0.2;
      stringDist      = _spatium * .7;
      fretDist        = _spatium * .8;

      qreal w = stringDist * (_strings - 1);
      qreal h = _frets * fretDist + fretDist * .5;
      qreal y = 0.0;
      qreal dotd = stringDist * .6;
      qreal x = -((dotd+lw1) * .5);
      w += dotd + lw1;
      if (_marker) {
            QFont scaledFont(font);
            scaledFont.setPointSize(font.pointSize() * _userMag);
            QFontMetricsF fm(scaledFont, MScore::paintDevice());
            y = -(fretDist * .1 + fm.height());
            h -= y;
            }
      bbox().setRect(x, y, w, h);

      setPos(-_spatium, -h - score()->styleP(StyleIdx::fretY) + _spatium );
      adjustReadPos();

      if (_harmony)
            _harmony->layout();

      if (!parent() || !parent()->isSegment()) {
            setPos(QPointF());
            return;
            }
//      Measure* m     = toSegment(parent())->measure();
//      int idx        = staffIdx();
//      MStaff* mstaff = m->mstaff(idx);
//      qreal dist = -(bbox().top());
//      mstaff->distanceUp = qMax(mstaff->distanceUp, dist + _spatium * 2);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FretDiagram::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("FretDiagram");
      Element::writeProperties(xml);

      writeProperty(xml, P_ID::FRET_STRINGS);
      writeProperty(xml, P_ID::FRET_FRETS);
      writeProperty(xml, P_ID::FRET_OFFSET);
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
      writeProperty(xml, P_ID::FRET_BARRE);
      writeProperty(xml, P_ID::MAG);
      if (_harmony)
            _harmony->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FretDiagram::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "strings")
                  _strings = e.readInt();
            else if (tag == "frets")
                  _frets = e.readInt();
            else if (tag == "fretOffset")
                  _fretOffset = e.readInt();
            else if (tag == "string") {
                  int no = e.intAttribute("no");
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "dot")
                              setDot(no, e.readInt());
                        else if (tag == "marker")
                              setMarker(no, e.readInt());
                        else if (tag == "fingering")
                              setFingering(no, e.readInt());
                        else
                              e.unknown();
                        }
                  }
            else if (tag == "barre")
                  setBarre(e.readInt());
            else if (tag == "mag")
                  _userMag = e.readDouble(0.1, 10.0);
            else if (tag == "Harmony") {
                  Harmony* h = new Harmony(score());
                  h->read(e);
                  add(h);
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
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

void FretDiagram::setMarker(int string, int m)
      {
      if (_marker == 0) {
            _marker = new char[_strings];
            memset(_marker, 0, _strings);
            }
      if (0 <= string && string < _strings)
            _marker[string] = m;
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
      if (e->type() == Element::Type::HARMONY) {
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

bool FretDiagram::acceptDrop(const DropData& data) const
      {
      return data.element->type() == Element::Type::HARMONY;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* FretDiagram::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() == Element::Type::HARMONY) {
            Harmony* h = static_cast<Harmony*>(e);
            h->setParent(parent());
            h->setTrack((track() / VOICES) * VOICES);
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

#if 0
//---------------------------------------------------------
//   Read MusicXML
//
// Set the FretDiagram state based on the MusicXML <figure> node de.
//---------------------------------------------------------

void FretDiagram::readMusicXML(XmlReader& e)
      {
      qDebug("FretDiagram::readMusicXML");

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "frame-frets") {
                  int val = e.readInt();
                  if (val > 0)
                        setFrets(val);
                  else
                        qDebug("FretDiagram::readMusicXML: illegal frame-fret %d", val);
                  }
            else if (tag == "frame-note") {
                  int fret   = -1;
                  int string = -1;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        int val = e.readInt();
                        if (tag == "fret")
                              fret = val;
                        else if (tag == "string")
                              string = val;
                        else
                              e.unknown();
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
                  int val = e.readInt();
                  if (val > 0) {
                        setStrings(val);
                        for (int i = 0; i < val; ++i)
                              setMarker(i, 88 /* ??? */);
                        }
                  else
                        qDebug("FretDiagram::readMusicXML: illegal frame-strings %d", val);
                  }
            else
                  e.unknown();
            }
      }
#endif

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void FretDiagram::writeMusicXML(Xml& xml) const
      {
      qDebug("FretDiagram::writeMusicXML() this %p harmony %p", this, _harmony);
      int _strings = strings();
      xml.stag("frame");
      xml.tag("frame-strings", _strings);
      xml.tag("frame-frets", frets());
      QString strDots = "'";
      QString strMarker = "'";
      QString strFingering = "'";
      for (int i = 0; i < _strings; ++i) {
            // TODO print frame note
            if (_dots) strDots += QString("%1'").arg(static_cast<int>(_dots[i]));
            if (_marker) strMarker += QString("%1'").arg(static_cast<int>(_marker[i]));
            if (_fingering) strFingering += QString("%1'").arg(static_cast<int>(_fingering[i]));
            if (_marker[i] != 88) {
                  xml.stag("frame-note");
                  xml.tag("string", _strings - i);
                  if (_dots)
                        xml.tag("fret", _dots[i]);
                  else
                        xml.tag("fret", "0");
                  xml.etag();
                  }
            }
      qDebug("FretDiagram::writeMusicXML() this %p dots %s marker %s fingering %s",
             this, qPrintable(strDots), qPrintable(strMarker), qPrintable(strFingering));
      /*
      xml.tag("root-step", tpc2stepName(rootTpc));
      int alter = tpc2alter(rootTpc);
      if (alter)
            xml.tag("root-alter", alter);
      */
      xml.etag();
      }

#ifdef SCRIPT_INTERFACE

//---------------------------------------------------------
//   undoSetUserMag
//---------------------------------------------------------

void FretDiagram::undoSetUserMag(qreal val)
      {
      undoChangeProperty(P_ID::MAG, val);
      }

//---------------------------------------------------------
//   undoSetStrings
//---------------------------------------------------------

void FretDiagram::undoSetStrings(int val)
      {
      undoChangeProperty(P_ID::FRET_STRINGS, val);
      }

//---------------------------------------------------------
//   undoSetFrets
//---------------------------------------------------------

void FretDiagram::undoSetFrets(int val)
      {
      undoChangeProperty(P_ID::FRET_FRETS, val);
      }

//---------------------------------------------------------
//   undoSetBarre
//---------------------------------------------------------

void FretDiagram::undoSetBarre(int val)
      {
      undoChangeProperty(P_ID::FRET_BARRE, val);
      }

//---------------------------------------------------------
//   undoSetFretOffset
//---------------------------------------------------------

void FretDiagram::undoSetFretOffset(int val)
      {
      undoChangeProperty(P_ID::FRET_OFFSET, val);
      }

#endif // SCRIPT_INTERFACE

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant FretDiagram::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::MAG:            return userMag();
            case P_ID::FRET_STRINGS:         return strings();
            case P_ID::FRET_FRETS:           return frets();
            case P_ID::FRET_BARRE:           return barre();
            case P_ID::FRET_OFFSET:          return fretOffset();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant FretDiagram::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::MAG:            return 1.0;
            case P_ID::FRET_STRINGS:         return DEFAULT_STRINGS;
            case P_ID::FRET_FRETS:           return DEFAULT_FRETS;
            case P_ID::FRET_BARRE:           return 0;
            case P_ID::FRET_OFFSET:          return 0;
            default:
                  return Element::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool FretDiagram::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::MAG:
                  setUserMag(v.toDouble());
                  break;
            case P_ID::FRET_STRINGS:
                  setStrings(v.toInt());
                  break;
            case P_ID::FRET_FRETS:
                  setFrets(v.toInt());
                  break;
            case P_ID::FRET_BARRE:
                  setBarre(v.toInt());
                  break;
            case P_ID::FRET_OFFSET:
                  setOffset(v.toInt());
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      score()->setLayoutAll();
      return true;
      }

}

