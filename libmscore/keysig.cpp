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

#include "sym.h"
#include "staff.h"
#include "clef.h"
#include "keysig.h"
#include "segment.h"
#include "score.h"
#include "undo.h"

namespace Ms {

const char* keyNames[15] = {
      QT_TRANSLATE_NOOP("MuseScore", "G major, E minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Cb major, Ab minor"),
      QT_TRANSLATE_NOOP("MuseScore", "D major, B minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Gb major, Eb minor"),
      QT_TRANSLATE_NOOP("MuseScore", "A major, F# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Db major, Bb minor"),
      QT_TRANSLATE_NOOP("MuseScore", "E major, C# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Ab major, F minor"),
      QT_TRANSLATE_NOOP("MuseScore", "B major, G# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Eb major, C minor"),
      QT_TRANSLATE_NOOP("MuseScore", "F# major, D# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Bb major, G minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C# major, A# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "F major,  D minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C major, A minor")
      };

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
      _showCourtesy = true;
	_showNaturals = true;
      }

KeySig::KeySig(const KeySig& k)
   : Element(k)
      {
	_showCourtesy = k._showCourtesy;
	_showNaturals = k._showNaturals;
	foreach(KeySym* ks, k.keySymbols)
            keySymbols.append(new KeySym(*ks));
      _sig = k._sig;
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal KeySig::mag() const
      {
      return staff() ? staff()->mag() : 1.0;
      }

//---------------------------------------------------------
//   setCustom
//---------------------------------------------------------

void KeySig::setCustom(const QList<KeySym*>& symbols)
      {
      _sig.setCustomType(0);
      keySymbols = symbols;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void KeySig::addLayout(int sym, qreal x, int line)
      {
      KeySym* ks = new KeySym;
      ks->sym    = sym;
      ks->spos   = QPointF(x, qreal(line) * .5);
      keySymbols.append(ks);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout()
      {
      qreal _spatium = spatium();
      setbbox(QRectF());

      if (staff() && !staff()->genKeySig()) {     // no key sigs on TAB staves
            foreach(KeySym* ks, keySymbols)
                  delete ks;
            keySymbols.clear();
            return;
            }

      if (isCustom()) {
            foreach(KeySym* ks, keySymbols) {
                  ks->pos = ks->spos * _spatium;
                  addbbox(symbols[score()->symIdx()][ks->sym].bbox(magS()).translated(ks->pos));
                  }
            return;
            }

      foreach(KeySym* ks, keySymbols)
            delete ks;
      keySymbols.clear();

      // determine current clef for this staff
      ClefType clef = ClefType::G;
      if (staff())
            clef = staff()->clef(segment());

      int t1   = _sig.accidentalType();
      int t2   = _sig.naturalType();
      qreal xo = 0.0;

      // check ranges and compute masks for accidentals and naturals
      int accidentals = 0, naturals = 0;
      switch (qAbs(t1)) {
            case 7: accidentals = 0x7f; break;
            case 6: accidentals = 0x3f; break;
            case 5: accidentals = 0x1f; break;
            case 4: accidentals = 0xf;  break;
            case 3: accidentals = 0x7;  break;
            case 2: accidentals = 0x3;  break;
            case 1: accidentals = 0x1;  break;
            case 0: accidentals = 0;    break;
            default:
                  qDebug("illegal t1 key %d (t2=%d)\n", t1, t2);
                  break;
            }
      switch (qAbs(t2)) {
            case 7: naturals = 0x7f; break;
            case 6: naturals = 0x3f; break;
            case 5: naturals = 0x1f; break;
            case 4: naturals = 0xf;  break;
            case 3: naturals = 0x7;  break;
            case 2: naturals = 0x3;  break;
            case 1: naturals = 0x1;  break;
            case 0: naturals = 0;    break;
            default:
                  qDebug("illegal t2 key %d (t1=%d)\n", t2, t1);
                  break;
            }

      xo = 0.0;
      int coffset = t2 < 0 ? 7 : 0;

      // remove redundant naturals
      if (!((t1 > 0) ^ (t2 > 0)))
            naturals &= ~accidentals;

      // manage display of naturals:
      // naturals are shown if there is some natural AND naturals are on for this key sig
      // AND style says they are not off
      // OR key sig is CMaj/Amin (in which case they are always shown)
      bool naturalsOn =
            t2 != 0 && (_showNaturals
            && (score()->styleI(ST_keySigNaturals) != NAT_NONE || t1 == 0) );
      // naturals shoud go BEFORE accidentals if style says so
      // OR going from sharps to flats or vice versa (i.e. t1 & t2 have opposite signs)
      bool prefixNaturals =
            naturalsOn
            && (score()->styleI(ST_keySigNaturals) == NAT_BEFORE || t1 * t2 < 0);
      // naturals should go AFTER accidentals if they should not go before!
      bool suffixNaturals = naturalsOn && !prefixNaturals;

      const char* lines = ClefInfo::lines(clef);

      // add prefixed naturals, if any
      if (prefixNaturals) {
            for (int i = 0; i < 7; ++i) {
                  if (naturals & (1 << i)) {
                        addLayout(naturalSym, xo, lines[i + coffset]);
                        xo += 1.0;
                        }
                  }
            }
      // add accidentals
      static const qreal sspread = 1.0;
      static const qreal fspread = 1.0;

      switch(t1) {
            case 7:  addLayout(sharpSym, xo + 6.0 * sspread, lines[6]);
            case 6:  addLayout(sharpSym, xo + 5.0 * sspread, lines[5]);
            case 5:  addLayout(sharpSym, xo + 4.0 * sspread, lines[4]);
            case 4:  addLayout(sharpSym, xo + 3.0 * sspread, lines[3]);
            case 3:  addLayout(sharpSym, xo + 2.0 * sspread, lines[2]);
            case 2:  addLayout(sharpSym, xo + 1.0 * sspread, lines[1]);
            case 1:  addLayout(sharpSym, xo,                 lines[0]);
                     break;
            case -7: addLayout(flatSym, xo + 6.0 * fspread, lines[13]);
            case -6: addLayout(flatSym, xo + 5.0 * fspread, lines[12]);
            case -5: addLayout(flatSym, xo + 4.0 * fspread, lines[11]);
            case -4: addLayout(flatSym, xo + 3.0 * fspread, lines[10]);
            case -3: addLayout(flatSym, xo + 2.0 * fspread, lines[9]);
            case -2: addLayout(flatSym, xo + 1.0 * fspread, lines[8]);
            case -1: addLayout(flatSym, xo,                 lines[7]);
            case 0:
                  break;
            default:
                  qDebug("illegal t1 key %d (t2=%d)\n", t1, t2);
                  break;
            }
      // add suffixed naturals, if any
      if (suffixNaturals) {
            xo += qAbs(t1);               // skip accidentals
            if(t1 > 0) {                  // after sharps, add a little more space
                  xo += 0.15;
                  // if last sharp (t1) is above next natural (t1+1)...
                  if (lines[t1] < lines[t1+1])
                        xo += 0.2;        // ... add more space
                  }
            for (int i = 0; i < 7; ++i) {
                  if (naturals & (1 << i)) {
                        addLayout(naturalSym, xo, lines[i + coffset]);
                        xo += 1.0;
                        }
                  }
            }

      // compute bbox
      setbbox(QRectF());
      foreach(KeySym* ks, keySymbols) {
            ks->pos = ks->spos * _spatium;
            addbbox(symbols[score()->symIdx()][ks->sym].bbox(magS()).translated(ks->pos));
            }
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void KeySig::draw(QPainter* p) const
      {
      p->setPen(curColor());
      foreach(const KeySym* ks, keySymbols)
            symbols[score()->symIdx()][ks->sym].draw(p, magS(), QPointF(ks->pos.x(), ks->pos.y()));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == KEYSIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* KeySig::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() != KEYSIG) {
            delete e;
            return 0;
            }

      KeySig* ks    = static_cast<KeySig*>(e);
      KeySigEvent k = ks->keySigEvent();
      if (k.custom() && (score()->customKeySigIdx(ks) == -1))
            score()->addCustomKeySig(ks);
      else
            delete ks;
      if (data.modifiers & Qt::ControlModifier) {
            // apply only to this stave
            if (k != keySigEvent())
                  score()->undoChangeKeySig(staff(), tick(), k);
            }
      else {
            // apply to all staves:
            foreach(Staff* s, score()->staves())
                  score()->undoChangeKeySig(s, tick(), k);
            }
      return this;
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void KeySig::setSig(int old, int newSig)
      {
      KeySigEvent ks;
      ks.setNaturalType(old);
      ks.setAccidentalType(newSig);
      setKeySigEvent(ks);
      }

//---------------------------------------------------------
//   setOldSig
//---------------------------------------------------------

void KeySig::setOldSig(int old)
      {
      _sig.setNaturalType(old);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space KeySig::space() const
      {
      return Space(point(score()->styleS(ST_keysigLeftMargin)), width());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void KeySig::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      if (_sig.custom()) {
            xml.tag("custom", _sig.customType());
            foreach(const KeySym* ks, keySymbols) {
                  xml.stag("KeySym");
                  xml.tag("sym", ks->sym);
                  xml.tag("pos", ks->spos);
                  xml.etag();
                  }
            }
      else {
            xml.tag("accidental", _sig.accidentalType());
            if (_sig.naturalType())
                  xml.tag("natural", _sig.naturalType());
            }
      if (!_showCourtesy)
            xml.tag("showCourtesySig", _showCourtesy);
      if (!_showNaturals)
            xml.tag("showNaturals",    _showNaturals);
	xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void KeySig::read(XmlReader& e)
      {
      _sig = KeySigEvent();   // invalidate _sig
      int subtype = 0;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "KeySym") {
                  KeySym* ks = new KeySym;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "sym")
                              ks->sym = e.readInt();
                        else if (tag == "pos")
                              ks->spos = e.readPoint();
                        else
                              e.unknown();
                        }
                  keySymbols.append(ks);
                  }
            else if (tag == "showCourtesySig")
		      _showCourtesy = e.readInt();
            else if (tag == "showNaturals")
		      _showNaturals = e.readInt();
            else if (tag == "accidental")
                  _sig.setAccidentalType(e.readInt());
            else if (tag == "natural")
                  _sig.setNaturalType(e.readInt());
            else if (tag == "custom")
                  _sig.setCustomType(e.readInt());
            else if (tag == "subtype")
                  subtype = e.readInt();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      if (_sig.invalid() && subtype)
            _sig.initFromSubtype(subtype);     // for backward compatibility
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool KeySig::operator==(const KeySig& k) const
      {
      bool ct1 = customType() != 0;
      bool ct2 = k.customType() != 0;
      if (ct1 != ct2)
            return false;

      if (ct1) {
            int n = keySymbols.size();
            if (n != k.keySymbols.size())
                  return false;
            for (int i = 0; i < n; ++i) {
                  KeySym* ks1 = keySymbols[i];
                  KeySym* ks2 = k.keySymbols[i];
                  if (ks1->sym != ks2->sym)
                        return false;
                  if (ks1->spos != ks2->spos)
                        return false;
                  }
            return true;
            }
      return _sig == k._sig;
      }

//---------------------------------------------------------
//   changeKeySigEvent
//---------------------------------------------------------

void KeySig::changeKeySigEvent(const KeySigEvent& t)
      {
      if (_sig == t)
            return;
      if (t.custom()) {
            KeySig* ks = _score->customKeySig(t.customType());
            if (!ks)
                  return;
            foreach(KeySym* k, keySymbols)
                  delete k;
            keySymbols.clear();
            foreach(KeySym* k, ks->keySymbols)
                  keySymbols.append(new KeySym(*k));
            }
      setKeySigEvent(t);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int KeySig::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   insertIntoKeySigChains
//
//    Adjusts the naturals of this key sig with respect to the previous
//    and the naturals of the next with respect to this one
//
//    Note: staff(), tick() and track() should return sensible values!
//---------------------------------------------------------

void KeySig::insertIntoKeySigChain()
      {
      if (generated())
            return;
      Staff* stf = staff();
      if (!stf)
            return;
      int tck = tick();
      // locate previous key sig to set our delta naturals
      KeySigEvent  oldKey = stf->key(tck-1);
      setOldSig(oldKey.accidentalType());
      // locate the following key sig, if any, to set into it the natural delta with this
      int nextKeyTick = stf->nextKeyTick(tck);
      Segment* nextKeySeg = 0;
      if (nextKeyTick) {
            nextKeySeg = score()->tick2segment(nextKeyTick, false, Segment::SegKeySig);
            while(nextKeySeg && !nextKeySeg->element(track()))
                  nextKeySeg = nextKeySeg->next1(Segment::SegKeySig);
            }
      if (nextKeySeg)
            static_cast<KeySig*>(nextKeySeg->elist()[track()])->setOldSig(_sig.accidentalType());
      }

//---------------------------------------------------------
//   removeFromKeySigChains
//
//    Adjusts the naturals of the next key sig with respect to the previous
//
//    Note: staff(), tick() and track() should return sensible values!
//---------------------------------------------------------

void KeySig::removeFromKeySigChain()
      {
      if (generated())
            return;
      Staff* stf = staff();
      if (!stf)
            return;
      int tck = tick();
      // locate previous and next key sig
      KeySigEvent  oldKey = stf->key(tck-1);
      // locate the following key sig, if any, to set into it the natural delta with this
      int nextKeyTick = stf->nextKeyTick(tck);
      Segment* nextKeySeg = 0;
      if (nextKeyTick) {
            nextKeySeg = score()->tick2segment(nextKeyTick, false, Segment::SegKeySig);
            while(nextKeySeg && !nextKeySeg->element(track()))
                  nextKeySeg = nextKeySeg->next1(Segment::SegKeySig);
            }
      if (nextKeySeg)
            static_cast<KeySig*>(nextKeySeg->elist()[track()])->setOldSig(oldKey.accidentalType());
      }

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void KeySig::undoSetShowCourtesy(bool v)
      {
      score()->undoChangeProperty(this, P_SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant KeySig::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SHOW_COURTESY: return int(showCourtesy());
            case P_SHOW_NATURALS: return int(showNaturals());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool KeySig::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SHOW_COURTESY:
                  setShowCourtesy(v.toBool());
                  break;
            case P_SHOW_NATURALS:
                  setShowNaturals(v.toBool());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant KeySig::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_SHOW_COURTESY:      return true;
            case P_SHOW_NATURALS:      return true;
            default:                   return Element::propertyDefault(id);
            }
      }

}



