//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mcursor.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/durationtype.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"

extern MScore* mscore;

//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

MCursor::MCursor(Score* s)
      {
      _score = s;
      move(0, 0);
      }

//---------------------------------------------------------
//   createMeasures
//---------------------------------------------------------

void MCursor::createMeasures()
      {
      Measure* measure;
      for (;;) {
            measure = _score->lastMeasure();
            if (measure) {
                  int t = measure->tick() + measure->ticks();
                  if (t > _tick)
                        break;
                  }
            measure = new Measure(_score);
            int t = 0;
            Measure* lm = _score->lastMeasure();
            if (lm) {
                  t = lm->tick() + lm->ticks();
                  }
            measure->setTick(t);
            measure->setTimesig(Fraction(4,4));
            measure->setLen(Fraction(4,4));
            _score->measures()->add(measure);
            }
      }

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void MCursor::addChord(int pitch, const TDuration& duration)
      {
      createMeasures();
      Measure* measure = _score->tick2measure(_tick);
      Segment* segment = measure->getSegment(SegChordRest, _tick);
      Chord* chord = static_cast<Chord*>(segment->element(_track));
      if (chord == 0) {
            chord = new Chord(_score);
            chord->setTrack(_track);
            chord->setDurationType(duration);
            chord->setDuration(duration.fraction());
            segment->add(chord);
            }
      Note* note = new Note(_score);
      chord->add(note);
      note->setPitch(pitch);
      note->setTpcFromPitch();
      _tick += duration.ticks();
      }

//---------------------------------------------------------
//   addKeySig
//---------------------------------------------------------

void MCursor::addKeySig(int sig)
      {
      createMeasures();
      Measure* measure = _score->tick2measure(_tick);
      Segment* segment = measure->getSegment(SegKeySig, _tick);
      int n = _score->nstaves();
      for (int i = 0; i < n; ++i) {
            KeySig* ks = new KeySig(_score);
            ks->setSig(0, sig);
            ks->setTrack(i * VOICES);
            segment->add(ks);
            }
      }

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

void MCursor::addTimeSig(const Fraction& f)
      {
      createMeasures();
      Measure* measure = _score->tick2measure(_tick);
      Segment* segment = measure->getSegment(SegTimeSig, _tick);
      int n = _score->nstaves();
      for (int i = 0; i < n; ++i) {
            TimeSig* ts = new TimeSig(_score, f);
            ts->setTrack(i * VOICES);
            segment->add(ts);
            }
      _score->sigmap()->add(_tick, SigEvent(f));
      }

//---------------------------------------------------------
//   createScore
//---------------------------------------------------------

void MCursor::createScore(const QString& name)
      {
      delete _score;
      _score = new Score(mscore->baseStyle());
      _score->setName(name);
      move(0, 0);
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void MCursor::move(int t, int tick)
      {
      _track = t;
      _tick = tick;
      }

//---------------------------------------------------------
//   addPart
//---------------------------------------------------------

void MCursor::addPart(const QString& instrument)
      {
      Part* part   = new Part(_score);
      Staff* staff = new Staff(_score, part, 0);
      InstrumentTemplate* it = searchTemplate(instrument);
      if (it == 0) {
            printf("did not found instrument <%s>\n", qPrintable(instrument));
            abort();
            }
      part->initFromInstrTemplate(it);
      _score->appendPart(part);
      _score->insertStaff(staff, 0);
      }

//---------------------------------------------------------
//   saveScore
//---------------------------------------------------------

void MCursor::saveScore()
      {
      QFile fp(_score->name() + ".mscx");
      if (!fp.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "open <%s> failed\n", qPrintable(fp.fileName()));
            abort();
            }
      _score->saveFile(&fp, false);
      fp.close();
      }


