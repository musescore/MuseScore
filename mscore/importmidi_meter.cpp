#include "importmidi_meter.h"
#include "libmscore/fraction.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"

#include <memory>


namespace Ms {
namespace Meter {

struct MaxLevel
      {
      int level = 0;         // 0 - the biggest, whole bar level; other: -1, -2, ...
      int levelCount = 0;    // number of ticks with 'level' value
      int lastPos = -1;      // position of last tick with value 'level'; -1 - undefined pos
      };

bool isSimple(const Fraction &barFraction)       // 2/2, 3/4, 4/4, ...
      {
      return barFraction.numerator() < 5;
      }

bool isCompound(const Fraction &barFraction)     // 6/8, 12/4, ...
      {
      return (barFraction.numerator() % 3 == 0 && barFraction.numerator() > 3);
      }

bool isComplex(const Fraction &barFraction)      // 5/4, 7/8, ...
      {
      return !isSimple(barFraction) && !isCompound(barFraction);
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


QList<int> metricDivisionsOfBar(const Fraction &barFraction)
      {
      int barLen = barFraction.ticks();
      // value in list is a length (in ticks) of every part of bar
      // on which bar is subdivided on each level
      QList<int> divLevels;
      divLevels.append(barLen);

      // pulse-level division
      if (Meter::isDuple(barFraction))
            divLevels.append(barLen / 2);
      else if (Meter::isTriple(barFraction))
            divLevels.append(barLen / 3);
      else if (Meter::isQuadruple(barFraction)) {
            divLevels.append(barLen / 2); // additional central accent
            divLevels.append(barLen / 4);
            }
      else {
            // if complex meter - not a complete solution: pos of central accent is unknown
            // so select the pos closest to center
            divLevels.append(barLen / (barFraction.numerator() / 2)); // additional central accent
            divLevels.append(barLen / barFraction.numerator());
            }

      if (Meter::isCompound(barFraction)) {
            // add invalid level to make further levels -1 smaller
            // (not so significant compared to beats)
            int pulseLen = divLevels.last();
            divLevels.append(-1);
            // subdivide pulse of compound meter into 3 parts
            divLevels.append(pulseLen / 3);
            }

      // smallest allowed subdivision is 1/128
      const int minDuration = MScore::division / 32;
      // subdivide duration further
      while ((divLevels.last() % 2 == 0) && (divLevels.last() > minDuration))
            divLevels.append(divLevels.last() / 2);

      return divLevels;
      }

Meter::MaxLevel maxLevelBetween(int startTickInBar, int endTickInBar,
                                const QList<int> &divLevels)
      {
      Meter::MaxLevel level;
      // (begin() + 1) because bar start (max level) cannot be between
      // level = -1 because we start from half bar division
      level.level = -1;
      for (auto p = divLevels.begin() + 1; p != divLevels.end(); ++p) {
            int divLen = *p;
            if (divLen > 0) {
                  int maxEndRaster = (endTickInBar / divLen) * divLen;
                  if (maxEndRaster == endTickInBar)
                        maxEndRaster -= divLen;
                  if (startTickInBar < maxEndRaster) {
                        // max level found
                        level.lastPos = maxEndRaster;
                        int maxStartRaster = (startTickInBar / divLen) * divLen;
                        level.levelCount = (maxEndRaster - maxStartRaster) / divLen;
                        break;
                        }
                  }
            --level.level;
            }
      return level;
      }

int levelOfTick(int tick, const QList<int> &divLevels)
      {
      int level = 0;
      for (const auto &divLen: divLevels) {
            if (divLen > 0) {
                  if (tick % divLen == 0)
                        return level;
                  }
            --level;
            }
      return level;
      }

bool isPowerOfTwo(unsigned int x)
      {
      return x && !(x & (x - 1));
      }

bool isSingleNoteDuration(int ticks)
      {
      int div = (ticks > MScore::division)
                  ? ticks / MScore::division
                  : MScore::division / ticks;
      if (div > 0)
            return isPowerOfTwo((unsigned int)div);
      return false;
      }

bool isQuarterDuration(int ticks)
      {
      Fraction f = Fraction::fromTicks(ticks);
      return (f.numerator() == 1 && f.denominator() == 4);
      }

int beatLength(const Fraction &barFraction)
      {
      int barLen = barFraction.ticks();
      int beatLen = barLen / 4;
      if (Meter::isDuple(barFraction))
            beatLen = barLen / 2;
      else if (Meter::isTriple(barFraction))
            beatLen = barLen / 3;
      else if (Meter::isQuadruple(barFraction))
            beatLen = barLen / 4;
      return beatLen;
      }

// If last 2/3 of beat in compound meter is rest,
// it should be splitted into 2 rests

bool is23EndOfBeatInCompoundMeter(int startTickInBar, int endTickInBar,
                                      const Fraction &barFraction)
      {
      if (endTickInBar - startTickInBar <= 0)
            return false;
      if (!isCompound(barFraction))
            return false;

      int beatLen = beatLength(barFraction);
      int divLen = beatLen / 3;

      if ((startTickInBar - (startTickInBar / beatLen) * beatLen == divLen)
                  && (endTickInBar % beatLen == 0))
            return true;
      return false;
      }

bool isHalfDuration(int ticks)
      {
      Fraction f = Fraction::fromTicks(ticks);
      return (f.numerator() == 1 && f.denominator() == 2);
      }

// 3/4: if half rest starts from beg of bar or ends on bar end
// then it is a bad practice - need to split rest into 2 quarter rests

bool isHalfRestOn34(int startTickInBar, int endTickInBar,
                     const Fraction &barFraction)
      {
      if (endTickInBar - startTickInBar <= 0)
            return false;
      if (barFraction.numerator() == 3 && barFraction.denominator() == 4
                  && (startTickInBar == 0 || endTickInBar == barFraction.ticks())
                  && isHalfDuration(endTickInBar - startTickInBar))
            return true;
      return false;
      }


// Node for binary tree of durations

struct Node
      {
      Node(int startTick, int endTick, int startLevel, int endLevel)
            : startTick(startTick), endTick(endTick)
            , startLevel(startLevel), endLevel(endLevel)
            , parent(nullptr)
            {}

      int startTick;
      int endTick;
      int startLevel;
      int endLevel;

      Node *parent;
      std::unique_ptr<Node> left;
      std::unique_ptr<Node> right;
      };

void treeToDurationList(Node *node, QList<TDuration> &dl, bool useDots)
      {
      if (node->left != nullptr && node->right != nullptr) {
            treeToDurationList(node->left.get(), dl, useDots);
            treeToDurationList(node->right.get(), dl, useDots);
            }
      else
            dl.append(toDurationList(
                  Fraction::fromTicks(node->endTick - node->startTick), useDots));
      }

// duration start/end should be quantisized at least to 1/128 note

QList<TDuration> toDurationList(int startTickInBar, int endTickInBar,
                                const Fraction &barFraction, DurationType durationType,
                                bool useDots)
      {
      QList<TDuration> durations;
      if (startTickInBar < 0 || endTickInBar <= startTickInBar
                  || endTickInBar > barFraction.ticks())
            return durations;

      // analyse mectric structure of bar
      QList<int> divLevels = metricDivisionsOfBar(barFraction);
      // create a root for binary tree of durationsstd::multimap<int, MidiChord>
      std::unique_ptr<Node> root(new Node(startTickInBar, endTickInBar,
                                          levelOfTick(startTickInBar, divLevels),
                                          levelOfTick(endTickInBar, divLevels)));
      QQueue<Node *> q;
      q.enqueue(root.get());

      // max allowed difference between start/end level of duration and split point level
      int tol = 0;
      if (durationType == DurationType::NOTE)
            tol = 1;
      else if (durationType == DurationType::REST)
            tol = 0;

      // the smallest allowed duration of node before splitting is 1/64
      // the lowest division level duration is 1/128
      // so each child node duration after division is not less than 1/128
      const int minDuration = MScore::division / 16;

      // build duration tree such that durations don't go across strong beat divisions
      while (!q.isEmpty()) {
            Node *node = q.dequeue();
            // don't split node if its duration is less than minDuration
            if (node->endTick - node->startTick < minDuration)
                  continue;
            auto splitPoint = maxLevelBetween(node->startTick, node->endTick, divLevels);
            // sum levels if there are several positions (beats) with max level value
            // for example, 8th + half duration + 8th in 3/4, and half is over two beats
            int effectiveLevel = splitPoint.level + splitPoint.levelCount - 1;
            if (effectiveLevel - node->startLevel > tol
                        || effectiveLevel - node->endLevel > tol
                        || isHalfRestOn34(node->startTick, node->endTick, barFraction)
                        || (durationType == DurationType::REST
                            && is23EndOfBeatInCompoundMeter(node->startTick, node->endTick, barFraction))
                        )
                  {

                  // split duration by splitPoint position
                  node->left.reset(new Node(node->startTick, splitPoint.lastPos,
                                            node->startLevel, splitPoint.level));
                  node->left->parent = node;
                  q.enqueue(node->left.get());

                  node->right.reset(new Node(splitPoint.lastPos, node->endTick,
                                             splitPoint.level, node->endLevel));
                  node->right->parent = node;
                  q.enqueue(node->right.get());
                  }
            }

      // collect the resulting durations
      treeToDurationList(root.get(), durations, useDots);
      return durations;
      }

} // namespace Meter
} // namespace Ms
