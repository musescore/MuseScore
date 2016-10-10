//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/* TO DO:
- XML export

NICE-TO-HAVE TODO:
- draggable handles of glissando segments
- re-attachable glissando extrema (with [Shift]+arrows, use SlurSegment::edit()
      and SlurSegment::changeAnchor() in slur.cpp as models)
*/

#include "arpeggio.h"
#include "glissando.h"
#include "chord.h"
#include "ledgerline.h"
#include "note.h"
#include "notedot.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "style.h"
#include "sym.h"
#include "xml.h"
#include "accidental.h"

namespace Ms {

static const qreal      GLISS_DEFAULT_LINE_TICKNESS   = 0.15;
static const qreal      GLISS_PALETTE_WIDTH           = 4.0;
static const qreal      GLISS_PALETTE_HEIGHT          = 4.0;

//---------------------------------------------------------
//   GlisandoSegment
//---------------------------------------------------------

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void GlissandoSegment::layout()
      {
      if (staff())
            setMag(staff()->mag());
      QRectF r = QRectF(0.0, 0.0, pos2().x(), pos2().y()).normalized();
      qreal lw = spatium() * glissando()->lineWidth().val() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void GlissandoSegment::draw(QPainter* painter) const
      {
      painter->save();
      qreal _spatium = spatium();

      QPen pen(glissando()->curColor());
      pen.setWidthF(glissando()->lineWidth().val() * spatium());
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
//      painter->drawLine(QPointF(), pos2());               // DEBUG

      // rotate painter so that the line become horizontal
      qreal w     = pos2().x();
      qreal h     = pos2().y();
      qreal l     = sqrt(w * w + h * h);
      qreal wi = asin(-h / l) * 180.0 / M_PI;
      qreal scale = painter->worldTransform().m11();
      painter->rotate(-wi);

      if (glissando()->glissandoType() == Glissando::Type::STRAIGHT) {
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
            }
      else if (glissando()->glissandoType() == Glissando::Type::WAVY) {
            QRectF b = symBbox(SymId::wiggleTrill);
            qreal w  = symAdvance(SymId::wiggleTrill);
            int n    = (int)(l / w);      // always round down (truncate) to avoid overlap
            qreal x  = (l - n*w) * 0.5;   // centre line in available space
            std::vector<SymId> ids;
            for (int i = 0; i < n; ++i)
                  ids.push_back(SymId::wiggleTrill);
            // this is very ugly but fix #68846 for now
//            bool tmp = MScore::pdfPrinting;
//            MScore::pdfPrinting = true;
            score()->scoreFont()->draw(ids, painter, magS(), QPointF(x, -(b.y() + b.height()*0.5) ), scale);
//            MScore::pdfPrinting = tmp;
            }
      if (glissando()->showText()) {
            const TextStyle& st = score()->textStyle(TextStyleType::GLISSANDO);
            QRectF r = st.fontMetrics(_spatium).boundingRect(glissando()->text());
            // if text longer than available space, skip it
            if (r.width() < l) {
                  qreal yOffset = r.height() + r.y();       // find text descender height
                  // raise text slightly above line and slightly more with WAVY than with STRAIGHT
                  yOffset += _spatium * (glissando()->glissandoType() == Glissando::Type::WAVY ? 0.4 : 0.1);
                  painter->setFont(st.font(_spatium * MScore::pixelRatio));
                  qreal x = (l - r.width()) * 0.5;
                  painter->drawText(QPointF(x, -yOffset), glissando()->text());
                  }
            }
      painter->restore();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant GlissandoSegment::getProperty(P_ID id) const
      {
      switch (id) {
            // route properties of the whole Glissando element to it
            case P_ID::GLISS_TYPE:
            case P_ID::GLISS_TEXT:
            case P_ID::GLISS_SHOW_TEXT:
            case P_ID::GLISSANDO_STYLE:
            case P_ID::PLAY:
                  return glissando()->getProperty(id);
            default:
                  return LineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool GlissandoSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::GLISS_TYPE:
            case P_ID::GLISS_TEXT:
            case P_ID::GLISS_SHOW_TEXT:
            case P_ID::GLISSANDO_STYLE:
            case P_ID::PLAY:
                  return glissando()->setProperty(id, v);
            default:
                  return LineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant GlissandoSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::GLISS_TYPE:
            case P_ID::GLISS_TEXT:
            case P_ID::GLISS_SHOW_TEXT:
            case P_ID::GLISSANDO_STYLE:
            case P_ID::PLAY:
                  return glissando()->propertyDefault(id);
            default:
                  return LineSegment::propertyDefault(id);
            }
      }

//=========================================================
//   Glissando
//=========================================================

Glissando::Glissando(Score* s)
  : SLine(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);

      _glissandoType = Type::STRAIGHT;
      _text          = "gliss.";
      _showText      = true;
      setDiagonal(true);
      setGlissandoStyle(MScore::GlissandoStyle::CHROMATIC);
      setPlayGlissando(true);
      setLineWidth(Spatium(GLISS_DEFAULT_LINE_TICKNESS));
      setAnchor(Spanner::Anchor::NOTE);
      }

Glissando::Glissando(const Glissando& g)
   : SLine(g)
      {
      _glissandoType = g._glissandoType;
      _glissandoStyle = g._glissandoStyle;
      _playGlissando = g._playGlissando;
      _text          = g._text;
      _showText      = g._showText;
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Glissando::createLineSegment()
      {
      GlissandoSegment* seg = new GlissandoSegment(score());
      seg->setFlag(ElementFlag::ON_STAFF, false);
      seg->setTrack(track());
      seg->setColor(color());
      return seg;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Glissando::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      // don't scan segments belonging to systems; the systems themselves will scan them
      for (SpannerSegment* seg : segments)
            if (!seg->parent() || seg->parent()->type() != Element::Type::SYSTEM)
                  seg->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Glissando::layout()
      {
      qreal _spatium    = spatium();

      if (score() == gscore || !startElement() || !endElement()) {  // for use in palettes or while dragging
            if (spannerSegments().empty())
                  add(createLineSegment());
            LineSegment* s = frontSegment();
            s->setPos(QPointF());
            s->setPos2(QPointF(_spatium * GLISS_PALETTE_WIDTH, -_spatium * GLISS_PALETTE_HEIGHT));
            s->layout();
            return;
            }

      SLine::layout();
      setPos(0.0, 0.0);
      adjustReadPos();

      Note*       anchor1     = static_cast<Note*>(startElement());
      Note*       anchor2     = static_cast<Note*>(endElement());
      Chord*      cr1         = anchor1->chord();
      Chord*      cr2         = anchor2->chord();
      GlissandoSegment*       segm1 = static_cast<GlissandoSegment*>(frontSegment());
      GlissandoSegment*       segm2 = static_cast<GlissandoSegment*>(backSegment());

      // Note: line segments are defined by
      // initial point: ipos() (relative to system origin)
      // ending point:  pos2() (relative to initial point)

      // LINE ENDING POINTS TO NOTEHEAD CENTRES

      // assume gliss. line goes from centre of initial note centre to centre of ending note:
      // move first segment origin and last segment ending point from notehead origin to notehead centre
      QPointF     offs1       = QPointF(anchor1->headWidth() * 0.5, 0.0);
      QPointF     offs2       = QPointF(anchor2->headWidth() * 0.5, 0.0);

      // AVOID HORIZONTAL LINES

      int         upDown      = (0 < (anchor2->pitch() - anchor1->pitch())) - ((anchor2->pitch() - anchor1->pitch()) < 0);
      // on TAB's, glissando are by necessity on the same string, this gives an horizontal glissando line;
      // make bottom end point lower and top ending point higher
      if (cr1->staff()->isTabStaff()) {
                  qreal yOff = cr1->staff()->lineDistance() * 0.4 * _spatium;
                  offs1.ry() += yOff * upDown;
                  offs2.ry() -= yOff * upDown;
            }
      // if not TAB, angle glissando between notes on the same line
      else {
            if (anchor1->line() == anchor2->line()) {
                  offs1.ry() += _spatium * 0.25 * upDown;
                  offs2.ry() -= _spatium * 0.25 * upDown;
                  }
            }

      // move initial point of first segment and adjust its length accordingly
      segm1->setPos (segm1->ipos()  + offs1);
      segm1->setPos2(segm1->ipos2() - offs1);
      // adjust ending point of last segment
      segm2->setPos2(segm2->ipos2() + offs2);

      // FINAL SYSTEM-INITIAL NOTE
      // if the last gliss. segment attaches to a system-initial note, some extra width has to be added
      if (cr2->segment()->measure() == cr2->segment()->system()->firstMeasure() && cr2->rtick() == 0
                  // but ignore graces after, as they are not the first note of the system,
                  // even if their segment is the first segment of the system
                  && !(cr2->noteType() == NoteType::GRACE8_AFTER
                        || cr2->noteType() == NoteType::GRACE16_AFTER || cr2->noteType() == NoteType::GRACE32_AFTER)
                  // also ignore if cr1 is a child of cr2, which means cr1 is a grace-before of cr2
                  && !(cr1->parent() == cr2))
            {
            segm2->rxpos() -= GLISS_STARTOFSYSTEM_WIDTH * _spatium;
            segm2->rxpos2()+= GLISS_STARTOFSYSTEM_WIDTH * _spatium;
            }

      // INTERPOLATION OF INTERMEDIATE POINTS
      // This probably belongs to SLine class itself; currently it does not seem
      // to be needed for anything else than Glissando, though

      // get total x-width and total y-height of all segments
      qreal xTot = 0.0;
      for (SpannerSegment* segm : spannerSegments())
            xTot += segm->ipos2().x();
      qreal y0   = segm1->ipos().y();
      qreal yTot = segm2->ipos().y() + segm2->ipos2().y() - y0;
      qreal ratio = yTot / xTot;
      // interpolate y-coord of intermediate points across total width and height
      qreal xCurr = 0.0;
      qreal yCurr;
      for (int i = 0; i < spannerSegments().count()-1; i++) {
           SpannerSegment* segm = segmentAt(i);
           xCurr += segm->ipos2().x();
           yCurr = y0 + ratio * xCurr;
           segm->rypos2() = yCurr - segm->ipos().y();       // position segm. end point at yCurr
           // next segment shall start where this segment stopped
           segm = segmentAt(i+1);
           segm->rypos2() += segm->ipos().y() - yCurr;      // adjust next segm. vertical length
           segm->rypos() = yCurr;                           // position next segm. start point at yCurr
            }

      // STAY CLEAR OF NOTE APPENDAGES

      // initial note dots / ledger line / notehead
      offs1 *= -1.0;          // discount changes already applied
      int dots = cr1->dots();
      LedgerLine * ledLin = cr1->ledgerLines();
      // if dots, start at right of last dot
      // if no dots, from right of ledger line, if any; from right of notehead, if no ledger line
      offs1.rx() += (dots && anchor1->dot(dots-1) ? anchor1->dot(dots-1)->pos().x() + anchor1->dot(dots-1)->width()
                  : (ledLin ? ledLin->pos().x() + ledLin->width() : anchor1->headWidth()) );

      // final note arpeggio / accidental / ledger line / accidental / arpeggio (i.e. from outermost to innermost)
      offs2 *= -1.0;          // discount changes already applied
      if (Arpeggio* a = cr2->arpeggio())
            offs2.rx() += a->pos().x() + a->userOff().x();
      else if (Accidental* a = anchor2->accidental())
            offs2.rx() += a->pos().x() + a->userOff().x();
      else if ( (ledLin = cr2->ledgerLines()) != nullptr)
            offs2.rx() += ledLin->pos().x();

      // add another a quarter spatium of 'air'
      offs1.rx() += _spatium * 0.25;
      offs2.rx() -= _spatium * 0.25;

      // apply offsets: shorten first segment by x1 (and proportionally y) and adjust its length accordingly
      offs1.ry() = segm1->ipos2().y() * offs1.x() / segm1->ipos2().x();
      segm1->setPos(segm1->ipos() + offs1);
      segm1->setPos2(segm1->ipos2() - offs1);
      // adjust last segment length by x2 (and proportionally y)
      offs2.ry() = segm2->ipos2().y() * offs2.x() / segm2->ipos2().x();
      segm2->setPos2(segm2->ipos2() + offs2);

      for (SpannerSegment* segm : spannerSegments())
            static_cast<GlissandoSegment*>(segm)->layout();

      // compute glissando bbox as the bbox of the last segment, relative to the end anchor note
      QPointF anchor2PagePos = anchor2->pagePos();
      QPointF system2PagePos = cr2->segment()->system()->pagePos();
      QPointF anchor2SystPos = anchor2PagePos - system2PagePos;
      QRectF r = QRectF(anchor2SystPos - segm2->pos(), anchor2SystPos - segm2->pos() - segm2->pos2()).normalized();
      qreal lw = _spatium * lineWidth().val() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Glissando::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      if (_showText && !_text.isEmpty())
            xml.tag("text", _text);
      xml.tag("subtype", int(_glissandoType));
      writeProperty(xml, P_ID::PLAY);
      writeProperty(xml, P_ID::GLISSANDO_STYLE);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Glissando::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      e.addSpanner(e.intAttribute("id", -1), this);

      _showText = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "text") {
                  _showText = true;
                  _text = e.readElementText();
                  }
            else if (tag == "subtype")
                  _glissandoType = Type(e.readInt());
            else if (tag == "glissandoStyle") {
                setProperty(P_ID::GLISSANDO_STYLE, Ms::getProperty(P_ID::GLISSANDO_STYLE, e));
            } else if ( tag == "play") {
                setPlayGlissando(e.readBool());
            }
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   computeStartElement
//---------------------------------------------------------
/*
void Glissando::computeStartElement()
      {
      // if there is already a start note, done.
      if (_startElement != nullptr && _startElement->type() == Element::Type::NOTE)
            return;
      // if neither a start note or an end note, we got a problem!
      if (_endElement == nullptr || _endElement->type() != Element::Type::NOTE) {
            // TODO: no start, no end: we probably should delete this glissando or just abort() ?
            return;
            }

      int         trk   = track();
      Part*       part  = _endElement->part();
      Segment*    segm  = static_cast<Note*>(_endElement)->chord()->segment();

      if (segm != nullptr)
            segm = segm->prev1();
      while (segm) {
            // if previous segment is a ChordRest segment
            if (segm->segmentType() == Segment::Type::ChordRest) {
                  // look for a Chord in the same track and get its top note, if found
                  if (segm->element(trk) && segm->element(trk)->type() == Element::Type::CHORD) {
                        _startElement = static_cast<Chord*>(segm->element(trk))->upNote();
                        _startElement->add(this);
                        return;
                        }
                  // if no chord, look for other chords in the same instrument
                  for (Element* currChord : segm->elist())
                        if (currChord != nullptr && currChord->type() == Element::Type::CHORD
                                    && static_cast<Chord*>(currChord)->part() == part) {
                              _startElement = static_cast<Chord*>(currChord->upNote();
                              _startElement->add(this);
                              return;
                        }
                  }
            segm = segm->prev1();
            }

      // we have a problem! delete this glissando? abort()?
      qDebug("no first note for glissando found");
      }
*/
//---------------------------------------------------------
//   undoSetGlissandoType
//---------------------------------------------------------

void Glissando::undoSetGlissandoType(Type t)
      {
      undoChangeProperty(P_ID::GLISS_TYPE, int(t));
      }

//---------------------------------------------------------
//   undoSetText
//---------------------------------------------------------

void Glissando::undoSetText(const QString& s)
      {
      undoChangeProperty(P_ID::GLISS_TEXT, s);
      }

//---------------------------------------------------------
//   undoSetShowText
//---------------------------------------------------------

void Glissando::undoSetShowText(bool f)
      {
      undoChangeProperty(P_ID::GLISS_SHOW_TEXT, f);
      }

//---------------------------------------------------------
//   STATIC FUNCTIONS: guessInitialNote
//
//    Used while reading old scores (either 1.x or transitional 2.0) to determine (guess!)
//    the glissando initial note from its final chord. Returns the top note of previous chord
//    of the same instrument, preferring the chord in the same track as chord, if it exists.
//
//    CANNOT be called if the final chord and/or its segment do not exist yet in the score
//
//    Parameter:  chord: the chord this glissando ends into
//    Returns:    the top note in a suitable previous chord or nullptr if none found.
//---------------------------------------------------------

Note* Glissando::guessInitialNote(Chord* chord)
      {
      switch (chord->noteType()) {
//            case NoteType::INVALID:
//                  return nullptr;
            // for grace notes before, previous chord is previous chord of parent chord
            case NoteType::ACCIACCATURA:
            case NoteType::APPOGGIATURA:
            case NoteType::GRACE4:
            case NoteType::GRACE16:
            case NoteType::GRACE32:
                  // move unto parent chord and proceed to standard case
                  if (chord->parent() && chord->parent()->type() == Element::Type::CHORD)
                        chord = static_cast<Chord*>(chord->parent());
                  else
                        return nullptr;
                  break;
            // for grace notes after, return top note of parent chord
            case NoteType::GRACE8_AFTER:
            case NoteType::GRACE16_AFTER:
            case NoteType::GRACE32_AFTER:
                  if (chord->parent() && chord->parent()->type() == Element::Type::CHORD)
                        return static_cast<Chord*>(chord->parent())->upNote();
                  else                          // no parent or parent is not a chord?
                        return nullptr;
            case NoteType::NORMAL:
                  {
                  // if chord has grace notes before, the last one is the previous note
                  QVector<Chord*>graces = chord->graceNotesBefore();
                  if (graces.size() > 0)
                        return graces.last()->upNote();
                  }
                  break;                        // else process to standard case
            default:
                  break;
            }

      // standard case (NORMAL or grace before chord)

      // if parent not a segment, can't locate a target note
      if (chord->parent()->type() != Element::Type::SEGMENT)
            return nullptr;

      int         chordTrack  = chord->track();
      Segment*    segm        = chord->segment();
      Part*       part        = chord->part();
      if (segm != nullptr)
            segm = segm->prev1();
      while (segm) {
            // if previous segment is a ChordRest segment
            if (segm->segmentType() == Segment::Type::ChordRest) {
                  Chord* target = nullptr;
                  // look for a Chord in the same track
                  if (segm->element(chordTrack) && segm->element(chordTrack)->type() == Element::Type::CHORD)
                        target = static_cast<Chord*>(segm->element(chordTrack));
                  else              // if no same track, look for other chords in the same instrument
                        for (Element* currChord : segm->elist())
                              if (currChord != nullptr && currChord->type() == Element::Type::CHORD
                                          && static_cast<Chord*>(currChord)->part() == part) {
                                    target = static_cast<Chord*>(currChord);
                                    break;
                                    }
                  // if we found a target previous chord
                  if (target) {
                        // if chord has grace notes after, the last one is the previous note
                        QVector<Chord*>graces = target->graceNotesAfter();
                        if (graces.size() > 0)
                              return graces.last()->upNote();
                        return target->upNote();      // if no grace after, return top note
                        }
                  }
            segm = segm->prev1();
            }
      qDebug("no first note for glissando found");
      return nullptr;
      }

//---------------------------------------------------------
//   STATIC FUNCTIONS: guessFinalNote
//
//    Used while dropping a glissando on a note to determine (guess!) the glissando final
//    note from its initial chord.
//    Returns the top note of next chord of the same instrument,
//    preferring the chord in the same track as chord, if it exists.
//
//    Parameter:  chord: the chord this glissando start from
//    Returns:    the top note in a suitable following chord or nullptr if none found
//---------------------------------------------------------

Note* Glissando::guessFinalNote(Chord* chord)
      {
      switch (chord->noteType()) {
//            case NoteType::INVALID:
//                  return nullptr;
            // for grace notes before, return top note of parent chord
            // TODO : if the grace-before is not the LAST ONE, this still returns the main note
            //    which is probably not correct; however a glissando between two grace notes
            //    probably makes little sense.
            case NoteType::ACCIACCATURA:
            case NoteType::APPOGGIATURA:
            case NoteType::GRACE4:
            case NoteType::GRACE16:
            case NoteType::GRACE32:
                  if (chord->parent() && chord->parent()->type() == Element::Type::CHORD)
                        return static_cast<Chord*>(chord->parent())->upNote();
                  else                          // no parent or parent is not a chord?
                        return nullptr;
            // for grace notes after, next chord is next chord of parent chord
            // TODO : same note as case above!
            case NoteType::GRACE8_AFTER:
            case NoteType::GRACE16_AFTER:
            case NoteType::GRACE32_AFTER:
                  // move unto parent chord and proceed to standard case
                  if (chord->parent() && chord->parent()->type() == Element::Type::CHORD)
                        chord = static_cast<Chord*>(chord->parent());
                  else
                        return nullptr;
                  break;
            case NoteType::NORMAL:
                  {
                  // if chord has grace notes after, the first one is the next note
                  QVector<Chord*>graces = chord->graceNotesAfter();
                  if (graces.size() > 0)
                        return graces.first()->upNote();
                  }
                  break;
            default:
                  break;
            }

      // standard case (NORMAL or grace after chord)

      // if parent not a segment, can't locate a target note
      if (chord->parent()->type() != Element::Type::SEGMENT)
            return nullptr;

      // look for first ChordRest segment after initial note is elapsed
      Segment*    segm        = chord->score()->tick2rightSegment(chord->tick() + chord->actualTicks());
      int         chordTrack  = chord->track();
      Part*       part        = chord->part();
      while (segm) {
            // if next segment is a ChordRest segment
            if (segm->segmentType() == Segment::Type::ChordRest) {
                  Chord* target = nullptr;

                  // look for a Chord in the same track
                  if (segm->element(chordTrack) && segm->element(chordTrack)->type() == Element::Type::CHORD)
                        target = static_cast<Chord*>(segm->element(chordTrack));
                  else              // if no same track, look for other chords in the same instrument
                        for (Element* currChord : segm->elist())
                              if (currChord != nullptr && currChord->type() == Element::Type::CHORD
                                          && static_cast<Chord*>(currChord)->part() == part) {
                                    target = static_cast<Chord*>(currChord);
                                    break;
                                    }

                  // if we found a target next chord
                  if (target) {
                        // if chord has grace notes before, the first one is the next note
                        QVector<Chord*>graces = target->graceNotesBefore();
                        if (graces.size() > 0)
                              return graces.first()->upNote();
                        return target->upNote();      // if no grace before, return top note
                        }
                  }
            segm = segm->next1();
            }
      qDebug("no second note for glissando found");
      return nullptr;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Glissando::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::GLISS_TYPE:
                  return int(glissandoType());
            case P_ID::GLISS_TEXT:
                  return text();
            case P_ID::GLISS_SHOW_TEXT:
                  return showText();
            case P_ID::GLISSANDO_STYLE:
                  return int(glissandoStyle());
            case P_ID::PLAY:
                  return bool(playGlissando());
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Glissando::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::GLISS_TYPE:
                  setGlissandoType(Type(v.toInt()));
                  break;
            case P_ID::GLISS_TEXT:
                  setText(v.toString());
                  break;
            case P_ID::GLISS_SHOW_TEXT:
                  setShowText(v.toBool());
                  break;
            case P_ID::GLISSANDO_STYLE:
                 setGlissandoStyle(MScore::GlissandoStyle(v.toInt()));
                 break;
            case P_ID::PLAY:
                 setPlayGlissando(v.toBool());
                 break;
            default:
                  if (!SLine::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Glissando::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::GLISS_TYPE:
                  return int(Type::STRAIGHT);
            case P_ID::GLISS_TEXT:
                  return "gliss.";
            case P_ID::GLISS_SHOW_TEXT:
                  return true;
            case P_ID::GLISSANDO_STYLE:
                  return int(MScore::GlissandoStyle::CHROMATIC);
            case P_ID::PLAY:
                  return true;
            default:
                  break;
            }
      return SLine::propertyDefault(propertyId);
      }
}

