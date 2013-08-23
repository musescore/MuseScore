#include "importmidi_swing.h"
#include "libmscore/score.h"
#include "libmscore/chordrest.h"
#include "libmscore/stafftext.h"
#include "libmscore/element.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"
#include "libmscore/staff.h"
#include "libmscore/tuplet.h"
#include "importmidi_fraction.h"


namespace Ms {
namespace Swing {

class SwingDetector
      {
   public:
      SwingDetector(MidiOperation::Swing st);

      void add(ChordRest *cr);
      bool wasSwingApplied() const { return swingApplied; }

   private:
      std::vector<ChordRest *> elements;
      ReducedFraction sumLen;
      const ReducedFraction FULL_LEN = {1, 4};
      MidiOperation::Swing swingType;
      bool swingApplied = false;

      void reset();
      void append(ChordRest *cr);
      void checkNormalSwing();
      void checkShuffle();
      void applySwing();
      bool areAllTuplets() const;
      bool areAllNonTuplets() const;
      };


SwingDetector::SwingDetector(MidiOperation::Swing st)
      : swingType(st)
      {
      }

void SwingDetector::add(ChordRest *cr)
      {
      if (elements.empty()) {
            if (ReducedFraction(cr->globalDuration()) >= FULL_LEN)
                  return;
            const int tickInBar = cr->tick() - cr->measure()->tick();
            if (tickInBar % MScore::division == 0)
                  append(cr);
            }
      else {
            if (sumLen + ReducedFraction(cr->globalDuration()) > FULL_LEN) {
                  reset();
                  return;
                  }
            append(cr);
            if (sumLen == FULL_LEN) {
                              // check for swing patterns
                  switch (swingType) {
                        case MidiOperation::Swing::SWING:
                              checkNormalSwing();
                              break;
                        case MidiOperation::Swing::SHUFFLE:
                              checkShuffle();
                              break;
                        default:
                              break;
                        }
                  reset();
                  }
            }
      }

void SwingDetector::reset()
      {
      elements.clear();
      sumLen = ReducedFraction(0);
      }

void SwingDetector::append(ChordRest *cr)
      {
      if (cr->type() == Element::CHORD || cr->type() == Element::REST) {
            elements.push_back(cr);
            sumLen += ReducedFraction(cr->globalDuration());
            }
      }

void SwingDetector::checkNormalSwing()
      {
      if (elements.size() == 2
                  && areAllTuplets()
                  && (elements[0]->type() == Element::CHORD
                      || elements[1]->type() == Element::CHORD)
                  && elements[0]->duration().reduced() == Fraction(1, 4)
                  && elements[1]->duration().reduced() == Fraction(1, 8))
            {
                        // swing with two 8th notes
                        // or 8th note + 8th rest
                        // or 8th rest + 8th note
            applySwing();
            }
      else if (elements.size() == 3
               && elements[0]->type() == Element::CHORD
               && elements[1]->type() == Element::REST
               && elements[2]->type() == Element::CHORD
               && elements[0]->duration().reduced() == Fraction(1, 8)
               && elements[1]->duration().reduced() == Fraction(1, 8)
               && elements[2]->duration().reduced() == Fraction(1, 8))
            {
                        // swing with two 8th notes
            applySwing();
            }
      }

void SwingDetector::checkShuffle()
      {
      if (elements.size() == 2
                  && areAllNonTuplets()
                  && elements[0]->type() == Element::CHORD
                  && (elements[1]->type() == Element::CHORD
                      || elements[1]->type() == Element::REST)
                  && elements[0]->duration().reduced() == Fraction(3, 16)  // dotted 8th
                  && elements[1]->duration().reduced() == Fraction(1, 16))
            {
                        // swing with two 8th notes
                        // or 8th note + 8th rest
            applySwing();
            }
      }

void SwingDetector::applySwing()
      {
      if (elements.size() != 2 && elements.size() != 3)
            return;

      Tuplet *tuplet = nullptr;
      for (ChordRest *el: elements) {
            el->setDurationType(TDuration::V_EIGHT);
            el->setDuration(Fraction(1, 8));
            el->setDots(0);
            if (el->tuplet()) {
                  if (!tuplet)
                        tuplet = el->tuplet();
                  el->setTuplet(nullptr);
                  }
            }
      if (elements.size() == 3) {
                  // remove central rest
            ChordRest *cr = elements[1];
            cr->score()->removeElement(cr);
            delete cr;
            }

      const ChordRest *first = elements.front();
      const int startTick = first->segment()->tick();
      ChordRest *last = elements.back();
      last->segment()->remove(last);
      Segment *s = last->measure()->getSegment(last, startTick + MScore::division / 2);
      s->add(last);

      if (tuplet) {
                  // delete tuplet
            delete tuplet;
            tuplet = nullptr;
            }
      if (!swingApplied)
            swingApplied = true;
      }

bool SwingDetector::areAllTuplets() const
      {
      for (const auto &el: elements) {
            if (!el->tuplet())
                  return false;
            }
      return true;
      }

bool SwingDetector::areAllNonTuplets() const
      {
      for (const auto &el: elements) {
            if (el->tuplet())
                  return false;
            }
      return true;
      }

// ---------------------------------------------------------------

QString swingCaption(MidiOperation::Swing swingType)
      {
      QString caption;
      switch (swingType) {
            case MidiOperation::Swing::SWING:
                  caption = "Swing";
                  break;
            case MidiOperation::Swing::SHUFFLE:
                  caption = "Shuffle";
                  break;
            case MidiOperation::Swing::NONE:
                  break;
            }
      return caption;
      }

void detectSwing(Staff *staff, MidiOperation::Swing swingType)
      {
      Score *score = staff->score();
      const int strack = staff->idx() * VOICES;
      SwingDetector swingDetector(swingType);

      for (Segment *seg = score->firstSegment(Segment::SegChordRest); seg;
                                      seg = seg->next1(Segment::SegChordRest)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
                  if (!cr)
                        continue;
                  swingDetector.add(cr);
                  }
            }
      if (swingDetector.wasSwingApplied()) {
                        // add swing label to the score
            StaffText* st = new StaffText(score);
            st->setTextStyleType(TEXT_STYLE_STAFF);
            st->setText(swingCaption(swingType));
            Segment* seg = score->firstSegment(Segment::SegChordRest);
            st->setParent(seg);
            st->setTrack(strack);   // voice == 0
            score->addElement(st);
            }
      }

} // namespace Swing
} // namespace Ms
