//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
 Implementation of Element, ElementList
*/

#include "element.h"
#include "accidental.h"
#include "ambitus.h"
#include "arpeggio.h"
#include "articulation.h"
#include "bagpembell.h"
#include "barline.h"
#include "bend.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "chordline.h"
#include "chordrest.h"
#include "clef.h"
#include "dynamic.h"
#include "figuredbass.h"
#include "fingering.h"
#include "fret.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "icon.h"
#include "image.h"
#include "iname.h"
#include "instrchange.h"
#include "jump.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "mscore.h"
#include "notedot.h"
#include "note.h"
#include "noteline.h"
#include "ossia.h"
#include "ottava.h"
#include "page.h"
#include "pedal.h"
#include "rehearsalmark.h"
#include "repeat.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "slur.h"
#include "spacer.h"
#include "staff.h"
#include "staffstate.h"
#include "stafftext.h"
#include "systemtext.h"
#include "stafftype.h"
#include "stem.h"
#include "style.h"
#include "symbol.h"
#include "sym.h"
#include "system.h"
#include "tempotext.h"
#include "textframe.h"
#include "text.h"
#include "textline.h"
#include "timesig.h"
#include "tremolobar.h"
#include "tremolo.h"
#include "trill.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"
#include "xml.h"
#include "systemdivider.h"
#include "stafftypechange.h"
#include "stafflines.h"

namespace Ms {

// extern bool showInvisible;

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
//   spatiumChanged
//---------------------------------------------------------

void Element::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userOff *= (newValue / oldValue);
      _readPos *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Element::localSpatiumChanged(qreal oldValue, qreal newValue)
      {
      _userOff *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Element::spatium() const
      {
      Staff* s = staff();
      return s ? s->spatium(tick()) : score()->spatium();
      }

//---------------------------------------------------------
//   magS
//---------------------------------------------------------

qreal Element::magS() const
      {
      return mag() * (score()->spatium() / SPATIUM20);
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString Element::subtypeName() const
      {
      return "";
      }

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

Element::Element(Score* s) :
   ScoreElement(s)
      {
      _placement     = Placement::BELOW;
      _track         = -1;
      _color         = MScore::defaultColor;
      _mag           = 1.0;
      _tag           = 1;
      itemDiscovered = false;
      _z             = -1;
      }

Element::Element(const Element& e)
   : ScoreElement(e)
      {
      _parent     = e._parent;
      _z          = e._z;
      _placement  = e._placement;
      _flags      = e._flags;
      _track      = e._track;
      _color      = e._color;
      _mag        = e._mag;
      _pos        = e._pos;
      _userOff    = e._userOff;
      _readPos    = e._readPos;
      _bbox       = e._bbox;
      _tag        = e._tag;
      itemDiscovered = false;
      }

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

Element* Element::linkedClone()
      {
      Element* e = clone();
      score()->undo(new Link(e, this));
      return e;
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
      if (all || visible() || score()->showInvisible())
            func(data, this);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Element::reset()
      {
      undoResetProperty(P_ID::AUTOPLACE);
      undoResetProperty(P_ID::PLACEMENT);
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
      if (_track == -1 || score()->staves().empty())
            return 0;

      return score()->staff(_track >> 2);
      }

//---------------------------------------------------------
//   z
//---------------------------------------------------------

int Element::z() const
      {
      if (_z == -1)
            _z = int(type()) * 100;
      return _z;
      }

//---------------------------------------------------------
//   part
//---------------------------------------------------------

Part* Element::part() const
      {
      Staff* s = staff();
      return s ? s->part() : 0;
      }

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

QColor Element::curColor() const
      {
      return curColor(this);
      }

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

QColor Element::curColor(const Element* proxy) const
      {
      // the default element color is always interpreted as black in
      // printing
      if (score() && score()->printing())
            return (proxy->color() == MScore::defaultColor) ? Qt::black : proxy->color();

      if (flag(ElementFlag::DROP_TARGET))
            return MScore::dropColor;
      bool marked = false;
      if (isNote()) {
            const Note* note = static_cast<const Note*>(this);
            marked = note->mark();
            }
      if (proxy->selected() || marked ) {
            if (track() == -1)
                  return MScore::selectColor[0];
            else
                  return MScore::selectColor[voice()];
            }
      if (!proxy->visible())
            return Qt::gray;
      return proxy->color();
      }

//---------------------------------------------------------
//   drag
///   Return update Rect relative to canvas.
//---------------------------------------------------------

QRectF Element::drag(EditData* data)
      {
      QRectF r(canvasBoundingRect());

      qreal x = data->delta.x();
      qreal y = data->delta.y();

      qreal _spatium = spatium();
      if (data->hRaster) {
            qreal hRaster = _spatium / MScore::hRaster();
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (data->vRaster) {
            qreal vRaster = _spatium / MScore::vRaster();
            int n = lrint(y / vRaster);
            y = vRaster * n;
            }

      setUserOff(QPointF(x, y));
      setGenerated(false);

      if (type() == ElementType::TEXT) {         // TODO: check for other types
            //
            // restrict move to page boundaries
            //
            QRectF r(canvasBoundingRect());
            Page* p = 0;
            Element* e = this;
            while (e) {
                  if (e->type() == ElementType::PAGE) {
                        p = static_cast<Page*>(e);
                        break;
                        }
                  e = e->parent();
                  }
            if (p) {
                  bool move = false;
                  QRectF pr(p->canvasBoundingRect());
                  if (r.right() > pr.right()) {
                        x -= r.right() - pr.right();
                        move = true;
                        }
                  else if (r.left() < pr.left()) {
                        x += pr.left() - r.left();
                        move = true;
                        }
                  if (r.bottom() > pr.bottom()) {
                        y -= r.bottom() - pr.bottom();
                        move = true;
                        }
                  else if (r.top() < pr.top()) {
                        y += pr.top() - r.top();
                        move = true;
                        }
                  if (move)
                        setUserOff(QPointF(x, y));
                  }
            }
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

      if (_flags & ElementFlag::ON_STAFF) {
            System* system = 0;
            Measure* measure = 0;
            if (parent()->isSegment())
                  measure = toSegment(parent())->measure();
            else if (parent()->isMeasure())           // used in measure number
                  measure = toMeasure(parent());
            else if (parent()->isSystem())
                  system = toSystem(parent());
            else
                  qFatal("this %s parent %s\n", name(), parent()->name());
            if (measure) {
                  system = measure->system();
                  p.ry() += measure->staffLines(vStaffIdx())->y();
                  }
            if (system)
                  p.ry() += system->staffYpage(vStaffIdx());
            p.rx() = pageX();
            }
      else {
            if (parent()->parent())
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

      if (_flags & ElementFlag::ON_STAFF) {
            System* system = 0;
            Measure* measure = 0;
            if (parent()->isSegment())
                  measure = toSegment(parent())->measure();
                  // system = toSegment(parent())->system();
            else if (parent()->isMeasure())     // used in measure number
                  measure = toMeasure(parent());
                  // system = toMeasure(parent())->system();
            else if (parent()->isSystem())
                  system = toSystem(parent());
            else if (parent()->isChord())       // grace chord
                  measure = toSegment(parent()->parent())->measure();
            else
                  qFatal("this %s parent %s\n", name(), parent()->name());
            if (measure) {
                  p.ry() += measure->staffLines(vStaffIdx())->y();
                  system = measure->system();
                  if (system) {
                        Page* page = system->page();
                        if (page)
                              p.ry() += page->y();
                        }
                  }
            if (system)
                  p.ry() += system->staffYpage(vStaffIdx());
            p.rx() = canvasX();
            }
      else
            p += parent()->canvasPos();
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
//    Return true if p is inside the shape of the object.
//    Note: p is in page coordinates
//---------------------------------------------------------

bool Element::contains(const QPointF& p) const
      {
      return shape().contains(p - pagePos());
      }

//---------------------------------------------------------
//  intersects
//    Return true if \a rr intersects bounding box of object.
//    Note: \a rr is in page coordinates
//---------------------------------------------------------

bool Element::intersects(const QRectF& rr) const
      {
      return shape().intersects(rr.translated(-pagePos()));
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Element::writeProperties(XmlWriter& xml) const
      {
      // copy paste should not keep links
      if (_links && (_links->size() > 1) && !xml.clipboardmode())
            xml.tag("lid", _links->lid());
      if (!autoplace() && !userOff().isNull()) {
            if (isVoltaSegment()
                || isGlissandoSegment()
                || isChordRest()
                || isRehearsalMark()
                || isDynamic()
                || isSystemDivider()
                || (xml.clipboardmode() && isSLineSegment()))
                  xml.tag("offset", userOff() / spatium());
            else
                  xml.tag("pos", pos() / score()->spatium());
            }
      if (((track() != xml.curTrack()) || (type() == ElementType::SLUR)) && (track() != -1)) {
            int t;
            t = track() + xml.trackDiff();
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
      writeProperty(xml, P_ID::COLOR);
      writeProperty(xml, P_ID::VISIBLE);
      writeProperty(xml, P_ID::Z);
      writeProperty(xml, P_ID::PLACEMENT);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "track")
            setTrack(e.readInt() + e.trackOffset());
      else if (tag == "color")
            setColor(e.readColor());
      else if (tag == "visible")
            setVisible(e.readInt());
      else if (tag == "selected") // obsolete
            e.readInt();
      else if (tag == "userOff") {
            _userOff = e.readPoint();
            setAutoplace(false);
            }
      else if (tag == "lid") {
            int id = e.readInt();
            _links = e.linkIds().value(id);
            if (!_links) {
                  if (!score()->isMaster())   // DEBUG
                        qDebug("---link %d not found (%d)", id, e.linkIds().size());
                  _links = new LinkedElements(score(), id);
                  e.linkIds().insert(id, _links);
                  }
#ifndef NDEBUG
            else {
                  for (ScoreElement* eee : *_links) {
                        Element* ee = static_cast<Element*>(eee);
                        if (ee->type() != type()) {
                              qFatal("link %s(%d) type mismatch %s linked to %s",
                                 ee->name(), id, ee->name(), name());
                              }
                        }
                  }
#endif
            Q_ASSERT(!_links->contains(this));
            _links->append(this);
            }
      else if (tag == "tick") {
            int val = e.readInt();
            if (val >= 0)
                  e.initTick(score()->fileDivision(val));
            }
      else if (tag == "offset") {
            setUserOff(e.readPoint() * spatium());
            setAutoplace(false);
            }
      else if (tag == "pos") {
            QPointF pt = e.readPoint();
            _readPos = pt * score()->spatium();
            setAutoplace(false);
            }
      else if (tag == "voice")
            setTrack((_track/VOICES)*VOICES + e.readInt());
      else if (tag == "tag") {
            QString val(e.readElementText());
            for (int i = 1; i < MAX_TAGS; i++) {
                  if (score()->layerTags()[i] == val) {
                        _tag = 1 << i;
                        break;
                        }
                  }
            }
      else if (tag == "placement")
            _placement = Placement(Ms::getProperty(P_ID::PLACEMENT, e).toInt());
      else if (tag == "z")
            setZ(e.readInt());
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Element::write(XmlWriter& xml) const
      {
      xml.stag(name());
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Element::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Element::startEdit(MuseScoreView*, const QPointF&)
      {
      undoPushProperty(P_ID::USER_OFF);
      }

//---------------------------------------------------------
//   remove
///   Remove \a el from the list. Return true on success.
//---------------------------------------------------------

bool ElementList::remove(Element* el)
      {
      auto i = find(begin(), end(), el);
      if (i == end())
            return false;
      erase(i);
      return true;
      }

//---------------------------------------------------------
//   replace
//---------------------------------------------------------

void ElementList::replace(Element* o, Element* n)
      {
      auto i = find(begin(), end(), o);
      if (i == end()) {
            qDebug("ElementList::replace: element not found");
            return;
            }
      *i = n;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ElementList::write(XmlWriter& xml) const
      {
      for (const Element* e : *this)
            e->write(xml);
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
//   spatiumChanged
//---------------------------------------------------------

void Line::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _width = (_width / oldValue) * newValue;
      _len   = (_len / oldValue) * newValue;
      layout();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Line::layout()
      {
      qreal w2 = _width * .5;
      if (vertical)
            bbox().setRect(-w2, -w2, _width, _len + _width);
      else
            bbox().setRect(-w2, -w2, _len + _width, _width);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Line::draw(QPainter* painter) const
      {
      painter->setPen(QPen(curColor(), _width));
      if (vertical)
            painter->drawLine(QLineF(0.0, 0.0, 0.0, _len));
      else
            painter->drawLine(QLineF(0.0, 0.0, _len, 0.0));
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Line::writeProperties(XmlWriter& xml) const
      {
      xml.tag("lineWidth", _width / spatium());
      xml.tag("lineLen", _len / spatium());
      if (!vertical)
            xml.tag("vertical", vertical);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Line::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "lineWidth")
            _width = e.readDouble() * spatium();
      else if (tag == "lineLen")
            _len = e.readDouble() * spatium();
      else if (tag == "vertical")
            vertical = e.readInt();
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
      elements.clear();
      foreach(Element* e, c.elements)
            elements.append(e->clone());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Compound::draw(QPainter* painter) const
      {
      foreach(Element* e, elements) {
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
      elements.push_back(e);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Compound::layout()
      {
      setbbox(QRectF());
      for (auto i = elements.begin(); i != elements.end(); ++i) {
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
      for (auto i = elements.begin(); i != elements.end(); ++i)
            (*i)->setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Compound::setVisible(bool f)
      {
      Element::setVisible(f);
      for (auto i = elements.begin(); i != elements.end(); ++i)
            (*i)->setVisible(f);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Compound::clear()
      {
      foreach(Element* e, elements) {
            if (e->selected())
                  score()->deselect(e);
            delete e;
            }
      elements.clear();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Element::dump() const
      {
      qDebug("---Element: %s, pos(%4.2f,%4.2f)"
         "\n   bbox(%g,%g,%g,%g)"
         "\n   abox(%g,%g,%g,%g)"
         "\n  parent: %p",
         name(), ipos().x(), ipos().y(),
         _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
         abbox().x(), abbox().y(), abbox().width(), abbox().height(),
         parent());
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Element::mimeData(const QPointF& dragOffset) const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      XmlWriter xml(score(), &buffer);
      xml.setClipboardmode(true);
      xml.stag("Element");
      if (isNote())
            xml.tag("duration", toNote(this)->chord()->duration());
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

ElementType Element::readType(XmlReader& e, QPointF* dragOffset,
   Fraction* duration)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "Element")
                  while (e.readNextStartElement()) {
                        const QStringRef& tag = e.name();
                        if (tag == "dragOffset")
                              *dragOffset = e.readPoint();
                        else if (tag == "duration")
                              *duration = e.readFraction();
                        else {
                              ElementType type = name2type(tag);
                              if (type == ElementType::INVALID)
                                    break;
                              return type;
                        }
                  }
            else
                  e.unknown();
            }
      return ElementType::INVALID;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Element::editDrag(const EditData& ed)
      {
      score()->addRefresh(canvasBoundingRect());
      setUserOff(userOff() + ed.delta);
      score()->addRefresh(canvasBoundingRect());
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Element::edit(MuseScoreView*, Grip, int key, Qt::KeyboardModifiers, const QString&)
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
      qDebug("Element: cannot add %s to %s", e->name(), name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Element::remove(Element* e)
      {
      qFatal("Element: cannot remove %s from %s", e->name(), name());
      }

//---------------------------------------------------------
//   create
//    Element factory
//---------------------------------------------------------

Element* Element::create(ElementType type, Score* score)
      {
      switch(type) {
            case ElementType::VOLTA:             return new Volta(score);
            case ElementType::OTTAVA:            return new Ottava(score);
            case ElementType::TEXTLINE:          return new TextLine(score);
            case ElementType::NOTELINE:          return new NoteLine(score);
            case ElementType::LYRICSLINE:        return new LyricsLine(score);
            case ElementType::TRILL:             return new Trill(score);
            case ElementType::PEDAL:             return new Pedal(score);
            case ElementType::HAIRPIN:           return new Hairpin(score);
            case ElementType::CLEF:              return new Clef(score);
            case ElementType::KEYSIG:            return new KeySig(score);
            case ElementType::TIMESIG:           return new TimeSig(score);
            case ElementType::BAR_LINE:          return new BarLine(score);
            case ElementType::SYSTEM_DIVIDER:    return new SystemDivider(score);
            case ElementType::ARPEGGIO:          return new Arpeggio(score);
            case ElementType::BREATH:            return new Breath(score);
            case ElementType::GLISSANDO:         return new Glissando(score);
            case ElementType::BRACKET:           return new Bracket(score);
            case ElementType::ARTICULATION:      return new Articulation(score);
            case ElementType::CHORDLINE:         return new ChordLine(score);
            case ElementType::ACCIDENTAL:        return new Accidental(score);
            case ElementType::DYNAMIC:           return new Dynamic(score);
            case ElementType::TEXT:              return new Text(score);
            case ElementType::INSTRUMENT_NAME:   return new InstrumentName(score);
            case ElementType::STAFF_TEXT:        return new StaffText(score);
            case ElementType::SYSTEM_TEXT:       return new SystemText(score);
            case ElementType::REHEARSAL_MARK:    return new RehearsalMark(score);
            case ElementType::INSTRUMENT_CHANGE: return new InstrumentChange(score);
            case ElementType::STAFFTYPE_CHANGE:  return new StaffTypeChange(score);
            case ElementType::NOTEHEAD:          return new NoteHead(score);
            case ElementType::NOTEDOT:           return new NoteDot(score);
            case ElementType::TREMOLO:           return new Tremolo(score);
            case ElementType::LAYOUT_BREAK:      return new LayoutBreak(score);
            case ElementType::MARKER:            return new Marker(score);
            case ElementType::JUMP:              return new Jump(score);
            case ElementType::REPEAT_MEASURE:    return new RepeatMeasure(score);
            case ElementType::ICON:              return new Icon(score);
            case ElementType::NOTE:              return new Note(score);
            case ElementType::SYMBOL:            return new Symbol(score);
            case ElementType::FSYMBOL:           return new FSymbol(score);
            case ElementType::CHORD:             return new Chord(score);
            case ElementType::REST:              return new Rest(score);
            case ElementType::SPACER:            return new Spacer(score);
            case ElementType::STAFF_STATE:       return new StaffState(score);
            case ElementType::TEMPO_TEXT:        return new TempoText(score);
            case ElementType::HARMONY:           return new Harmony(score);
            case ElementType::FRET_DIAGRAM:      return new FretDiagram(score);
            case ElementType::BEND:              return new Bend(score);
            case ElementType::TREMOLOBAR:        return new TremoloBar(score);
            case ElementType::LYRICS:            return new Lyrics(score);
            case ElementType::FIGURED_BASS:      return new FiguredBass(score);
            case ElementType::STEM:              return new Stem(score);
            case ElementType::SLUR:              return new Slur(score);
            case ElementType::FINGERING:          return new Fingering(score);
            case ElementType::HBOX:              return new HBox(score);
            case ElementType::VBOX:              return new VBox(score);
            case ElementType::TBOX:              return new TBox(score);
            case ElementType::FBOX:              return new FBox(score);
            case ElementType::MEASURE:           return new Measure(score);
            case ElementType::TAB_DURATION_SYMBOL: return new TabDurationSymbol(score);
            case ElementType::OSSIA:               return new Ossia(score);
            case ElementType::IMAGE:             return new Image(score);
            case ElementType::BAGPIPE_EMBELLISHMENT: return new BagpipeEmbellishment(score);
            case ElementType::AMBITUS:           return new Ambitus(score);

            case ElementType::TEXTLINE_BASE:
            case ElementType::TEXTLINE_SEGMENT:
            case ElementType::GLISSANDO_SEGMENT:
            case ElementType::SLUR_SEGMENT:
            case ElementType::TIE_SEGMENT:
            case ElementType::STEM_SLASH:
            case ElementType::LINE:
            case ElementType::TIE:
            case ElementType::PAGE:
            case ElementType::BEAM:
            case ElementType::HOOK:
            case ElementType::TUPLET:
            case ElementType::HAIRPIN_SEGMENT:
            case ElementType::OTTAVA_SEGMENT:
            case ElementType::TRILL_SEGMENT:
            case ElementType::VOLTA_SEGMENT:
            case ElementType::PEDAL_SEGMENT:
            case ElementType::LYRICSLINE_SEGMENT:
            case ElementType::LEDGER_LINE:
            case ElementType::STAFF_LINES:
            case ElementType::SELECTION:
            case ElementType::LASSO:
            case ElementType::SHADOW_NOTE:
            case ElementType::SEGMENT:
            case ElementType::SYSTEM:
            case ElementType::COMPOUND:
            case ElementType::ELEMENT:
            case ElementType::ELEMENT_LIST:
            case ElementType::STAFF_LIST:
            case ElementType::MEASURE_LIST:
            case ElementType::MAXTYPE:
            case ElementType::INVALID:
            case ElementType::PART:
            case ElementType::STAFF:
            case ElementType::SCORE:
                  break;
            }
      qDebug("cannot create type %d <%s>", int(type), Element::name(type));
      return 0;
      }

//---------------------------------------------------------
//   name2Element
//---------------------------------------------------------

Element* Element::name2Element(const QStringRef& s, Score* sc)
      {
      ElementType type = Element::name2type(s);
      if (type == ElementType::INVALID)
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

QPointF Element::getGrip(Grip) const
      {
      qreal _spatium = score()->spatium();
      return QPointF(userOff().x() / _spatium, userOff().y() / _spatium);
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void Element::setGrip(Grip, const QPointF& pt)
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
//   undoSetPlacement
//---------------------------------------------------------

void Element::undoSetPlacement(Placement v)
      {
      undoChangeProperty(P_ID::PLACEMENT, int(v));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Element::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::TRACK:
                  return track();
            case P_ID::GENERATED:
                  return generated();
            case P_ID::COLOR:
                  return color();
            case P_ID::VISIBLE:
                  return visible();
            case P_ID::SELECTED:
                  return selected();
            case P_ID::USER_OFF:
                  return _userOff;
            case P_ID::PLACEMENT:
                  return int(_placement);
            case P_ID::AUTOPLACE:
                  return autoplace();
            case P_ID::Z:
                  return z();
            case P_ID::SYSTEM_FLAG:
                  return systemFlag();
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Element::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::TRACK:
                  setTrack(v.toInt());
                  break;
            case P_ID::GENERATED:
                  setGenerated(v.toBool());
                  break;
            case P_ID::COLOR:
                  setColor(v.value<QColor>());
                  break;
            case P_ID::VISIBLE:
                  setVisible(v.toBool());
                  break;
            case P_ID::SELECTED:
                  setSelected(v.toBool());
                  break;
            case P_ID::USER_OFF:
                  score()->addRefresh(canvasBoundingRect());
                  _userOff = v.toPointF();
                  break;
            case P_ID::PLACEMENT:
                  _placement = Placement(v.toInt());
                  break;
            case P_ID::AUTOPLACE:
                  setAutoplace(v.toBool());
                  break;
            case P_ID::Z:
                  setZ(v.toInt());
                  break;
            case P_ID::SYSTEM_FLAG:
                  setSystemFlag(v.toBool());
                  break;
            default:
                  qFatal("Element::setProperty: unknown <%s>(%d), data <%s>",
                     propertyName(propertyId), static_cast<int>(propertyId), qPrintable(v.toString()));
                  return false;
            }
      score()->setLayout(tick());
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Element::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::GENERATED:
                  return false;
            case P_ID::VISIBLE:
                  return true;
            case P_ID::COLOR:
                  return MScore::defaultColor;
            case P_ID::PLACEMENT:
                  return int(Placement::BELOW);
            case P_ID::SELECTED:
                  return false;
            case P_ID::USER_OFF:
                  return QPointF();
            case P_ID::AUTOPLACE:
                  return true;
            case P_ID::Z:
                  return int(type()) * 100;
            case P_ID::SYSTEM_FLAG:
                  return false;
            default:    // not all properties have a default
                  break;
            }
      return QVariant();
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void Element::initSubStyle(SubStyle st)
      {
      auto l = subStyle(st);
      for (const StyledProperty& p : l) {
            setProperty(p.propertyIdx, score()->styleV(p.styleIdx));
            setPropertyFlags(p.propertyIdx, PropertyFlags::STYLED);
            }
      }

//---------------------------------------------------------
//   custom
//    check if property is != default
//---------------------------------------------------------

bool Element::custom(P_ID id) const
      {
      return propertyDefault(id) != getProperty(id);
      }

//---------------------------------------------------------
//   readProperty
//---------------------------------------------------------

bool Element::readProperty(const QStringRef& s, XmlReader& e, P_ID id)
      {
      if (s == propertyName(id)) {
            setProperty(id, Ms::getProperty(id, e));
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   isPrintable
//---------------------------------------------------------

bool Element::isPrintable() const
      {
      switch (type()) {
            case ElementType::PAGE:
            case ElementType::SYSTEM:
            case ElementType::MEASURE:
            case ElementType::SEGMENT:
            case ElementType::VBOX:
            case ElementType::HBOX:
            case ElementType::TBOX:
            case ElementType::FBOX:
            case ElementType::SPACER:
            case ElementType::SHADOW_NOTE:
            case ElementType::LASSO:
            case ElementType::ELEMENT_LIST:
            case ElementType::STAFF_LIST:
            case ElementType::MEASURE_LIST:
            case ElementType::SELECTION:
                  return false;
            default:
                  return true;
            }
      }

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

Element* Element::findMeasure()
      {
      if (type() == ElementType::MEASURE)
            return this;
      else if (_parent)
            return _parent->findMeasure();
      else
            return 0;
      }

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void Element::undoSetColor(const QColor& c)
      {
      undoChangeProperty(P_ID::COLOR, c);
      }

//---------------------------------------------------------
//   undoSetVisible
//---------------------------------------------------------

void Element::undoSetVisible(bool v)
      {
      undoChangeProperty(P_ID::VISIBLE, v);
      }

//---------------------------------------------------------
//   bbox() function for scripts
//
//    use spatium units rather than raster units
//---------------------------------------------------------

QRectF Element::scriptBbox() const
      {
      qreal  _sp = spatium();
      QRectF _bbox = bbox();
      return QRectF(_bbox.x() / _sp, _bbox.y() / _sp, _bbox.width() / _sp, _bbox.height() / _sp);
      }

//---------------------------------------------------------
//   positioning functions for scripts
//
//    use spatium units rather than raster units
//    are undoable
//    route pos changes to usefOff
//---------------------------------------------------------

QPointF Element::scriptPagePos() const
      {
      return pagePos() / spatium();
      }

QPointF Element::scriptPos() const
      {
      return (_pos + _userOff) / spatium();
      }

void Element::scriptSetPos(const QPointF& p)
      {
      undoChangeProperty(P_ID::USER_OFF, p*spatium() - ipos());
      }

QPointF Element::scriptUserOff() const
      {
      return _userOff / spatium();
      }

void Element::scriptSetUserOff(const QPointF& o)
      {
      undoChangeProperty(P_ID::USER_OFF, o * spatium());
      }

//void Element::draw(SymId id, QPainter* p) const { score()->scoreFont()->draw(id, p, magS()); }

//---------------------------------------------------------
//   drawSymbol
//---------------------------------------------------------

void Element::drawSymbol(SymId id, QPainter* p, const QPointF& o) const
      {
      score()->scoreFont()->draw(id, p, magS(), o);
      }

void Element::drawSymbol(SymId id, QPainter* p, const QPointF& o, int n) const
      {
      score()->scoreFont()->draw(id, p, magS(), o, n);
      }

void Element::drawSymbols(const std::vector<SymId>& s, QPainter* p, const QPointF& o) const
      {
      score()->scoreFont()->draw(s, p, magS(), o);
      }

//---------------------------------------------------------
//   symHeight
//---------------------------------------------------------

qreal Element::symHeight(SymId id) const
      {
      return score()->scoreFont()->height(id, magS());
      }

//---------------------------------------------------------
//   symWidth
//---------------------------------------------------------

qreal Element::symWidth(SymId id) const
      {
      return score()->scoreFont()->width(id, magS());
      }
qreal Element::symWidth(const std::vector<SymId>& s) const
      {
      return score()->scoreFont()->width(s, magS());
      }

//---------------------------------------------------------
//   symAdvance
//---------------------------------------------------------

qreal Element::symAdvance(SymId id) const
      {
      return score()->scoreFont()->advance(id, magS());
      }

//---------------------------------------------------------
//   symBbox
//---------------------------------------------------------

QRectF Element::symBbox(SymId id) const
      {
      return score()->scoreFont()->bbox(id, magS());
      }

QRectF Element::symBbox(const std::vector<SymId>& s) const
      {
      return score()->scoreFont()->bbox(s, magS());
      }

//---------------------------------------------------------
//   symStemDownNW
//---------------------------------------------------------

QPointF Element::symStemDownNW(SymId id) const
      {
      return score()->scoreFont()->stemDownNW(id, magS());
      }

//---------------------------------------------------------
//   symStemUpSE
//---------------------------------------------------------

QPointF Element::symStemUpSE(SymId id) const
      {
      return score()->scoreFont()->stemUpSE(id, magS());
      }

//---------------------------------------------------------
//   symCutOutNE / symCutOutNW / symCutOutSE / symCutOutNW
//---------------------------------------------------------

QPointF Element::symCutOutNE(SymId id) const
      {
      return score()->scoreFont()->cutOutNE(id, magS());
      }

QPointF Element::symCutOutNW(SymId id) const
      {
      return score()->scoreFont()->cutOutNW(id, magS());
      }

QPointF Element::symCutOutSE(SymId id) const
      {
      return score()->scoreFont()->cutOutSE(id, magS());
      }

QPointF Element::symCutOutSW(SymId id) const
      {
      return score()->scoreFont()->cutOutSW(id, magS());
      }

//---------------------------------------------------------
//   symIsValid
//---------------------------------------------------------

bool Element::symIsValid(SymId id) const
      {
      return score()->scoreFont()->isValid(id);
      }

//---------------------------------------------------------
//   toTimeSigString
//---------------------------------------------------------

std::vector<SymId> Element::toTimeSigString(const QString& s) const
      {
      std::vector<SymId> d;
      for (int i = 0; i < s.size(); ++i) {
            switch (s[i].unicode()) {
                  case 43: d.push_back(SymId::timeSigPlusSmall); break; // '+'
                  case 48: d.push_back(SymId::timeSig0); break;         // '0'
                  case 49: d.push_back(SymId::timeSig1); break;         // '1'
                  case 50: d.push_back(SymId::timeSig2); break;         // '2'
                  case 51: d.push_back(SymId::timeSig3); break;         // '3'
                  case 52: d.push_back(SymId::timeSig4); break;         // '4'
                  case 53: d.push_back(SymId::timeSig5); break;         // '5'
                  case 54: d.push_back(SymId::timeSig6); break;         // '6'
                  case 55: d.push_back(SymId::timeSig7); break;         // '7'
                  case 56: d.push_back(SymId::timeSig8); break;         // '8'
                  case 57: d.push_back(SymId::timeSig9); break;         // '9'
                  case 67: d.push_back(SymId::timeSigCommon); break;    // 'C'
                  case 40: d.push_back(SymId::timeSigParensLeftSmall); break;  // '('
                  case 41: d.push_back(SymId::timeSigParensRightSmall); break; // ')'
                  case 162: d.push_back(SymId::timeSigCutCommon); break;    // '¢'
                  case 59664: d.push_back(SymId::mensuralProlation1); break;
                  case 79:                                          // 'O'
                  case 59665: d.push_back(SymId::mensuralProlation2); break;
                  case 216:                                        // 'Ø'
                  case 59666: d.push_back(SymId::mensuralProlation3); break;
                  case 59667: d.push_back(SymId::mensuralProlation4); break;
                  case 59668: d.push_back(SymId::mensuralProlation5); break;
                  case 59670: d.push_back(SymId::mensuralProlation7); break;
                  case 59671: d.push_back(SymId::mensuralProlation8); break;
                  case 59673: d.push_back(SymId::mensuralProlation10); break;
                  case 59674: d.push_back(SymId::mensuralProlation11); break;
                  default:  break;  // d += s[i]; break;
                  }
            }
      return d;
      }

//---------------------------------------------------------
//   concertPitch
//---------------------------------------------------------

bool Element::concertPitch() const
      {
      return score()->styleB(StyleIdx::concertPitch);
      }

//------------------------------------------------------------------------------------------
//   nextElement
//   This function is used in for the next-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the next one and overrides this method
//------------------------------------------------------------------------------------------

Element* Element::nextElement()
      {
      Element* p = this;
      while (p) {
            switch (p->type()) {
                  case ElementType::NOTE:
                        if(static_cast<Note*>(p)->chord()->isGrace())
                              break;
                        return p;
                  case ElementType::REST:
                        return p;
                  case ElementType::CHORD: {
                        Chord* c = static_cast<Chord*>(p);
                        if (!c->isGrace())
                              return c->notes().back();
                        }
                        break;
                  case ElementType::SEGMENT: {
                        Segment* s = static_cast<Segment*>(p);
                        return s->firstElement(staffIdx());
                        }
                  case ElementType::MEASURE: {
                        Measure* m = static_cast<Measure*>(p);
                        return m->nextElementStaff(staffIdx());
                        }
                  case ElementType::SYSTEM: {
                        System* sys = static_cast<System*>(p);
                        return sys->nextElement();
                        }
                  default:
                        break;
                  }
            p = p->parent();
            }
      return score()->firstElement();
      }

//------------------------------------------------------------------------------------------
//   prevElement
//   This function is used in for the prev-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the previous one and overrides this method
//------------------------------------------------------------------------------------------

Element* Element::prevElement()
      {
      Element* p = this;
      while (p) {
            switch (p->type()) {
                  case ElementType::NOTE:
                        if(static_cast<Note*>(p)->chord()->isGrace())
                              break;
                        return p;
                  case ElementType::REST:
                        return p;
                  case ElementType::CHORD: {
                        Chord* c = static_cast<Chord*>(p);
                        if (!c->isGrace())
                              return c->notes().front();
                        }
                        break;
                  case ElementType::SEGMENT: {
                        Segment* s = static_cast<Segment*>(p);
                        return s->lastElement(staffIdx());
                        }
                  case ElementType::MEASURE: {
                        Measure* m = static_cast<Measure*>(p);
                        return m->prevElementStaff(staffIdx());
                        }
                  case ElementType::SYSTEM: {
                        System* sys = static_cast<System*>(p);
                        return sys->prevElement();
                        }
                  default:
                        break;
                  }
            p = p->parent();
            }
      return score()->firstElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Element::accessibleInfo() const
      {
      return Element::userName();
      }

//---------------------------------------------------------
//   nextGrip
//---------------------------------------------------------

bool Element::nextGrip(Grip* grip) const
      {
      int i = int(*grip) + 1;
      if (i >= grips()) {
            *grip = Grip(0);
            return false;
            }
      *grip = Grip(i);
      return true;
      }

//---------------------------------------------------------
//   prevGrip
//---------------------------------------------------------

bool Element::prevGrip(Grip* grip) const
      {
      int i = int(*grip) - 1;
      if (i < 0) {
            *grip = Grip(grips() - 1);
            return false;
            }
      *grip = Grip(i);
      return true;
      }

//---------------------------------------------------------
//   isUserModified
//    Check if this element was modified by user and
//    therefore must be saved.
//---------------------------------------------------------

bool Element::isUserModified() const
      {
      return !visible() || !userOff().isNull() || (color() != MScore::defaultColor);
      }

//---------------------------------------------------------
//   tick
//    utility, searches for segment / segment parent
//---------------------------------------------------------

int Element::tick() const
      {
      const Element* e = this;
      while (e) {
            if (e->isSegment())
                  return toSegment(e)->tick();
            else if (e->isMeasureBase())
                  return toMeasureBase(e)->tick();
            e = e->parent();
            }
      return -1;
      }

//---------------------------------------------------------
//   rtick
//    utility, searches for segment / segment parent
//---------------------------------------------------------

int Element::rtick() const
      {
      const Element* e = this;
      while (e) {
            if (e->isSegment())
                  return toSegment(e)->rtick();
            e = e->parent();
            }
      return -1;
      }

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Element::triggerLayout() const
      {
      score()->setLayout(tick());
      }

}
