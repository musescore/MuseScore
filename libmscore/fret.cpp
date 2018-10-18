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
//   fretStyle
//---------------------------------------------------------

static const ElementStyle fretStyle {
      { Sid::fretNumPos,                         Pid::FRET_NUM_POS            },
      { Sid::fretMag,                            Pid::MAG                     },
      { Sid::fretPlacement,                      Pid::PLACEMENT               },
      { Sid::fretStrings,                        Pid::FRET_STRINGS            },
      { Sid::fretFrets,                          Pid::FRET_FRETS              },
      { Sid::fretOffset,                         Pid::FRET_OFFSET             },
      { Sid::fretBarre,                          Pid::FRET_BARRE              },
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
            if (c == 'X' || c == 'O')
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
            fd->setFretOffset(offset);
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
      if (parent()->isSegment()) {
            Measure* m = toSegment(parent())->measure();
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
      if (parent()->isSegment()) {
            System* system = toSegment(parent())->measure()->system();
            yp = system->staffCanvasYpage(staffIdx());
            }
      else
            yp = parent()->canvasPos().y();
      QPointF p1(xp, yp);
      return QLineF(p1, canvasPos());
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
            if (_align & Align::BOTTOM)
                  y = th;
            else if (_align & Align::VCENTER)
                  y = (th * .5);
            else if (_align & Align::BASELINE)
                  y = baseLine();
            if (_align & Align::RIGHT)
                  x = tw;
            else if (_align & Align::HCENTER)
                  x = (tw * .5);
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
      qreal _spatium = spatium() * _userMag * score()->styleD(Sid::fretMag);
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
      scaledFont.setPointSizeF(font.pointSize() * _userMag * score()->styleD(Sid::fretMag));
      QFontMetricsF fm(scaledFont, MScore::paintDevice());
      scaledFont.setPointSizeF(scaledFont.pointSizeF() * MScore::pixelRatio);

      painter->setFont(scaledFont);
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
                  if (_dots && _dots[i] == _barre) {
                        string = i;
                        break;
                        }
                  }
            if (string != -1) {
                  qreal x1   = stringDist * string;
                  qreal y    = fretDist * (_barre-1) + fretDist * .5;
                  pen.setWidthF((dotd + lw2 * .5) * score()->styleD(Sid::barreLineWidth));
                  pen.setCapStyle(Qt::RoundCap);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x1, y, x2, y));
                  }
            }
      if (_fretOffset > 0) {
            qreal fretNumMag = score()->styleD(Sid::fretNumMag);
            scaledFont.setPointSizeF(font.pointSize() * fretNumMag * _userMag * score()->styleD(Sid::fretMag) * MScore::pixelRatio);
            painter->setFont(scaledFont);
            if (_numPos == 0) {
                  painter->drawText(QRectF(-stringDist *.4, .0, .0, fretDist),
                     Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
                     QString("%1").arg(_fretOffset+1));
                  }
            else {
                  painter->drawText(QRectF(x2 + (stringDist * 0.4), .0, .0, fretDist),
                     Qt::AlignVCenter|Qt::AlignLeft|Qt::TextDontClip,
                     QString("%1").arg(_fretOffset+1));
                  }
            painter->setFont(font);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
      {
      qreal _spatium  = spatium() * _userMag * score()->styleD(Sid::fretMag);
      lw1             = _spatium * 0.08;
      lw2             = _fretOffset ? lw1 : _spatium * 0.2;
      stringDist      = _spatium * .7;
      fretDist        = _spatium * .8;

      qreal w    = stringDist * (_strings - 1);
      qreal h    = _frets * fretDist + fretDist * .5;
      qreal y    = 0.0;
      qreal dotd = stringDist * .6;
      qreal x    = -((dotd+lw1) * .5);
      w         += dotd + lw1;
      if (_marker) {
            QFont scaledFont(font);
            scaledFont.setPointSize(font.pointSize() * _userMag);
            QFontMetricsF fm(scaledFont, MScore::paintDevice());
            y  = -(fretDist * .1 + fm.height());
            h -= y;
            }

      bbox().setRect(x, y, w, h);

      setPos(-_spatium, -h - styleP(Sid::fretY) + _spatium );

      if (!parent() || !parent()->isSegment()) {
            setPos(QPointF());
            return;
            }
      autoplaceSegmentElement(styleP(Sid::fretMinDistance));
      if (_harmony)
            _harmony->layout();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FretDiagram::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("FretDiagram");
      Element::writeProperties(xml);

      writeProperty(xml, Pid::FRET_STRINGS);
      writeProperty(xml, Pid::FRET_FRETS);
      writeProperty(xml, Pid::FRET_OFFSET);
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
      writeProperty(xml, Pid::FRET_BARRE);
      writeProperty(xml, Pid::MAG);
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
                        const QStringRef& t(e.name());
                        if (t == "dot")
                              setDot(no, e.readInt());
                        else if (t == "marker")
                              setMarker(no, e.readInt());
                        else if (t == "fingering")
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
            setMarker(string, 0);   // TODO: does not work with undo/redo; should be called explicit
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
      if (e->isHarmony()) {
            _harmony = toHarmony(e);
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

bool FretDiagram::acceptDrop(EditData& data) const
      {
      return data.element->type() == ElementType::HARMONY;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* FretDiagram::drop(EditData& data)
      {
      Element* e = data.element;
      if (e->isHarmony()) {
            Harmony* h = toHarmony(e);
            h->setParent(parent());
            h->setTrack(track());
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
//   Write MusicXML
//---------------------------------------------------------

void FretDiagram::writeMusicXML(XmlWriter& xml) const
      {
      qDebug("FretDiagram::writeMusicXML() this %p harmony %p", this, _harmony);
      int strings_ = strings();
      xml.stag("frame");
      xml.tag("frame-strings", strings_);
      xml.tag("frame-frets", frets());
      QString strDots = "'";
      QString strMarker = "'";
      QString strFingering = "'";
      for (int i = 0; i < strings_; ++i) {
            // TODO print frame note
            if (_dots)
                  strDots += QString("%1'").arg(static_cast<int>(_dots[i]));
            if (_marker)
                  strMarker += QString("%1'").arg(static_cast<int>(_marker[i]));
            if (_fingering)
                  strFingering += QString("%1'").arg(static_cast<int>(_fingering[i]));
            if (_marker[i] != 88) {
                  xml.stag("frame-note");
                  xml.tag("string", strings_ - i);
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
            case Pid::FRET_BARRE:
                  return barre();
            case Pid::FRET_OFFSET:
                  return fretOffset();
            case Pid::FRET_NUM_POS:
                  return _numPos;
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
            case Pid::FRET_BARRE:
                  setBarre(v.toInt());
                  break;
            case Pid::FRET_OFFSET:
                  setFretOffset(v.toInt());
                  break;
            case Pid::FRET_NUM_POS:
                  _numPos = v.toInt();
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
      for (const StyledProperty& p : *styledProperties()) {
            if (p.pid == pid) {
                  if (propertyType(pid) == P_TYPE::SP_REAL)
                        return score()->styleP(p.sid);
                  return score()->styleV(p.sid);
                  }
            }
      return Element::propertyDefault(pid);
      }

}

