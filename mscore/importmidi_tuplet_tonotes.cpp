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
#include "libmscore/chordrest.h"


namespace Ms {
namespace MidiTuplet {

void addElementToTuplet(int voice,
                        const ReducedFraction &onTime,
                        const ReducedFraction &len,
                        DurationElement *el,
                        std::multimap<ReducedFraction, TupletData> &tuplets)
      {
      const auto foundTuplets = findTupletsForTimeRange(voice, onTime, len, tuplets);

#ifdef QT_DEBUG
      if (foundTuplets.size() > 1) {
            qDebug() << "Measure number (from 1):" << el->measure()->no() + 1
                     << ", staff index (from 0):" << el->staff()->idx();

            Q_ASSERT_X(false, "MidiTuplet::addElementToTuplet",
                       "More than one tuplet contains specified duration");
            }
#endif

      if (!foundTuplets.empty()) {
            auto &tuplet = const_cast<TupletData &>(foundTuplets.front()->second);
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
            tuplet->setVoice(tupletData.voice);
            Measure* measure = score->tick2measure(tupletData.onTime.ticks());
            tuplet->setParent(measure);

            for (DurationElement *el: tupletData.elements) {
                  tuplet->add(el);
                  el->setTuplet(tuplet);
                  }
            }
      }


#ifdef QT_DEBUG

void printInvalidTupletLocation(int measureIndex, int staffIndex)
      {
      qDebug() << "Tuplet is invalid; measure number (from 1):"
               << measureIndex + 1
               << ", staff index (from 0):" << staffIndex;
      }

bool haveTupletsEnoughElements(const Staff *staff)
      {
      const int strack = staff->idx() * VOICES;

      for (int voice = 0; voice < VOICES; ++voice) {
            for (Segment *seg = staff->score()->firstSegment(); seg; seg = seg->next1()) {
                  if (seg->segmentType() == Segment::Type::ChordRest) {
                        const ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
                        if (!cr)
                              continue;
                        const Tuplet *tuplet = cr->tuplet();
                        if (tuplet) {
                              if (tuplet->elements().size() <= 1) {
                                    printInvalidTupletLocation(seg->measure()->no(), staff->idx());
                                    return false;
                                    }
                              int chordCount = 0;
                              for (const auto &e: tuplet->elements()) {
                                    const ChordRest *cr = static_cast<ChordRest *>(e);
                                    if (cr && cr->type() == Element::Type::CHORD)
                                          ++chordCount;
                                    }
                              if (chordCount == 0) {
                                    printInvalidTupletLocation(seg->measure()->no(), staff->idx());
                                    return false;
                                    }
                              }
                        }
                  }
            }
      return true;
      }

#endif

} // namespace MidiTuplet
} // namespace Ms
