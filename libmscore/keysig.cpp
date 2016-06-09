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
#include "undo.h"
#include "xml.h"

namespace Ms {

const char* keyNames[] = {
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
      QT_TRANSLATE_NOOP("MuseScore", "F major, D minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C major, A minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Open/Atonal")
      };

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Score* s)
  : Element(s)
      {
      setFlags(ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
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
      return staff() ? staff()->mag() : 1.0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void KeySig::addLayout(SymId sym, qreal x, int line)
      {
      qreal stepDistance = staff() ? staff()->logicalLineDistance() * 0.5 : 0.5;
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
            for (KeySym& ks: _sig.keySymbols()) {
                  ks.pos = ks.spos * _spatium;
                  addbbox(symBbox(ks.sym).translated(ks.pos));
                  }
            return;
            }

      _sig.keySymbols().clear();
      if (staff() && !staff()->genKeySig())     // no key sigs on TAB staves
            return;

      // determine current clef for this staff
      ClefType clef = ClefType::G;
      if (staff())
            clef = staff()->clef(segment()->tick());

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
      if (!_hideNaturals)
            naturalsOn = (prevMeasure && !prevMeasure->sectionBreak()
               && (score()->styleI(StyleIdx::keySigNaturals) != int(KeySigNatural::NONE))) || (t1 == 0);


      // Don't repeat naturals if shown in courtesy
      if (prevMeasure && prevMeasure->findSegment(Segment::Type::KeySigAnnounce, segment()->tick())
          && !segment()->isKeySigAnnounceType())
            naturalsOn = false;
      if (track() == -1)
            naturalsOn = false;

      int coffset = 0;
      Key t2      = Key::C;
      if (naturalsOn) {
            t2 = staff()->key(segment()->tick() - 1);
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
            && (score()->styleI(StyleIdx::keySigNaturals) == int(KeySigNatural::BEFORE) || t1 * int(t2) < 0);

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
            case 6:  addLayout(SymId::accidentalSharp, xo + 5.0 * sspread, lines[5]);
            case 5:  addLayout(SymId::accidentalSharp, xo + 4.0 * sspread, lines[4]);
            case 4:  addLayout(SymId::accidentalSharp, xo + 3.0 * sspread, lines[3]);
            case 3:  addLayout(SymId::accidentalSharp, xo + 2.0 * sspread, lines[2]);
            case 2:  addLayout(SymId::accidentalSharp, xo + 1.0 * sspread, lines[1]);
            case 1:  addLayout(SymId::accidentalSharp, xo,                 lines[0]);
                     break;
            case -7: addLayout(SymId::accidentalFlat, xo + 6.0 * fspread, lines[13]);
            case -6: addLayout(SymId::accidentalFlat, xo + 5.0 * fspread, lines[12]);
            case -5: addLayout(SymId::accidentalFlat, xo + 4.0 * fspread, lines[11]);
            case -4: addLayout(SymId::accidentalFlat, xo + 3.0 * fspread, lines[10]);
            case -3: addLayout(SymId::accidentalFlat, xo + 2.0 * fspread, lines[9]);
            case -2: addLayout(SymId::accidentalFlat, xo + 1.0 * fspread, lines[8]);
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
      for (const KeySym& ks: _sig.keySymbols())
            drawSymbol(ks.sym, p, QPointF(ks.pos.x(), ks.pos.y()));
      if (!parent() && (isAtonal() || isCustom()) && _sig.keySymbols().empty()) {
            // empty custom or atonal key signature - draw something for palette
            p->setPen(Qt::gray);
            drawSymbol(SymId::timeSigX, p, QPointF(symWidth(SymId::timeSigX) * -0.5, 2.0 * spatium()));
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(const DropData& data) const
      {
      return data.element->type() == Element::Type::KEYSIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* KeySig::drop(const DropData& data)
      {
      KeySig* ks = static_cast<KeySig*>(data.element);
      if (ks->type() != Element::Type::KEYSIG) {
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

void KeySig::write(Xml& xml) const
      {
      xml.stag(name());
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
            case KeyMode::NONE:     xml.tag("mode", "none"); break;
            case KeyMode::MAJOR:    xml.tag("mode", "major"); break;
            case KeyMode::MINOR:    xml.tag("mode", "minor"); break;
            case KeyMode::UNKNOWN:
            default:
                  ;
            }
      if (!_showCourtesy)
            xml.tag("showCourtesySig", _showCourtesy);
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
                        const QStringRef& tag(e.name());
                        if (tag == "sym") {
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
                        else if (tag == "pos")
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
            else if (tag == "accidental")
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
                  else
                        _sig.setMode(KeyMode::UNKNOWN);
                  }
            else if (tag == "subtype")
                  subtype = e.readInt();
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
//   changeKeySigEvent
//---------------------------------------------------------

void KeySig::changeKeySigEvent(const KeySigEvent& t)
      {
      if (_sig == t)
            return;
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
//   undoSetShowCourtesy
//---------------------------------------------------------

void KeySig::undoSetShowCourtesy(bool v)
      {
      undoChangeProperty(P_ID::SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant KeySig::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::SHOW_COURTESY: return int(showCourtesy());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool KeySig::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::SHOW_COURTESY:
                  if (generated())
                        return false;
                  setShowCourtesy(v.toBool());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll();
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant KeySig::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SHOW_COURTESY:     return true;
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* KeySig::nextElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* KeySig::prevElement()
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
            return QString("%1: %2").arg(Element::accessibleInfo()).arg(qApp->translate("MuseScore", keyNames[15]));
      else if (isCustom())
            return tr("%1: Custom").arg(Element::accessibleInfo());

      if (key() == Key::C)
            return QString("%1: %2").arg(Element::accessibleInfo()).arg(qApp->translate("MuseScore", keyNames[14]));
      int keyInt = static_cast<int>(key());
      if (keyInt < 0)
            keySigType = qApp->translate("MuseScore", keyNames[(7 + keyInt) * 2 + 1]);
      else
            keySigType = qApp->translate("MuseScore", keyNames[(keyInt - 1) * 2]);
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(keySigType);
      }

}



