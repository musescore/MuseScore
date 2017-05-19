//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of class JianpuChord
*/

#include "jianpuchord.h"
#include "jianpunote.h"
#include "jianpuhook.h"
#include "xml.h"
#include "style.h"
#include "segment.h"
#include "text.h"
#include "measure.h"
#include "system.h"
#include "tuplet.h"
#include "tie.h"
#include "arpeggio.h"
#include "score.h"
#include "tremolo.h"
#include "glissando.h"
#include "staff.h"
#include "part.h"
#include "utils.h"
#include "articulation.h"
#include "undo.h"
#include "chordline.h"
#include "lyrics.h"
#include "navigate.h"
#include "stafftype.h"
#include "stem.h"
#include "mscore.h"
#include "accidental.h"
#include "noteevent.h"
#include "pitchspelling.h"
#include "stemslash.h"
#include "ledgerline.h"
#include "drumset.h"
#include "key.h"
#include "sym.h"
#include "stringdata.h"
#include "beam.h"
#include "slur.h"
#include "stafffactory.h"


namespace Ms {

JianpuChord::JianpuChord(Score* s)
   : Chord(s)
      {
      }

JianpuChord::JianpuChord(const Chord& c, bool link, StaffFactory* fac)
   : Chord(c, link, fac)
      {
      }

JianpuChord::JianpuChord(const JianpuChord& c, bool link, StaffFactory* fac)
   : Chord(c, link, fac)
      {
      }

JianpuChord::~JianpuChord()
      {
      }

void JianpuChord::read(XmlReader& xml)
      {
      while (xml.readNextStartElement()) {
            if (!readProperties(xml))
                  xml.unknown();
            }
      }

bool JianpuChord::readProperties(XmlReader& xml)
      {
      const QStringRef& tag(xml.name());

      if (tag == "Note") {
            Note* note = new JianpuNote(score());
            // the note needs to know the properties of the track it belongs to
            note->setTrack(track());
            note->setChord(this);
            note->read(xml);
            add(note);
            }
      else if (ChordRest::readProperties(xml))
            ;
//      else if (tag == "Stem") {
//            Stem* s = new Stem(score());
//            s->read(e);
//            add(s);
//            }
      else if (tag == "Hook") {
            _hook = new JianpuHook(score());
            _hook->read(xml);
            add(_hook);
            }
      else if (tag == "appoggiatura") {
            _noteType = NoteType::APPOGGIATURA;
            xml.readNext();
            }
      else if (tag == "acciaccatura") {
            _noteType = NoteType::ACCIACCATURA;
            xml.readNext();
            }
      else if (tag == "grace4") {
            _noteType = NoteType::GRACE4;
            xml.readNext();
            }
      else if (tag == "grace16") {
            _noteType = NoteType::GRACE16;
            xml.readNext();
            }
      else if (tag == "grace32") {
            _noteType = NoteType::GRACE32;
            xml.readNext();
            }
      else if (tag == "grace8after") {
            _noteType = NoteType::GRACE8_AFTER;
            xml.readNext();
            }
      else if (tag == "grace16after") {
            _noteType = NoteType::GRACE16_AFTER;
            xml.readNext();
            }
      else if (tag == "grace32after") {
            _noteType = NoteType::GRACE32_AFTER;
            xml.readNext();
            }
      else if (tag == "StemSlash") {
            StemSlash* ss = new StemSlash(score());
            ss->read(xml);
            add(ss);
            }
//      else if (tag == "StemDirection")
//            readProperty(e, P_ID::STEM_DIRECTION);
//      else if (tag == "noStem")
//            _noStem = xml.readInt();
      else if (tag == "Arpeggio") {
            _arpeggio = new Arpeggio(score());
            _arpeggio->setTrack(track());
            _arpeggio->read(xml);
            _arpeggio->setParent(this);
            }
      // old glissando format, chord-to-chord, attached to its final chord
      else if (tag == "Glissando") {
            // the measure we are reading is not inserted in the score yet
            // as well as, possibly, the glissando intended initial chord;
            // then we cannot fully link the glissando right now;
            // temporarily attach the glissando to its final note as a back spanner;
            // after the whole score is read, Score::connectTies() will look for
            // the suitable initial note
            Note* finalNote = upNote();
            Glissando* gliss = new Glissando(score());
            gliss->read(xml);
            gliss->setAnchor(Spanner::Anchor::NOTE);
            gliss->setStartElement(nullptr);
            gliss->setEndElement(nullptr);
            gliss->setGlissandoType(GlissandoType::STRAIGHT);
            finalNote->addSpannerBack(gliss);
            }
      else if (tag == "Tremolo") {
            _tremolo = new Tremolo(score());
            _tremolo->setTrack(track());
            _tremolo->read(xml);
            _tremolo->setParent(this);
            }
      else if (tag == "tickOffset")       // obsolete
            ;
      else if (tag == "ChordLine") {
            ChordLine* cl = new ChordLine(score());
            cl->read(xml);
            add(cl);
            }
      else
            return false;
      return true;
      }

void JianpuChord::write(XmlWriter& xml) const
      {
      for (Chord* c : _graceNotes) {
            c->writeBeam(xml);
            c->write(xml);
            }
      xml.stag("Chord");
      ChordRest::writeProperties(xml);
      switch (_noteType) {
            case NoteType::NORMAL:
                  break;
            case NoteType::ACCIACCATURA:
                  xml.tagE("acciaccatura");
                  break;
            case NoteType::APPOGGIATURA:
                  xml.tagE("appoggiatura");
                  break;
            case NoteType::GRACE4:
                  xml.tagE("grace4");
                  break;
            case NoteType::GRACE16:
                  xml.tagE("grace16");
                  break;
            case NoteType::GRACE32:
                  xml.tagE("grace32");
                  break;
            case NoteType::GRACE8_AFTER:
                  xml.tagE("grace8after");
                  break;
            case NoteType::GRACE16_AFTER:
                  xml.tagE("grace16after");
                  break;
            case NoteType::GRACE32_AFTER:
                  xml.tagE("grace32after");
                  break;
            default:
                  break;
            }

//      if (_noStem)
//            xml.tag("noStem", _noStem);
//      else if (_stem && (_stem->isUserModified() || (_stem->userLen() != 0.0)))
//            _stem->write(xml);

      if (_hook && _hook->isUserModified())
            _hook->write(xml);
      if (_stemSlash && _stemSlash->isUserModified())
            _stemSlash->write(xml);
      for (Note* n : _notes)
            n->write(xml);
      if (_arpeggio)
            _arpeggio->write(xml);
      if (_tremolo && tremoloChordType() != TremoloChordType::TremoloSecondNote)
            _tremolo->write(xml);
      for (Element* e : el())
            e->write(xml);
      xml.etag();
      }

void JianpuChord::layoutStem()
      {
      // Note: Jianpu does not have Stems. This function is used only for Hooks.

      //for (Chord* c : _graceNotes)
      //      c->layoutStem();

      if (_beam)
            return;

      // Create hooks for unbeamed chords.
      int hookIdx  = durationType().hooks();
      if (hookIdx && !measure()->slashStyle(staffIdx())) {
            if (!hook()) {
                  Hook* hook = new JianpuHook(score());
                  hook->setParent(this);
                  hook->setGenerated(true);
                  score()->undoAddElement(hook);
                  }
            //hook()->setHookType(hookIdx);
            }
      else if (hook())
            score()->undoRemoveElement(hook());

      if (_hook)
            _hook->layout();

      //if (_stemSlash)
      //      _stemSlash->layout();
      }

void JianpuChord::processSiblings(std::function<void(Element*)> func) const
      {
      if (_hook)
            func(_hook);
      if (_arpeggio)
            func(_arpeggio);
      if (_tremolo)
            func(_tremolo);
      for (Note* note : _notes)
            func(note);
      for (Element* e : el())
            func(e);
      for (Chord* chord : _graceNotes)    // process grace notes last, needed for correct shape calculation
            func(chord);
      }

void JianpuChord::layout()
      {
      // TODO: Currently this is a copy from Chord with some tweaks for Jianpu.
      //       Re-examine the code and optimize it.

      if (_notes.empty())
            return;

      int gi = 0;
      for (Chord* c : _graceNotes) {
            // HACK: graceIndex is not well-maintained on add & remove
            // so rebuild now
            c->setGraceIndex(gi++);
            if (c->isGraceBefore())
                  c->layout();
            }
      QVector<Chord*> graceNotesBefore = Chord::graceNotesBefore();
      int gnb = graceNotesBefore.size();

      // lay out grace notes after separately so they are processed left to right
      // (they are normally stored right to left)

      QVector<Chord*> gna = graceNotesAfter();
      for (Chord* c : gna)
            c->layout();

      qreal _spatium         = spatium();
      qreal _mag             = staff() ? staff()->mag(tick()) : 1.0;    // palette elements do not have a staff
      qreal dotNoteDistance  = score()->styleP(Sid::dotNoteDistance)  * _mag;
      qreal minNoteDistance  = score()->styleP(Sid::minNoteDistance)  * _mag;
      qreal minTieLength     = score()->styleP(Sid::MinTieLength)     * _mag;

      qreal graceMag         = score()->styleD(Sid::graceNoteMag);
      qreal chordX           = (_noteType == NoteType::NORMAL) ? ipos().x() : 0.0;

      qreal lll    = 0.0;         // space to leave at left of chord
      qreal rrr    = 0.0;         // space to leave at right of chord
      qreal lhead  = 0.0;         // amount of notehead to left of chord origin
      Note* upnote = upNote();

      //-----------------------------------------
      //  process notes
      //-----------------------------------------

      int noteCount = _notes.size();
      for (int i = 0; i < noteCount; i++) {
            Note* note = _notes.at(i);
            note->layout();

            // Calculate and set note's position in the chord.
            // Jianpu bar-line span: -4 to +4
            qreal x = 0.0;
            qreal y = JianpuNote::NOTE_BASELINE * spatium() * 0.5;
            for (int j = 1; j <= i; j++)
                  y -= _notes.at(j)->height();
            note->setPos(x, y);

            qreal x1 = note->pos().x() + chordX;
            qreal x2 = x1 + note->width();
            lll      = qMax(lll, -x1);
            rrr      = qMax(rrr, x2);
            // track amount of space due to notehead only
            lhead    = qMax(lhead, -x1);

            Accidental* accidental = note->accidental();
            if (accidental && !note->fixed()) {
                  // convert x position of accidental to segment coordinate system
                  qreal x = accidental->pos().x() + note->pos().x() + chordX;
                  // distance from accidental to note already taken into account
                  // but here perhaps we create more padding in *front* of accidental?
                  x -= score()->styleP(Sid::accidentalDistance) * _mag;
                  lll = qMax(lll, -x);
                  }

            // allow extra space for shortened ties
            // this code must be kept synchronized
            // with the tie positioning code in Tie::slurPos()
            // but the allocation of space needs to be performed here
            Tie* tie;
            tie = note->tieBack();
            if (tie) {
                  tie->calculateDirection();
                  qreal overlap = 0.0;
                  bool shortStart = false;
                  Note* sn = tie->startNote();
                  Chord* sc = sn->chord();
                  if (sc && sc->measure() == measure() && sc == prevChordRest(this)) {
                        if (sc->notes().size() > 1 || (sc->stem() && sc->up() == tie->up())) {
                              shortStart = true;
                              if (sc->width() > sn->width()) {
                                    // chord with second?
                                    // account for noteheads further to right
                                    qreal snEnd = sn->x() + sn->width();
                                    qreal scEnd = snEnd;
                                    for (unsigned i = 0; i < sc->notes().size(); ++i)
                                          scEnd = qMax(scEnd, sc->notes().at(i)->x() + sc->notes().at(i)->width());
                                    overlap += scEnd - snEnd;
                                    }
                              else
                                    overlap -= sn->width() * 0.12;
                              }
                        else
                              overlap += sn->width() * 0.35;
                        if (notes().size() > 1 || (stem() && !up() && !tie->up())) {
                              // for positive offset:
                              //    use available space
                              // for negative x offset:
                              //    space is allocated elsewhere, so don't re-allocate here
                              if (note->ipos().x() != 0.0)
                                    overlap += qAbs(note->ipos().x());
                              else
                                    overlap -= note->width() * 0.12;
                              }
                        else {
                              if (shortStart)
                                    overlap += note->width() * 0.15;
                              else
                                    overlap += note->width() * 0.35;
                              }
                        qreal d = qMax(minTieLength - overlap, 0.0);
                        lll = qMax(lll, d);
                        }
                  }
            }

      if (_arpeggio) {
            qreal arpeggioDistance = score()->styleP(Sid::ArpeggioNoteDistance) * _mag;
            _arpeggio->layout();    // only for width() !
            lll        += _arpeggio->width() + arpeggioDistance + chordX;
            qreal y1   = upnote->pos().y() - upnote->headHeight() * .5;
            _arpeggio->setPos(-lll, y1);
            _arpeggio->adjustReadPos();
            }

      // allocate enough room for glissandi
      if (_endsGlissando) {
            if (rtick()                                     // if not at beginning of measure
                        || graceNotesBefore.size() > 0)     // or there are graces before
                  lll += _spatium * 0.5 + minTieLength;
            // special case of system-initial glissando final note is handled in Glissando::layout() itself
            }

      if (dots()) {
            qreal x = dotPosX() + dotNoteDistance
               + (dots()-1) * score()->styleP(Sid::dotDotDistance) * _mag;
            x += symWidth(SymId::augmentationDot);
            rrr = qMax(rrr, x);
            }

      if (_hook) {
            if (beam())
                  score()->undoRemoveElement(_hook);
            else {
                  _hook->layout();
                  }
            }

      _spaceLw = lll;
      _spaceRw = rrr;

      if (gnb){
              qreal xl = -(_spaceLw + minNoteDistance) - chordX;
              for (int i = gnb-1; i >= 0; --i) {
                    Chord* g = graceNotesBefore.value(i);
                    xl -= g->_spaceRw/* * 1.2*/;
                    g->setPos(xl, 0);
                    xl -= g->_spaceLw + minNoteDistance * graceMag;
                    }
              if (-xl > _spaceLw)
                    _spaceLw = -xl;
              }
       if (!gna.empty()) {
            qreal xr = _spaceRw;
            int n = gna.size();
            for (int i = 0; i <= n - 1; i++) {
                  Chord* g = gna.value(i);
                  xr += g->_spaceLw + g->_spaceRw + minNoteDistance * graceMag;
                  }
           if (xr > _spaceRw)
                 _spaceRw = xr;
           }

      for (Element* e : el()) {
            if (e->type() == ElementType::SLUR)     // we cannot at this time as chordpositions are not fixed
                  continue;
            e->layout();
            if (e->type() == ElementType::CHORDLINE) {
                  QRectF tbbox = e->bbox().translated(e->pos());
                  qreal lx = tbbox.left() + chordX;
                  qreal rx = tbbox.right() + chordX;
                  if (-lx > _spaceLw)
                        _spaceLw = -lx;
                  if (rx > _spaceRw)
                        _spaceRw = rx;
                  }
            }

      for (Note* note : _notes)
            note->layout2();

      QRectF bb;
      processSiblings([&bb] (Element* e) { bb |= e->bbox().translated(e->pos()); } );
      setbbox(bb.translated(_spatium*2, 0));

      //qDebug("bbox x=%.0f y=%.0f w=%.0f h=%.0f", bbox().x(), bbox().y(), bbox().width(), bbox().height());
      //Q_ASSERT(bbox().x() < 20000 && bbox().y() < 20000);
      //Q_ASSERT(bbox().width() < 20000 && bbox().height() < 20000);
      }

//---------------------------------------------------------
//   layout2
//    Called after horizontal positions of all elements
//    are fixed.
//---------------------------------------------------------
void JianpuChord::layout2()
      {
#if 0
      // TODO: Currently this is a copy from Chord with some tweaks for Jianpu.
      //       Re-examine the code and optimize it.

      for (Chord* c : _graceNotes)
            c->layout2();

      qreal _spatium = spatium();
      qreal _mag = staff()->mag();

      //
      // Experimental:
      //    look for colliding ledger lines
      //

      const qreal minDist = _spatium * .17;

      Segment* s = segment()->prev(Segment::Type::ChordRest);
      if (s) {
            int strack = staff2track(staffIdx());
            int etrack = strack + VOICES;

            for (LedgerLine* h = _ledgerLines; h; h = h->next()) {
                  qreal len = h->len();
                  qreal y   = h->y();
                  qreal x   = h->x();
                  bool found = false;
                  qreal cx  = h->measureXPos();

                  for (int track = strack; track < etrack; ++track) {
                        Chord* e = static_cast<Chord*>(s->element(track));
                        if (!e || e->type() != Element::Type::CHORD)
                              continue;
                        for (LedgerLine* ll = e->ledgerLines(); ll; ll = ll->next()) {
                              if (ll->y() != y)
                                    continue;

                              qreal d = cx - ll->measureXPos() - ll->len();
                              if (d < minDist) {
                                    //
                                    // the ledger lines overlap
                                    //
                                    qreal shorten = (minDist - d) * .5;
                                    x   += shorten;
                                    len -= shorten;
                                    ll->setLen(ll->len() - shorten);
                                    h->setLen(len);
                                    h->setPos(x, y);
                                    }
                              found = true;
                              break;
                              }
                        if (found)
                              break;
                        }
                  }
            }

      //
      // position after-chord grace notes
      // room for them has been reserved in JianpuChord::layout()
      //

      QVector<Chord*> gna = graceNotesAfter();
      if (!gna.empty()) {
            qreal minNoteDist = score()->styleP(StyleIdx::minNoteDistance) * _mag * score()->styleD(StyleIdx::graceNoteMag);
            // position grace notes from the rightmost to the leftmost
            // get segment (of whatever type) at the end of this chord; if none, get measure last segment
            Segment* s = measure()->tick2segment(segment()->tick() + actualTicks(), Segment::Type::All);
            if (s == nullptr)
                  s = measure()->last();
            if (s == segment())           // if our segment is the last, no adjacent segment found
                  s = nullptr;
            // start from the right (if next segment found, x of it relative to this chord;
            // chord right space otherwise)
            qreal xOff =  s ? s->pos().x() - (segment()->pos().x() + pos().x()) : _spaceRw;
            // final distance: if near to another chord, leave minNoteDist at right of last grace
            // else leave note-to-barline distance;
            xOff -= (s != nullptr && s->segmentType() != Segment::Type::ChordRest)
                  ? score()->styleP(StyleIdx::noteBarDistance) * _mag
                  : minNoteDist;
            // scan grace note list from the end
            int n = gna.size();
            for (int i = n-1; i >= 0; i--) {
                  Chord* g = gna.value(i);
                  xOff -= g->_spaceRw;                  // move to left by grace note left space (incl. grace own width)
                  g->rxpos() = xOff;
                  xOff -= minNoteDist + g->_spaceLw;    // move to left by grace note right space and inter-grace distance
                  }
            }
#endif
      }

} // namespace Ms
