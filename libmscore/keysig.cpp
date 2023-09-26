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
#include "measure.h"
#include "segment.h"
#include "score.h"
#include "system.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

const char* keyNames[] = {
      QT_TRANSLATE_NOOP("MuseScore", "G major, E minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C♭ major, A♭ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "D major, B minor"),
      QT_TRANSLATE_NOOP("MuseScore", "G♭ major, E♭ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "A major, F♯ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "D♭ major, B♭ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "E major, C♯ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "A♭ major, F minor"),
      QT_TRANSLATE_NOOP("MuseScore", "B major, G♯ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "E♭ major, C minor"),
      QT_TRANSLATE_NOOP("MuseScore", "F♯ major, D♯ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "B♭ major, G minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C♯ major, A♯ minor"),
      QT_TRANSLATE_NOOP("MuseScore", "F major, D minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C major, A minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Open/Atonal")
      };

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Score* s)
  : Element(s, ElementFlag::ON_STAFF)
      {
      _showCourtesy = true;
      _hideNaturals = false;
      }

KeySig::KeySig(const KeySig& k)
   : Element(k)
      {
      _showCourtesy = k._showCourtesy;
      _sig          = k._sig;
      _hideNaturals = false;
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal KeySig::mag() const
      {
      return staff() ? staff()->mag(tick()) : 1.0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void KeySig::addLayout(SymId sym, qreal x, int line)
      {
      qreal stepDistance = staff() ? staff()->lineDistance(tick()) * 0.5 : 0.5;
      KeySym ks;
      ks.sym    = sym;
      ks.spos   = QPointF(x, qreal(line) * stepDistance);
      _sig.keySymbols().append(ks);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout()
      {
      qreal _spatium = spatium();
      setbbox(QRectF());

      if (isCustom() && !isAtonal()) {
            qreal step = _spatium * (staff() ? staff()->staffTypeForElement(this)->lineDistance().val() : 1);
            for (KeySym& ks: _sig.keySymbols()) {
                  ks.pos = QPointF((ks.spos.x() * _spatium), (ks.spos.y() * step));
                  addbbox(symBbox(ks.sym).translated(ks.pos));
                  }
            return;
            }

      _sig.keySymbols().clear();
      if (staff() && !staff()->staffType(tick())->genKeysig())
            return;

      // determine current clef for this staff
      ClefType clef = ClefType::G;
      if (staff()) {
            // Look for a clef before the key signature at the same tick
            Clef* c = nullptr;
            if (segment()) {
                  for (Segment* seg = segment()->prev1(); !c && seg && seg->tick() == tick(); seg = seg->prev1())
                        if (seg->isClefType() || seg->isHeaderClefType())
                              c = toClef(seg->element(track()));
                  }
            if (c)
                  clef = c->clefType();
            else
                  // no clef found, so get the clef type from the clefs list, using the previous tick
                  clef = staff()->clef(tick() - Fraction::fromTicks(1));
            }

      int accidentals = 0, naturals = 0;
      int t1 = int(_sig.key());
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
                  qDebug("illegal t1 key %d", t1);
                  break;
            }

      // manage display of naturals:
      // naturals are shown if there is some natural AND prev. measure has no section break
      // AND style says they are not off
      // OR key sig is CMaj/Amin (in which case they are always shown)

      bool naturalsOn = false;
      Measure* prevMeasure = measure() ? measure()->prevMeasure() : 0;

      // If we're not force hiding naturals (Continuous panel), use score style settings
      if (!_hideNaturals) {
            const bool newSection = (!segment()
               || (segment()->rtick().isZero() && (!prevMeasure || prevMeasure->sectionBreak()))
               );
            naturalsOn = !newSection && (score()->styleI(Sid::keySigNaturals) != int(KeySigNatural::NONE) || (t1 == 0));
            }


      // Don't repeat naturals if shown in courtesy
      if (measure() && measure()->system() && measure()->isFirstInSystem()
          && prevMeasure && prevMeasure->findSegment(SegmentType::KeySigAnnounce, tick())
          && !segment()->isKeySigAnnounceType())
            naturalsOn = false;
      if (track() == -1)
            naturalsOn = false;

      int coffset = 0;
      Key t2      = Key::C;
      if (naturalsOn) {
            if (staff())
                  t2 = staff()->key(tick() - Fraction(1, 480*4));
            if (t2 == Key::C)
                  naturalsOn = false;
            else {
                  switch (qAbs(int(t2))) {
                        case 7: naturals = 0x7f; break;
                        case 6: naturals = 0x3f; break;
                        case 5: naturals = 0x1f; break;
                        case 4: naturals = 0xf;  break;
                        case 3: naturals = 0x7;  break;
                        case 2: naturals = 0x3;  break;
                        case 1: naturals = 0x1;  break;
                        case 0: naturals = 0;    break;
                        default:
                              qDebug("illegal t2 key %d", int(t2));
                              break;
                        }
                  // remove redundant naturals
                  if (!((t1 > 0) ^ (t2 > 0)))
                        naturals &= ~accidentals;
                  if (t2 < 0)
                        coffset = 7;
                  }
            }

      // naturals should go BEFORE accidentals if style says so
      // OR going from sharps to flats or vice versa (i.e. t1 & t2 have opposite signs)

      bool prefixNaturals =
            naturalsOn
            && (score()->styleI(Sid::keySigNaturals) == int(KeySigNatural::BEFORE) || t1 * int(t2) < 0);

      // naturals should go AFTER accidentals if they should not go before!
      bool suffixNaturals = naturalsOn && !prefixNaturals;

      const signed char* lines = ClefInfo::lines(clef);

      // add prefixed naturals, if any

      qreal xo = 0.0;
      if (prefixNaturals) {
            for (int i = 0; i < 7; ++i) {
                  if (naturals & (1 << i)) {
                        addLayout(SymId::accidentalNatural, xo, lines[i + coffset]);
                        xo += 1.0;
                        }
                  }
            }
      // add accidentals
      static const qreal sspread = 1.0;
      static const qreal fspread = 1.0;

      switch(t1) {
            case 7:  addLayout(SymId::accidentalSharp, xo + 6.0 * sspread, lines[6]);
                     // fall through
            case 6:  addLayout(SymId::accidentalSharp, xo + 5.0 * sspread, lines[5]);
                     // fall through
            case 5:  addLayout(SymId::accidentalSharp, xo + 4.0 * sspread, lines[4]);
                     // fall through
            case 4:  addLayout(SymId::accidentalSharp, xo + 3.0 * sspread, lines[3]);
                     // fall through
            case 3:  addLayout(SymId::accidentalSharp, xo + 2.0 * sspread, lines[2]);
                     // fall through
            case 2:  addLayout(SymId::accidentalSharp, xo + 1.0 * sspread, lines[1]);
                     // fall through
            case 1:  addLayout(SymId::accidentalSharp, xo,                 lines[0]);
                     break;
            case -7: addLayout(SymId::accidentalFlat, xo + 6.0 * fspread, lines[13]);
                     // fall through
            case -6: addLayout(SymId::accidentalFlat, xo + 5.0 * fspread, lines[12]);
                     // fall through
            case -5: addLayout(SymId::accidentalFlat, xo + 4.0 * fspread, lines[11]);
                     // fall through
            case -4: addLayout(SymId::accidentalFlat, xo + 3.0 * fspread, lines[10]);
                     // fall through
            case -3: addLayout(SymId::accidentalFlat, xo + 2.0 * fspread, lines[9]);
                     // fall through
            case -2: addLayout(SymId::accidentalFlat, xo + 1.0 * fspread, lines[8]);
                     // fall through
            case -1: addLayout(SymId::accidentalFlat, xo,                 lines[7]);
            case 0:
                  break;
            default:
                  qDebug("illegal t1 key %d", t1);
                  break;
            }
      // add suffixed naturals, if any
      if (suffixNaturals) {
            xo += qAbs(t1);               // skip accidentals
            if (t1 > 0) {                 // after sharps, add a little more space
                  xo += 0.15;
                  // if last sharp (t1) is above next natural (t1+1)...
                  if (lines[t1] < lines[t1+1])
                        xo += 0.2;        // ... add more space
                  }
            for (int i = 0; i < 7; ++i) {
                  if (naturals & (1 << i)) {
                        addLayout(SymId::accidentalNatural, xo, lines[i + coffset]);
                        xo += 1.0;
                        }
                  }
            }

      // Follow stepOffset
      if (staffType())
            rypos() = staffType()->stepOffset() * 0.5 * _spatium;

      // compute bbox
      for (KeySym& ks : _sig.keySymbols()) {
            ks.pos = ks.spos * _spatium;
            addbbox(symBbox(ks.sym).translated(ks.pos));
            }
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void KeySig::draw(QPainter* p) const
      {
      p->setPen(curColor());
      qreal _spatium = spatium();
      int lines = staff() ? staff()->staffTypeForElement(this)->lines() : 5;
      qreal step = spatium() * (staff() ? staff()->staffTypeForElement(this)->lineDistance().val() : 1.0);
      for (const KeySym& ks: _sig.keySymbols()) {
            drawSymbol(ks.sym, p, QPointF(ks.pos.x(), ks.pos.y()));
            // draw ledger lines
            qreal x = ks.pos.x() - ((ks.sym == SymId::accidentalSharp) ? (_spatium * .15) : (_spatium * .25));
            int i = static_cast<int>(ks.pos.y() / step);
            while (i < 0) { // above staff
                  drawSymbol(SymId::legerLine, p, QPointF(x, (i * step)));
                  ++i;
                  }
            while (i >= lines) { // below staff
                  drawSymbol(SymId::legerLine, p, QPointF(x, (i * step)));
                  --i;
                  }
            }
      if (!parent() && (isAtonal() || isCustom()) && _sig.keySymbols().empty()) {
            // empty custom or atonal key signature - draw something for palette
            p->setPen(Qt::gray);
            drawSymbol(SymId::timeSigX, p, QPointF(symWidth(SymId::timeSigX) * -0.5, 2.0 * spatium()));
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(EditData& data) const
      {
      return data.dropElement->type() == ElementType::KEYSIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* KeySig::drop(EditData& data)
      {
      KeySig* ks = toKeySig(data.dropElement);
      if (ks->type() != ElementType::KEYSIG) {
            delete ks;
            return 0;
            }
      KeySigEvent k = ks->keySigEvent();
      delete ks;
      if (data.modifiers & Qt::ControlModifier) {
            // apply only to this stave
            if (!(k == keySigEvent()))
                  score()->undoChangeKeySig(staff(), tick(), k);
            }
      else {
            // apply to all staves:
            foreach(Staff* s, score()->masterScore()->staves())
                  score()->undoChangeKeySig(s, tick(), k);
            }
      return this;
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void KeySig::setKey(Key key)
      {
      KeySigEvent e;
      e.setKey(key);
      setKeySigEvent(e);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void KeySig::write(XmlWriter& xml) const
      {
      xml.stag(this);
      Element::writeProperties(xml);
      if (_sig.isAtonal()) {
            xml.tag("custom", 1);
            }
      else if (_sig.custom()) {
            xml.tag("custom", 1);
            for (const KeySym& ks : _sig.keySymbols()) {
                  xml.stag("KeySym");
                  xml.tag("sym", Sym::id2name(ks.sym));
                  xml.tag("pos", ks.spos);
                  xml.etag();
                  }
            }
      else {
            xml.tag("accidental", int(_sig.key()));
            }
      switch (_sig.mode()) {
            case KeyMode::NONE:       xml.tag("mode", "none"); break;
            case KeyMode::MAJOR:      xml.tag("mode", "major"); break;
            case KeyMode::MINOR:      xml.tag("mode", "minor"); break;
            case KeyMode::DORIAN:     xml.tag("mode", "dorian"); break;
            case KeyMode::PHRYGIAN:   xml.tag("mode", "phrygian"); break;
            case KeyMode::LYDIAN:     xml.tag("mode", "lydian"); break;
            case KeyMode::MIXOLYDIAN: xml.tag("mode", "mixolydian"); break;
            case KeyMode::AEOLIAN:    xml.tag("mode", "aeolian"); break;
            case KeyMode::IONIAN:     xml.tag("mode", "ionian"); break;
            case KeyMode::LOCRIAN:    xml.tag("mode", "locrian"); break;
            case KeyMode::UNKNOWN:
            default:
                  ;
            }
      if (!_showCourtesy)
            xml.tag("showCourtesySig", _showCourtesy);
      if (forInstrumentChange())
            xml.tag("forInstrumentChange", true);
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
                  KeySym ks;
                  while (e.readNextStartElement()) {
                        const QStringRef& t(e.name());
                        if (t == "sym") {
                              QString val(e.readElementText());
                              bool valid;
                              SymId id = SymId(val.toInt(&valid));
                              if (!valid)
                                    id = Sym::name2id(val);
                              if (score()->mscVersion() <= 114) {
                                    if (valid)
                                          id = KeySig::convertFromOldId(val.toInt(&valid));
                                    else
                                          id = Sym::oldName2id(val);
                                    }
                              ks.sym = id;
                              }
                        else if (t == "pos")
                              ks.spos = e.readPoint();
                        else
                              e.unknown();
                        }
                  _sig.keySymbols().append(ks);
                  }
            else if (tag == "showCourtesySig")
                  _showCourtesy = e.readInt();
            else if (tag == "showNaturals")           // obsolete
                  e.readInt();
            else if (tag == "accidental" || tag == "concertKey") // + 4.x compat
                  _sig.setKey(Key(e.readInt()));
            else if (tag == "natural")                // obsolete
                  e.readInt();
            else if (tag == "custom") {
                  e.readInt();
                  _sig.setCustom(true);
                  }
            else if (tag == "mode") {
                  QString m(e.readElementText());
                  if (m == "none")
                        _sig.setMode(KeyMode::NONE);
                  else if (m == "major")
                        _sig.setMode(KeyMode::MAJOR);
                  else if (m == "minor")
                        _sig.setMode(KeyMode::MINOR);
                  else if (m == "dorian")
                        _sig.setMode(KeyMode::DORIAN);
                  else if (m == "phrygian")
                        _sig.setMode(KeyMode::PHRYGIAN);
                  else if (m == "lydian")
                        _sig.setMode(KeyMode::LYDIAN);
                  else if (m == "mixolydian")
                        _sig.setMode(KeyMode::MIXOLYDIAN);
                  else if (m == "aeolian")
                        _sig.setMode(KeyMode::AEOLIAN);
                  else if (m == "ionian")
                        _sig.setMode(KeyMode::IONIAN);
                  else if (m == "locrian")
                        _sig.setMode(KeyMode::LOCRIAN);
                  else
                        _sig.setMode(KeyMode::UNKNOWN);
                  }
            else if (tag == "subtype")
                  subtype = e.readInt();
            else if (tag == "forInstrumentChange")
                  setForInstrumentChange(e.readBool());
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      // for backward compatibility
      if (!_sig.isValid())
            _sig.initFromSubtype(subtype);
      if (_sig.custom() && _sig.keySymbols().empty())
            _sig.setMode(KeyMode::NONE);
      }

//---------------------------------------------------------
//   convertFromOldId
//
//    for import of 1.3 scores
//---------------------------------------------------------

SymId KeySig::convertFromOldId(int val) const
      {
      SymId symId = SymId::noSym;
      switch (val) {
            case 32: symId = SymId::accidentalSharp; break;
            case 33: symId = SymId::accidentalThreeQuarterTonesSharpArrowUp; break;
            case 34: symId = SymId::accidentalQuarterToneSharpArrowDown; break;
            // case 35: // "sharp arrow both" missing in SMuFL
            case 36: symId = SymId::accidentalQuarterToneSharpStein; break;
            case 37: symId = SymId::accidentalBuyukMucennebSharp; break;
            case 38: symId = SymId::accidentalKomaSharp; break;
            case 39: symId = SymId::accidentalThreeQuarterTonesSharpStein; break;
            case 40: symId = SymId::accidentalNatural; break;
            case 41: symId = SymId::accidentalQuarterToneSharpNaturalArrowUp; break;
            case 42: symId = SymId::accidentalQuarterToneFlatNaturalArrowDown; break;
            // case 43: // "natural arrow both" missing in SMuFL
            case 44: symId = SymId::accidentalFlat; break;
            case 45: symId = SymId::accidentalQuarterToneFlatArrowUp; break;
            case 46: symId = SymId::accidentalThreeQuarterTonesFlatArrowDown; break;
            // case 47: // "flat arrow both" missing in SMuFL
            case 48: symId = SymId::accidentalBakiyeFlat; break;
            case 49: symId = SymId::accidentalBuyukMucennebFlat; break;
            case 50: symId = SymId::accidentalThreeQuarterTonesFlatZimmermann; break;
            case 51: symId = SymId::accidentalQuarterToneFlatStein; break;
            // case 52: // "mirrored flat slash" missing in SMuFL
            case 53: symId = SymId::accidentalDoubleFlat; break;
            // case 54: // "flat flat slash" missing in SMuFL
            case 55: symId = SymId::accidentalDoubleSharp; break;
            case 56: symId = SymId::accidentalSori; break;
            case 57: symId = SymId::accidentalKoron; break;
            default:
                  qDebug("MuseScore 1.3 symbol id corresponding to <%d> not found", val);
                  symId = SymId::noSym;
                  break;
            }
      return symId;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool KeySig::operator==(const KeySig& k) const
      {
      return _sig == k._sig;
      }

//---------------------------------------------------------
//   isChange
//---------------------------------------------------------

bool KeySig::isChange() const
      {
      if (!staff())
            return false;
      if (!segment() || segment()->segmentType() != SegmentType::KeySig)
            return false;
      Fraction keyTick = tick();
      return staff()->currentKeyTick(keyTick) == keyTick;
      }

//---------------------------------------------------------
//   changeKeySigEvent
//---------------------------------------------------------

void KeySig::changeKeySigEvent(const KeySigEvent& t)
      {
      if (_sig == t)
            return;
      setKeySigEvent(t);
      }

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void KeySig::undoSetShowCourtesy(bool v)
      {
      undoChangeProperty(Pid::SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   undoSetMode
//---------------------------------------------------------

void KeySig::undoSetMode(KeyMode v)
      {
      undoChangeProperty(Pid::KEYSIG_MODE, int(v));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant KeySig::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::KEY:
                  return int(key());
            case Pid::SHOW_COURTESY:
                  return int(showCourtesy());
            case Pid::KEYSIG_MODE:
                  return int(mode());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool KeySig::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::KEY:
                  if (generated())
                        return false;
                  setKey(Key(v.toInt()));
                  break;
            case Pid::SHOW_COURTESY:
                  if (generated())
                        return false;
                  setShowCourtesy(v.toBool());
                  break;
            case Pid::KEYSIG_MODE:
                  if (generated())
                        return false;
                  setMode(KeyMode(v.toInt()));
                  staff()->setKey(tick(), keySigEvent());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayoutAll();
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant KeySig::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::KEY:
                  return int(Key::INVALID);
            case Pid::SHOW_COURTESY:
                  return true;
            case Pid::KEYSIG_MODE:
                  return int(KeyMode::UNKNOWN);
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* KeySig::nextSegmentElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* KeySig::prevSegmentElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString KeySig::accessibleInfo() const
      {
      QString keySigType;
      if (isAtonal())
            return QString("%1: %2").arg(Element::accessibleInfo(), qApp->translate("MuseScore", keyNames[15]));
      else if (isCustom())
            return QObject::tr("%1: Custom").arg(Element::accessibleInfo());

      if (key() == Key::C)
            return QString("%1: %2").arg(Element::accessibleInfo(), qApp->translate("MuseScore", keyNames[14]));
      int keyInt = static_cast<int>(key());
      if (keyInt < 0)
            keySigType = qApp->translate("MuseScore", keyNames[(7 + keyInt) * 2 + 1]);
      else
            keySigType = qApp->translate("MuseScore", keyNames[(keyInt - 1) * 2]);
      return QString("%1: %2").arg(Element::accessibleInfo(), keySigType);
      }

}
