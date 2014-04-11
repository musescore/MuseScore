#include "importmidi_tuplet_tonotes.h"
#include "importmidi_tuplet.h"
#include "importmidi_fraction.h"
#include "importmidi_inner.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/fraction.h"
#include "libmscore/duration.h"
#include "libmscore/measure.h"
#include "libmscore/tuplet.h"
#include "libmscore/mscore.h"


namespace Ms {
namespace MidiTuplet {

void addElementToTuplet(int voice,
                        const ReducedFraction &onTime,
                        const ReducedFraction &len,
                        DurationElement *el,
                        std::multimap<ReducedFraction, TupletData> &tuplets)
      {
      const auto it = findTupletForTimeRange(voice, onTime, len, tuplets);
      if (it != tuplets.end()) {
            auto &tuplet = const_cast<TupletData &>(it->second);
            tuplet.elements.push_back(el);       // add chord/rest to the tuplet
            }
      }

void createTuplets(Staff *staff,
                   const std::multimap<ReducedFraction, TupletData> &tuplets)
      {
      Score* score = staff->score();
      const int track = staff->idx() * VOICES;

      for (const auto &tupletEvent: tuplets) {
            const auto &tupletData = tupletEvent.second;
            if (tupletData.elements.empty())
                  continue;

            Tuplet* tuplet = new Tuplet(score);
            const auto &tupletRatio = tupletLimits(tupletData.tupletNumber).ratio;
            tuplet->setRatio(tupletRatio.fraction());

            tuplet->setDuration(tupletData.len.fraction());
            const TDuration baseLen = tupletData.len.fraction() / tupletRatio.denominator();
            tuplet->setBaseLen(baseLen);

            tuplet->setTrack(track);
            tuplet->setTick(tupletData.onTime.ticks());
            Measure* measure = score->tick2measure(tupletData.onTime.ticks());
            tuplet->setParent(measure);

            for (DurationElement *el: tupletData.elements) {
                  tuplet->add(el);
                  el->setTuplet(tuplet);
                  }
            }
      }

} // namespace MidiTuplet
} // namespace Ms