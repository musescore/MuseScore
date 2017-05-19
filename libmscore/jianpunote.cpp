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
 Implementation of JianpuNote class.
*/

#include "jianpunote.h"
#include "jianpunotedot.h"
#include "score.h"
#include "key.h"
#include "chord.h"
#include "sym.h"
#include "xml.h"
#include "slur.h"
#include "tie.h"
#include "text.h"
#include "clef.h"
#include "staff.h"
#include "pitchspelling.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "utils.h"
#include "image.h"
#include "system.h"
#include "tuplet.h"
#include "articulation.h"
#include "drumset.h"
#include "segment.h"
#include "measure.h"
#include "undo.h"
#include "part.h"
#include "stafftype.h"
#include "stringdata.h"
#include "fret.h"
#include "harmony.h"
#include "fingering.h"
#include "bend.h"
#include "accidental.h"
#include "page.h"
#include "icon.h"
#include "notedot.h"
#include "spanner.h"
#include "glissando.h"
#include "bagpembell.h"
#include "hairpin.h"
#include "textline.h"
#include "durationtype.h"


namespace Ms {

// Ratios used to reduce width and height of font bounding-box returned by QFontMetricsF.
const float JianpuNote::FONT_BBOX_WIDTH_RATIO = 0.7f;
const float JianpuNote::FONT_BBOX_HEIGHT_RATIO = 0.7f;

JianpuNote::JianpuNote(Score* score)
   : Note(score)
      {
      initialize();
      }

JianpuNote::JianpuNote(const Note& note, bool link)
   : Note(note, link)
      {
      initialize();
      // If call to Note(note, link) above added NoteDots, remove them first.
      if (!_dots.empty()) {
            for (NoteDot* dot : _dots)
                  delete dot;
            _dots.clear();
            }
      // Add JianpuNoteDots if any.
      for (NoteDot* dot : note._dots)
            add(new JianpuNoteDot(*dot));
      }

JianpuNote::JianpuNote(const JianpuNote& note, bool link)
   : Note(note, link)
      {
      _noteNumber = note._noteNumber;
      _noteOctave = note._noteOctave;
      _durationDashCount = note._durationDashCount;
      _noteNumberBox = note._noteNumberBox;
      _octaveDotBox = note._octaveDotBox;
      _durationDashBox = note._durationDashBox;
      _dots = note.dots();
      }

JianpuNote::~JianpuNote()
      {
      }

void JianpuNote::initialize()
      {
      _noteNumber = 0;
      _noteOctave = 0;
      _durationDashCount = 0;
      _noteNumberBox.setRect(0, 0, 0, 0);
      _octaveDotBox.setRect(0, 0, 0, 0);
      _durationDashBox.setRect(0, 0, 0, 0);
      }

void JianpuNote::setTpc(int v)
      {
      Note::setTpc(v);
      setNoteByPitch(pitch(), tpc(), chord()->durationType().type());
      }

void JianpuNote::setPitch(int val)
      {
      Q_ASSERT(0 <= val && val <= 127);
      if (pitch() != val) {
            Note::setPitch(val);
            setNoteByPitch(pitch(), tpc(), chord()->durationType().type());
            }
      }

void JianpuNote::read(XmlReader& xml)
      {
      // TODO: Currently this is a copy from Note. Re-examine the code and optimize it.

      setTpc1(Tpc::TPC_INVALID);
      setTpc2(Tpc::TPC_INVALID);

      while (xml.readNextStartElement()) {
            if (!readProperties(xml))
                  xml.unknown();
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
      // see note in InstrumentChange::read() about a known case of tpc corruption produced in 2.0.x
      // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
      // including perhaps some we don't know about yet,
      // we will attempt to fix some problems here regardless of version

      if (!xml.pasteMode() && !MScore::testMode) {
            int tpc1Pitch = (tpc2pitch(_tpc[0]) + 12) % 12;
            int tpc2Pitch = (tpc2pitch(_tpc[1]) + 12) % 12;
            int concertPitch = _pitch % 12;
            if (tpc1Pitch != concertPitch) {
                  qDebug("bad tpc1 - concertPitch = %d, tpc1 = %d", concertPitch, tpc1Pitch);
                  _pitch += tpc1Pitch - concertPitch;
                  }
            Interval v = staff()->part()->instrument(xml.tick())->transpose();
            int transposedPitch = (_pitch - v.chromatic) % 12;
            if (tpc2Pitch != transposedPitch) {
                  qDebug("bad tpc2 - transposedPitch = %d, tpc2 = %d", transposedPitch, tpc2Pitch);
                  }
            }
      }

bool JianpuNote::readProperties(XmlReader& xml)
      {
      // TODO: Currently this is a copy from Note. Re-examine the code and optimize it.

      const QStringRef& tag(xml.name());
      if (tag == "pitch")
            _pitch = xml.readInt();
      else if (tag == "tpc") {
            _tpc[0] = xml.readInt();
            _tpc[1] = _tpc[0];
            }
      else if (tag == "track")            // for performance
            setTrack(xml.readInt());
      else if (tag == "Accidental") {
            Accidental* a = new Accidental(score());
            a->setTrack(track());
            a->read(xml);
            add(a);
            }
      else if (tag == "Tie") {
            Tie* tie = new Tie(score());
            tie->setParent(this);
            tie->setTrack(track());
            tie->read(xml);
            tie->setStartNote(this);
            _tieFor = tie;
            }
      else if (tag == "tpc2")
            _tpc[1] = xml.readInt();
      else if (tag == "small")
            setSmall(xml.readInt());
      else if (tag == "mirror")
            setProperty(Pid::MIRROR_HEAD, Ms::getProperty(Pid::MIRROR_HEAD, xml));
      else if (tag == "dotPosition")
            setProperty(Pid::DOT_POSITION, Ms::getProperty(Pid::DOT_POSITION, xml));
      else if (tag == "fixed")
            setFixed(xml.readBool());
      else if (tag == "fixedLine")
            setFixedLine(xml.readInt());
      else if (tag == "head")
            setProperty(Pid::HEAD_GROUP, Ms::getProperty(Pid::HEAD_GROUP, xml));
      else if (tag == "velocity")
            setVeloOffset(xml.readInt());
      else if (tag == "play")
            setPlay(xml.readInt());
      else if (tag == "tuning")
            setTuning(xml.readDouble());
      else if (tag == "fret")
            setFret(xml.readInt());
      else if (tag == "string")
            setString(xml.readInt());
      else if (tag == "ghost")
            setGhost(xml.readInt());
      else if (tag == "headType")
            setProperty(Pid::HEAD_TYPE, Ms::getProperty(Pid::HEAD_TYPE, xml));
      else if (tag == "veloType")
            setProperty(Pid::VELO_TYPE, Ms::getProperty(Pid::VELO_TYPE, xml));
      else if (tag == "line")
            _line = xml.readInt();
      else if (tag == "Fingering") {
            Fingering* f = new Fingering(score());
            f->read(xml);
            add(f);
            }
      else if (tag == "Symbol") {
            Symbol* s = new Symbol(score());
            s->setTrack(track());
            s->read(xml);
            add(s);
            }
      else if (tag == "Image") {
            if (MScore::noImages)
                  xml.skipCurrentElement();
            else {
                  Image* image = new Image(score());
                  image->setTrack(track());
                  image->read(xml);
                  add(image);
                  }
            }
      else if (tag == "Bend") {
            Bend* b = new Bend(score());
            b->setTrack(track());
            b->read(xml);
            add(b);
            }
      else if (tag == "NoteDot") {
            NoteDot* dot = new JianpuNoteDot(score());
            dot->read(xml);
            add(dot);
            }
      else if (tag == "Events") {
            _playEvents.clear();    // remove default event
            while (xml.readNextStartElement()) {
                  const QStringRef& tag(xml.name());
                  if (tag == "Event") {
                        NoteEvent ne;
                        ne.read(xml);
                        _playEvents.append(ne);
                        }
                  else
                        xml.unknown();
                  }
            if (chord())
                  chord()->setPlayEventType(PlayEventType::User);
            }
      else if (tag == "endSpanner") {
            int id = xml.intAttribute("id");
            Spanner* sp = xml.findSpanner(id);
            if (sp) {
                  sp->setEndElement(this);
                  if (sp->isTie())
                        _tieBack = toTie(sp);
                  else {
                        if (sp->isGlissando() && parent() && parent()->isChord())
                              toChord(parent())->setEndsGlissando(true);
                        addSpannerBack(sp);
                        }
                  xml.removeSpanner(sp);
                  }
            else {
                  // End of a spanner whose start element will appear later;
                  // may happen for cross-staff spanner from a lower to a higher staff
                  // (for instance a glissando from bass to treble staff of piano).
                  // Create a place-holder spanner with end data
                  // (a TextLine is used only because both Spanner or SLine are abstract,
                  // the actual class does not matter, as long as it is derived from Spanner)
                  int id = xml.intAttribute("id", -1);
                  if (id != -1 &&
                              // DISABLE if pasting into a staff with linked staves
                              // because the glissando is not properly cloned into the linked staves
                              (!xml.pasteMode() || !staff()->linkedStaves() || staff()->linkedStaves()->empty())) {
                        Spanner* placeholder = new TextLine(score());
                        placeholder->setAnchor(Spanner::Anchor::NOTE);
                        placeholder->setEndElement(this);
                        placeholder->setTrack2(track());
                        placeholder->setTick(0);
                        placeholder->setTick2(xml.tick());
                        xml.addSpanner(id, placeholder);
                        }
                  }
            xml.readNext();
            }
      else if (tag == "TextLine"
            || tag == "Glissando") {
            Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score()));
            // check this is not a lower-to-higher cross-staff spanner we already got
            int id = xml.intAttribute("id");
            Spanner* placeholder = xml.findSpanner(id);
            if (placeholder) {
                  // if it is, fill end data from place-holder
                  sp->setAnchor(Spanner::Anchor::NOTE);           // make sure we can set a Note as end element
                  sp->setEndElement(placeholder->endElement());
                  sp->setTrack2(placeholder->track2());
                  sp->setTick(xml.tick());                          // make sure tick2 will be correct
                  sp->setTick2(placeholder->tick2());
                  static_cast<Note*>(placeholder->endElement())->addSpannerBack(sp);
                  // remove no longer needed place-holder before reading the new spanner,
                  // as reading it also adds it to XML reader list of spanners,
                  // which would overwrite the place-holder
                  xml.removeSpanner(placeholder);
                  delete placeholder;
                  }
            sp->setTrack(track());
            sp->read(xml);
            // DISABLE pasting of glissandi into staves with other lionked staves
            // because the glissando is not properly cloned into the linked staves
            if (xml.pasteMode() && staff()->linkedStaves() && !staff()->linkedStaves()->empty()) {
                  xml.removeSpanner(sp);    // read() added the element to the XMLReader: remove it
                  delete sp;
                  }
            else {
                  sp->setAnchor(Spanner::Anchor::NOTE);
                  sp->setStartElement(this);
                  sp->setTick(xml.tick());
                  addSpannerFor(sp);
                  sp->setParent(this);
                  }
            }
      else if (tag == "offset")
            Element::readProperties(xml);
      else if (Element::readProperties(xml))
            ;
      else
            return false;
      return true;
      }

void JianpuNote::write(XmlWriter& xml) const
      {
      // TODO: Currently this is a copy from Note. Re-examine the code and optimize it.

      xml.stag("Note");
      Element::writeProperties(xml);

      if (_accidental)
            _accidental->write(xml);
      _el.write(xml);
      for (NoteDot* dot : _dots) {
            if (!dot->userOff().isNull() || !dot->visible() || dot->color() != Qt::black || dot->visible() != visible()) {
                  dot->write(xml);
                  break;
                  }
            }
      if (_tieFor)
            _tieFor->write(xml);
      if (_tieBack) {
            int id = xml.spannerId(_tieBack);
            xml.tagE(QString("endSpanner id=\"%1\"").arg(id));
            }
      if ((chord() == 0 || chord()->playEventType() != PlayEventType::Auto) && !_playEvents.empty()) {
            xml.stag("Events");
            for (const NoteEvent& e : _playEvents)
                  e.write(xml);
            xml.etag();
            }
      for (Pid id : { Pid::PITCH, Pid::TPC1, Pid::TPC2, Pid::SMALL, Pid::MIRROR_HEAD, Pid::DOT_POSITION,
         Pid::HEAD_GROUP, Pid::VELO_OFFSET, Pid::PLAY, Pid::TUNING, Pid::FRET, Pid::STRING,
         Pid::GHOST, Pid::HEAD_TYPE, Pid::VELO_TYPE, Pid::FIXED, Pid::FIXED_LINE
            }) {
            writeProperty(xml, id);
            }

      for (Spanner* e : _spannerFor)
            e->write(xml);
      for (Spanner* e : _spannerBack)
            xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(e)));

      xml.etag();
      }

void JianpuNote::setDotY(Direction pos)
      {
      bool onLine = false;
      qreal y = 0;

      onLine = !(line() & 1);

      bool oddVoice = voice() & 1;
      if (onLine) {
            // displace dots by half spatium up or down according to voice
            if (pos == Direction::AUTO)
                  y = oddVoice ? 0.5 : -0.5;
            else if (pos == Direction::UP)
                  y = -0.5;
            else
                  y = 0.5;
            }
      else {
            if (pos == Direction::UP && !oddVoice)
                  y -= 1.0;
            else if (pos == Direction::DOWN && oddVoice)
                  y += 1.0;
            }
      y *= spatium() * staff()->lineDistance(tick());

      // apply to dots

      int cdots = chord()->dots();
      int ndots = _dots.size();

      int n = cdots - ndots;
      for (int i = 0; i < n; ++i) {
            NoteDot* dot = new JianpuNoteDot(score());
            dot->setParent(this);
            dot->setTrack(track());  // needed to know the staff it belongs to (and detect tablature)
            dot->setVisible(visible());
            score()->undoAddElement(dot);
            }
      if (n < 0) {
            for (int i = 0; i < -n; ++i)
                  score()->undoRemoveElement(_dots.back());
            }
      for (NoteDot* dot : _dots) {
            dot->layout();
            dot->rypos() = y;
            }
      }

void JianpuNote::layout()
      {
      // Lay out note-number and octave-dot boxes.
      // Always anchor note-number box to (0, 0) position so that we can have
      // both rest and note numbers on the same level regardless of octave dots.

      // Update note number.
      setNoteByPitch(pitch(), tpc(), chord()->durationType().type());

      // Get note font metrics.
      StaffType* st = staff()->staffType(tick());
      QFontMetricsF fm(st->jianpuNoteFont(), MScore::paintDevice());
      QString txt = QString::number(_noteNumber);
      QRectF rect = fm.tightBoundingRect(txt);
      // Font bounding rectangle height is too large; make it smaller.
      //_noteNumberBox.setRect(0, 0, rect.width() * FONT_BBOX_WIDTH_RATIO, rect.height() * FONT_BBOX_HEIGHT_RATIO);
      _noteNumberBox.setRect(0, 0, rect.width(), rect.height() * FONT_BBOX_HEIGHT_RATIO);

      // Lay out octave-dot box.
      if (_noteOctave < 0) {
            // Lower octave.
            _octaveDotBox.setRect(0, _noteNumberBox.y() + _noteNumberBox.height() + OCTAVE_DOTBOX_Y_OFFSET,
                                  _noteNumberBox.width(), OCTAVE_DOTBOX_HEIGHT);
            }
      else if (_noteOctave > 0) {
            // Upper octave.
            _octaveDotBox.setRect(0, _noteNumberBox.y() - OCTAVE_DOTBOX_HEIGHT - OCTAVE_DOTBOX_Y_OFFSET,
                                  _noteNumberBox.width(), OCTAVE_DOTBOX_HEIGHT);
            }
      else {
            // No octave.
            _octaveDotBox.setRect(0, 0, 0, 0);
            }

      // Lay out duration-dash box for base note of the chord.
      if (_durationDashCount > 0 && this == chord()->downNote()) {
            // TODO: calculate dash/space widths based on available space in the measure.
            _durationDashBox.setRect(_noteNumberBox.width(), _noteNumberBox.y(),
                                     _durationDashCount * (DURATION_DASH_X_SPACE + DURATION_DASH_WIDTH),
                                     _noteNumberBox.height());
            }

      // Update main bounding box.
      setbbox(_noteNumberBox | _octaveDotBox | _durationDashBox);

      //qDebug("bbox x=%.0f y=%.0f w=%.0f h=%.0f", bbox().x(), bbox().y(), bbox().width(), bbox().height());
      //Q_ASSERT(bbox().x() < 20000 && bbox().y() < 20000);
      //Q_ASSERT(bbox().width() < 20000 && bbox().height() < 20000);
      }

//---------------------------------------------------------
//   layout2
//    called after final position of note is set
//---------------------------------------------------------
void JianpuNote::layout2()
      {
      // TODO: Currently this is a copy from Note. Re-examine the code and optimize it.
      int dots = chord()->dots();
      if (dots) {
            qreal d  = score()->point(score()->styleS(Sid::dotNoteDistance)) * mag();
            qreal dd = score()->point(score()->styleS(Sid::dotDotDistance)) * mag();
            qreal x  = chord()->dotPosX() - pos().x() - chord()->pos().x();
            // apply to dots
            qreal xx = x + d;
            for (NoteDot* dot : _dots) {
                  dot->rxpos() = xx;
                  dot->adjustReadPos();
                  xx += dd;
                  }
            }

      // layout elements attached to note
      for (Element* e : _el) {
            if (!score()->tagIsValid(e->tag()))
                  continue;
            e->setMag(mag());
            if (e->isSymbol()) {
                  qreal w = headWidth();
                  Symbol* sym = toSymbol(e);
                  QPointF rp = e->readPos();
                  e->layout();
                  if (sym->sym() == SymId::noteheadParenthesisRight) {
                        e->rxpos() += w;
                        }
                  else if (sym->sym() == SymId::noteheadParenthesisLeft) {
                        e->rxpos() -= symWidth(SymId::noteheadParenthesisLeft);
                        }
                  if (sym->sym() == SymId::noteheadParenthesisLeft || sym->sym() == SymId::noteheadParenthesisRight) {
                        // adjustReadPos() was called too early in layout(), adjust:
                        if (!rp.isNull()) {
                              e->setUserOff(QPointF());
                              e->setReadPos(rp);
                              e->adjustReadPos();
                              }
                        }
                  }
            else
                  e->layout();
            }
      //qDebug("bbox x=%.0f y=%.0f w=%.0f h=%.0f", bbox().x(), bbox().y(), bbox().width(), bbox().height());
      //Q_ASSERT(bbox().x() < 20000 && bbox().y() < 20000);
      //Q_ASSERT(bbox().width() < 20000 && bbox().height() < 20000);
      }

void JianpuNote::draw(QPainter* painter) const
      {
      if (hidden())
            return;

      // Draw note number.
      QString txt = QString::number(_noteNumber);
      StaffType* st = staff()->staffType(tick());
      QFont f(st->jianpuNoteFont());
      f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
      painter->setFont(f);
      painter->setPen(QColor(curColor()));
      // We take bounding box y-position as top of the note number.
      // But function "drawText" takes y-position as baseline of the font, which is the bottom of note number.
      // So adjust y-position for "drawText" to the bottom of the bounding box.
      painter->drawText(QPointF(pos().x() + _noteNumberBox.x(),
                                pos().y() + _noteNumberBox.y() + _noteNumberBox.height()), txt);

      // Prepare paint brush for octave dots and duration dash drawing.
      QBrush brush(curColor(), Qt::SolidPattern);
      painter->setBrush(brush);
      painter->setPen(Qt::NoPen);

      // Draw octave dots in 2x2 square as shown below for lower octaves.
      //    o   o o    o o    o o
      //                o     o o
      // Shapes of upper octaves are mirror image of lower-octave shapes.
      //
      // Initialize starting position and offsets.
      int dotCount = 0;
      qreal x = 0;
      qreal y = 0;
      qreal xOffset = 0;
      qreal yOffset = 0;
      if (_noteOctave < 0) {
            // Lower octave.
            // Start drawing dots at top of _octaveDotBox.
            dotCount = -_noteOctave;
            y = pos().y() + _octaveDotBox.y();
            yOffset = OCTAVE_DOT_ROW_HEIGHT;
            }
      else if (_noteOctave > 0) {
            // Upper octave.
            // Start drawing dots at bottom of _octaveDotBox.
            dotCount = _noteOctave;
            y = pos().y() + _octaveDotBox.y() + _octaveDotBox.height() - OCTAVE_DOT_ROW_HEIGHT;
            yOffset = -OCTAVE_DOT_ROW_HEIGHT;
            }
      if (dotCount == 1) {
            // Draw dot at middle of _octaveDotBox.
            x = pos().x() + _octaveDotBox.x() + (_octaveDotBox.width() - OCTAVE_DOT_WIDTH) / 2;
            xOffset = 0;
            }
      else if (dotCount > 1) {
            // Start drawing dots at left side of _octaveDotBox.
            x = pos().x() + _octaveDotBox.x() + OCTAVE_DOT_X_SPACE / 2;
            xOffset = OCTAVE_DOT_COL_WIDTH;
            }
      // Draw octave dots.
      if (dotCount > MAX_OCTAVE_DOTS)
            dotCount = MAX_OCTAVE_DOTS;
      qreal xStart = x;
      QRectF rect(0, 0, OCTAVE_DOT_WIDTH, OCTAVE_DOT_HEIGHT);
      for (int i = 0; i < dotCount; i++) {
            rect.moveTo(x, y);
            painter->drawEllipse(rect);
            if (i & 1) {
                  // Start next row.
                  if (dotCount == 3)
                        // Draw dot at middle of _octaveDotBox.
                        x = pos().x() + _octaveDotBox.x() + (_octaveDotBox.width() - OCTAVE_DOT_WIDTH) / 2;
                  else
                        x = xStart;
                  y += yOffset;
                  }
            else {
                  // Stay in the same row.
                  x += xOffset;
                  }
            }

      // Draw duration dashes for whole and half notes.
      // Draw dashes only for the base-note of the chord.
      if (_durationDashCount > 0 && this == chord()->downNote()) {
            // TODO: calculate dash/space widths based on available space of the measure.
            qreal space = DURATION_DASH_X_SPACE;
            qreal width = DURATION_DASH_WIDTH;
            qreal height = DURATION_DASH_HEIGHT;
            qreal x = pos().x() + _durationDashBox.x() + space;
            qreal y = pos().y() + _durationDashBox.y() + (_durationDashBox.height() - height) / 2;
            QRectF dash(x, y, width, height);
            for (int i = 0; i < _durationDashCount; i++) {
                  painter->fillRect(dash, brush);
                  x += width + space ;
                  dash.moveLeft(x); // Move rect's left edge to next dash position.
                  }
            }
      }

void JianpuNote::setNoteByNumber(int number, int octave, TDuration::DurationType duration)
      {
      Q_ASSERT(1 <= number && number <= 7);
      //Q_Q_ASSERT(-MAX_OCTAVE_DOTS <= octave && octave <= MAX_OCTAVE_DOTS);

      _noteNumber = number;
      if (octave < -MAX_OCTAVE_DOTS)
            _noteOctave = -MAX_OCTAVE_DOTS;
      else if (octave > MAX_OCTAVE_DOTS)
            _noteOctave = MAX_OCTAVE_DOTS;
      else
            _noteOctave = octave;
      _durationDashCount = calcDashCount(duration);
      }

void JianpuNote::setNoteByPitch(int pitch, int tpc, TDuration::DurationType duration)
      {
      Segment* seg = chord()->segment();
      int tick = seg ? seg->tick() : 0;
      Key key = staff() ? staff()->key(tick) : Key::C;
      int number = tpc2numberNoteByKey(tpc, key);
      int octave = pitch2octaveByKey(pitch, key);
      setNoteByNumber(number, octave, duration);
      }

// Convert standard note pitch and key to Jianpu octave number:
//     0 for middle octave (octave #4); negative for lower and positive for upper octaves.
int JianpuNote::pitch2octaveByKey(int pitch, Key key)
      {
      static const int keyNotePitch[(int) Key::NUM_OF] =
      //KEY --> C_B, G_B, D_B, A_B, E_B, B_B,   F,   C,   G,   D,   A,   E,   B, F_S, C_S
              {  71,  66,  61,  68,  63,  70,  65,  60,  67,  62,  69,  64,  71,  66,  61 };

      // Get key note pitch based on middle C octave (octave #4).
      int basePitch = keyNotePitch[(int) key - (int) Key::MIN];

      // Calculate octave number.
      int octave = (pitch - basePitch) / PITCH_DELTA_OCTAVE;
      int remainder = (pitch - basePitch) % PITCH_DELTA_OCTAVE;
      if (pitch < basePitch && remainder != 0)
            octave--;

      return octave;
      }

// Convert standard note tpc and key to Jianpu note number (1 to 7).
int JianpuNote::tpc2numberNoteByKey(int tpc, Key key)
      {
      // Standard notes of C Major scale -->              F  C  G  D  A  E  B
      static const int numberNotes[STEP_DELTA_OCTAVE] = { 4, 1, 5, 2, 6, 3, 7 };
      int index = (tpc - Tpc::TPC_MIN) - ((int) key - (int) Key::MIN);
      while (index < 0)
            index += STEP_DELTA_OCTAVE;

      return numberNotes[index % STEP_DELTA_OCTAVE];
      }

int JianpuNote::calcDashCount(TDuration::DurationType duration)
      {
      int dashCount;
      if (duration == TDuration::DurationType::V_WHOLE)
            dashCount = 3;
      else if (duration == TDuration::DurationType::V_HALF)
            dashCount = 1;
      else
            dashCount = 0;
      return dashCount;
      }

} // namespace Ms
