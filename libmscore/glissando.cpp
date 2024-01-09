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

#include "global/log.h"

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
#include "measure.h"
#include "style.h"
#include "sym.h"
#include "xml.h"
#include "accidental.h"
#include "utils.h"

namespace Ms {

static const ElementStyle glissandoElementStyle {
      { Sid::glissandoFontFace,                  Pid::FONT_FACE               },
      { Sid::glissandoFontSize,                  Pid::FONT_SIZE               },
      { Sid::glissandoFontStyle,                 Pid::FONT_STYLE              },
      { Sid::glissandoLineWidth,                 Pid::LINE_WIDTH              },
      { Sid::glissandoText,                      Pid::GLISS_TEXT              },
      };

static const qreal      GLISS_PALETTE_WIDTH           = 4.0;
static const qreal      GLISS_PALETTE_HEIGHT          = 4.0;

const std::array<const char *, 2> Glissando::glissandoTypeNames = {
      QT_TRANSLATE_NOOP("Palette", "Straight glissando"),
      QT_TRANSLATE_NOOP("Palette", "Wavy glissando")
      };

//---------------------------------------------------------
//   GlisandoSegment
//---------------------------------------------------------

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void GlissandoSegment::layout()
      {
      if (staff())
            setMag(staff()->mag(tick()));
      QRectF r = QRectF(0.0, 0.0, pos2().x(), pos2().y()).normalized();
      qreal lw = glissando()->lineWidth() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void GlissandoSegment::draw(QPainter* painter) const
      {
      painter->save();
      qreal _spatium = spatium();

      QPen pen(curColor(visible(), glissando()->lineColor()));
      pen.setWidthF(glissando()->lineWidth());
      pen.setCapStyle(Qt::FlatCap);
      painter->setPen(pen);

      // rotate painter so that the line become horizontal
      qreal w     = pos2().x();
      qreal h     = pos2().y();
      qreal l     = sqrt(w * w + h * h);
      qreal wi    = asin(-h / l) * 180.0 / M_PI;
      qreal scale = painter->worldTransform().m11();
      painter->rotate(-wi);

      if (glissando()->glissandoType() == GlissandoType::STRAIGHT) {
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
            }
      else if (glissando()->glissandoType() == GlissandoType::WAVY) {
            QRectF b = symBbox(SymId::wiggleTrill);
            qreal a  = symAdvance(SymId::wiggleTrill);
            int n    = static_cast<int>(l / a);      // always round down (truncate) to avoid overlap
            qreal x  = (l - n*a) * 0.5;   // centre line in available space
            std::vector<SymId> ids;
            for (int i = 0; i < n; ++i)
                  ids.push_back(SymId::wiggleTrill);
            // this is very ugly but fix #68846 for now
            bool tmp = MScore::pdfPrinting;
            MScore::pdfPrinting = true;
            score()->scoreFont()->draw(ids, painter, magS(), QPointF(x, -(b.y() + b.height()*0.5) ), scale);
            MScore::pdfPrinting = tmp;
            }

      if (glissando()->showText()) {
            QFont f(glissando()->fontFace());
            f.setPointSizeF(glissando()->fontSize() * MScore::pixelRatio * _spatium / SPATIUM20);
            f.setBold(glissando()->fontStyle() & FontStyle::Bold);
            f.setItalic(glissando()->fontStyle() & FontStyle::Italic);
            f.setUnderline(glissando()->fontStyle() & FontStyle::Underline);
            f.setStrikeOut(glissando()->fontStyle() & FontStyle::Strike);
            QFontMetricsF fm(f, painter->device()); // use the QPaintDevice, otherwise calculations will be done in screen metrics
            QRectF r = fm.boundingRect(glissando()->text());

            // if text longer than available space, skip it
            if (r.width() < l) {
                  qreal yOffset = r.height() + r.y();       // find text descender height
                  // raise text slightly above line and slightly more with WAVY than with STRAIGHT
                  yOffset += _spatium * (glissando()->glissandoType() == GlissandoType::WAVY ? 0.4 : 0.1);
                  painter->setFont(f);
                  qreal x = (l - r.width()) * 0.5;
                  painter->drawText(QPointF(x, -yOffset), glissando()->text());
                  }
            }
      painter->restore();
      }

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

Element* GlissandoSegment::propertyDelegate(Pid pid)
      {
      switch (pid) {
            case Pid::GLISS_TYPE:
            case Pid::GLISS_TEXT:
            case Pid::GLISS_SHOW_TEXT:
            case Pid::GLISS_STYLE:
            case Pid::GLISS_EASEIN:
            case Pid::GLISS_EASEOUT:
            case Pid::PLAY:
            case Pid::FONT_FACE:
            case Pid::FONT_SIZE:
            case Pid::FONT_STYLE:
            case Pid::LINE_WIDTH:
                  return glissando();
            default:
                  return LineSegment::propertyDelegate(pid);
            }
      }

//=========================================================
//   Glissando
//=========================================================

Glissando::Glissando(Score* s)
   : SLine(s, ElementFlag::MOVABLE)
      {
      setAnchor(Spanner::Anchor::NOTE);
      setDiagonal(true);

      initElementStyle(&glissandoElementStyle);

      resetProperty(Pid::GLISS_SHOW_TEXT);
      resetProperty(Pid::PLAY);
      resetProperty(Pid::GLISS_STYLE);
      resetProperty(Pid::GLISS_TYPE);
      resetProperty(Pid::GLISS_TEXT);
      resetProperty(Pid::GLISS_EASEIN);
      resetProperty(Pid::GLISS_EASEOUT);
      }

Glissando::Glissando(const Glissando& g)
   : SLine(g)
      {
      _text           = g._text;
      _fontFace       = g._fontFace;
      _fontSize       = g._fontSize;
      _glissandoType  = g._glissandoType;
      _glissandoStyle = g._glissandoStyle;
      _easeIn         = g._easeIn;
      _easeOut        = g._easeOut;
      _showText       = g._showText;
      _playGlissando  = g._playGlissando;
      _fontStyle      = g._fontStyle;
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Glissando::createLineSegment()
      {
      GlissandoSegment* seg = new GlissandoSegment(this, score());
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
      for (SpannerSegment* seg : spannerSegments()) {
            if (!seg->parent() || !seg->parent()->isSystem())
                  seg->scanElements(data, func, all);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Glissando::layout()
      {
      qreal _spatium = spatium();

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
      if (spannerSegments().empty()) {
            qDebug("no segments");
            return;
            }
      setPos(0.0, 0.0);

      Note*       anchor1     = toNote(startElement());
      Note*       anchor2     = toNote(endElement());
      Chord*      cr1         = anchor1->chord();
      Chord*      cr2         = anchor2->chord();
      GlissandoSegment* segm1 = toGlissandoSegment(frontSegment());
      GlissandoSegment* segm2 = toGlissandoSegment(backSegment());

      // Note: line segments are defined by
      // initial point: ipos() (relative to system origin)
      // ending point:  pos2() (relative to initial point)

      // LINE ENDING POINTS TO NOTEHEAD CENTRES

      // assume gliss. line goes from centre of initial note centre to centre of ending note:
      // move first segment origin and last segment ending point from notehead origin to notehead centre
      // For TAB: begin at the right-edge of initial note rather than centre
      QPointF offs1 = (cr1->staff()->isTabStaff(cr1->tick())) ? QPointF(anchor1->bbox().right(), 0.0) : QPointF(anchor1->headWidth() * 0.5, 0.0);
      QPointF offs2 = QPointF(anchor2->headWidth() * 0.5, 0.0);

      // AVOID HORIZONTAL LINES

      int upDown = (0 < (anchor2->pitch() - anchor1->pitch())) - ((anchor2->pitch() - anchor1->pitch()) < 0);
      // on TAB's, glissando are by necessity on the same string, this gives an horizontal glissando line;
      // make bottom end point lower and top ending point higher
      if (cr1->staff()->isTabStaff(cr1->tick())) {
            qreal yOff = cr1->staff()->lineDistance(cr1->tick()) * 0.4 * _spatium;
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
      if (cr2->segment()->measure()->isFirstInSystem() && cr2->rtick().isZero()
         // but ignore graces after, as they are not the first note of the system,
         // even if their segment is the first segment of the system
         && !(cr2->noteType() == NoteType::GRACE8_AFTER
            || cr2->noteType() == NoteType::GRACE16_AFTER || cr2->noteType() == NoteType::GRACE32_AFTER)
         // also ignore if cr1 is a child of cr2, which means cr1 is a grace-before of cr2
         && !(cr1->parent() == cr2)) {
            // in theory we should be reserving space for the gliss prior to the first note of a system
            // but in practice we are not (and would be difficult to get right in current layout algorithms)
            // so, a compromise is to at least use the available space to the left -
            // the default layout for lines left a margin after the header
            segm2->rxpos() -= _spatium;
            segm2->rxpos2()+= _spatium;
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
      yTot -= yStaffDifference(segm2->system(), track2staff(track2()), segm1->system(), track2staff(track()));
      qreal ratio = yTot / xTot;
      // interpolate y-coord of intermediate points across total width and height
      qreal xCurr = 0.0;
      qreal yCurr;
      for (unsigned i = 0; i + 1 < spannerSegments().size(); i++) {
            SpannerSegment* segm = segmentAt(i);
            xCurr += segm->ipos2().x();
            yCurr = y0 + ratio * xCurr;
            segm->rypos2() = yCurr - segm->ipos().y();       // position segm. end point at yCurr
            // next segment shall start where this segment stopped, corrected for the staff y-difference
            SpannerSegment* nextSeg = segmentAt(i + 1);
            yCurr += yStaffDifference(nextSeg->system(), track2staff(track2()), segm->system(), track2staff(track()));
            segm = nextSeg;
            segm->rypos2() += segm->ipos().y() - yCurr;      // adjust next segm. vertical length
            segm->rypos() = yCurr;                           // position next segm. start point at yCurr
            }

      // STAY CLEAR OF NOTE APPENDAGES

      // initial note dots / ledger line / notehead
      offs1 *= -1.0;          // discount changes already applied
      int dots = anchor1->dots().size();
      LedgerLine * ledLin = cr1->ledgerLines();

      // If TAB: completely zero first offset since it was already applied as right edge of first note
      if (cr1->staff()->isTabStaff(cr1->tick()))
            offs1.rx() = 0.0;
      // if dots, start at right of last dot
      // if no dots, from right of ledger line, if any; from right of notehead, if no ledger line
      else offs1.rx() += (dots && anchor1->dot(dots-1) ? anchor1->dot(dots-1)->pos().x() + anchor1->dot(dots-1)->width()
                       : (ledLin ? ledLin->pos().x() + ledLin->width() : anchor1->headWidth()) );

      // final note arpeggio / accidental / ledger line / accidental / arpeggio (i.e. from outermost to innermost)
      offs2 *= -1.0;          // discount changes already applied
      if (Arpeggio* ap = cr2->arpeggio())
            offs2.rx() += ap->pos().x() + ap->offset().x();
      else if (Accidental* ac = anchor2->accidental())
            offs2.rx() += ac->pos().x() + ac->offset().x();
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
            segm->layout();

      // compute glissando bbox as the bbox of the last segment, relative to the end anchor note
      QPointF anchor2PagePos = anchor2->pagePos();
      QPointF system2PagePos;
      IF_ASSERT_FAILED(cr2->segment()->system()) {
            system2PagePos = segm2->pos();
            }
      else {
            system2PagePos = cr2->segment()->system()->pagePos();
            }

      QPointF anchor2SystPos = anchor2PagePos - system2PagePos;
      QRectF r = QRectF(anchor2SystPos - segm2->pos(), anchor2SystPos - segm2->pos() - segm2->pos2()).normalized();
      qreal lw = lineWidth() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Glissando::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      if (_showText && !_text.isEmpty())
            xml.tag("text", _text);

      for (auto id : { Pid::GLISS_TYPE, Pid::PLAY, Pid::GLISS_STYLE, Pid::GLISS_EASEIN, Pid::GLISS_EASEOUT })
            writeProperty(xml, id);
      for (const StyledProperty& spp : *styledProperties())
            writeProperty(xml, spp.pid);

      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Glissando::read(XmlReader& e)
      {
      eraseSpannerSegments();

      if (score()->mscVersion() < 301)
            e.addSpanner(e.intAttribute("id", -1), this);

      _showText = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "text") {
                  _showText = true;
                  readProperty(e, Pid::GLISS_TEXT);
                  }
            else if (tag == "subtype")
                  _glissandoType = GlissandoType(e.readInt());
            else if (tag == "glissandoStyle")
                  readProperty(e, Pid::GLISS_STYLE);
            else if (tag == "easeInSpin")
                  _easeIn = e.readInt();
            else if (tag == "easeOutSpin")
                  _easeOut = e.readInt();
            else if (tag == "play")
                  setPlayGlissando(e.readBool());
            else if (readStyledProperty(e, tag))
                  ;
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
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
//                  return 0;
            // for grace notes before, previous chord is previous chord of parent chord
            case NoteType::ACCIACCATURA:
            case NoteType::APPOGGIATURA:
            case NoteType::GRACE4:
            case NoteType::GRACE16:
            case NoteType::GRACE32:
                  // move unto parent chord and proceed to standard case
                  if (chord->parent() && chord->parent()->isChord())
                        chord = toChord(chord->parent());
                  else
                        return 0;
                  break;
            // for grace notes after, return top note of parent chord
            case NoteType::GRACE8_AFTER:
            case NoteType::GRACE16_AFTER:
            case NoteType::GRACE32_AFTER:
                  if (chord->parent() && chord->parent()->isChord())
                        return toChord(chord->parent())->upNote();
                  else                          // no parent or parent is not a chord?
                        return nullptr;
            case NoteType::NORMAL:
                  {
                  // if chord has grace notes before, the last one is the previous note
                  QVector<Chord*> graces = chord->graceNotesBefore();
                  if (graces.size() > 0)
                        return graces.last()->upNote();
                  }
                  break;                        // else process to standard case
            default:
                  break;
            }

      // standard case (NORMAL or grace before chord)

      // if parent not a segment, can't locate a target note
      if (!chord->parent()->isSegment())
            return 0;

      int         chordTrack  = chord->track();
      Segment*    segm        = chord->segment();
      Part*       part        = chord->part();
      if (segm != nullptr)
            segm = segm->prev1();
      while (segm) {
            // if previous segment is a ChordRest segment
            if (segm->segmentType() == SegmentType::ChordRest) {
                  Chord* target = nullptr;
                  // look for a Chord in the same track
                  if (segm->element(chordTrack) && segm->element(chordTrack)->isChord())
                        target = toChord(segm->element(chordTrack));
                  else {             // if no same track, look for other chords in the same instrument
                        for (Element* currChord : segm->elist()) {
                              if (currChord && currChord->isChord() && toChord(currChord)->part() == part) {
                                    target = toChord(currChord);
                                    break;
                                    }
                              }
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
      return 0;
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
                  if (chord->parent() && chord->parent()->isChord())
                        return toChord(chord->parent())->upNote();
                  else                          // no parent or parent is not a chord?
                        return nullptr;
            // for grace notes after, next chord is next chord of parent chord
            // TODO : same note as case above!
            case NoteType::GRACE8_AFTER:
            case NoteType::GRACE16_AFTER:
            case NoteType::GRACE32_AFTER:
                  // move unto parent chord and proceed to standard case
                  if (chord->parent() && chord->parent()->isChord())
                        chord = toChord(chord->parent());
                  else
                        return 0;
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
      if (!chord->parent()->isSegment())
            return 0;

      // look for first ChordRest segment after initial note is elapsed
      Segment*    segm        = chord->score()->tick2rightSegment(chord->tick() + chord->actualTicks());
      int         chordTrack  = chord->track();
      Part*       part        = chord->part();
      while (segm) {
            // if next segment is a ChordRest segment
            if (segm->segmentType() == SegmentType::ChordRest) {
                  Chord* target = nullptr;

                  // look for a Chord in the same track
                  if (segm->element(chordTrack) && segm->element(chordTrack)->isChord())
                        target = toChord(segm->element(chordTrack));
                  else {              // if no same track, look for other chords in the same instrument
                        for (Element* currChord : segm->elist()) {
                              if (currChord && currChord->isChord() && toChord(currChord)->part() == part) {
                                    target = toChord(currChord);
                                    break;
                                    }
                              }
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
      return 0;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Glissando::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::GLISS_TYPE:
                  return int(glissandoType());
            case Pid::GLISS_TEXT:
                  return text();
            case Pid::GLISS_SHOW_TEXT:
                  return showText();
            case Pid::GLISS_STYLE:
                  return int(glissandoStyle());
            case Pid::GLISS_EASEIN:
                  return easeIn();
            case Pid::GLISS_EASEOUT:
                  return easeOut();
            case Pid::PLAY:
                  return bool(playGlissando());
            case Pid::FONT_FACE:
                  return _fontFace;
            case Pid::FONT_SIZE:
                  return _fontSize;
            case Pid::FONT_STYLE:
                  return int(_fontStyle);
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Glissando::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::GLISS_TYPE:
                  setGlissandoType(GlissandoType(v.toInt()));
                  break;
            case Pid::GLISS_TEXT:
                  setText(v.toString());
                  break;
            case Pid::GLISS_SHOW_TEXT:
                  setShowText(v.toBool());
                  break;
            case Pid::GLISS_STYLE:
                  setGlissandoStyle(GlissandoStyle(v.toInt()));
                  break;
            case Pid::GLISS_EASEIN:
                  setEaseIn(v.toInt());
                  break;
            case Pid::GLISS_EASEOUT:
                  setEaseOut(v.toInt());
                  break;
            case Pid::PLAY:
                  setPlayGlissando(v.toBool());
                  break;
            case Pid::FONT_FACE:
                  setFontFace(v.toString());
                  break;
            case Pid::FONT_SIZE:
                  setFontSize(v.toReal());
                  break;
            case Pid::FONT_STYLE:
                  setFontStyle(FontStyle(v.toInt()));
                  break;
            default:
                  if (!SLine::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Glissando::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::GLISS_TYPE:
                  return int(GlissandoType::STRAIGHT);
            case Pid::GLISS_SHOW_TEXT:
                  return true;
            case Pid::GLISS_STYLE:
                  return int(GlissandoStyle::CHROMATIC);
            case Pid::GLISS_EASEIN:
            case Pid::GLISS_EASEOUT:
                  return 0;
            case Pid::PLAY:
                  return true;
            default:
                  break;
            }
      return SLine::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   Glissando::propertyId
//---------------------------------------------------------

Pid Glissando::propertyId(const QStringRef& name) const
      {
      if (name == propertyName(Pid::GLISS_TYPE))
            return Pid::GLISS_TYPE;
      return SLine::propertyId(name);
      }
}

