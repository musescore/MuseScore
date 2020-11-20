//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#ifndef __RENDERMIDI_H__
#define __RENDERMIDI_H__

#include "fraction.h"
#include "measure.h"

namespace Ms {

class EventMap;
class MasterScore;
class Staff;
class SynthesizerState;

enum class DynamicsRenderMethod : signed char {
      FIXED_MAX,
      SEG_START,
      SIMPLE
      };

//---------------------------------------------------------
//   RangeMap
///   Helper class to keep track of status of status of
///   certain parts of score or MIDI representation.
//---------------------------------------------------------

class RangeMap {
      enum class Range { BEGIN, END };
      std::map<int, Range> status;

   public:
      void setOccupied(int tick1, int tick2);
      void setOccupied(std::pair<int, int> range) { setOccupied(range.first, range.second); }

      int occupiedRangeEnd(int tick) const;

      void clear() { status.clear(); }
      };

//---------------------------------------------------------
//   MidiRenderer
///   MIDI renderer for a score
//---------------------------------------------------------

class MidiRenderer {
      Score* score{nullptr};
      bool needUpdate = true;
      int minChunkSize = 0;

   public:
      class Chunk {
            int _tickOffset;
            Measure const * first;
            Measure const * last;

         public:
            Chunk(int tickOffset, Measure const * fst, Measure const * lst)
               : _tickOffset(tickOffset), first(fst), last(lst) {}

            Chunk() // "invalid chunk" constructor
               : _tickOffset(0), first(nullptr), last(nullptr) {}

            operator bool() const { return bool(first); }
            int tickOffset() const { return _tickOffset; }
            Measure const * startMeasure() const { return first; }
            Measure const * endMeasure() const { return last ? last->nextMeasure() : nullptr; }
            Measure const * lastMeasure() const { return last; }
            int tick1() const { return first->tick().ticks(); }
            int tick2() const { return last ? last->endTick().ticks() : tick1(); }
            int utick1() const { return tick1() + tickOffset(); }
            int utick2() const { return tick2() + tickOffset(); }
            };

   private:
      std::vector<Chunk> chunks;

      struct StaffContext
            {
            Staff* staff{nullptr};
            DynamicsRenderMethod method{DynamicsRenderMethod::SIMPLE};
            int cc{0};
            bool renderHarmony{false};
            };

      void updateChunksPartition();
      static bool canBreakChunk(const Measure* last);
      void updateState();

      void renderStaffChunk(const Chunk&, EventMap* events, const StaffContext& sctx);
      void renderSpanners(const Chunk&, EventMap* events);
      void renderMetronome(const Chunk&, EventMap* events);
      void renderMetronome(EventMap* events, Measure const * m, const Fraction& tickOffset);

      void collectMeasureEvents(EventMap* events, Measure const * m, const MidiRenderer::StaffContext& sctx, int tickOffset);
      void collectMeasureEventsSimple(EventMap* events, Measure const * m, const StaffContext& sctx, int tickOffset);
      void collectMeasureEventsDefault(EventMap* events, Measure const * m, const StaffContext& sctx, int tickOffset);

   public:
      explicit MidiRenderer(Score* s) : score(s) {}

      struct Context
            {
            const SynthesizerState& synthState;
            bool metronome{true};
            bool renderHarmony{false};
            Context(const SynthesizerState& ss) : synthState(ss) {}
            };

      void renderScore(EventMap* events, const Context& ctx);
      void renderChunk(const Chunk&, EventMap* events, const Context& ctx);

      void setScoreChanged() { needUpdate = true; }
      void setMinChunkSize(int sizeMeasures) { minChunkSize = sizeMeasures; needUpdate = true; }

      Chunk getChunkAt(int utick);

      static const int ARTICULATION_CONV_FACTOR { 100000 };
      };

class Spanner;
extern bool glissandoPitchOffsets(const Spanner* spanner, std::vector<int>* pitchOffsets);

} // namespace Ms

#endif
