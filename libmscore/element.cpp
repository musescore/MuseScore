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
#include "connector.h"
#include "dynamic.h"
#include "figuredbass.h"
#include "fingering.h"
#include "fret.h"
#include "fraction.h"
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
#include "sticking.h"
#include "style.h"
#include "symbol.h"
#include "sym.h"
#include "system.h"
#include "tempotext.h"
#include "textframe.h"
#include "text.h"
#include "measurenumber.h"
#include "mmrestrange.h"
#include "textline.h"
#include "tie.h"
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
#include "letring.h"
#include "vibrato.h"
#include "palmmute.h"
#include "fermata.h"
#include "shape.h"
//#include "musescoreCore.h"

namespace Ms {

// extern bool showInvisible;

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Element::spatiumChanged(qreal oldValue, qreal newValue)
      {
      if (offsetIsSpatiumDependent())
            _offset *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Element::localSpatiumChanged(qreal oldValue, qreal newValue)
      {
      if (offsetIsSpatiumDependent())
            _offset *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Element::spatium() const
      {
      if (systemFlag() || (parent() && parent()->systemFlag())) {
            return score()->spatium();
            }
      else {
            Staff* s = staff();
            return s ? s->spatium(this) : score()->spatium();
            }
      }

//---------------------------------------------------------
//   offsetIsSpatiumDependent
//---------------------------------------------------------

bool Element::offsetIsSpatiumDependent() const
      {
      return sizeIsSpatiumDependent() || (_flags & ElementFlag::ON_STAFF);
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

Element::Element(Score* s, ElementFlags f)
   : ScoreElement(s)
      {
      _flags         = f;
      _track         = -1;
      _color         = MScore::defaultColor;
      _mag           = 1.0;
      _tag           = 1;
      _z             = -1;
      _offsetChanged = OffsetChange::NONE;
      _minDistance   = Spatium(0.0);
      }

Element::Element(const Element& e)
   : ScoreElement(e)
      {
      _parent     = e._parent;
      _bbox       = e._bbox;
      _mag        = e._mag;
      _pos        = e._pos;
      _offset     = e._offset;
      _track      = e._track;
      _flags      = e._flags;
      setFlag(ElementFlag::SELECTED, false);
      _tag        = e._tag;
      _z          = e._z;
      _color      = e._color;
      _offsetChanged = e._offsetChanged;
      _minDistance   = e._minDistance;
      itemDiscovered = false;
      }

//---------------------------------------------------------
//   ~Element
//---------------------------------------------------------

Element::~Element()
      {
      Score::onElementDestruction(this);
      }

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

Element* Element::linkedClone()
      {
      Element* e = clone();
      e->setAutoplace(true);
      score()->undo(new Link(e, this));
      return e;
      }

//---------------------------------------------------------
//   deleteLater
//---------------------------------------------------------

void Element::deleteLater()
      {
      if (selected())
            score()->deselect(this);
      masterScore()->deleteLater(this);
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
      undoResetProperty(Pid::AUTOPLACE);
      undoResetProperty(Pid::PLACEMENT);
      undoResetProperty(Pid::MIN_DISTANCE);
      undoResetProperty(Pid::OFFSET);
      setOffsetChanged(false);
      ScoreElement::reset();
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
//   staffType
//---------------------------------------------------------

const StaffType* Element::staffType() const
      {
      Staff* s = staff();
      return s ? s->staffTypeForElement(this) : nullptr;
      }

//---------------------------------------------------------
//   onTabStaff
//---------------------------------------------------------

bool Element::onTabStaff() const
      {
      const StaffType* stt = staffType();
      return stt ? stt->isTabStaff() : false;
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
//   tick
//---------------------------------------------------------

Fraction Element::tick() const
      {
      const Element* e = this;
      while (e->parent()) {
            if (e->parent()->isSegment())
                  return toSegment(e->parent())->tick();
            else if (e->parent()->isMeasureBase())
                  return toMeasureBase(e->parent())->tick();
            e = e->parent();
            }
      return Fraction(0, 1);
      }

//---------------------------------------------------------
//   rtick
//---------------------------------------------------------

Fraction Element::rtick() const
      {
      const Element* e = this;
      while (e->parent()) {
            if (e->parent()->isSegment())
                  return toSegment(e->parent())->rtick();
            e = e->parent();
            }
      return Fraction(0, 1);
      }

//---------------------------------------------------------
//   playTick
//---------------------------------------------------------

Fraction Element::playTick() const
      {
      // Play from the element's tick position by default.
      return tick();
      }


//---------------------------------------------------------
//   beat
//---------------------------------------------------------

Fraction Element::beat() const
      {
      // Returns an appropriate fraction of ticks for use as a "Beat" reference
      // in the Select All Similar filter.
      int bar, beat, ticks;
      TimeSigMap* tsm = score()->sigmap();
      tsm->tickValues(tick().ticks(), &bar, &beat, &ticks);
      int ticksB = ticks_beat(tsm->timesig(tick().ticks()).timesig().denominator());

      Fraction complexFraction((++beat * ticksB) + ticks, ticksB);
      return complexFraction.reduced();
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
      return curColor(visible());
      }

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

QColor Element::curColor(bool isVisible) const
      {
      return curColor(isVisible, color());
      }

QColor Element::curColor(bool isVisible, QColor normalColor) const
      {
      // the default element color is always interpreted as black in
      // printing
      if (score() && score()->printing())
            return (normalColor == MScore::defaultColor) ? Qt::black : normalColor;

      if (flag(ElementFlag::DROP_TARGET))
            return MScore::dropColor;
      bool marked = false;
      if (isNote()) {
            //const Note* note = static_cast<const Note*>(this);
            marked = toNote(this)->mark();
            }
      if (selected() || marked ) {
            QColor originalColor;
            if (track() == -1)
                  originalColor = MScore::selectColor[0];
            else
                  originalColor = MScore::selectColor[voice()];
            if (isVisible)
                  return originalColor;
            else {
                  int red = originalColor.red();
                  int green = originalColor.green();
                  int blue = originalColor.blue();
                  float tint = .6f;  // Between 0 and 1. Higher means lighter, lower means darker
                  return QColor(red + tint * (255 - red), green + tint * (255 - green), blue + tint * (255 - blue));
                  }
            }
      if (!isVisible)
            return Qt::gray;
      return normalColor;
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
            else if (parent()->isFretDiagram())
                  return p + parent()->pagePos();
            else
                  qFatal("this %s parent %s\n", name(), parent()->name());
            if (measure) {
                  system = measure->system();
                  p.ry() += measure->staffLines(vStaffIdx())->y();
                  }
            if (system) {
                  if (system->staves()->size() <= vStaffIdx()) {
                        qDebug("staffIdx out of bounds: %s", name());
                        }
                  p.ry() += system->staffYpage(vStaffIdx());
                  }
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
      if (parent() == nullptr)
            return p;

      if (_flags & ElementFlag::ON_STAFF) {
            System* system = nullptr;
            Measure* measure = nullptr;
            if (parent()->isSegment())
                  measure = toSegment(parent())->measure();
            else if (parent()->isMeasure())     // used in measure number
                  measure = toMeasure(parent());
                  // system = toMeasure(parent())->system();
            else if (parent()->isSystem())
                  system = toSystem(parent());
            else if (parent()->isChord())       // grace chord
                  measure = toSegment(parent()->parent())->measure();
            else if (parent()->isFretDiagram())
                  return p + parent()->canvasPos() + QPointF(toFretDiagram(parent())->centerX(), 0.0);
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
      bool autoplaceEnabled = score()->styleB(Sid::autoplaceEnabled);
      if (!autoplaceEnabled) {
            score()->setStyleValue(Sid::autoplaceEnabled, true);
            writeProperty(xml, Pid::AUTOPLACE);
            score()->setStyleValue(Sid::autoplaceEnabled, autoplaceEnabled);
            }
      else {
            writeProperty(xml, Pid::AUTOPLACE);
            }

      // copy paste should not keep links
      if (_links && (_links->size() > 1) && !xml.clipboardmode()) {
            if (MScore::debugMode)
                  xml.tag("lid", _links->lid());
            Element* me = static_cast<Element*>(_links->mainElement());
            Q_ASSERT(type() == me->type());
            Staff* s = staff();
            if (!s) {
                  s = score()->staff(xml.curTrack() / VOICES);
                  if (!s)
                        qDebug("Element::writeProperties: linked element's staff not found (%s)", name());
                  }
            Location loc = Location::positionForElement(this);
            if (me == this) {
                  xml.tagE("linkedMain");
                  xml.setLidLocalIndex(_links->lid(), xml.assignLocalIndex(loc));
                  }
            else {
                  if (s && s->links()) {
                        Staff* linkedStaff = toStaff(s->links()->mainElement());
                        loc.setStaff(linkedStaff->idx());
                        }
                  xml.stag("linked");
                  if (!me->score()->isMaster()) {
                        if (me->score() == score()) {
                              xml.tag("score", "same");
                              }
                        else {
                              qDebug("Element::writeProperties: linked elements belong to different scores but none of them is master score: (%s lid=%d)", name(), _links->lid());
                              }
                        }
                  Location mainLoc = Location::positionForElement(me);
                  const int guessedLocalIndex = xml.assignLocalIndex(mainLoc);
                  if (loc != mainLoc) {
                        mainLoc.toRelative(loc);
                        mainLoc.write(xml);
                        }
                  const int indexDiff = xml.lidLocalIndex(_links->lid()) - guessedLocalIndex;
                  xml.tag("indexDiff", indexDiff, 0);
                  xml.etag(); // </linked>
                  }
            }
      if ((xml.writeTrack() || track() != xml.curTrack())
         && (track() != -1) && !isBeam()) {
            // Writing track number for beams is redundant as it is calculated
            // during layout.
            int t = track() + xml.trackDiff();
            xml.tag("track", t);
            }
      if (xml.writePosition())
            xml.tag(Pid::POSITION, rtick());
      if (_tag != 0x1) {
            for (int i = 1; i < MAX_TAGS; i++) {
                  if (_tag == ((unsigned)1 << i)) {
                        xml.tag("tag", score()->layerTags()[i]);
                        break;
                        }
                  }
            }
      for (Pid pid : { Pid::OFFSET, Pid::COLOR, Pid::VISIBLE, Pid::Z, Pid::PLACEMENT}) {
            if (propertyFlags(pid) == PropertyFlags::NOSTYLE)
                  writeProperty(xml, pid);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (readProperty(tag, e, Pid::SIZE_SPATIUM_DEPENDENT))
            ;
      else if (readProperty(tag, e, Pid::OFFSET))
            ;
      else if (readProperty(tag, e, Pid::MIN_DISTANCE))
            ;
      else if (readProperty(tag, e, Pid::AUTOPLACE))
            ;
      else if (tag == "track")
            setTrack(e.readInt() + e.trackOffset());
      else if (tag == "color")
            setColor(e.readColor());
      else if (tag == "visible")
            setVisible(e.readInt());
      else if (tag == "selected") // obsolete
            e.readInt();
      else if ((tag == "linked") || (tag == "linkedMain")) {
            Staff* s = staff();
            if (!s) {
                  s = score()->staff(e.track() / VOICES);
                  if (!s) {
                        qDebug("Element::readProperties: linked element's staff not found (%s)", name());
                        e.skipCurrentElement();
                        return true;
                        }
                  }
            if (tag == "linkedMain") {
                  _links = new LinkedElements(score());
                  _links->push_back(this);
                  e.addLink(s, _links);
                  e.readNext();
                  }
            else {
                  Staff* ls = s->links() ? toStaff(s->links()->mainElement()) : nullptr;
                  bool linkedIsMaster = ls ? ls->score()->isMaster() : false;
                  Location loc = e.location(true);
                  if (ls)
                        loc.setStaff(ls->idx());
                  Location mainLoc = Location::relative();
                  bool locationRead = false;
                  int localIndexDiff = 0;
                  while (e.readNextStartElement()) {
                        const QStringRef& ntag(e.name());

                        if (ntag == "score") {
                              QString val(e.readElementText());
                              if (val == "same")
                                    linkedIsMaster = score()->isMaster();
                              }
                        else if (ntag == "location") {
                              mainLoc.read(e);
                              mainLoc.toAbsolute(loc);
                              locationRead = true;
                              }
                        else if (ntag == "indexDiff")
                              localIndexDiff = e.readInt();
                        else
                              e.unknown();
                        }
                  if (!locationRead)
                        mainLoc = loc;
                  LinkedElements* link = e.getLink(linkedIsMaster, mainLoc, localIndexDiff);
                  if (link) {
                        ScoreElement* linked = link->mainElement();
                        if (linked->type() == type())
                              linkTo(linked);
                        else
                              qDebug("Element::readProperties: linked elements have different types: %s, %s. Input file corrupted?", name(), linked->name());
                        }
                  if (!_links)
                        qDebug("Element::readProperties: could not link %s at staff %d", name(), mainLoc.staff() + 1);
                  }
            }
      else if (tag == "lid") {
            if (score()->mscVersion() >= 301) {
                  e.skipCurrentElement();
                  return true;
                  }
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
                  e.setTick(Fraction::fromTicks(score()->fileDivision(val)));       // obsolete
            }
      else if (tag == "pos")             // obsolete
            readProperty(e, Pid::OFFSET);
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
      else if (readProperty(tag, e, Pid::PLACEMENT))
            ;
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
      xml.stag(this);
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
            xml.tag("duration", toNote(this)->chord()->ticks());
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
//   readMimeData
//---------------------------------------------------------

Element* Element::readMimeData(Score* score, const QByteArray& data, QPointF* dragOffset, Fraction* duration)
      {
      XmlReader e(data);
      const ElementType type = Element::readType(e, dragOffset, duration);
      e.setPasteMode(true);

      if (type == ElementType::INVALID) {
            qDebug("cannot read type");
            return nullptr;
            }

      Element* el = Element::create(type, score);
      if (el)
            el->read(e);

      return el;
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
      switch (type) {
            case ElementType::VOLTA:             return new Volta(score);
            case ElementType::OTTAVA:            return new Ottava(score);
            case ElementType::TEXTLINE:          return new TextLine(score);
            case ElementType::NOTELINE:          return new NoteLine(score);
            case ElementType::TRILL:             return new Trill(score);
            case ElementType::LET_RING:          return new LetRing(score);
            case ElementType::VIBRATO:           return new Vibrato(score);
            case ElementType::PALM_MUTE:         return new PalmMute(score);
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
            case ElementType::FERMATA:           return new Fermata(score);
            case ElementType::CHORDLINE:         return new ChordLine(score);
            case ElementType::ACCIDENTAL:        return new Accidental(score);
            case ElementType::DYNAMIC:           return new Dynamic(score);
            case ElementType::TEXT:              return new Text(score);
            case ElementType::MEASURE_NUMBER:    return new MeasureNumber(score);
            case ElementType::MMREST_RANGE:      return new MMRestRange(score);
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
            case ElementType::TIE:               return new Tie(score);
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
            case ElementType::STICKING:          return new Sticking(score);

            case ElementType::LYRICSLINE:
            case ElementType::TEXTLINE_BASE:
            case ElementType::TEXTLINE_SEGMENT:
            case ElementType::GLISSANDO_SEGMENT:
            case ElementType::SLUR_SEGMENT:
            case ElementType::TIE_SEGMENT:
            case ElementType::STEM_SLASH:
            case ElementType::PAGE:
            case ElementType::BEAM:
            case ElementType::HOOK:
            case ElementType::TUPLET:
            case ElementType::HAIRPIN_SEGMENT:
            case ElementType::OTTAVA_SEGMENT:
            case ElementType::TRILL_SEGMENT:
            case ElementType::LET_RING_SEGMENT:
            case ElementType::VIBRATO_SEGMENT:
            case ElementType::PALM_MUTE_SEGMENT:
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
            case ElementType::BRACKET_ITEM:
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
      if (type == ElementType::INVALID) {
            qDebug("invalid <%s>\n", qPrintable(s.toString()));
            return 0;
            }
      return Element::create(type, sc);
      }

//---------------------------------------------------------
//   elementLessThan
//---------------------------------------------------------

bool elementLessThan(const Element* const e1, const Element* const e2)
      {
      if (e1->z() == e2->z()) {
            if (e1->selected())
                  return false;
            else if (e2->selected())
                  return true;
            else if (!e1->visible())
                  return true;
            else if (!e2->visible())
                  return false;
            else
                  return e1->track() > e2->track();
            }
      return e1->z() <= e2->z();
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
//   autoplace
//---------------------------------------------------------

bool Element::autoplace() const
      {
      if (!score() || !score()->styleB(Sid::autoplaceEnabled))
          return false;
      return !flag(ElementFlag::NO_AUTOPLACE);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Element::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::TICK:
                  return tick();
            case Pid::TRACK:
                  return track();
            case Pid::VOICE:
                  return voice();
            case Pid::POSITION:
                  return rtick();
            case Pid::GENERATED:
                  return generated();
            case Pid::COLOR:
                  return color();
            case Pid::VISIBLE:
                  return visible();
            case Pid::SELECTED:
                  return selected();
            case Pid::OFFSET:
                  return _offset;
            case Pid::MIN_DISTANCE:
                  return _minDistance;
            case Pid::PLACEMENT:
                  return int(placement());
            case Pid::AUTOPLACE:
                  return autoplace();
            case Pid::Z:
                  return z();
            case Pid::SYSTEM_FLAG:
                  return systemFlag();
            case Pid::SIZE_SPATIUM_DEPENDENT:
                  return sizeIsSpatiumDependent();
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Element::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::TRACK:
                  setTrack(v.toInt());
                  break;
            case Pid::VOICE:
                  setVoice(v.toInt());
                  break;
            case Pid::GENERATED:
                  setGenerated(v.toBool());
                  break;
            case Pid::COLOR:
                  setColor(v.value<QColor>());
                  break;
            case Pid::VISIBLE:
                  setVisible(v.toBool());
                  break;
            case Pid::SELECTED:
                  setSelected(v.toBool());
                  break;
            case Pid::OFFSET:
                  _offset = v.toPointF();
                  break;
            case Pid::MIN_DISTANCE:
                  setMinDistance(v.value<Spatium>());
                  break;
            case Pid::PLACEMENT:
                  setPlacement(Placement(v.toInt()));
                  break;
            case Pid::AUTOPLACE:
                  setAutoplace(v.toBool());
                  break;
            case Pid::Z:
                  setZ(v.toInt());
                  break;
            case Pid::SYSTEM_FLAG:
                  setSystemFlag(v.toBool());
                  break;
            case Pid::SIZE_SPATIUM_DEPENDENT:
                  setSizeIsSpatiumDependent(v.toBool());
                  break;
            default:
                  qDebug("%s unknown <%s>(%d), data <%s>", name(), propertyName(propertyId), int(propertyId), qPrintable(v.toString()));
                  return false;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Element::undoChangeProperty(Pid pid, const QVariant& val, PropertyFlags ps)
      {
      if (pid == Pid::AUTOPLACE && (val.toBool() == true && !autoplace())) {
            // Switching autoplacement on. Save user-defined
            // placement properties to undo stack.
            undoPushProperty(Pid::PLACEMENT);
            undoPushProperty(Pid::OFFSET);
            }
      ScoreElement::undoChangeProperty(pid, val, ps);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Element::propertyDefault(Pid pid) const
      {
      switch (pid) {
            case Pid::GENERATED:
                  return false;
            case Pid::VISIBLE:
                  return true;
            case Pid::COLOR:
                  return MScore::defaultColor;
            case Pid::PLACEMENT: {
                  QVariant v = ScoreElement::propertyDefault(pid);
                  if (v.isValid())        // if it's a styled property
                        return v;
                  return int(Placement::BELOW);
                  }
            case Pid::SELECTED:
                  return false;
            case Pid::OFFSET: {
                  QVariant v = ScoreElement::propertyDefault(pid);
                  if (v.isValid())        // if it's a styled property
                        return v;
                  return QPointF();
                  }
            case Pid::MIN_DISTANCE: {
                  QVariant v = ScoreElement::propertyDefault(pid);
                  if (v.isValid())
                        return v;
                  return 0.0;
                  }
            case Pid::AUTOPLACE:
                  return true;
            case Pid::Z:
                  return int(type()) * 100;
            default:
                  return ScoreElement::propertyDefault(pid);
            }
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Element::propertyId(const QStringRef& name) const
      {
      if (name == "pos" || name == "offset")
            return Pid::OFFSET;
      return ScoreElement::propertyId(name);
      }

//---------------------------------------------------------
//   propertyUserValue
//---------------------------------------------------------

QString Element::propertyUserValue(Pid pid) const
      {
      switch(pid) {
            case Pid::SUBTYPE:
                  return subtypeName();
            default:
                  return ScoreElement::propertyUserValue(pid);
            }
      }

//---------------------------------------------------------
//   custom
//    check if property is != default
//---------------------------------------------------------

bool Element::custom(Pid id) const
      {
      return propertyDefault(id) != getProperty(id);
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
//   findAncestor
//---------------------------------------------------------

Element* Element::findAncestor(ElementType t)
      {
      Element* e = this;
      while (e && e->type() != t)
            e = e->parent();
      return e;
      }

const Element* Element::findAncestor(ElementType t) const
      {
      const Element* e = this;
      while (e && e->type() != t)
            e = e->parent();
      return e;
      }

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

Measure* Element::findMeasure()
      {
      if (isMeasure())
            return toMeasure(this);
      else if (_parent)
            return _parent->findMeasure();
      else
            return 0;
      }

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

const Measure* Element::findMeasure() const
      {
      Element* e = const_cast<Element*>(this);
      return e->findMeasure();
      }

//---------------------------------------------------------
//   findMeasureBase
//---------------------------------------------------------

MeasureBase* Element::findMeasureBase()
      {
      if (isMeasureBase())
            return toMeasureBase(this);
      else if (_parent)
            return _parent->findMeasureBase();
      else
            return 0;
      }

//---------------------------------------------------------
//   findMeasureBase
//---------------------------------------------------------

const MeasureBase* Element::findMeasureBase() const
      {
      Element* e = const_cast<Element*>(this);
      return e->findMeasureBase();
      }

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void Element::undoSetColor(const QColor& c)
      {
      undoChangeProperty(Pid::COLOR, c);
      }

//---------------------------------------------------------
//   undoSetVisible
//---------------------------------------------------------

void Element::undoSetVisible(bool v)
      {
      undoChangeProperty(Pid::VISIBLE, v);
      }

//---------------------------------------------------------
//   drawSymbol
//---------------------------------------------------------

void Element::drawSymbol(SymId id, QPainter* p, const QPointF& o, qreal scale) const
      {
      score()->scoreFont()->draw(id, p, magS() * scale, o);
      }

void Element::drawSymbol(SymId id, QPainter* p, const QPointF& o, int n) const
      {
      score()->scoreFont()->draw(id, p, magS(), o, n);
      }

void Element::drawSymbols(const std::vector<SymId>& s, QPainter* p, const QPointF& o, qreal scale) const
      {
      score()->scoreFont()->draw(s, p, magS() * scale, o);
      }

void Element::drawSymbols(const std::vector<SymId>& s, QPainter* p, const QPointF& o, const QSizeF& scale) const
      {
      score()->scoreFont()->draw(s, p, magS() * scale, o);
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
//   symStemDownSW
//---------------------------------------------------------

QPointF Element::symStemDownSW(SymId id) const
      {
      return score()->scoreFont()->stemDownSW(id, magS());
      }

//---------------------------------------------------------
//   symStemUpNW
//---------------------------------------------------------

QPointF Element::symStemUpNW(SymId id) const
      {
      return score()->scoreFont()->stemUpNW(id, magS());
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
//   concertPitch
//---------------------------------------------------------

bool Element::concertPitch() const
      {
      return score()->styleB(Sid::concertPitch);
      }
//---------------------------------------------------------
//   nextElement
//   selects the next score element
//---------------------------------------------------------

Element* Element::nextElement()
      {
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty())
            e = score()->selection().elements().first();
      if (e) {
            switch (e->type()) {
                  case ElementType::SEGMENT: {
                        Segment* s = toSegment(e);
                        return s->nextElement(staffIdx());
                        }
                  case ElementType::MEASURE: {
                        Measure* m = toMeasure(e);
                        return m->nextElementStaff(staffIdx());
                        }
                  case ElementType::CLEF:
                  case ElementType::KEYSIG:
                  case ElementType::TIMESIG:
                  case ElementType::BAR_LINE:
                        return nextSegmentElement();
                  default: {
                        return e->parent()->nextElement();
                        }
                  }
            }
      return nullptr;
      }


//---------------------------------------------------------
//   prevElement
//   selects the previous score element
//---------------------------------------------------------

Element* Element::prevElement()
      {
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty() )
            e = score()->selection().elements().last();
      if (e) {
            switch(e->type()) {
                  case ElementType::SEGMENT: {
                        Segment* s = toSegment(e);
                        return s->prevElement(staffIdx());
                        }
                  case ElementType::MEASURE: {
                        Measure* m = toMeasure(e);
                        return m->prevElementStaff(staffIdx());
                        }
                  case ElementType::CLEF:
                  case ElementType::KEYSIG:
                  case ElementType::TIMESIG:
                  case ElementType::BAR_LINE:
                        return prevSegmentElement();
                  default: {
                        return e->parent()->prevElement();
                        }
                  }
            }
      return nullptr;
      }


//------------------------------------------------------------------------------------------
//   nextSegmentElement
//   This function is used in for the next-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the next one and overrides this method
//------------------------------------------------------------------------------------------

Element* Element::nextSegmentElement()
      {
      Element* p = this;
      while (p) {
            switch (p->type()) {
                  case ElementType::NOTE:
                        if (toNote(p)->chord()->isGrace())
                              break;
                        return p;
                  case ElementType::REST:
                        return p;
                  case ElementType::CHORD: {
                        Chord* c = toChord(p);
                        if (!c->isGrace())
                              return c->notes().back();
                        }
                        break;
                  case ElementType::SEGMENT: {
                        Segment* s = toSegment(p);
                        return s->firstElement(staffIdx());
                        }
                  case ElementType::MEASURE: {
                        Measure* m = toMeasure(p);
                        return m->nextElementStaff(staffIdx());
                        }
                  case ElementType::SYSTEM: {
                        System* sys = toSystem(p);
                        return sys->nextSegmentElement();
                        }
                  default:
                        break;
                  }
            p = p->parent();
            }
      return score()->firstElement();
      }

//------------------------------------------------------------------------------------------
//   prevSegmentElement
//   This function is used in for the prev-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the previous one and overrides this method
//------------------------------------------------------------------------------------------

Element* Element::prevSegmentElement()
      {
      Element* p = this;
      while (p) {
            switch (p->type()) {
                  case ElementType::NOTE:
                        if (toNote(p)->chord()->isGrace())
                              break;
                        return p;
                  case ElementType::REST:
                        return p;
                  case ElementType::CHORD: {
                        Chord* c = toChord(p);
                        if (!c->isGrace())
                              return c->notes().front();
                        }
                        break;
                  case ElementType::SEGMENT: {
                        Segment* s = toSegment(p);
                        return s->lastElement(staffIdx());
                        }
                  case ElementType::MEASURE: {
                        Measure* m = toMeasure(p);
                        return m->prevElementStaff(staffIdx());
                        }
                  case ElementType::SYSTEM: {
                        System* sys = toSystem(p);
                        return sys->prevSegmentElement();
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

bool Element::nextGrip(EditData& ed) const
      {
      int i = int(ed.curGrip) + 1;
      if (i >= ed.grips) {
            ed.curGrip = Grip(0);
            return false;
            }
      ed.curGrip = Grip(i);
      return true;
      }

//---------------------------------------------------------
//   prevGrip
//---------------------------------------------------------

bool Element::prevGrip(EditData& ed) const
      {
      int i = int(ed.curGrip) - 1;
      if (i < 0) {
            ed.curGrip = Grip(ed.grips - 1);
            return false;
            }
      ed.curGrip = Grip(i);
      return true;
      }

//---------------------------------------------------------
//   isUserModified
//    Check if this element was modified by user and
//    therefore must be saved.
//---------------------------------------------------------

bool Element::isUserModified() const
      {
      for (const StyledProperty& spp : *styledProperties()) {
            Pid pid               = spp.pid;
            QVariant val          = getProperty(pid);
            QVariant defaultValue = propertyDefault(pid);

            if (propertyType(pid) == P_TYPE::SP_REAL) {
                  if (qAbs(val.toReal() - defaultValue.toReal()) > 0.0001)    // we don’t care spatium diffs that small
                        return true;
                  }
            else  {
                  if (getProperty(pid) != propertyDefault(pid))
                        return true;
                  }
            }
      for (Pid p : {Pid::VISIBLE, Pid::OFFSET, Pid::COLOR, Pid::Z, Pid::AUTOPLACE}) {
            if (getProperty(p) != propertyDefault(p))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Element::triggerLayout() const
      {
      if (parent())
            score()->setLayout(tick(), staffIdx(), this);
      }

//----------------------------------------------------------------------
//   triggerLayoutAll
//
//   *************************** CAUTION *******************************
//   This causes a layout of the entire score: extremely expensive and
//   likely unnecessary! Consider overriding triggerLayout() instead.
//----------------------------------------------------------------------

void Element::triggerLayoutAll() const
      {
      if (parent())
            score()->setLayoutAll(staffIdx(), this);
      }

//---------------------------------------------------------
//   control
//---------------------------------------------------------

bool EditData::control(bool textEditing) const
      {
      if (textEditing)
            return modifiers & CONTROL_MODIFIER;
      else
            return modifiers & Qt::ControlModifier;
      }

//---------------------------------------------------------
//   clearData
//---------------------------------------------------------

void EditData::clearData()
      {
      qDeleteAll(data);
      data.clear();
      }

//---------------------------------------------------------
//   getData
//---------------------------------------------------------

ElementEditData* EditData::getData(const Element* e) const
      {
      for (ElementEditData* ed : data) {
            if (ed->e == e)
                  return ed;
            }
      return 0;
      }

//---------------------------------------------------------
//   addData
//---------------------------------------------------------

void EditData::addData(ElementEditData* ed)
      {
      data.push_back(ed);
      }

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void Element::drawEditMode(QPainter* p, EditData& ed)
      {
      QPen pen(MScore::defaultColor, 0.0);
      p->setPen(pen);
      for (int i = 0; i < ed.grips; ++i) {
            if (Grip(i) == ed.curGrip)
                  p->setBrush(MScore::frameMarginColor);
            else
                  p->setBrush(Qt::NoBrush);
            p->drawRect(ed.grip[i]);
            }
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void Element::startDrag(EditData& ed)
      {
      if (!isMovable())
            return;
      ElementEditData* eed = new ElementEditData();
      eed->e = this;
      eed->pushProperty(Pid::OFFSET);
      eed->pushProperty(Pid::AUTOPLACE);
      eed->initOffset = offset();
      ed.addData(eed);
      if (ed.modifiers & Qt::AltModifier)
            setAutoplace(false);
      }

//---------------------------------------------------------
//   drag
///   Return update Rect relative to canvas.
//---------------------------------------------------------

QRectF Element::drag(EditData& ed)
      {
      if (!isMovable())
            return QRectF();

      const QRectF r0(canvasBoundingRect());

      const ElementEditData* eed = ed.getData(this);

      const QPointF offset0 = ed.moveDelta + eed->initOffset;
      qreal x = offset0.x();
      qreal y = offset0.y();

      qreal _spatium = spatium();
      if (ed.hRaster) {
            qreal hRaster = _spatium / MScore::hRaster();
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (ed.vRaster) {
            qreal vRaster = _spatium / MScore::vRaster();
            int n = lrint(y / vRaster);
            y = vRaster * n;
            }

      setOffset(QPointF(x, y));
      setOffsetChanged(true);
//      setGenerated(false);

      if (isTextBase()) {         // TODO: check for other types
            //
            // restrict move to page boundaries
            //
            const QRectF r(canvasBoundingRect());
            Page* p = 0;
            Element* e = this;
            while (e) {
                  if (e->isPage()) {
                        p = toPage(e);
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
                        setOffset(QPointF(x, y));
                  }
            }
      return canvasBoundingRect() | r0;
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Element::endDrag(EditData& ed)
      {
      if (!isMovable())
            return;
      ElementEditData* eed = ed.getData(this);
      if (!eed)
            return;
      for (const PropertyData &pd : qAsConst(eed->propertyData)) {
            setPropertyFlags(pd.id, pd.f); // reset initial property flags state
            PropertyFlags f = pd.f;
            if (f == PropertyFlags::STYLED)
                  f = PropertyFlags::UNSTYLED;
            score()->undoPropertyChanged(this, pd.id, pd.data, f);
            setGenerated(false);
            }
      }

//---------------------------------------------------------
//   genericDragAnchorLines
//---------------------------------------------------------

QVector<QLineF> Element::genericDragAnchorLines() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp;
      if (parent()->isSegment()) {
            System* system = toSegment(parent())->measure()->system();
            const int stIdx = staffIdx();
            yp = system ? system->staffCanvasYpage(stIdx) : 0.0;
            if (placement() == Placement::BELOW)
                  yp += system ? system->staff(stIdx)->bbox().height() : 0.0;
            //adjust anchor Y positions to staffType offset
            if (staff())
                yp += staff()->staffTypeForElement(this)->yoffset().val()* spatium();
            }
      else
            yp = parent()->canvasPos().y();
      QPointF p1(xp, yp);
      QLineF anchorLine(p1, canvasPos());
      return { anchorLine };
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Element::updateGrips(EditData& ed) const
      {
      const auto positions(gripsPositions(ed));
      const size_t ngrips = positions.size();
      for (int i = 0; i < int(ngrips); ++i)
            ed.grip[i].translate(positions[i]);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Element::startEdit(EditData& ed)
      {
      ElementEditData* elementData = new ElementEditData();
      elementData->e = this;
      ed.addData(elementData);
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Element::edit(EditData& ed)
      {
      if (ed.key ==  Qt::Key_Home) {
            setOffset(QPoint());
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Element::startEditDrag(EditData& ed)
      {
      ElementEditData* eed = ed.getData(this);
      if (!eed) {
            eed = new ElementEditData();
            eed->e = this;
            ed.addData(eed);
            }
      eed->pushProperty(Pid::OFFSET);
      eed->pushProperty(Pid::AUTOPLACE);
      if (ed.modifiers & Qt::AltModifier)
            setAutoplace(false);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Element::editDrag(EditData& ed)
      {
      score()->addRefresh(canvasBoundingRect());
      setOffset(offset() + ed.delta);
      setOffsetChanged(true);
      score()->addRefresh(canvasBoundingRect());
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void Element::endEditDrag(EditData& ed)
      {
      ElementEditData* eed = ed.getData(this);
      bool changed = false;
      if (eed) {
            for (const PropertyData &pd : qAsConst(eed->propertyData)) {
                  setPropertyFlags(pd.id, pd.f); // reset initial property flags state
                  PropertyFlags f = pd.f;
                  if (f == PropertyFlags::STYLED)
                        f = PropertyFlags::UNSTYLED;
                  if (score()->undoPropertyChanged(this, pd.id, pd.data, f))
                        changed = true;
                  }
            eed->propertyData.clear();
            }
      if (changed) {
            undoChangeProperty(Pid::GENERATED, false);
            }
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Element::endEdit(EditData&)
      {
      }

//---------------------------------------------------------
//   styleP
//---------------------------------------------------------

qreal Element::styleP(Sid idx) const
      {
      return score()->styleP(idx);
      }

//---------------------------------------------------------
//   autoplaceSegmentElement
//---------------------------------------------------------

#if 0
void Element::autoplaceSegmentElement(qreal minDistance)
      {
      if (visible() && autoplace() && parent()) {
            setOffset(QPointF());
            Segment* s        = toSegment(parent());
            Measure* m        = s->measure();
            int si            = staffIdx();
            Shape s1          = m->staffShape(si);
            Shape s2          = shape().translated(s->pos() + pos());

            if (isTextBase()) {
                  // look for collisions in next measures
                  qreal totalWidth = m->width();
                  for (Measure* nm = m->nextMeasure(); nm; nm = nm->nextMeasure()) {
                        if (s2.right() > totalWidth) {
                              s1.add(nm->staffShape(si).translated(QPointF(totalWidth, 0.0)));
                              totalWidth += nm->width();
                              }
                        else
                              break;
                        }

                  // look for collisions in previous measures
                  totalWidth = 0;
                  for (Measure* pm = m->prevMeasure(); pm; pm = pm->prevMeasure()) {
                        if (s2.left() > totalWidth) {
                              s1.add(pm->staffShape(si).translated(QPointF(-(totalWidth + pm->width()), 0.0)));
                              totalWidth += pm->width();
                              }
                        else
                              break;
                        }

                  // actually check for collisions
                  bool intersection = true;
                  qreal totalYOff = 0;
                  while (intersection) {
                        intersection = false;
                        for (ShapeElement se : s1) {
                              if (s2.intersects(se)){
                                    intersection = true;
                                    break;
                                    }
                              }
                        if (! intersection)
                              break;
                        else {
                              qreal yd = -1;
                              if (placeAbove())
                                    yd = 1;
                              totalYOff -= yd;
                              s2.translateY(-yd);
                              }
                        }

                  // margin of 5 to stop slight overlap, hardcoded for now
                  qreal textMarginBottom = 5;
                  s2.translateY(-textMarginBottom);
                  rUserYoffset() = totalYOff - textMarginBottom;

                  m->staffShape(si).add(s2);

                  Shape s3 = s2.translated(QPointF()); // make a copy of s2
                  totalWidth = m->width();
                  for (Measure* nm = m->nextMeasure(); nm; nm = nm->nextMeasure()) {
                        if (s2.right() > totalWidth) {
                              s3.translateX(-totalWidth);
                              nm->staffShape(staffIdx()).add(s3);
                              totalWidth += nm->width();
                              s3 = s2.translated(QPointF()); // reset translation
                              }
                        else
                              break;
                        }

                  s3 = s2.translated(QPointF());
                  totalWidth = 0;
                  for (Measure* pm = m->prevMeasure(); pm; pm = pm->prevMeasure()) {
                        if (s2.left() > totalWidth) {
                              s3.translateX(totalWidth+pm->width());
                              pm->staffShape(staffIdx()).add(s3);
                              totalWidth += pm->width();
                              s3 = s2.translated(QPointF()); // reset translation
                              }
                        else
                              break;
                        }

                  }
            else {
                  // look for collisions in the next measure
                  // if necessary
                  bool cnm = (s2.right() > m->width()) && m->nextMeasure() && m->nextMeasure()->system() == m->system();
                  if (cnm) {
                        Measure* nm = m->nextMeasure();
                        s1.add(nm->staffShape(si).translated(QPointF(m->width(), 0.0)));
                        }
                  qreal d = placeAbove() ? s2.minVerticalDistance(s1) : s1.minVerticalDistance(s2);
                  if (d > -minDistance) {
                        qreal yd = d + minDistance;
                        if (placeAbove())
                              yd *= -1.0;
                        rUserYoffset() = yd;
                        s2.translateY(yd);
                        }
                  m->staffShape(si).add(s2);
                  if (cnm) {
                        Measure* nm = m->nextMeasure();
                        s2.translateX(-m->width());
                        nm->staffShape(staffIdx()).add(s2);
                        }
                  }
            }
      }
#endif

//---------------------------------------------------------
//   setOffsetChanged
//---------------------------------------------------------

void Element::setOffsetChanged(bool v, bool absolute, const QPointF& diff)
      {
      if (v)
            _offsetChanged = absolute ? OffsetChange::ABSOLUTE_OFFSET : OffsetChange::RELATIVE_OFFSET;
      else
            _offsetChanged = OffsetChange::NONE;
      _changedPos = pos() + diff;
      }

//---------------------------------------------------------
//   rebaseOffset
//    calculates new offset for moved elements
//    for drag & other actions that result in absolute position, apply the new offset
//    for nudge & other actions that result in relative adjustment, return the vertical difference
//---------------------------------------------------------

qreal Element::rebaseOffset(bool nox)
      {
      QPointF off = offset();
      QPointF p = _changedPos - pos();
      if (nox)
            p.rx() = 0.0;
      //OffsetChange saveChangedValue = _offsetChanged;

      bool staffRelative = staff() && parent() && !(parent()->isNote() || parent()->isRest());
      if (staffRelative && propertyFlags(Pid::PLACEMENT) != PropertyFlags::NOSTYLE) {
            // check if flipped
            // TODO: elements that support PLACEMENT but not as a styled property (add supportsPlacement() method?)
            // TODO: refactor to take advantage of existing cmdFlip() algorithms
            // TODO: adjustPlacement() (from read206.cpp) on read for 3.0 as well
            QRectF r = bbox().translated(_changedPos);
            qreal staffHeight = staff()->height();
            Element* e = isSpannerSegment() ? toSpannerSegment(this)->spanner() : this;
            bool multi = e->isSpanner() && toSpanner(e)->spannerSegments().size() > 1;
            bool above = e->placeAbove();
            bool flipped = above ? r.top() > staffHeight : r.bottom() < 0.0;
            if (flipped && !multi) {
                  off.ry() += above ? -staffHeight : staffHeight;
                  undoChangeProperty(Pid::OFFSET, off + p);
                  _offsetChanged = OffsetChange::ABSOLUTE_OFFSET;       //saveChangedValue;
                  rypos() += above ? staffHeight : -staffHeight;
                  PropertyFlags pf = e->propertyFlags(Pid::PLACEMENT);
                  if (pf == PropertyFlags::STYLED)
                        pf = PropertyFlags::UNSTYLED;
                  Placement place = above ? Placement::BELOW : Placement::ABOVE;
                  e->undoChangeProperty(Pid::PLACEMENT, int(place), pf);
                  undoResetProperty(Pid::MIN_DISTANCE);
                  // TODO
                  //MuseScoreCore::mscoreCore->updateInspector();
                  return 0.0;
                  }
            }

      if (offsetChanged() == OffsetChange::ABSOLUTE_OFFSET) {
            undoChangeProperty(Pid::OFFSET, off + p);
            _offsetChanged = OffsetChange::ABSOLUTE_OFFSET;             //saveChangedValue;
            // allow autoplace to manage min distance even when not needed
            undoResetProperty(Pid::MIN_DISTANCE);
            return 0.0;
            }

      // allow autoplace to manage min distance even when not needed
      undoResetProperty(Pid::MIN_DISTANCE);
      return p.y();
      }

//---------------------------------------------------------
//   rebaseMinDistance
//    calculates new minDistance for moved elements
//    if necessary, also rebases offset
//    updates md, yd
//    returns true if shape needs to be rebased
//---------------------------------------------------------

bool Element::rebaseMinDistance(qreal& md, qreal& yd, qreal sp, qreal rebase, bool above, bool fix)
      {
      bool rc = false;
      PropertyFlags pf = propertyFlags(Pid::MIN_DISTANCE);
      if (pf == PropertyFlags::STYLED)
            pf = PropertyFlags::UNSTYLED;
      qreal adjustedY = pos().y() + yd;
      qreal diff = _changedPos.y() - adjustedY;
      if (fix) {
            undoChangeProperty(Pid::MIN_DISTANCE, -999.0, pf);
            yd = 0.0;
            }
      else if (!isStyled(Pid::MIN_DISTANCE)) {
            md = (above ? md + yd : md - yd) / sp;
            undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
            yd += diff;
            }
      else {
            // min distance still styled
            // user apparently moved element into skyline
            // but perhaps not really, if performing a relative adjustment
            if (_offsetChanged == OffsetChange::RELATIVE_OFFSET) {
                  // relative movement (cursor): fix only if moving vertically into direction of skyline
                  if ((above && diff > 0.0) || (!above && diff < 0.0)) {
                        // rebase offset
                        QPointF p = offset();
                        p.ry() += rebase;
                        undoChangeProperty(Pid::OFFSET, p);
                        md = (above ? md - diff : md + diff) / sp;
                        undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
                        rc = true;
                        yd = 0.0;
                        }
                  }
            else {
                  // absolute movement (drag): fix unconditionally
                  md = (above ? md + yd : md - yd) / sp;
                  undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
                  yd = 0.0;
                  }
            }
      // TODO
      //MuseScoreCore::mscoreCore->updateInspector();
      return rc;
      }

//---------------------------------------------------------
//   autoplaceSegmentElement
//---------------------------------------------------------

void Element::autoplaceSegmentElement(bool above, bool add)
      {
      // rebase vertical offset on drag
      qreal rebase = 0.0;
      if (offsetChanged() != OffsetChange::NONE)
            rebase = rebaseOffset();

      if (autoplace() && parent()) {
            Segment* s = toSegment(parent());
            Measure* m = s->measure();

            qreal sp = score()->spatium();
            int si = staffIdx();
            if (systemFlag()) {
                  const int firstVis = m->system()->firstVisibleStaff();
                  if (firstVis < score()->nstaves())
                        si = firstVis;
                  }
            else {
                  qreal mag = staff()->mag(this);
                  sp *= mag;
                  }
            qreal minDistance = _minDistance.val() * sp;

            SysStaff* ss = m->system()->staff(si);
            QRectF r = bbox().translated(m->pos() + s->pos() + pos());

            // Adjust bbox Y pos for staffType offset 
            if (staffType()) {
                  qreal stYOffset = staffType()->yoffset().val() * sp;
                  r.translate(0.0, stYOffset);
                  }
            
            SkylineLine sk(!above);
            qreal d;
            if (above) {
                  sk.add(r.x(), r.bottom(), r.width());
                  d = sk.minDistance(ss->skyline().north());
                  }
            else {
                  sk.add(r.x(), r.top(), r.width());
                  d = ss->skyline().south().minDistance(sk);
                  }

            if (d > -minDistance) {
                  qreal yd = d + minDistance;
                  if (above)
                        yd *= -1.0;
                  if (offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                        if (rebaseMinDistance(minDistance, yd, sp, rebase, above, inStaff))
                              r.translate(0.0, rebase);
                        }
                  rypos() += yd;
                  r.translate(QPointF(0.0, yd));
                  }
            if (add && addToSkyline())
                  ss->skyline().add(r);
            }
      setOffsetChanged(false);
      }

//---------------------------------------------------------
//   autoplaceMeasureElement
//---------------------------------------------------------

void Element::autoplaceMeasureElement(bool above, bool add)
      {
      // rebase vertical offset on drag
      qreal rebase = 0.0;
      if (offsetChanged() != OffsetChange::NONE)
            rebase = rebaseOffset();

      if (autoplace() && parent()) {
            Measure* m = toMeasure(parent());
            int si     = staffIdx();

            qreal sp = score()->spatium();
            qreal minDistance = _minDistance.val() * sp;

            SysStaff* ss = m->system()->staff(si);
            // shape rather than bbox is good for tuplets especially
            Shape sh = shape().translated(m->pos() + pos());

            SkylineLine sk(!above);
            qreal d;
            if (above) {
                  sk.add(sh);
                  d = sk.minDistance(ss->skyline().north());
                  }
            else {
                  sk.add(sh);
                  d = ss->skyline().south().minDistance(sk);
                  }
            if (d > -minDistance) {
                  qreal yd = d + minDistance;
                  if (above)
                        yd *= -1.0;
                  if (offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? sh.bottom() + rebase > 0.0 : sh.top() + rebase < staff()->height();
                        if (rebaseMinDistance(minDistance, yd, sp, rebase, above, inStaff))
                              sh.translateY(rebase);
                        }
                  rypos() += yd;
                  sh.translateY(yd);
                  }
            if (add && addToSkyline())
                  ss->skyline().add(sh);
            }
      setOffsetChanged(false);
      }

//---------------------------------------------------------
//   barbeat
//---------------------------------------------------------

std::pair<int, float> Element::barbeat() const
      {
      int bar = 0;
      int beat = 0;
      int ticks = 0;
      TimeSigMap* tsm = this->score()->sigmap();
      const Element* p = this;
      int ticksB = ticks_beat(tsm->timesig(0).timesig().denominator());
      while(p && p->type() != ElementType::SEGMENT && p->type() != ElementType::MEASURE)
            p = p->parent();

      if (!p) {
            return std::pair<int, float>(0, 0.0F);
            }
      else if (p->type() == ElementType::SEGMENT) {
            const Segment* seg = static_cast<const Segment*>(p);
            tsm->tickValues(seg->tick().ticks(), &bar, &beat, &ticks);
            ticksB = ticks_beat(tsm->timesig(seg->tick().ticks()).timesig().denominator());
            }
      else if (p->type() == ElementType::MEASURE) {
            const Measure* m = static_cast<const Measure*>(p);
            bar = m->no();
            beat = -1;
            ticks = 0;
            }
      return std::pair<int,float>(bar + 1, beat + 1 + ticks / static_cast<float>(ticksB));
      }

//---------------------------------------------------------
//   accessibleBarbeat
//---------------------------------------------------------

QString Element::accessibleBarbeat() const
      {
      QString barsAndBeats = "";
      std::pair<int, float>bar_beat = barbeat();
      if (bar_beat.first) {
            barsAndBeats += "; " + QObject::tr("Measure: %1").arg(QString::number(bar_beat.first));
            if (bar_beat.second)
                  barsAndBeats += "; " + QObject::tr("Beat: %1").arg(QString::number(bar_beat.second));
            }
      if (staffIdx() + 1)
            barsAndBeats += "; " + QObject::tr("Staff: %1").arg(QString::number(staffIdx() + 1));
      return barsAndBeats;
      }
}
