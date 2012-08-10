//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: element.cpp 5629 2012-05-15 12:38:33Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of Element, ElementList, StaffLines.
*/

#include "element.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "staff.h"
#include "utils.h"
#include "sym.h"
#include "symbol.h"
#include "clef.h"
#include "volta.h"
#include "ottava.h"
#include "textline.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "keysig.h"
#include "timesig.h"
#include "barline.h"
#include "arpeggio.h"
#include "breath.h"
#include "bracket.h"
#include "chordrest.h"
#include "accidental.h"
#include "dynamic.h"
#include "text.h"
#include "note.h"
#include "tremolo.h"
#include "layoutbreak.h"
#include "repeat.h"
#include "page.h"
#include "system.h"
#include "stafftext.h"
#include "glissando.h"
#include "articulation.h"
#include "chord.h"
#include "spacer.h"
#include "tempotext.h"
#include "harmony.h"
#include "lyrics.h"
#include "rest.h"
#include "slur.h"
#include "measure.h"
#include "fret.h"
#include "staffstate.h"
#include "fingering.h"
#include "bend.h"
#include "tremolobar.h"
#include "chordline.h"
#include "undo.h"
#include "segment.h"
#include "box.h"
#include "instrchange.h"
#include "stafftype.h"
#include "stem.h"
#include "iname.h"
#include "mscore.h"
#include "icon.h"
#include "ossia.h"
#include "figuredbass.h"
#include "rehearsalmark.h"
#include "notedot.h"
#include "textframe.h"

extern bool showInvisible;

//
// list has to be synchronized with ElementType enum
//
static const char* elementNames[] = {
      QT_TRANSLATE_NOOP("elementName", "invalid"),
      QT_TRANSLATE_NOOP("elementName", "Symbol"),
      QT_TRANSLATE_NOOP("elementName", "Text"),
      QT_TRANSLATE_NOOP("elementName", "InstrumentName"),
      QT_TRANSLATE_NOOP("elementName", "SlurSegment"),
      QT_TRANSLATE_NOOP("elementName", "StaffLines"),
      QT_TRANSLATE_NOOP("elementName", "BarLine"),
      QT_TRANSLATE_NOOP("elementName", "StemSlash"),
      QT_TRANSLATE_NOOP("elementName", "Line"),
      QT_TRANSLATE_NOOP("elementName", "Bracket"),
      QT_TRANSLATE_NOOP("elementName", "Arpeggio"),
      QT_TRANSLATE_NOOP("elementName", "Accidental"),
      QT_TRANSLATE_NOOP("elementName", "Note"),
      QT_TRANSLATE_NOOP("elementName", "Stem"),             // 10
      QT_TRANSLATE_NOOP("elementName", "Clef"),
      QT_TRANSLATE_NOOP("elementName", "KeySig"),
      QT_TRANSLATE_NOOP("elementName", "TimeSig"),
      QT_TRANSLATE_NOOP("elementName", "Rest"),
      QT_TRANSLATE_NOOP("elementName", "Breath"),
      QT_TRANSLATE_NOOP("elementName", "Glissando"),
      QT_TRANSLATE_NOOP("elementName", "RepeatMeasure"),
      QT_TRANSLATE_NOOP("elementName", "Image"),
      QT_TRANSLATE_NOOP("elementName", "Tie"),
      QT_TRANSLATE_NOOP("elementName", "Articulation"),     // 20
      QT_TRANSLATE_NOOP("elementName", "ChordLine"),
      QT_TRANSLATE_NOOP("elementName", "Dynamic"),
      QT_TRANSLATE_NOOP("elementName", "Beam"),
      QT_TRANSLATE_NOOP("elementName", "Hook"),
      QT_TRANSLATE_NOOP("elementName", "Lyrics"),
      QT_TRANSLATE_NOOP("elementName", "FiguredBass"),
      QT_TRANSLATE_NOOP("elementName", "Marker"),
      QT_TRANSLATE_NOOP("elementName", "Jump"),
      QT_TRANSLATE_NOOP("elementName", "Fingering"),
      QT_TRANSLATE_NOOP("elementName", "Tuplet"),
      QT_TRANSLATE_NOOP("elementName", "Tempo"),
      QT_TRANSLATE_NOOP("elementName", "StaffText"),
      QT_TRANSLATE_NOOP("elementName", "RehearsalMark"),
      QT_TRANSLATE_NOOP("elementName", "InstrumentChange"),
      QT_TRANSLATE_NOOP("elementName", "Harmony"),
      QT_TRANSLATE_NOOP("elementName", "FretDiagram"),
      QT_TRANSLATE_NOOP("elementName", "Bend"),
      QT_TRANSLATE_NOOP("elementName", "TremoloBar"),
      QT_TRANSLATE_NOOP("elementName", "Volta"),
      QT_TRANSLATE_NOOP("elementName", "HairpinSegment"),
      QT_TRANSLATE_NOOP("elementName", "OttavaSegment"),
      QT_TRANSLATE_NOOP("elementName", "TrillSegment"),
      QT_TRANSLATE_NOOP("elementName", "TextLineSegment"),
      QT_TRANSLATE_NOOP("elementName", "VoltaSegment"),
      QT_TRANSLATE_NOOP("elementName", "LayoutBreak"),
      QT_TRANSLATE_NOOP("elementName", "Spacer"),
      QT_TRANSLATE_NOOP("elementName", "StaffState"),
      QT_TRANSLATE_NOOP("elementName", "LedgerLine"),
      QT_TRANSLATE_NOOP("elementName", "NoteHead"),
      QT_TRANSLATE_NOOP("elementName", "NoteDot"),
      QT_TRANSLATE_NOOP("elementName", "Tremolo"),
      QT_TRANSLATE_NOOP("elementName", "Measure"),
      QT_TRANSLATE_NOOP("elementName", "Selection"),
      QT_TRANSLATE_NOOP("elementName", "Lasso"),
      QT_TRANSLATE_NOOP("elementName", "ShadowNote"),
      QT_TRANSLATE_NOOP("elementName", "RubberBand"),
      QT_TRANSLATE_NOOP("elementName", "TabDurationSymbol"),
      QT_TRANSLATE_NOOP("elementName", "FSymbol"),
      QT_TRANSLATE_NOOP("elementName", "Page"),
      QT_TRANSLATE_NOOP("elementName", "HairPin"),
      QT_TRANSLATE_NOOP("elementName", "Ottava"),
      QT_TRANSLATE_NOOP("elementName", "Pedal"),
      QT_TRANSLATE_NOOP("elementName", "Trill"),
      QT_TRANSLATE_NOOP("elementName", "TextLine"),
      QT_TRANSLATE_NOOP("elementName", "Segment"),
      QT_TRANSLATE_NOOP("elementName", "System"),
      QT_TRANSLATE_NOOP("elementName", "Compound"),
      QT_TRANSLATE_NOOP("elementName", "Chord"),
      QT_TRANSLATE_NOOP("elementName", "Slur"),
      QT_TRANSLATE_NOOP("elementName", "Element"),
      QT_TRANSLATE_NOOP("elementName", "ElementList"),
      QT_TRANSLATE_NOOP("elementName", "StaffList"),
      QT_TRANSLATE_NOOP("elementName", "MeasureList"),
      QT_TRANSLATE_NOOP("elementName", "Layout"),
      QT_TRANSLATE_NOOP("elementName", "HBox"),
      QT_TRANSLATE_NOOP("elementName", "VBox"),
      QT_TRANSLATE_NOOP("elementName", "TBox"),
      QT_TRANSLATE_NOOP("elementName", "FBox"),
      QT_TRANSLATE_NOOP("elementName", "AccidentalBracket"),
      QT_TRANSLATE_NOOP("elementName", "Icon"),
      QT_TRANSLATE_NOOP("elementName", "Ossia")
      };

//---------------------------------------------------------
//   DropData
//---------------------------------------------------------

DropData::DropData()
      {
      view = 0;
      element = 0;
      duration = Fraction(1,4);
      modifiers = 0;
      }

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

LinkedElements::LinkedElements(Score* score)
      {
      _lid = score->linkId(); // create new unique id
      }

LinkedElements::LinkedElements(Score* score, int id)
      {
      _lid = id;
      score->linkId(id);
      }

//---------------------------------------------------------
//   setLid
//---------------------------------------------------------

void LinkedElements::setLid(Score* score, int id)
      {
      _lid = id;
      score->linkId(id);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Element::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userOff *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Element::spatium() const
      {
      Staff* s = staff();
      qreal v = _score->spatium();
      return s ? v * s->mag() : v;
      }

//---------------------------------------------------------
//   magS
//---------------------------------------------------------

qreal Element::magS() const
      {
      return _mag * (_score->spatium() /(MScore::DPI * SPATIUM20));
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* Element::name() const
      {
      return name(type());
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString Element::userName() const
      {
      return qApp->translate("elementName", name(type()));
      }

//---------------------------------------------------------
//   ~Element
//---------------------------------------------------------

Element::~Element()
      {
      if (_links) {
            _links->removeOne(this);
            if (_links->isEmpty()) {
                  //DEBUG:
                  score()->links().remove(_links->lid());
                  //
                  delete _links;
                  }
            }
      if (score()) {
            foreach(Element* e, score()->selection().elements()) {
                  if (e == this) {
//                        if (MScore::debugMode)
                              qDebug("======~Element: %p still in selection! generated %d\n",
                                 this, generated());
//                        if (MScore::debugMode)
//                              abort();
                        score()->deselect(this);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

Element::Element(Score* s) :
   QObject(0),
   _links(0),
   _parent(0),
   _selected(false),
   _generated(false),
   _visible(true),
   _flags(ELEMENT_SELECTABLE),
   _track(-1),
   _color(MScore::defaultColor),
   _mag(1.0),
   _tag(1),
   _score(s),
   _mxmlOff(0),
   itemDiscovered(0)
      {
      }

Element::Element(const Element& e)
   : QObject(0)
      {
      _links      = 0;
      _parent     = e._parent;
      _selected   = e._selected;
      _generated  = e._generated;
      _visible    = e._visible;
      _flags      = e._flags;
      _track      = e._track;
      _color      = e._color;
      _mag        = e._mag;
      _pos        = e._pos;
      _userOff    = e._userOff;
      _readPos    = e._readPos;
      _score      = e._score;
      _mxmlOff    = e._mxmlOff;
      _bbox       = e._bbox;
      _tag        = e._tag;
      itemDiscovered = 0;
      }

//---------------------------------------------------------
//   linkTo
//---------------------------------------------------------

void Element::linkTo(Element* element)
      {
      Q_ASSERT(!_links || !element->links() | (_links == element->links()));
      if (!_links) {
            if (element->links()) {
                  _links = element->links();
                  Q_ASSERT(_links->contains(element));
                  }
            else {
                  _links = new LinkedElements(score());
                  _links->append(element);
                  element->setLinks(_links);
                  }
            _links->append(this);
            }
      else {
            Q_ASSERT(_links->contains(this));
            _links->append(element);
            element->setLinks(_links);
            }
      }

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

Element* Element::linkedClone()
      {
      Element* e = clone();
      linkTo(e);
      return e;
      }

//---------------------------------------------------------
//   linkList
//---------------------------------------------------------

QList<Element*> Element::linkList() const
      {
      QList<Element*> el;
      if (links())
            el.append(*links());
      else
            el.append((Element*)this);
      return el;
      }

//---------------------------------------------------------
//   adjustReadPos
//---------------------------------------------------------

void Element::adjustReadPos()
      {
      if (!_readPos.isNull()) {
            _userOff = _readPos - _pos;
            _readPos = QPointF();
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Element::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (all || _visible || score()->showInvisible())
            func(data, this);
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Element::toDefault()
      {
      if (!_userOff.isNull())
            score()->undoChangeUserOffset(this, QPointF());
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void Element::change(Element* o, Element* n)
      {
      remove(o);
      add(n);
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Element::staff() const
      {
      if (_track == -1)
            return 0;
      return score()->staff(staffIdx());
      }

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

QColor Element::curColor() const
      {
      // the default element color is always interpreted as black in
      // printing
      if (score() && score()->printing())
            return (_color == MScore::defaultColor) ? Qt::black : _color;

      if (flag(ELEMENT_DROP_TARGET))
            return MScore::dropColor;
      if (_selected) {
            if (track() == -1)
                  return MScore::selectColor[0];
            else
                  return MScore::selectColor[voice()];
            }
      if (!_visible)
            return Qt::gray;
      return _color;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

/**
 Return update Rect relative to canvas.
*/

QRectF Element::drag(const EditData& data)
      {
      QRectF r(canvasBoundingRect());

      qreal x = data.pos.x();
      qreal y = data.pos.y();

      qreal _spatium = spatium();
      if (data.hRaster) {
            qreal hRaster = _spatium / MScore::hRaster();
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (data.vRaster) {
            qreal vRaster = _spatium / MScore::vRaster();
            int n = lrint(y / vRaster);
            y = vRaster * n;
            }
      setUserOff(QPointF(x, y));
      return canvasBoundingRect() | r;
      }

//---------------------------------------------------------
//   pagePos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF Element::pagePos() const
      {
      QPointF p(pos());
      if (parent() == 0)
            return p;

      if (_flags & ELEMENT_ON_STAFF) {
            System* system = static_cast<Segment*>(parent())->measure()->system();
            if (system) {
                  int si = staffIdx();
                  if (type() == CHORD || type() == REST)
                        si += static_cast<const ChordRest*>(this)->staffMove();
                  p.ry() += system->staff(si)->y() + system->y();
                  }
            p.rx() = pageX();
            }
      else {
            if (parent() && parent()->parent())
                  p += parent()->pagePos();
            }
      return p;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Element::canvasPos() const
      {
      QPointF p(pos());
      if (parent() == 0)
            return p;
      if (_flags & ELEMENT_ON_STAFF) {
            System* system = static_cast<Segment*>(parent())->measure()->system();
            if (system) {
                  int si = staffIdx();
                  if (type() == CHORD || type() == REST)
                        si += static_cast<const ChordRest*>(this)->staffMove();
                  p.ry() += system->staff(si)->y() + system->y();
                  Page* page = system->page();
                  if (page)
                        p.ry() += page->y();
                  }
            p.rx() = canvasX();
            }
      else {
            if (parent())
                  p += parent()->canvasPos();
            }
      return p;
      }

//---------------------------------------------------------
//   pageX
//---------------------------------------------------------

qreal Element::pageX() const
      {
      qreal xp = x();
      for (Element* e = parent(); e && e->parent(); e = e->parent())
            xp += e->x();
      return xp;
      }

//---------------------------------------------------------
//    canvasX
//---------------------------------------------------------

qreal Element::canvasX() const
      {
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      return xp;
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

/**
 Return true if \a p is inside the shape of the object.

 Note: \a p is in page coordinates
*/

bool Element::contains(const QPointF& p) const
      {
      return shape().contains(p - pagePos());
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

/**
  Returns the shape of this element as a QPainterPath in local
  coordinates. The shape is used for collision detection and
  hit tests (contains())

  The default implementation calls bbox() to return a simple rectangular
  shape, but subclasses can reimplement this function to return a more
  accurate shape for non-rectangular elements.
*/

QPainterPath Element::shape() const
      {
      QPainterPath pp;
      pp.addRect(bbox());
      return pp;
      }

//---------------------------------------------------------
//  intersects
//---------------------------------------------------------

/**
 Return true if \a rr intersects bounding box of object.

 Note: \a rr is in page coordinates
*/

bool Element::intersects(const QRectF& rr) const
      {
      return shape().intersects(rr.translated(-pagePos()));
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Element::writeProperties(Xml& xml) const
      {
      if (_links && (_links->size() > 1))
            xml.tag("lid", _links->lid());
      if (!userOff().isNull()) {
            if (type() == VOLTA_SEGMENT)
                  xml.tag("offset", userOff() / spatium());
            else
                  xml.tag("pos", pos() / spatium());
            }
      if ((track() != xml.curTrack) && (track() != -1)) {
            int t;
            t = track() + xml.trackDiff;
            xml.tag("track", t);
            }
      if (_tag != 0x1) {
            for (int i = 1; i < MAX_TAGS; i++) {
                  if (_tag == ((unsigned)1 << i)) {
                        xml.tag("tag", score()->layerTags()[i]);
                        break;
                        }
                  }
            }
      if (color() != Qt::black)
            xml.tag("color", color());
      if (!visible())
            xml.tag("visible", visible());
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(const QDomElement& e)
      {
      const QString& tag(e.tagName());
      const QString& val(e.text());

      if (tag == "color")
            _color = readColor(e);
      else if (tag == "visible")
            _visible = val.toInt();
      else if (tag == "selected")
            _selected = val.toInt();
      else if (tag == "userOff")
            _userOff = readPoint(e);
      else if (tag == "lid") {
            _links = score()->links().value(val.toInt());
            if (!_links) {
                  int i = val.toInt();
                  if (score()->parentScore())   // DEBUG
                        qDebug("---link %d not found (%d)\n", i, score()->links().size());
                  _links = new LinkedElements(score(), i);
                  score()->links().insert(i, _links);
                  }
            _links->append(this);
            }
      else if (tag == "tick")
            score()->curTick = score()->fileDivision(val.toInt());
      else if (tag == "offset") {         // ??obsolete -> used for volta
            qreal _spatium = spatium();
            QPointF pt(readPoint(e) * _spatium);
            setUserOff(pt);
            _readPos = QPointF();
            }
      else if (tag == "pos") {
            qreal _spatium = spatium();
            setUserOff(QPointF());
            _readPos = readPoint(e) * _spatium;
            }
      else if (tag == "voice")
            setTrack((_track/VOICES)*VOICES + val.toInt());
      else if (tag == "track")
            setTrack(val.toInt());
/*      else if (tag == "systemFlag") {
            int i = val.toInt();
            setFlag(ELEMENT_SYSTEM_FLAG, i);
            if (i)
                  _track = 0;
            }
*/
      else if (tag == "tag") {
            for (int i = 1; i < MAX_TAGS; i++) {
                  if (score()->layerTags()[i] == val) {
                        _tag = 1 << i;
                        break;
                        }
                  }
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Element::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Element::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

/**
 Remove \a el from the list. Return true on success.
*/

bool ElementList::remove(Element* el)
      {
      int idx = indexOf(el);
      if (idx == -1)
            return false;
      removeAt(idx);
      return true;
      }

//---------------------------------------------------------
//   replace
//---------------------------------------------------------

void ElementList::replace(Element* o, Element* n)
      {
      int idx = indexOf(o);
      if (idx == -1) {
            qDebug("ElementList::replace: element not found\n");
            return;
            }
      QList<Element*>::replace(idx, n);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ElementList::write(Xml& xml) const
      {
      for (ciElement ie = begin(); ie != end(); ++ie)
            (*ie)->write(xml);
      }


//---------------------------------------------------------
//   StaffLines
//---------------------------------------------------------

StaffLines::StaffLines(Score* s)
   : Element(s)
      {
      setWidth(1.0);      // dummy
      lines = 5;
      setSelectable(false);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF StaffLines::pagePos() const
      {
      System* system = measure()->system();
      return QPointF(measure()->x() + system->x(),
         system->staff(staffIdx())->y() + system->y());
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffLines::layout()
      {
      StaffType* st = staff() ? staff()->staffType() : 0;
      qreal _spatium = spatium();
      if (st) {
            dist  = st->lineDistance().val() * _spatium;
            lines = st->lines();
            }
      else {
            dist  = _spatium;
            lines = 5;
            }

//      qDebug("StaffLines::layout:: dist %f st %p\n", dist, st);

      lw = score()->styleS(ST_staffLineWidth).val() * _spatium;

      qreal w = width();
      switch (lines) {
            case 0:
                  setbbox(QRectF(0.0, - 2.0 * dist - lw*.5, w, 4 * dist + lw));
                  break;
            case 1:
                  setbbox(QRectF(0.0,  -lw*.5, w, lw));
                  break;
            case 2:
                  setbbox(QRectF(0.0, -lw*.5, w, lines * dist + lw));
                  break;
            default:
                  setbbox(QRectF(0.0, -lw*.5, w, lines * dist + lw));
                  break;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffLines::draw(QPainter* painter) const
      {
      QPointF _pos(0.0, 0.0);

      qreal x1 = _pos.x();
      qreal x2 = x1 + width();

      QVector<QLineF> ll(lines);
      qreal y = _pos.y();
      for (int i = 0; i < lines; ++i) {
            ll[i].setLine(x1, y, x2, y);
            y += dist;
            }
      if (MScore::debugMode) {
            painter->setPen(QPen(Qt::lightGray, lw, Qt::SolidLine, Qt::FlatCap));
            y = _pos.y() - 3 * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() - 2 * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() - dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + lines * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + (lines+1) * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + (lines+2) * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            }
      painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
      painter->drawLines(ll);
      }

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

qreal StaffLines::y1() const
      {
     System* system = measure()->system();
     if (system == 0)
           return 0.0;

      qreal y = system->staff(staffIdx())->y() + ipos().y();

      switch (lines) {
            case 1:
                  return y - spatium();
            default:
                  return y;
            }
      }

//---------------------------------------------------------
//   y2
//---------------------------------------------------------

qreal StaffLines::y2() const
      {
      System* system = measure()->system();
      if (system == 0)
            return 0.0;

      qreal y = system->staff(staffIdx())->y() + ipos().y();

      switch (lines) {
            case 1:
                  return y + spatium();
            default:
                  return y + (lines - 1) * dist;
            }
      }

//---------------------------------------------------------
//   Line
//---------------------------------------------------------

Line::Line(Score* s, bool v)
   : Element(s)
      {
      vertical = v;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Line::dump() const
      {
      qDebug("  width:%g height:%g vert:%d\n", point(_width), point(_len), vertical);
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Line::setLen(Spatium l)
      {
      _len = l;
      }

//---------------------------------------------------------
//   setLineWidth
//---------------------------------------------------------

void Line::setLineWidth(Spatium w)
      {
      _width = w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Line::layout()
      {
      qreal sp = spatium();
      qreal w  = _width.val() * sp;
      qreal l  = _len.val() * sp;
      qreal w2 = w * .5;
      if (vertical)
            setbbox(QRectF(-w2, -w2, w, l + w));
      else
            setbbox(QRectF(-w2, -w2, l + w, w));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Line::draw(QPainter* painter) const
      {
      qreal sp = spatium();
      painter->setPen(QPen(curColor(), _width.val() * sp));

      qreal l = _len.val() * sp;
      if (vertical)
            painter->drawLine(QLineF(0.0, 0.0, 0.0, l));
      else
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Line::writeProperties(Xml& xml) const
      {
      xml.tag("lineWidth", _width.val());
      xml.tag("lineLen", _len.val());
      if (!vertical)
            xml.tag("vertical", vertical);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Line::readProperties(const QDomElement& e)
      {
      const QString& tag(e.tagName());
      const QString& val(e.text());

      if (tag == "lineWidth")
            _width = Spatium(val.toDouble());
      else if (tag == "lineLen")
            _len = Spatium(val.toDouble());
      else if (tag == "vertical")
            vertical = val.toInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   Compound
//---------------------------------------------------------

Compound::Compound(Score* s)
   : Element(s)
      {
      }

Compound::Compound(const Compound& c)
   : Element(c)
      {
      elemente.clear();
      foreach(Element* e, c.elemente)
            elemente.append(e->clone());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Compound::draw(QPainter* painter) const
      {
      foreach(Element* e, elemente) {
            QPointF pt(e->pos());
            painter->translate(pt);
            e->draw(painter);
            painter->translate(-pt);
            }
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 offset \a x and \a y are in Point units
*/

void Compound::addElement(Element* e, qreal x, qreal y)
      {
      e->setPos(x, y);
      e->setParent(this);
      elemente.push_back(e);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Compound::layout()
      {
      setbbox(QRectF());
      for (iElement i = elemente.begin(); i != elemente.end(); ++i) {
            Element* e = *i;
            e->layout();
            addbbox(e->bbox().translated(e->pos()));
            }
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Compound::setSelected(bool f)
      {
      Element::setSelected(f);
      for (ciElement i = elemente.begin(); i != elemente.end(); ++i)
            (*i)->setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Compound::setVisible(bool f)
      {
      Element::setVisible(f);
      for (ciElement i = elemente.begin(); i != elemente.end(); ++i)
            (*i)->setVisible(f);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Compound::clear()
      {
      foreach(Element* e, elemente) {
            if (e->selected())
                  score()->deselect(e);
            delete e;
            }
      elemente.clear();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Element::dump() const
      {
      qDebug("---Element: %s, pos(%4.2f,%4.2f)\n"
         "   bbox(%g,%g,%g,%g)\n"
         "   abox(%g,%g,%g,%g)\n"
         "  parent: %p\n",
         name(), ipos().x(), ipos().y(),
         _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
         abbox().x(), abbox().y(), abbox().width(), abbox().height(),
         parent());
      }

//---------------------------------------------------------
//   RubberBand::draw
//---------------------------------------------------------

void RubberBand::draw(QPainter* painter) const
      {
      if (!showRubberBand)
            return;
      painter->setPen(Qt::red);
      painter->drawLine(QLineF(_p1.x(), _p1.y(), _p2.x(), _p2.y()));
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Element::mimeData(const QPointF& dragOffset) const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.clipboardmode = true;
      xml.stag("Element");
      if (type() == NOTE)
            xml.fTag("duration", static_cast<const Note*>(this)->chord()->duration());
      if (!dragOffset.isNull())
            xml.tag("dragOffset", dragOffset);
      write(xml);
      xml.etag();
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   readType
//    return new position of QDomElement in e
//---------------------------------------------------------

ElementType Element::readType(QDomElement& e, QPointF* dragOffset, Fraction* duration)
      {
      ElementType type = INVALID;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "dragOffset")
                  *dragOffset = readPoint(e);
            else if (e.tagName() == "duration")
                  *duration = readFraction(e);
            else if ((type = name2type(e.tagName())) == INVALID) {
                  domError(e);
                  break;
                  }
            if (type != INVALID)
                  break;
            }
      return type;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Element::editDrag(const EditData& ed)
      {
      score()->addRefresh(abbox());
      setUserOff(userOff() + ed.delta);
      score()->addRefresh(abbox());
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Element::edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers, const QString&)
      {
      if (key ==  Qt::Key_Home) {
            setUserOff(QPoint());
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Element::add(Element* e)
      {
      qDebug("Element: cannot add %s to %s\n", e->name(), name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Element::remove(Element* e)
      {
      qDebug("Element: cannot remove %s from %s\n", e->name(), name());
      abort();
      }

//---------------------------------------------------------
//   create
//    Element factory
//---------------------------------------------------------

Element* Element::create(ElementType type, Score* score)
      {
      switch(type) {
            case VOLTA:             return new Volta(score);
            case OTTAVA:            return new Ottava(score);
            case TEXTLINE:          return new TextLine(score);
            case TRILL:             return new Trill(score);
            case PEDAL:             return new Pedal(score);
            case HAIRPIN:           return new Hairpin(score);
            case CLEF:              return new Clef(score);
            case KEYSIG:            return new KeySig(score);
            case TIMESIG:           return new TimeSig(score);
            case BAR_LINE:          return new BarLine(score);
            case ARPEGGIO:          return new Arpeggio(score);
            case BREATH:            return new Breath(score);
            case GLISSANDO:         return new Glissando(score);
            case BRACKET:           return new Bracket(score);
            case ARTICULATION:      return new Articulation(score);
            case CHORDLINE:         return new ChordLine(score);
            case ACCIDENTAL:        return new Accidental(score);
            case DYNAMIC:           return new Dynamic(score);
            case TEXT:              return new Text(score);
            case INSTRUMENT_NAME:   return new InstrumentName(score);
            case STAFF_TEXT:        return new StaffText(score);
            case REHEARSAL_MARK:    return new RehearsalMark(score);
            case INSTRUMENT_CHANGE: return new InstrumentChange(score);
            case NOTEHEAD:          return new NoteHead(score);
            case NOTEDOT:           return new NoteDot(score);
            case TREMOLO:           return new Tremolo(score);
            case LAYOUT_BREAK:      return new LayoutBreak(score);
            case MARKER:            return new Marker(score);
            case JUMP:              return new Jump(score);
            case REPEAT_MEASURE:    return new RepeatMeasure(score);
            case ICON:              return new Icon(score);
            case NOTE:              return new Note(score);
            case SYMBOL:            return new Symbol(score);
            case FSYMBOL:           return new FSymbol(score);
            case CHORD:             return new Chord(score);
            case REST:              return new Rest(score);
            case SPACER:            return new Spacer(score);
            case STAFF_STATE:       return new StaffState(score);
            case TEMPO_TEXT:        return new TempoText(score);
            case HARMONY:           return new Harmony(score);
            case FRET_DIAGRAM:      return new FretDiagram(score);
            case BEND:              return new Bend(score);
            case TREMOLOBAR:        return new TremoloBar(score);
            case LYRICS:            return new Lyrics(score);
            case FIGURED_BASS:      return new FiguredBass(score);
            case STEM:              return new Stem(score);
            case SLUR:              return new Slur(score);
            case ACCIDENTAL_BRACKET: return new AccidentalBracket(score);
            case FINGERING:          return new Fingering(score);
            case HBOX:              return new HBox(score);
            case VBOX:              return new VBox(score);
            case TBOX:              return new TBox(score);
            case FBOX:              return new FBox(score);
            case MEASURE:           return new Measure(score);
            case TAB_DURATION_SYMBOL: return new TabDurationSymbol(score);
            case OSSIA:               return new Ossia(score);

            case TEXTLINE_SEGMENT:    // return new TextLineSegment(score);

            case SLUR_SEGMENT:
            case STEM_SLASH:
            case LINE:
            case IMAGE:
            case TIE:
            case PAGE:
            case BEAM:
            case HOOK:
            case TUPLET:
            case HAIRPIN_SEGMENT:
            case OTTAVA_SEGMENT:
            case TRILL_SEGMENT:
            case VOLTA_SEGMENT:
            case LEDGER_LINE:
            case STAFF_LINES:
            case SELECTION:
            case LASSO:
            case SHADOW_NOTE:
            case RUBBERBAND:
            case SEGMENT:
            case SYSTEM:
            case COMPOUND:
            case ELEMENT:
            case ELEMENT_LIST:
            case STAFF_LIST:
            case MEASURE_LIST:
            case LAYOUT:
            case MAXTYPE:
            case INVALID:  break;
            }
      qDebug("cannot create type <%s>\n", Element::name(type));
      return 0;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* Element::name(ElementType type)
      {
      switch(type) {
            case SYMBOL:            return "Symbol";
            case FSYMBOL:           return "FSymbol";
            case TEXT:              return "Text";
            case INSTRUMENT_NAME:   return "InstrumentName";
            case SLUR_SEGMENT:      return "SlurSegment";
            case BAR_LINE:          return "BarLine";
            case STEM_SLASH:        return "StemSlash";
            case LINE:              return "Line";
            case BRACKET:           return "Bracket";
            case ARPEGGIO:          return "Arpeggio";
            case ACCIDENTAL:        return "Accidental";
            case NOTE:              return "Note";
            case STEM:              return "Stem";
            case CLEF:              return "Clef";
            case KEYSIG:            return "KeySig";
            case TIMESIG:           return "TimeSig";
            case REST:              return "Rest";
            case BREATH:            return "Breath";
            case GLISSANDO:         return "Glissando";
            case REPEAT_MEASURE:    return "RepeatMeasure";
            case IMAGE:             return "Image";
            case TIE:               return "Tie";
            case ARTICULATION:      return "Articulation";
            case CHORDLINE:         return "ChordLine";
            case DYNAMIC:           return "Dynamic";
            case PAGE:              return "Page";
            case BEAM:              return "Beam";
            case HOOK:              return "Hook";
            case LYRICS:            return "Lyrics";
            case FIGURED_BASS:      return "FiguredBass";
            case MARKER:            return "Marker";
            case JUMP:              return "Jump";
            case FINGERING:         return "Fingering";
            case TUPLET:            return "Tuplet";
            case TEMPO_TEXT:        return "Tempo";
            case STAFF_TEXT:        return "StaffText";
            case REHEARSAL_MARK:    return "RehearsalMark";
            case INSTRUMENT_CHANGE: return "InstrumentChange";
            case HARMONY:           return "Harmony";
            case FRET_DIAGRAM:      return "FretDiagram";
            case BEND:              return "Bend";
            case TREMOLOBAR:        return "TremoloBar";
            case VOLTA:             return "Volta";
            case HAIRPIN_SEGMENT:   return "HairpinSegment";
            case OTTAVA_SEGMENT:    return "OttavaSegment";
            case TRILL_SEGMENT:     return "TrillSegment";
            case TEXTLINE_SEGMENT:  return "TextLineSegment";
            case VOLTA_SEGMENT:     return "VoltaSegment";
            case LAYOUT_BREAK:      return "LayoutBreak";
            case SPACER:            return "Spacer";
            case STAFF_STATE:       return "StaffState";
            case LEDGER_LINE:       return "LedgerLine";
            case NOTEHEAD:          return "NoteHead";
            case NOTEDOT:           return "NoteDot";
            case TREMOLO:           return "Tremolo";
            case MEASURE:           return "Measure";
            case STAFF_LINES:       return "StaffLines";
            case SELECTION:         return "Selection";
            case LASSO:             return "Lasso";
            case SHADOW_NOTE:       return "ShadowNote";
            case RUBBERBAND:        return "RubberBand";
            case HAIRPIN:           return "HairPin";
            case OTTAVA:            return "Ottava";
            case PEDAL:             return "Pedal";
            case TRILL:             return "Trill";
            case TEXTLINE:          return "TextLine";
            case SEGMENT:           return "Segment";
            case SYSTEM:            return "System";
            case COMPOUND:          return "Compound";
            case CHORD:             return "Chord";
            case SLUR:              return "Slur";
            case ELEMENT:           return "Element";
            case ELEMENT_LIST:      return "ElementList";
            case STAFF_LIST:        return "StaffList";
            case MEASURE_LIST:      return "MeasureList";
            case LAYOUT:            return "Layout";
            case HBOX:              return "HBox";
            case VBOX:              return "VBox";
            case TBOX:              return "TBox";
            case FBOX:              return "FBox";
            case ICON:              return "Icon";
            case OSSIA:             return "Ossia";
            case ACCIDENTAL_BRACKET:  return "AccidentalBracket";
            case TAB_DURATION_SYMBOL: return "TabDurationSymbol";
            case INVALID:
            case MAXTYPE:
                  break;
            }
      return "??";
      }

//---------------------------------------------------------
//   name2type
//---------------------------------------------------------

ElementType Element::name2type(const QString& s)
      {
      for (int i = 0; i < MAXTYPE; ++i) {
            if (s == elementNames[i])
                  return ElementType(i);
            }
qDebug("name2type: invalid type <%s>\n", qPrintable(s));
      return INVALID;
      }

//---------------------------------------------------------
//   name2Element
//---------------------------------------------------------

Element* Element::name2Element(const QString& s, Score* sc)
      {
      ElementType type = Element::name2type(s);
      if (type == INVALID)
            return 0;
      return Element::create(type, sc);
      }

//---------------------------------------------------------
//   elementLessThan
//---------------------------------------------------------

bool elementLessThan(const Element* const e1, const Element* const e2)
      {
      return e1->z() <= e2->z();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF Element::getGrip(int) const
      {
      qreal _spatium = score()->spatium();
      return QPointF(userOff().x() / _spatium, userOff().y() / _spatium);
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void Element::setGrip(int, const QPointF& pt)
      {
      qreal _spatium = score()->spatium();
      setUserOff(QPointF(pt.x() * _spatium, pt.y() * _spatium));
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void collectElements(void* data, Element* e)
      {
      QList<Element*>* el = static_cast<QList<Element*>*>(data);
      el->append(e);
      }

//---------------------------------------------------------
//   Space::operator+=
//---------------------------------------------------------

Space& Space::operator+=(const Space& s)
      {
      _lw += s._lw;
      _rw += s._rw;
      return *this;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Element::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_COLOR:     return _color;
            case P_VISIBLE:   return _visible;
            case P_SELECTED:  return _selected;
            case P_USER_OFF:  return _userOff;
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Element::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_COLOR:
                  _color = v.value<QColor>();
                  break;
            case P_VISIBLE:
                  _visible = v.toBool();
                  break;
            case P_SELECTED:
                  _selected = v.toBool();
                  break;
            case P_USER_OFF:
                  _userOff = v.toPointF();
                  break;
            default:
                  qDebug("Element::setProperty: unknown id %d, data <%s>", propertyId, qPrintable(v.toString()));
                  return false;
            }
      setGenerated(false);
      score()->addRefresh(canvasBoundingRect());
      qDebug("Element::setProperty: unknown id %d, data <%s>", propertyId, qPrintable(v.toString()));
      return true;
      }

//---------------------------------------------------------
//   isChordRest
//---------------------------------------------------------

bool Element::isChordRest() const
      {
      return type() == REST || type() == CHORD;
      }

//---------------------------------------------------------
//   isDurationElement
//---------------------------------------------------------

bool Element::isDurationElement() const
      {
      return isChordRest() || (type() == TUPLET);
      }

//---------------------------------------------------------
//   isSLine
//---------------------------------------------------------

bool Element::isSLine() const
      {
      return type() == HAIRPIN || type() == OTTAVA || type() == PEDAL
         || type() == TRILL || type() == VOLTA || type() == TEXTLINE;
      }

//---------------------------------------------------------
//   isText
//---------------------------------------------------------

bool Element::isText() const
      {
      return type()  == TEXT
         || type() == LYRICS
         || type() == DYNAMIC
         || type() == FINGERING
         || type() == HARMONY
         || type() == MARKER
         || type() == JUMP
         || type() == STAFF_TEXT
         || type() == REHEARSAL_MARK
         || type() == INSTRUMENT_CHANGE
         || type() == FIGURED_BASS
         || type() == TEMPO_TEXT;
      }

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void Element::undoSetColor(const QColor& c)
      {
      score()->undoChangeProperty(this, P_COLOR, c);
      }

