//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "accidental.h"
#include "arpeggio.h"
#include "articulation.h"
#include "bend.h"
#include "chord.h"
#include "chordline.h"
#include "figuredbass.h"
#include "fingering.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "hook.h"
#include "image.h"
#include "instrchange.h"
#include "jump.h"
#include "letring.h"
#include "marker.h"
#include "measure.h"
#include "notedot.h"
#include "palmmute.h"
#include "pedal.h"
#include "rest.h"
#include "slur.h"
#include "stafftext.h"
#include "stafftypechange.h"
#include "tempotext.h"
#include "textline.h"
#include "tie.h"
#include "trill.h"
#include "undo.h"
#include "utils.h"
#include "vibrato.h"
#include "volta.h"
#include "stem.h"
#include "stemslash.h"
#include "timesig.h"
#include "tuplet.h"
#include "xml.h"
#include "score.h"
#include "staff.h"
#include "revisions.h"
#include "part.h"
#include "page.h"
#include "style.h"
#include "sym.h"
#include "audio.h"
#include "sig.h"
#include "barline.h"
#include "excerpt.h"
#include "spanner.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

namespace Ms {

//---------------------------------------------------------
//   Clef::read300old
//---------------------------------------------------------

void Clef::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "concertClefType")
                  _clefTypes._concertClef = Clef::clefType(e.readElementText());
            else if (tag == "transposingClefType")
                  _clefTypes._transposingClef = Clef::clefType(e.readElementText());
            else if (tag == "showCourtesyClef")
                  _showCourtesy = e.readInt();
            else if (!Element::readProperties300old(e))
                  e.unknown();
            }
      if (clefType() == ClefType::INVALID)
            setClefType(ClefType::G);
      }

//---------------------------------------------------------
//   Harmony::read300old
//---------------------------------------------------------

void Harmony::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "base")
                  setBaseTpc(e.readInt());
            else if (tag == "baseCase")
                  _baseCase = static_cast<NoteCaseType>(e.readInt());
            else if (tag == "extension")
                  setId(e.readInt());
            else if (tag == "name")
                  _textName = e.readElementText();
            else if (tag == "root")
                  setRootTpc(e.readInt());
            else if (tag == "rootCase")
                  _rootCase = static_cast<NoteCaseType>(e.readInt());
            else if (tag == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType = "";
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "degree-value")
                              degreeValue = e.readInt();
                        else if (tag == "degree-alter")
                              degreeAlter = e.readInt();
                        else if (tag == "degree-type")
                              degreeType = e.readElementText();
                        else
                              e.unknown();
                        }
                  if (degreeValue <= 0 || degreeValue > 13
                      || degreeAlter < -2 || degreeAlter > 2
                      || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                        qDebug("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s",
                               degreeValue, degreeAlter, qPrintable(degreeType));
                        }
                  else {
                        if (degreeType == "add")
                              addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                        else if (degreeType == "alter")
                              addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                        else if (degreeType == "subtract")
                              addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                        }
                  }
            else if (tag == "leftParen") {
                  _leftParen = true;
                  e.readNext();
                  }
            else if (tag == "rightParen") {
                  _rightParen = true;
                  e.readNext();
                  }
            else if (!TextBase::readProperties300old(e))
                  e.unknown();
            }

      // TODO: now that we can render arbitrary chords,
      // we could try to construct a full representation from a degree list.
      // These will typically only exist for chords imported from MusicXML prior to MuseScore 2.0
      // or constructed in the Chord Symbol Properties dialog.

      if (_rootTpc != Tpc::TPC_INVALID) {
            if (_id > 0) {
                  // positive id will happen only for scores that were created with explicit chord lists
                  // lookup id in chord list and generate new description if necessary
                  getDescription();
                  }
            else
                  {
                  // default case: look up by name
                  // description will be found for any chord already read in this score
                  // and we will generate a new one if necessary
                  getDescription(_textName);
                  }
            }
      else if (_textName == "") {
            // unrecognized chords prior to 2.0 were stored as text with markup
            // we need to strip away the markup
            // this removes any user-applied formatting,
            // but we no longer support user-applied formatting for chord symbols anyhow
            // with any luck, the resulting text will be parseable now, so give it a shot
            createLayout();
            QString s = plainText();
            if (!s.isEmpty()) {
                  setHarmony(s);
                  return;
                  }
            // empty text could also indicate a root-less slash chord ("/E")
            // we'll fall through and render it normally
            }

      // render chord from description (or _textName)
      render();
      }

//---------------------------------------------------------
//   TextBase::read300old
//---------------------------------------------------------

void TextBase::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Text::read300old
//---------------------------------------------------------

void Text::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "style") {
                  QString sn = e.readElementText();
                  if (sn == "Tuplet")          // ugly hack for compatibility
                        continue;
                  SubStyleId s = subStyleFromName(sn);
                  initSubStyle(s);
                  }
            else if (!readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Dynamic::read300old
//---------------------------------------------------------

void Dynamic::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "subtype")
                  setDynamicType(e.readElementText());
            else if (tag == "velocity")
                  _velocity = e.readInt();
            else if (tag == "dynType")
                  _dynRange = Range(e.readInt());
            else if (!TextBase::readProperties300old(e))
                  e.unknown();
            }
      if (subStyleId() == SubStyleId::DEFAULT)
            initSubStyle(SubStyleId::DYNAMICS);
      }

//---------------------------------------------------------
//   FiguredBass::read300old
//---------------------------------------------------------

void FiguredBass::read300old(XmlReader& e)
      {
      QString normalizedText;
      int idx = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "ticks")
                  setTicks(e.readInt());
            else if (tag == "onNote")
                  setOnNote(e.readInt() != 0l);
            else if (tag == "FiguredBassItem") {
                  FiguredBassItem * pItem = new FiguredBassItem(score(), idx++);
                  pItem->setTrack(track());
                  pItem->setParent(this);
                  pItem->read300old(e);
                  items.push_back(pItem);
                  // add item normalized text
                  if(!normalizedText.isEmpty())
                        normalizedText.append('\n');
                  normalizedText.append(pItem->normalizedText());
                  }
//            else if (tag == "style")
//                  setStyledPropertyListIdx(e.readElementText());
            else if (!TextBase::readProperties300old(e))
                  e.unknown();
            }
      // if items could be parsed set normalized text
      if (items.size() > 0)
            setXmlText(normalizedText);      // this is the text to show while editing
      }

//---------------------------------------------------------
//   InstrumentChange::read300old
//---------------------------------------------------------

void InstrumentChange::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Instrument")
                  _instrument->read(e, part());
            else if (!TextBase::readProperties300old(e))
                  e.unknown();
            }
      if (score()->mscVersion() <= 206) {
            // previous versions did not honor transposition of instrument change
            // except in ways that it should not have
            // notes entered before the instrument change was added would not be altered,
            // so original transposition remained in effect
            // notes added afterwards would be transposed by both intervals, resulting in tpc corruption
            // here we set the instrument change to inherit the staff transposition to emulate previous versions
            // in Note::read(), we attempt to fix the tpc corruption

            Interval v = staff() ? staff()->part()->instrument()->transpose() : 0;
            _instrument->setTranspose(v);
            }
      }

//---------------------------------------------------------
//   Jump::read300old
//---------------------------------------------------------

void Jump::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "jumpTo")
                  _jumpTo = e.readElementText();
            else if (tag == "playUntil")
                  _playUntil = e.readElementText();
            else if (tag == "continueAt")
                  _continueAt = e.readElementText();
            else if (tag == "playRepeats")
                  _playRepeats = e.readBool();
            else if (!TextBase::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Marker::read300old
//---------------------------------------------------------

void Marker::read300old(XmlReader& e)
      {
      Type mt = Type::SEGNO;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "label") {
                  QString s(e.readElementText());
                  setLabel(s);
                  mt = markerType(s);
                  }
            else if (!TextBase::readProperties300old(e))
                  e.unknown();
            }
      setMarkerType(mt);
      }

//---------------------------------------------------------
//   StaffText::read300old
//---------------------------------------------------------

void StaffText::read300old(XmlReader& e)
      {
      for (int voice = 0; voice < VOICES; ++voice)
            _channelNames[voice].clear();
      clearAeolusStops();
      while (e.readNextStartElement()) {
            if (!readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   StaffText::readProperties300old
//---------------------------------------------------------

bool StaffText::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "MidiAction") {
            int channel = e.intAttribute("channel", 0);
            QString name = e.attribute("name");
            bool found = false;
            int n = _channelActions.size();
            for (int i = 0; i < n; ++i) {
                  ChannelActions* a = &_channelActions[i];
                  if (a->channel == channel) {
                        a->midiActionNames.append(name);
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  ChannelActions a;
                  a.channel = channel;
                  a.midiActionNames.append(name);
                  _channelActions.append(a);
                  }
            e.readNext();
            }
      else if (tag == "channelSwitch" || tag == "articulationChange") {
            int voice = e.intAttribute("voice", -1);
            if (voice >= 0 && voice < VOICES)
                  _channelNames[voice] = e.attribute("name");
            else if (voice == -1) {
                  // no voice applies channel to all voices for
                  // compatibility
                  for (int i = 0; i < VOICES; ++i)
                        _channelNames[i] = e.attribute("name");
                  }
            e.readNext();
            }
      else if (tag == "aeolus") {
            int group = e.intAttribute("group", -1);
            if (group >= 0 && group < 4)
                  aeolusStops[group] = e.readInt();
            else
                  e.readNext();
            _setAeolusStops = true;
            }
      else if (tag == "swing") {
            QString swingUnit = e.attribute("unit","");
            int unit = 0;
            if (swingUnit == TDuration(TDuration::DurationType::V_EIGHTH).name())
                  unit = MScore::division / 2;
            else if (swingUnit == TDuration(TDuration::DurationType::V_16TH).name())
                  unit = MScore:: division / 4;
            else if (swingUnit == TDuration(TDuration::DurationType::V_ZERO).name())
                  unit = 0;
            int ratio = e.intAttribute("ratio", 60);
            setSwing(true);
            setSwingParameters(unit, ratio);
            e.readNext();
            }
      else if (!TextBase::readProperties300old(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   TempoText::read300old
//---------------------------------------------------------

void TempoText::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "tempo")
                  setTempo(e.readDouble());
            else if (tag == "followText")
                  _followText = e.readInt();
            else if (!TextBase::readProperties300old(e))
                  e.unknown();
            }
      // check sanity
      if (xmlText().isEmpty()) {
            setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(lrint(60 * _tempo)));
            setVisible(false);
            }
      }

//---------------------------------------------------------
//   MeasureBase::readProperties300old
//---------------------------------------------------------

bool MeasureBase::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "LayoutBreak") {
            LayoutBreak* lb = new LayoutBreak(score());
            lb->read(e);
            bool doAdd = true;
            switch (lb->layoutBreakType()) {
                  case LayoutBreak::LINE:
                        if (lineBreak())
                              doAdd = false;
                        break;
                  case LayoutBreak::PAGE:
                        if (pageBreak())
                              doAdd = false;
                        break;
                  case LayoutBreak::SECTION:
                        if (sectionBreak())
                              doAdd = false;
                        break;
                  case LayoutBreak::NOBREAK:
                        if (noBreak())
                              doAdd = false;
                        break;
                  }
            if (doAdd) {
                  add(lb);
                  cleanupLayoutBreaks(false);
                  }
            else
                  delete lb;
            }
      else if (tag == "StaffTypeChange") {
            StaffTypeChange* stc = new StaffTypeChange(score());
            stc->setTrack(e.track());
            stc->setParent(this);
            stc->read(e);
            add(stc);
            }
      else if (Element::readProperties300old(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   Glissando::read300old
//---------------------------------------------------------

void Glissando::read300old(XmlReader& e)
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
                  _glissandoType = GlissandoType(e.readInt());
            else if (tag == "glissandoStyle")
                  setProperty(Pid::GLISSANDO_STYLE, Ms::getProperty(Pid::GLISSANDO_STYLE, e));
            else if (tag == "play")
                  setPlayGlissando(e.readBool());
            else if (readStyledProperty(e, tag))
                  ;
            else if (!SLine::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   HairPin::read300old
//---------------------------------------------------------

void Hairpin::read300old(XmlReader& e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();

      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setHairpinType(HairpinType(e.readInt()));
            else if (readStyledProperty(e, tag))
                  ;
            else if (tag == "hairpinCircledTip")
                  _hairpinCircledTip = e.readInt();
            else if (tag == "veloChange")
                  _veloChange = e.readInt();
            else if (tag == "dynType")
                  _dynRange = Dynamic::Range(e.readInt());
            else if (tag == "useTextLine") {      // obsolete
                  e.readInt();
                  if (hairpinType() == HairpinType::CRESC_HAIRPIN)
                        setHairpinType(HairpinType::CRESC_LINE);
                  else if (hairpinType() == HairpinType::DECRESC_HAIRPIN)
                        setHairpinType(HairpinType::DECRESC_LINE);
                  }
            else if (!TextLineBase::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   LetRing::read300old
//---------------------------------------------------------

void LetRing::read300old(XmlReader& e)
      {
      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);
      while (e.readNextStartElement()) {
            if (!TextLineBase::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   SLine::read300old
//---------------------------------------------------------

void SLine::read300old(XmlReader& e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      e.addSpanner(e.intAttribute("id", -1), this);

      while (e.readNextStartElement()) {
            if (!SLine::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   PalmMute::read300old
//---------------------------------------------------------

void PalmMute::read300old(XmlReader& e)
      {
      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);
      while (e.readNextStartElement()) {
            if (!TextLineBase::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Pedal::read300old
//---------------------------------------------------------

void Pedal::read300old(XmlReader& e)
      {
      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);
      while (e.readNextStartElement()) {
            if (!TextLineBase::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   SlurTie::readProperties300old
//---------------------------------------------------------

bool SlurTie::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (readProperty(tag, e, Pid::SLUR_DIRECTION))
            ;
      else if (tag == "lineType")
            _lineType = e.readInt();
      else if (tag == "SlurSegment") {
            SlurTieSegment* s = newSlurTieSegment();
            s->read(e);
            add(s);
            }
      else if (!Element::readProperties300old(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   Slur::read300old
//---------------------------------------------------------

void Slur::read300old(XmlReader& e)
      {
      setTrack(e.track());      // set staff
      e.addSpanner(e.intAttribute("id"), this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "track2")
                  setTrack2(e.readInt());
            else if (tag == "startTrack")       // obsolete
                  setTrack(e.readInt());
            else if (tag == "endTrack")         // obsolete
                  e.readInt();
            else if (!SlurTie::readProperties300old(e))
                  e.unknown();
            }
      if (track2() == -1)
            setTrack2(track());
      }

//---------------------------------------------------------
//   TextLineBase::read300old
//---------------------------------------------------------

void TextLineBase::read300old(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      e.addSpanner(e.intAttribute("id", -1), this);

      while (e.readNextStartElement()) {
            if (!readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Tie::read300old
//---------------------------------------------------------

void Tie::read300old(XmlReader& e)
      {
      e.addSpanner(e.intAttribute("id"), this);
      while (e.readNextStartElement()) {
            if (SlurTie::readProperties300old(e))
                  ;
            else
                  e.unknown();
            }
      if (score()->mscVersion() <= 114 && spannerSegments().size() == 1) {
            // ignore manual adjustments to single-segment ties in older scores
            TieSegment* ss = frontSegment();
            QPointF zeroP;
            ss->ups(Grip::START).off     = zeroP;
            ss->ups(Grip::BEZIER1).off   = zeroP;
            ss->ups(Grip::BEZIER2).off   = zeroP;
            ss->ups(Grip::END).off       = zeroP;
            ss->setUserOff(zeroP);
            ss->setUserOff2(zeroP);
            }
      }

//---------------------------------------------------------
//   Trill::read300old
//---------------------------------------------------------

void Trill::read300old(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setTrillType(e.readElementText());
            else if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read300old(e);
                  _accidental->setParent(this);
                  }
            else if ( tag == "ornamentStyle")
                  setProperty(Pid::ORNAMENT_STYLE, Ms::getProperty(Pid::ORNAMENT_STYLE, e));
            else if ( tag == "play")
                  setPlayArticulation(e.readBool());
            else if (!SLine::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Vibrato::read300old
//---------------------------------------------------------

void Vibrato::read300old(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setVibratoType(e.readElementText());
            else if ( tag == "play")
                  setPlayArticulation(e.readBool());
            else if (!SLine::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Volta::read300old
//---------------------------------------------------------

void Volta::read300old(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "endings") {
                  QString s = e.readElementText();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  for (const QString& l : sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLineBase::readProperties300old(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Note::read300old
//---------------------------------------------------------

void Note::read300old(XmlReader& e)
      {
      setTpc1(Tpc::TPC_INVALID);
      setTpc2(Tpc::TPC_INVALID);

      while (e.readNextStartElement()) {
            if (readProperties300old(e))
                  ;
            else
                  e.unknown();
            }
      // ensure sane values:
      _pitch = limit(_pitch, 0, 127);

      if (!tpcIsValid(_tpc[0]) && !tpcIsValid(_tpc[1])) {
            Key key = (staff() && chord()) ? staff()->key(chord()->tick()) : Key::C;
            int tpc = pitch2tpc(_pitch, key, Prefer::NEAREST);
            if (concertPitch())
                  _tpc[0] = tpc;
            else
                  _tpc[1] = tpc;
            }
      if (!(tpcIsValid(_tpc[0]) && tpcIsValid(_tpc[1]))) {
            int tick = chord() ? chord()->tick() : -1;
            Interval v = staff() ? part()->instrument(tick)->transpose() : Interval();
            if (tpcIsValid(_tpc[0])) {
                  v.flip();
                  if (v.isZero())
                        _tpc[1] = _tpc[0];
                  else
                        _tpc[1] = Ms::transposeTpc(_tpc[0], v, true);
                  }
            else {
                  if (v.isZero())
                        _tpc[0] = _tpc[1];
                  else
                        _tpc[0] = Ms::transposeTpc(_tpc[1], v, true);
                  }
            }

      // check consistency of pitch, tpc1, tpc2, and transposition
      // see note in InstrumentChange::read300old() about a known case of tpc corruption produced in 2.0.x
      // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
      // including perhaps some we don't know about yet,
      // we will attempt to fix some problems here regardless of version

      if (!e.pasteMode() && !MScore::testMode) {
            int tpc1Pitch = (tpc2pitch(_tpc[0]) + 12) % 12;
            int tpc2Pitch = (tpc2pitch(_tpc[1]) + 12) % 12;
            int concertPitch = _pitch % 12;
            if (tpc1Pitch != concertPitch) {
                  qDebug("bad tpc1 - concertPitch = %d, tpc1 = %d", concertPitch, tpc1Pitch);
                  _pitch += tpc1Pitch - concertPitch;
                  }
            Interval v = staff()->part()->instrument(e.tick())->transpose();
            int transposedPitch = (_pitch - v.chromatic) % 12;
            if (tpc2Pitch != transposedPitch) {
                  qDebug("bad tpc2 - transposedPitch = %d, tpc2 = %d", transposedPitch, tpc2Pitch);
                  }
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Note::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "pitch")
            _pitch = e.readInt();
      else if (tag == "tpc") {
            _tpc[0] = e.readInt();
            _tpc[1] = _tpc[0];
            }
      else if (tag == "track")            // for performance
            setTrack(e.readInt());
      else if (tag == "Accidental") {
            Accidental* a = new Accidental(score());
            a->setTrack(track());
            a->read300old(e);
            add(a);
            }
      else if (tag == "Tie") {
            Tie* tie = new Tie(score());
            tie->setParent(this);
            tie->setTrack(track());
            tie->read300old(e);
            tie->setStartNote(this);
            _tieFor = tie;
            }
      else if (tag == "tpc2")
            _tpc[1] = e.readInt();
      else if (tag == "small")
            setSmall(e.readInt());
      else if (tag == "mirror")
            setProperty(Pid::MIRROR_HEAD, Ms::getProperty(Pid::MIRROR_HEAD, e));
      else if (tag == "dotPosition")
            setProperty(Pid::DOT_POSITION, Ms::getProperty(Pid::DOT_POSITION, e));
      else if (tag == "fixed")
            setFixed(e.readBool());
      else if (tag == "fixedLine")
            setFixedLine(e.readInt());
      else if (tag == "head")
            setProperty(Pid::HEAD_GROUP, Ms::getProperty(Pid::HEAD_GROUP, e));
      else if (tag == "velocity")
            setVeloOffset(e.readInt());
      else if (tag == "play")
            setPlay(e.readInt());
      else if (tag == "tuning")
            setTuning(e.readDouble());
      else if (tag == "fret")
            setFret(e.readInt());
      else if (tag == "string")
            setString(e.readInt());
      else if (tag == "ghost")
            setGhost(e.readInt());
      else if (tag == "headType")
            setProperty(Pid::HEAD_TYPE, Ms::getProperty(Pid::HEAD_TYPE, e));
      else if (tag == "veloType")
            setProperty(Pid::VELO_TYPE, Ms::getProperty(Pid::VELO_TYPE, e));
      else if (tag == "line")
            setLine(e.readInt());
      else if (tag == "Fingering") {
            Fingering* f = new Fingering(score());
            f->read300old(e);
            add(f);
            }
      else if (tag == "Symbol") {
            Symbol* s = new Symbol(score());
            s->setTrack(track());
            s->read300old(e);
            add(s);
            }
      else if (tag == "Image") {
            if (MScore::noImages)
                  e.skipCurrentElement();
            else {
                  Image* image = new Image(score());
                  image->setTrack(track());
                  image->read300old(e);
                  add(image);
                  }
            }
      else if (tag == "Bend") {
            Bend* b = new Bend(score());
            b->setTrack(track());
            b->read300old(e);
            add(b);
            }
      else if (tag == "NoteDot") {
            NoteDot* dot = new NoteDot(score());
            dot->read300old(e);
            add(dot);
            }
      else if (tag == "Events") {
            _playEvents.clear();    // remove default event
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());
                  if (tag == "Event") {
                        NoteEvent ne;
                        ne.read(e);
                        _playEvents.append(ne);
                        }
                  else
                        e.unknown();
                  }
            if (chord())
                  chord()->setPlayEventType(PlayEventType::User);
            }
      else if (tag == "endSpanner") {
            int id = e.intAttribute("id");
            Spanner* sp = e.findSpanner(id);
            if (sp) {
                  sp->setEndElement(this);
                  if (sp->isTie())
                        _tieBack = toTie(sp);
                  else {
                        if (sp->isGlissando() && parent() && parent()->isChord())
                              toChord(parent())->setEndsGlissando(true);
                        addSpannerBack(sp);
                        }
                  e.removeSpanner(sp);
                  }
            else {
                  // End of a spanner whose start element will appear later;
                  // may happen for cross-staff spanner from a lower to a higher staff
                  // (for instance a glissando from bass to treble staff of piano).
                  // Create a place-holder spanner with end data
                  // (a TextLine is used only because both Spanner or SLine are abstract,
                  // the actual class does not matter, as long as it is derived from Spanner)
                  int id = e.intAttribute("id", -1);
                  if (id != -1 &&
                              // DISABLE if pasting into a staff with linked staves
                              // because the glissando is not properly cloned into the linked staves
                              (!e.pasteMode() || !staff()->links() || staff()->links()->empty())) {
                        Spanner* placeholder = new TextLine(score());
                        placeholder->setAnchor(Spanner::Anchor::NOTE);
                        placeholder->setEndElement(this);
                        placeholder->setTrack2(track());
                        placeholder->setTick(0);
                        placeholder->setTick2(e.tick());
                        e.addSpanner(id, placeholder);
                        }
                  }
            e.readNext();
            }
      else if (tag == "TextLine"
            || tag == "Glissando") {
            Spanner* sp = toSpanner(Element::name2Element(tag, score()));
            // check this is not a lower-to-higher cross-staff spanner we already got
            int id = e.intAttribute("id");
            Spanner* placeholder = e.findSpanner(id);
            if (placeholder && placeholder->endElement()) {
                  // if it is, fill end data from place-holder
                  sp->setAnchor(Spanner::Anchor::NOTE);           // make sure we can set a Note as end element
                  sp->setEndElement(placeholder->endElement());
                  sp->setTrack2(placeholder->track2());
                  sp->setTick(e.tick());                          // make sure tick2 will be correct
                  sp->setTick2(placeholder->tick2());
                  toNote(placeholder->endElement())->addSpannerBack(sp);
                  // remove no longer needed place-holder before reading the new spanner,
                  // as reading it also adds it to XML reader list of spanners,
                  // which would overwrite the place-holder
                  e.removeSpanner(placeholder);
                  delete placeholder;
                  }
            sp->setTrack(track());
            sp->read300old(e);
            // DISABLE pasting of glissandi into staves with other lionked staves
            // because the glissando is not properly cloned into the linked staves
            if (e.pasteMode() && staff()->links() && !staff()->links()->empty()) {
                  e.removeSpanner(sp);    // read300old() added the element to the XMLReader: remove it
                  delete sp;
                  }
            else {
                  sp->setAnchor(Spanner::Anchor::NOTE);
                  sp->setStartElement(this);
                  sp->setTick(e.tick());
                  addSpannerFor(sp);
                  sp->setParent(this);
                  }
            }
      else if (tag == "offset")
            Element::readProperties300old(e);
      else if (Element::readProperties300old(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   DurationElement::readProperties300old
//---------------------------------------------------------

bool DurationElement::readProperties300old(XmlReader& e)
      {
      if (e.name() == "Tuplet") {
            int i = e.readInt();
            Tuplet* t = e.findTuplet(i);
            if (!t) {
                  qDebug("DurationElement:read300old(): Tuplet id %d not found", i);
                  t = score()->searchTuplet(e, i);
                  if (t) {
                        qDebug("   ...found outside measure, input file corrupted?");
                        e.addTuplet(t);
                        }
                  }
            if (t) {
                  setTuplet(t);
                  if (!score()->undoStack()->active())     // HACK, also added in Undo::AddElement()
                        t->add(this);
                  }
            return true;
            }
      else if (Element::readProperties300old(e))
            return true;
      return false;
      }


//---------------------------------------------------------
//   Tuplet::read300old
//---------------------------------------------------------

void Tuplet::read300old(XmlReader& e)
      {
      _id    = e.intAttribute("id", 0);
      while (e.readNextStartElement()) {
            if (readProperties300old(e))
                  ;
            else
                  e.unknown();
            }
      Fraction f(_ratio.denominator(), _baseLen.fraction().denominator());
      setDuration(f.reduced());
      }

//---------------------------------------------------------
//   Tuplet::readProperties300old
//---------------------------------------------------------

bool Tuplet::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (readStyledProperty(e, tag))
            ;
      else if (tag == "normalNotes")
            _ratio.setDenominator(e.readInt());
      else if (tag == "actualNotes")
            _ratio.setNumerator(e.readInt());
      else if (tag == "p1")
            _p1 = e.readPoint();
      else if (tag == "p2")
            _p2 = e.readPoint();
      else if (tag == "baseNote")
            _baseLen = TDuration(e.readElementText());
      else if (tag == "Number") {
            _number = new Text(score());
            _number->setComposition(true);
            _number->setParent(this);
//            _number->setSubStyleId(SubStyleId::TUPLET);
//            initSubStyle(SubStyleId::TUPLET);   // hack: initialize number
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_BOLD, Pid::FONT_ITALIC, Pid::FONT_UNDERLINE, Pid::ALIGN })
                  _number->resetProperty(p);
            _number->read300old(e);
            _number->setVisible(visible());     //?? override saved property
            _number->setTrack(track());
            // move property flags from _number
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_BOLD, Pid::FONT_ITALIC, Pid::FONT_UNDERLINE, Pid::ALIGN })
                  setPropertyFlags(p, _number->propertyFlags(p));
            }
      else if (!DurationElement::readProperties300old(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   ChordRest::readProperties300old
//---------------------------------------------------------

bool ChordRest::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "durationType") {
            setDurationType(e.readElementText());
            if (actualDurationType().type() != TDuration::DurationType::V_MEASURE) {
                  if (score()->mscVersion() < 112 && (type() == ElementType::REST) &&
                              // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                              duration().numerator() != 0 &&
                              // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read300old() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                              (actualDurationType()==TDuration::DurationType::V_WHOLE && duration() <= Fraction(4, 4)) ) {
                        // old pre 2.0 scores: convert
                        setDurationType(TDuration::DurationType::V_MEASURE);
                        }
                  else  // not from old score: set duration fraction from duration type
                        setDuration(actualDurationType().fraction());
                  }
            else {
                  if (score()->mscVersion() <= 114) {
                        SigEvent event = score()->sigmap()->timesig(e.tick());
                        setDuration(event.timesig());
                        }
                  }
            }
      else if (tag == "BeamMode") {
            QString val(e.readElementText());
            Beam::Mode bm = Beam::Mode::AUTO;
            if (val == "auto")
                  bm = Beam::Mode::AUTO;
            else if (val == "begin")
                  bm = Beam::Mode::BEGIN;
            else if (val == "mid")
                  bm = Beam::Mode::MID;
            else if (val == "end")
                  bm = Beam::Mode::END;
            else if (val == "no")
                  bm = Beam::Mode::NONE;
            else if (val == "begin32")
                  bm = Beam::Mode::BEGIN32;
            else if (val == "begin64")
                  bm = Beam::Mode::BEGIN64;
            else
                  bm = Beam::Mode(val.toInt());
            _beamMode = Beam::Mode(bm);
            }
      else if (tag == "Articulation") {
            Articulation* atr = new Articulation(score());
            atr->setTrack(track());
            atr->read300old(e);
            add(atr);
            }
      else if (tag == "leadingSpace" || tag == "trailingSpace") {
            qDebug("ChordRest: %s obsolete", tag.toLocal8Bit().data());
            e.skipCurrentElement();
            }
      else if (tag == "Beam") {
            int id = e.readInt();
            Beam* beam = e.findBeam(id);
            if (beam)
                  beam->add(this);        // also calls this->setBeam(beam)
            else
                  qDebug("Beam id %d not found", id);
            }
      else if (tag == "small")
            _small = e.readInt();
      else if (tag == "duration")
            setDuration(e.readFraction());
      else if (tag == "ticklen") {      // obsolete (version < 1.12)
            int mticks = score()->sigmap()->timesig(e.tick()).timesig().ticks();
            int i = e.readInt();
            if (i == 0)
                  i = mticks;
            if ((type() == ElementType::REST) && (mticks == i)) {
                  setDurationType(TDuration::DurationType::V_MEASURE);
                  setDuration(Fraction::fromTicks(i));
                  }
            else {
                  Fraction f = Fraction::fromTicks(i);
                  setDuration(f);
                  setDurationType(TDuration(f));
                  }
            }
      else if (tag == "dots")
            setDots(e.readInt());
      else if (tag == "move")
            _staffMove = e.readInt();
      else if (tag == "Slur") {
            int id = e.intAttribute("id");
            if (id == 0)
                  id = e.intAttribute("number");                  // obsolete
            Spanner* spanner = e.findSpanner(id);
            QString atype(e.attribute("type"));

            if (!spanner) {
                  if (atype == "stop") {
                        SpannerValues sv;
                        sv.spannerId = id;
                        sv.track2    = track();
                        sv.tick2     = e.tick();
                        e.addSpannerValues(sv);
                        }
                  else if (atype == "start")
                        qDebug("spanner: start without spanner");
                  }
            else {
                  if (atype == "start") {
                        if (spanner->ticks() > 0 && spanner->tick() == -1) // stop has been read first
                              spanner->setTicks(spanner->ticks() - e.tick() - 1);
                        spanner->setTick(e.tick());
                        spanner->setTrack(track());
                        if (spanner->type() == ElementType::SLUR)
                              spanner->setStartElement(this);
                        if (e.pasteMode()) {
                              for (ScoreElement* e : spanner->linkList()) {
                                    if (e == spanner)
                                          continue;
                                    Spanner* ls = static_cast<Spanner*>(e);
                                    ls->setTick(spanner->tick());
                                    for (ScoreElement* ee : linkList()) {
                                          ChordRest* cr = toChordRest(ee);
                                          if (cr->score() == ee->score() && cr->staffIdx() == ls->staffIdx()) {
                                                ls->setTrack(cr->track());
                                                if (ls->type() == ElementType::SLUR)
                                                      ls->setStartElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              }
                        }
                  else if (atype == "stop") {
                        spanner->setTick2(e.tick());
                        spanner->setTrack2(track());
                        if (spanner->isSlur())
                              spanner->setEndElement(this);
                        ChordRest* start = toChordRest(spanner->startElement());
                        if (start)
                              spanner->setTrack(start->track());
                        if (e.pasteMode()) {
                              for (ScoreElement* e : spanner->linkList()) {
                                    if (e == spanner)
                                          continue;
                                    Spanner* ls = static_cast<Spanner*>(e);
                                    ls->setTick2(spanner->tick2());
                                    for (ScoreElement* ee : linkList()) {
                                          ChordRest* cr = toChordRest(ee);
                                          if (cr->score() == ee->score() && cr->staffIdx() == ls->staffIdx()) {
                                                ls->setTrack2(cr->track());
                                                if (ls->type() == ElementType::SLUR)
                                                      ls->setEndElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              }
                        }
                  else
                        qDebug("ChordRest::read300old(): unknown Slur type <%s>", qPrintable(atype));
                  }
            e.readNext();
            }
      else if (tag == "Lyrics" /*|| tag == "FiguredBass"*/) {
            Element* element = Element::name2Element(tag, score());
            element->setTrack(e.track());
            element->read300old(e);
            add(element);
            }
      else if (tag == "pos") {
            QPointF pt = e.readPoint();
            setUserOff(pt * spatium());
            }
      else if (tag == "offset")
            DurationElement::readProperties300old(e);
      else if (!DurationElement::readProperties300old(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   Rest::read300old
//---------------------------------------------------------

void Rest::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->setTrack(track());
                  s->read300old(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Image* image = new Image(score());
                        image->setTrack(track());
                        image->read300old(e);
                        add(image);
                        }
                  }
            else if (ChordRest::readProperties300old(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Chord::read300old
//---------------------------------------------------------

void Chord::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (readProperties300old(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Chord::readProperties300old
//---------------------------------------------------------

bool Chord::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "Note") {
            Note* note = new Note(score());
            // the note needs to know the properties of the track it belongs to
            note->setTrack(track());
            note->setChord(this);
            note->read300old(e);
            add(note);
            }
      else if (ChordRest::readProperties300old(e))
            ;
      else if (tag == "Stem") {
            Stem* s = new Stem(score());
            s->read300old(e);
            add(s);
            }
      else if (tag == "Hook") {
            _hook = new Hook(score());
            _hook->read300old(e);
            add(_hook);
            }
      else if (tag == "appoggiatura") {
            _noteType = NoteType::APPOGGIATURA;
            e.readNext();
            }
      else if (tag == "acciaccatura") {
            _noteType = NoteType::ACCIACCATURA;
            e.readNext();
            }
      else if (tag == "grace4") {
            _noteType = NoteType::GRACE4;
            e.readNext();
            }
      else if (tag == "grace16") {
            _noteType = NoteType::GRACE16;
            e.readNext();
            }
      else if (tag == "grace32") {
            _noteType = NoteType::GRACE32;
            e.readNext();
            }
      else if (tag == "grace8after") {
            _noteType = NoteType::GRACE8_AFTER;
            e.readNext();
            }
      else if (tag == "grace16after") {
            _noteType = NoteType::GRACE16_AFTER;
            e.readNext();
            }
      else if (tag == "grace32after") {
            _noteType = NoteType::GRACE32_AFTER;
            e.readNext();
            }
      else if (tag == "StemSlash") {
            StemSlash* ss = new StemSlash(score());
            ss->read300old(e);
            add(ss);
            }
      else if (readProperty(tag, e, Pid::STEM_DIRECTION))
            ;
      else if (tag == "noStem")
            _noStem = e.readInt();
      else if (tag == "Arpeggio") {
            _arpeggio = new Arpeggio(score());
            _arpeggio->setTrack(track());
            _arpeggio->read300old(e);
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
            gliss->read300old(e);
            gliss->setAnchor(Spanner::Anchor::NOTE);
            gliss->setStartElement(nullptr);
            gliss->setEndElement(nullptr);
            // in TAB, use straight line with no text
            if (score()->staff(e.track() >> 2)->isTabStaff(tick())) {
                  gliss->setGlissandoType(GlissandoType::STRAIGHT);
                  gliss->setShowText(false);
                  }
            finalNote->addSpannerBack(gliss);
            }
      else if (tag == "Tremolo") {
            _tremolo = new Tremolo(score());
            _tremolo->setTrack(track());
            _tremolo->read300old(e);
            _tremolo->setParent(this);
            }
      else if (tag == "tickOffset")       // obsolete
            ;
      else if (tag == "ChordLine") {
            ChordLine* cl = new ChordLine(score());
            cl->read300old(e);
            add(cl);
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   Score::readStaff300old
//---------------------------------------------------------

void Score::readStaff300old(XmlReader& e)
      {
      int staff = e.intAttribute("id", 1) - 1;
      e.initTick(0);
      e.setTrack(staff * VOICES);

      if (staff == 0) {
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        Measure* measure = 0;
                        measure = new Measure(this);
                        measure->setTick(e.tick());
                        //
                        // inherit timesig from previous measure
                        //
                        Measure* m = e.lastMeasure(); // measure->prevMeasure();
                        Fraction f(m ? m->timesig() : Fraction(4,4));
                        measure->setLen(f);
                        measure->setTimesig(f);

                        measure->read300old(e, staff);
                        measure->checkMeasure(staff);
                        if (!measure->isMMRest()) {
                              measures()->add(measure);
                              e.setLastMeasure(measure);
                              e.initTick(measure->tick() + measure->ticks());
                              }
                        else {
                              // this is a multi measure rest
                              // always preceded by the first measure it replaces
                              Measure* m = e.lastMeasure();

                              if (m) {
                                    m->setMMRest(measure);
                                    measure->setTick(m->tick());
                                    }
                              }
                        }
                  else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                        MeasureBase* mb = toMeasureBase(Element::name2Element(tag, this));
                        mb->read300old(e);
                        mb->setTick(e.tick());
                        measures()->add(mb);
                        }
                  else if (tag == "tick")
                        e.initTick(fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
            }
      else {
            Measure* measure = firstMeasure();
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        if (measure == 0) {
                              qDebug("Score::readStaff300old(): missing measure!");
                              measure = new Measure(this);
                              measure->setTick(e.tick());
                              measures()->add(measure);
                              }
                        e.initTick(measure->tick());
                        measure->read300old(e, staff);
                        measure->checkMeasure(staff);
                        if (measure->isMMRest())
                              measure = e.lastMeasure()->nextMeasure();
                        else {
                              e.setLastMeasure(measure);
                              if (measure->mmRest())
                                    measure = measure->mmRest();
                              else
                                    measure = measure->nextMeasure();
                              }
                        }
                  else if (tag == "tick")
                        e.initTick(fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
            }
      }

//---------------------------------------------------------
//   read300old
//    return false on error
//---------------------------------------------------------

bool Score::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaff300old(e);
            else if (tag == "Omr") {
#ifdef OMR
                  masterScore()->setOmr(new Omr(this));
                  masterScore()->omr()->read(e);
#else
                  e.skipCurrentElement();
#endif
                  }
            else if (tag == "Audio") {
                  _audio = new Audio;
                  _audio->read(e);
                  }
            else if (tag == "showOmr")
                  masterScore()->setShowOmr(e.readInt());
            else if (tag == "playMode")
                  _playMode = PlayMode(e.readInt());
            else if (tag == "LayerTag") {
                  int id = e.intAttribute("id");
                  const QString& tag = e.attribute("tag");
                  QString val(e.readElementText());
                  if (id >= 0 && id < 32) {
                        _layerTags[id] = tag;
                        _layerTagComments[id] = val;
                        }
                  }
            else if (tag == "Layer") {
                  Layer layer;
                  layer.name = e.attribute("name");
                  layer.tags = e.attribute("mask").toUInt();
                  _layer.append(layer);
                  e.readNext();
                  }
            else if (tag == "currentLayer")
                  _currentLayer = e.readInt();
            else if (tag == "Synthesizer")
                  _synthesizerState.read(e);
            else if (tag == "Division")
                  _fileDivision = e.readInt();
            else if (tag == "showInvisible")
                  _showInvisible = e.readInt();
            else if (tag == "showUnprintable")
                  _showUnprintable = e.readInt();
            else if (tag == "showFrames")
                  _showFrames = e.readInt();
            else if (tag == "showMargins")
                  _showPageborders = e.readInt();
            else if (tag == "Style") {
                  qreal sp = style().value(Sid::spatium).toDouble();
                  style().load(e);
                  // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
                  if (_layoutMode == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        style().set(Sid::spatium, sp);
                        }
                  _scoreFont = ScoreFont::fontFactory(style().value(Sid::MusicalSymbolFont).toString());
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read300old(e);
                  setMetaTag("copyright", text->xmlText());
                  delete text;
                  }
            else if (tag == "movement-number")
                  setMetaTag("movementNumber", e.readElementText());
            else if (tag == "movement-title")
                  setMetaTag("movementTitle", e.readElementText());
            else if (tag == "work-number")
                  setMetaTag("workNumber", e.readElementText());
            else if (tag == "work-title")
                  setMetaTag("workTitle", e.readElementText());
            else if (tag == "source")
                  setMetaTag("source", e.readElementText());
            else if (tag == "metaTag") {
                  QString name = e.attribute("name");
                  setMetaTag(name, e.readElementText());
                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  part->read(e);
                  _parts.push_back(part);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Slur")
                || (tag == "Pedal")) {
                  Spanner* s = toSpanner(Element::name2Element(tag, this));
                  s->read300old(e);
                  addSpanner(s);
                  }
            else if (tag == "Excerpt") {
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        if (isMaster()) {
                              Excerpt* ex = new Excerpt(static_cast<MasterScore*>(this));
                              ex->read(e);
                              excerpts().append(ex);
                              }
                        else {
                              qDebug("Score::read(): part cannot have parts");
                              e.skipCurrentElement();
                              }
                        }
                  }
            else if (e.name() == "Tracklist") {
                  int strack = e.intAttribute("sTrack",   -1);
                  int dtrack = e.intAttribute("dstTrack", -1);
                  if (strack != -1 && dtrack != -1)
                        e.tracks().insert(strack, dtrack);
                  e.skipCurrentElement();
                  }
            else if (tag == "Score") {          // recursion
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        e.tracks().clear();     // ???
                        MasterScore* m = masterScore();
                        Score* s       = new Score(m);
                        Excerpt* ex    = new Excerpt(m);

                        ex->setPartScore(s);
                        ex->setTracks(e.tracks());
                        e.setLastMeasure(nullptr);
                        s->read300old(e);
                        m->addExcerpt(ex);
                        }
                  }
            else if (tag == "name") {
                  QString n = e.readElementText();
                  if (!isMaster()) //ignore the name if it's not a child score
                        excerpt()->setTitle(n);
                  }
            else if (tag == "layoutMode") {
                  QString s = e.readElementText();
                  if (s == "line")
                        _layoutMode = LayoutMode::LINE;
                  else if (s == "system")
                        _layoutMode = LayoutMode::SYSTEM;
                  else
                        qDebug("layoutMode: %s", qPrintable(s));
                  }
            else
                  e.unknown();
            }
      if (e.error() != QXmlStreamReader::NoError) {
            qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
            MScore::lastError = QObject::tr("XML read error at line %1 column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(e.name().toString());
            return false;
            }

      connectTies();

      _fileDivision = MScore::division;

#if 0 // TODO:barline
      //
      //    sanity check for barLineSpan
      //
      for (Staff* st : staves()) {
            int barLineSpan = st->barLineSpan();
            int idx = st->idx();
            int n   = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span until last staff
                  barLineSpan = n - idx;
                  st->setBarLineSpan(barLineSpan);
                  }
            else if (idx == 0 && barLineSpan == 0) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span from the first staff until the start of the next span
                  barLineSpan = 1;
                  for (int i = 1; i < n; ++i) {
                        if (staff(i)->barLineSpan() == 0)
                              ++barLineSpan;
                        else
                              break;
                        }
                  st->setBarLineSpan(barLineSpan);
                  }
            // check spanFrom
            int minBarLineFrom = st->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
            if (st->barLineFrom() < minBarLineFrom)
                  st->setBarLineFrom(minBarLineFrom);
            if (st->barLineFrom() > st->lines(0) * 2)
                  st->setBarLineFrom(st->lines(0) * 2);
            // check spanTo
            Staff* stTo = st->barLineSpan() <= 1 ? st : staff(idx + st->barLineSpan() - 1);
            // 1-line staves have special bar line spans
            int maxBarLineTo        = stTo->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_TO : stTo->lines(0) * 2;
            if (st->barLineTo() < MIN_BARLINE_SPAN_FROMTO)
                  st->setBarLineTo(MIN_BARLINE_SPAN_FROMTO);
            if (st->barLineTo() > maxBarLineTo)
                  st->setBarLineTo(maxBarLineTo);
            // on single staff span, check spanFrom and spanTo are distant enough
            if (st->barLineSpan() == 1) {
                  if (st->barLineTo() - st->barLineFrom() < MIN_BARLINE_FROMTO_DIST) {
                        st->setBarLineFrom(0);
                        st->setBarLineTo(0);
                        }
                  }
            }
#endif

      if (!masterScore()->omr())
            masterScore()->setShowOmr(false);

      fixTicks();
      masterScore()->rebuildMidiMapping();
      masterScore()->updateChannel();
      createPlayEvents();
      return true;
      }

//---------------------------------------------------------
//   read300old
//---------------------------------------------------------

bool MasterScore::read300old(XmlReader& e)
      {
      if (!Score::read300old(e))
            return false;
      int id = 1;
      for (LinkedElements* le : e.linkIds())
            le->setLid(this, id++);
      for (Staff* s : staves())
            s->updateOttava();
      setCreated(false);
      return true;
      }

//---------------------------------------------------------
//   read300old1
//---------------------------------------------------------

Score::FileError MasterScore::read300old1(XmlReader& e)
      {
      bool top = true;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readIntHex());
            else if (tag == "Score") {
                  MasterScore* score;
                  if (top) {
                        score = this;
                        top   = false;
                        }
                  else {
                        score = new MasterScore();
                        addMovement(score);
                        }
                  if (!score->read300old(e))
                        return FileError::FILE_BAD_FORMAT;
                  }
            else if (tag == "Revision") {
                  Revision* revision = new Revision;
                  revision->read(e);
                  revisions()->add(revision);
                  }
            }
      return FileError::FILE_NO_ERROR;
      }

}

