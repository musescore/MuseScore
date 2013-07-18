#include "importmidi_meter.h"
#include "libmscore/fraction.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "importmidi_tuplet.h"
#include "importmidi_inner.h"

#include <memory>


namespace Ms {
namespace Meter {

bool isSimple(const Fraction &barFraction)       // 2/2, 3/4, 4/4, ...
      {
      return barFraction.numerator() < 5;
      }

bool isCompound(const Fraction &barFraction)     // 6/8, 12/4, ...
      {
      return barFraction.numerator() % 3 == 0 && barFraction.numerator() > 3;
      }

bool isComplex(const Fraction &barFraction)      // 5/4, 7/8, ...
      {
      return barFraction.numerator() == 5 || barFraction.numerator() == 7;
      }

bool isDuple(const Fraction &barFraction)        // 2/2, 6/8, ...
      {
      return barFraction.numerator() == 2 || barFraction.numerator() == 6;
      }

bool isTriple(const Fraction &barFraction)       // 3/4, 9/4, ...
      {
      return barFraction.numerator() == 3 || barFraction.numerator() == 9;
      }

bool isQuadruple(const Fraction &barFraction)    // 4/4, 12/8, ...
      {
      return barFraction.numerator() % 4 == 0;
      }

Fraction minAllowedDuration()
      {
      return Fraction::fromTicks(MScore::division / 32);    // smallest allowed duration is 1/128
      }


// list of bar division lengths in ticks (whole bar len, half bar len, ...)
// and its corresponding levels
// tuplets are not taken into account here

DivisionInfo metricDivisionsOfBar(const Fraction &barFraction)
      {
      DivisionInfo barDivInfo;
      barDivInfo.len = barFraction;
                  // first value of each element in list is a length (in ticks) of every part of bar
                  // on which bar is subdivided on each level
                  // the level value is a second value of each element
      auto &divLengths = barDivInfo.divLengths;
      int level = 0;
      divLengths.push_back({barFraction, level});
                  // pulse-level division
      if (Meter::isDuple(barFraction))
            divLengths.push_back({barFraction / 2, --level});
      else if (Meter::isTriple(barFraction))
            divLengths.push_back({barFraction / 3, --level});
      else if (Meter::isQuadruple(barFraction)) {
            divLengths.push_back({barFraction / 2, --level});    // additional central accent
            divLengths.push_back({barFraction / 4, --level});
            }
      else {
                        // if complex meter - not a complete solution: pos of central accent is unknown
            divLengths.push_back({barFraction / barFraction.numerator(), --level});
            }

      if (Meter::isCompound(barFraction)) {
            --level;    // additional min level for pulse divisions
                        // subdivide pulse of compound meter into 3 parts
            divLengths.push_back({divLengths.back().len / 3, --level});
            }

      while (divLengths.back().len >= minAllowedDuration() * 2)
            divLengths.push_back({divLengths.back().len / 2, --level});

      return barDivInfo;
      }

DivisionInfo metricDivisionsOfTuplet(const MidiTuplet::TupletData &tuplet,
                                     int tupletStartLevel)
      {
      DivisionInfo tupletDivInfo;
      tupletDivInfo.onTime = tuplet.onTime;
      tupletDivInfo.len = tuplet.len;
      tupletDivInfo.isTuplet = true;
      tupletDivInfo.divLengths.push_back({tuplet.len, TUPLET_BOUNDARY_LEVEL});
      Fraction divLen = tuplet.len / tuplet.tupletNumber;
      tupletDivInfo.divLengths.push_back({divLen, tupletStartLevel--});
      while (tupletDivInfo.divLengths.back().len >= minAllowedDuration() * 2) {
            tupletDivInfo.divLengths.push_back({
                        tupletDivInfo.divLengths.back().len / 2, tupletStartLevel--});
            }
      return tupletDivInfo;
      }

Fraction beatLength(const Fraction &barFraction)
      {
      Fraction beatLen = barFraction / 4;
      if (Meter::isDuple(barFraction))
            beatLen = barFraction / 2;
      else if (Meter::isTriple(barFraction))
            beatLen = barFraction / 3;
      else if (Meter::isQuadruple(barFraction))
            beatLen = barFraction / 4;
      else if (Meter::isComplex(barFraction))
            beatLen = barFraction / barFraction.numerator();
      return beatLen;
      }

std::vector<Fraction> divisionsOfBarForTuplets(const Fraction &barFraction)
      {
      DivisionInfo info = metricDivisionsOfBar(barFraction);
      std::vector<Fraction> divLengths;
      Fraction beatLen = beatLength(barFraction);
      for (const auto &i: info.divLengths) {
                        // in compound meter tuplet starts from beat level, not the whole bar
            if (Meter::isCompound(barFraction) && i.len > beatLen)
                  continue;
            divLengths.push_back(i.len);
            }
      return divLengths;
      }

// result in vector: first elements - all tuplets info, one at the end - bar division info

std::vector<DivisionInfo> divisionInfo(const Fraction &barFraction,
                                       const std::vector<MidiTuplet::TupletData> &tupletsInBar)
      {
      std::vector<DivisionInfo> divsInfo;

      auto barDivisionInfo = metricDivisionsOfBar(barFraction);
      for (const auto &tuplet: tupletsInBar) {
            int tupletStartLevel = 0;
            for (const auto &divLenInfo: barDivisionInfo.divLengths) {
                  if (divLenInfo.len == tuplet.len) {
                        tupletStartLevel = divLenInfo.level;
                        break;
                        }
                  }
            divsInfo.push_back(metricDivisionsOfTuplet(tuplet, --tupletStartLevel));
            }
      divsInfo.push_back(barDivisionInfo);

      return divsInfo;
      }

// tick is counted from the beginning of bar

int levelOfTick(const Fraction &tick, const std::vector<DivisionInfo> &divsInfo)
      {
      for (const auto &divInfo: divsInfo) {
            if (tick < divInfo.onTime || tick > divInfo.onTime + divInfo.len)
                  continue;
            for (const auto &divLenInfo: divInfo.divLengths) {
                  if ((tick - divInfo.onTime).ticks() % divLenInfo.len.ticks() == 0)
                        return divLenInfo.level;
                  }
            }
      return 0;
      }

// return level with lastPos == Fraction(-1, 1) if undefined - see MaxLevel class

Meter::MaxLevel maxLevelBetween(const Fraction &startTickInBar,
                                const Fraction &endTickInBar,
                                const DivisionInfo &divInfo)
      {
      Meter::MaxLevel level;
      Fraction startTickInDiv = startTickInBar - divInfo.onTime;
      Fraction endTickInDiv = endTickInBar - divInfo.onTime;
      if (startTickInDiv >= endTickInDiv
                  || startTickInDiv < 0 || endTickInDiv > divInfo.len)
            return level;

      for (const auto &divLengthInfo: divInfo.divLengths) {
            const auto &divLen = divLengthInfo.len;
            Fraction maxEndRaster = divLen * (endTickInDiv.ticks() / divLen.ticks());
            if (maxEndRaster == endTickInDiv)
                  maxEndRaster -= divLen;
            if (startTickInDiv < maxEndRaster) {
                              // max level is found
                  level.lastPos = maxEndRaster + divInfo.onTime;
                  Fraction maxStartRaster = divLen * (startTickInDiv.ticks() / divLen.ticks());
                  Fraction count = (maxEndRaster - maxStartRaster) / divLen;
                  level.levelCount = qRound(count.numerator() * 1.0 / count.denominator());
                  level.level = divLengthInfo.level;
                  break;
                  }
            }
      return level;
      }

// vector<DivisionInfo>:
//    first elements - tuplet division info, if there are any tuplets
//    last element - always the whole bar division info

// here we use levelCount = 1 always for simplicity
// because TUPLET_BOUNDARY_LEVEL is 'max enough'

Meter::MaxLevel findMaxLevelBetween(const Fraction &startTickInBar,
                                    const Fraction &endTickInBar,
                                    const std::vector<DivisionInfo> &divsInfo)
      {
      Meter::MaxLevel level;

      for (const auto &divInfo: divsInfo) {
            if (divInfo.isTuplet) {
                  if (startTickInBar < divInfo.onTime + divInfo.len
                              && endTickInBar > divInfo.onTime + divInfo.len) {
                        level.level = TUPLET_BOUNDARY_LEVEL;
                        level.levelCount = 1;
                        level.lastPos = divInfo.onTime + divInfo.len;
                        break;
                        }
                  if (startTickInBar < divInfo.onTime
                              && endTickInBar > divInfo.onTime
                              && endTickInBar <= divInfo.onTime + divInfo.len) {
                        level.level = TUPLET_BOUNDARY_LEVEL;
                        level.levelCount = 1;
                        level.lastPos = divInfo.onTime;
                        break;
                        }
                  if (startTickInBar >= divInfo.onTime
                              && endTickInBar <= divInfo.onTime + divInfo.len) {
                        level = maxLevelBetween(startTickInBar, endTickInBar, divInfo);
                        break;
                        }
                  }
            else {
                  level = maxLevelBetween(startTickInBar, endTickInBar, divInfo);
                  break;
                  }
            }
      return level;
      }

int tupletNumberForDuration(const Fraction &startTick,
                            const Fraction &endTick,
                            const std::vector<MidiTuplet::TupletData> &tupletsInBar)
      {
      for (const auto &tupletData: tupletsInBar) {
            if (startTick >= tupletData.onTime
                        && endTick <= tupletData.onTime + tupletData.len)
                  return tupletData.tupletNumber;
            }
      return -1;  // this duration is not inside any tuplet
      }

bool isPowerOfTwo(unsigned int x)
      {
      return x && !(x & (x - 1));
      }

bool isSingleNoteDuration(const Fraction &ticks)
      {
      Fraction division = Fraction::fromTicks(MScore::division);
      Fraction div = (ticks > division) ? ticks / division : division / ticks;
      if (div > Fraction(0))
            return isPowerOfTwo((unsigned int)div.ticks());
      return false;
      }

bool isQuarterDuration(const Fraction &ticks)
      {
      return (ticks.numerator() == 1 && ticks.denominator() == 4);
      }

// If last 2/3 of beat in compound meter is rest,
// it should be splitted into 2 rests

bool is23EndOfBeatInCompoundMeter(const Fraction &startTickInBar,
                                  const Fraction &endTickInBar,
                                  const Fraction &barFraction)
      {
      if (endTickInBar <= startTickInBar)
            return false;
      if (!isCompound(barFraction))
            return false;

      Fraction beatLen = beatLength(barFraction);
      Fraction divLen = beatLen / 3;

      if ((startTickInBar - beatLen * (startTickInBar.ticks() / beatLen.ticks()) == divLen)
                  && (endTickInBar.ticks() % beatLen.ticks() == 0))
            return true;
      return false;
      }

bool isHalfDuration(const Fraction &f)
      {
      Fraction ff = f.reduced();
      return ff.numerator() == 1 && ff.denominator() == 2;
      }

// 3/4: if half rest starts from beg of bar or ends on bar end
// then it is a bad practice - need to split rest into 2 quarter rests

bool isHalfRestOn34(const Fraction &startTickInBar,
                    const Fraction &endTickInBar,
                    const Fraction &barFraction)
      {
      if (endTickInBar - startTickInBar <= Fraction(0))
            return false;
      if (barFraction.numerator() == 3 && barFraction.denominator() == 4
                  && (startTickInBar == Fraction(0) || endTickInBar == barFraction)
                  && isHalfDuration(endTickInBar - startTickInBar))
            return true;
      return false;
      }


// Node for binary tree of durations

struct Node
      {
      Node(Fraction startTick, Fraction endTick, int startLevel, int endLevel)
            : startPos(startTick)
            , endPos(endTick)
            , startLevel(startLevel)
            , endLevel(endLevel)
            , tupletRatio({2, 2})
            , parent(nullptr)
            {}

      Fraction startPos;
      Fraction endPos;
      int startLevel;
      int endLevel;
             // all durations inside tuplets are smaller/larger than their regular versions
             // this difference is represented by tuplet ratio: 3/2 for triplets, etc.
      Fraction tupletRatio;

      Node *parent;
      std::unique_ptr<Node> left;
      std::unique_ptr<Node> right;
      };

void treeToDurationList(Node *node,
                        QList<std::pair<Fraction, TDuration>> &dl,
                        bool useDots)
      {
      if (node->left != nullptr && node->right != nullptr) {
            treeToDurationList(node->left.get(), dl, useDots);
            treeToDurationList(node->right.get(), dl, useDots);
            }
      else {
            const int MAX_DOTS = 1;
            QList<TDuration> list = toDurationList(
                              node->tupletRatio * (node->endPos - node->startPos),
                              useDots, MAX_DOTS);
            for (const auto &duration: list)
                  dl.push_back({node->tupletRatio, duration});
            }
      }

// duration start/end should be quantisized, quantum >= 1/128 note
// pair here represents the tuplet ratio of duration and the duration itself
// for regular (non-tuplet) durations fraction.numerator == fraction.denominator

QList<std::pair<Fraction, TDuration> >
toDurationList(const Fraction &startTickInBar,
               const Fraction &endTickInBar,
               const Fraction &barFraction,
               const std::vector<MidiTuplet::TupletData> &tupletsInBar,
               DurationType durationType,
               bool useDots)
      {
      QList<std::pair<Fraction, TDuration>> durations;
      if (startTickInBar < 0 || endTickInBar <= startTickInBar || endTickInBar > barFraction)
            return durations;
                  // analyse mectric structure of bar
      auto divInfo = divisionInfo(barFraction, tupletsInBar);
                  // create a root for binary tree of durations
      std::unique_ptr<Node> root(new Node(startTickInBar, endTickInBar,
                                          levelOfTick(startTickInBar, divInfo),
                                          levelOfTick(endTickInBar, divInfo)));
      QQueue<Node *> nodesToProcess;
      nodesToProcess.enqueue(root.get());
                  // max allowed difference between start/end level of duration and split point level
      int tol = 0;
      if (durationType == DurationType::NOTE)
            tol = 1;
      else if (durationType == DurationType::REST)
            tol = 0;
                  // each child node duration after division is not less than minDuration()
      const Fraction minDuration = minAllowedDuration() * 2;
                  // build duration tree such that durations don't go across strong beat divisions
      while (!nodesToProcess.isEmpty()) {
            Node *node = nodesToProcess.dequeue();
                        // if node duration is completely inside some tuplet
                        // then assign to the node tuplet-to-regular-duration conversion coefficient
            if (node->tupletRatio.numerator() == node->tupletRatio.denominator()) {
                  int tupletNumber = tupletNumberForDuration(node->startPos,
                                                             node->endPos, tupletsInBar);
                  if (tupletNumber != -1) {
                        auto it = MidiTuplet::tupletRatios().find(tupletNumber);
                        if (it != MidiTuplet::tupletRatios().end())
                              node->tupletRatio = it->second;
                        else
                              qDebug("Tuplet ratio not found for tuplet number: %d", tupletNumber);
                        }
                  }
                        // don't split node if its duration is less than minDuration
            if (node->endPos - node->startPos < minDuration)
                  continue;
            auto splitPoint = findMaxLevelBetween(node->startPos, node->endPos, divInfo);
                        // sum levels if there are several positions (beats) with max level value
                        // for example, 8th + half duration + 8th in 3/4, and half is over two beats
            if (splitPoint.lastPos == Fraction(-1, 1))     // undefined
                  continue;
            int effectiveLevel = splitPoint.level + splitPoint.levelCount - 1;
            if (effectiveLevel - node->startLevel > tol
                        || effectiveLevel - node->endLevel > tol
                        || isHalfRestOn34(node->startPos, node->endPos, barFraction)
                        || (durationType == DurationType::REST
                            && is23EndOfBeatInCompoundMeter(node->startPos, node->endPos, barFraction))
                        )
                  {
                              // split duration in splitPoint position
                  node->left.reset(new Node(node->startPos, splitPoint.lastPos,
                                            node->startLevel, splitPoint.level));
                  node->left->parent = node;
                  nodesToProcess.enqueue(node->left.get());

                  node->right.reset(new Node(splitPoint.lastPos, node->endPos,
                                             splitPoint.level, node->endLevel));
                  node->right->parent = node;
                  nodesToProcess.enqueue(node->right.get());

                  if (node->tupletRatio.reduced() != Fraction(1, 1)) {
                        node->left->tupletRatio = node->tupletRatio;
                        node->right->tupletRatio = node->tupletRatio;
                        }
                  }
            }
                  // collect the resulting durations
      treeToDurationList(root.get(), durations, useDots);
      return durations;
      }

} // namespace Meter
} // namespace Ms
