//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __IMPORTOVE_H__
#define __IMPORTOVE_H__

#include "ove.h"

#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/score.h"

using namespace Ms;

//---------------------------------------------------------
//   MeasureToTick
//---------------------------------------------------------

class MeasureToTick {
      int _quarter;
      Ove::OveSong* _ove;

   public:
      MeasureToTick();
      ~MeasureToTick() {}

      void build(Ove::OveSong* ove, int quarter);

      int tick(int measure, int tick_pos);
      static int unitToTick(int unit, int quarter);

      struct TimeTick {
            int _numerator;
            int _denominator;
            int _measure;
            int _tick;
            bool _isSymbol;

            TimeTick() :
                  _numerator(4), _denominator(4), _measure(0), _tick(0), _isSymbol(false) {}
            };
      QList<TimeTick> timeTicks() const { return _tts; }

   private:
      QList<TimeTick> _tts;
      };

//---------------------------------------------------------
//   OveToMScore
//---------------------------------------------------------

class OveToMScore {
      Ove::OveSong* _ove;
      Score* _score;
      MeasureToTick* _mtt;

      Pedal* _pedal;

   public:
      OveToMScore();
      ~OveToMScore();

      void convert(Ove::OveSong* oveData, Score* score);

   private:
      void createStructure();
      void convertHeader();
      void convertGroups();
      void convertTrackHeader(Ove::Track* track, Part* part);
      void convertTrackElements(int track);
      void convertLineBreak();
      void convertSignature();
      void convertMeasures();
      void convertMeasure(Measure* measure);
      void convertMeasureMisc(Measure* measure, int part, int staff, int track);
      void convertNote(Measure* measure, int part, int staff, int track);
      void convertArticulation(Measure* measure, ChordRest* cr, int track, int absTick, Ove::Articulation* art);
      void convertLyric(Measure* measure, int part, int staff, int track);
      void convertHarmony(Measure* measure, int part, int staff, int track);
      void convertRepeat(Measure* measure, int part, int staff, int track);
      void convertDynamic(Measure* measure, int part, int staff, int track);
      void convertExpression(Measure* measure, int part, int staff, int track);
      void convertLine(Measure* measure);
      void convertSlur(Measure* measure, int part, int staff, int track);
      void convertGlissando(Measure* measure, int part, int staff, int track);
      void convertWedge(Measure* measure, int part, int staff, int track);
      // void convertOctaveShift(Measure* measure, int part, int staff, int track);

      Ove::NoteContainer* containerByPos(int part, int staff, const Ove::MeasurePos& pos);
      // Ove::MusicData* musicDataByUnit(int part, int staff, int measure, int unit, Ove::MusicDataType type);
      Ove::MusicData* crossMeasureElementByPos(int part, int staff, const Ove::MeasurePos& pos, int voice, Ove::MusicDataType type);
      // ChordRest* findChordRestByPos(int absTick, int track);

      void clearUp();
      };

#endif
