#include "importmidi_tempo.h"

#include "importmidi_inner.h"
#include "importmidi_beat.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/tempo.h"
#include "libmscore/tempotext.h"
#include "importmidi_operations.h"


namespace Ms {
namespace MidiTempo {

ReducedFraction time2Tick(double time, double ticksPerSec)
      {
      return ReducedFraction::fromTicks(int(ticksPerSec * time));
      }

// tempo in beats per second

double findBasicTempo(const std::multimap<int, MTrack> &tracks, bool isHumanPerformance)
      {
      for (const auto &track: tracks) {
                        // don't read tempo from tempo track for human performed files
                        // because very often the tempo in such track is completely erroneous
            if (isHumanPerformance && track.second.chords.empty())
                  continue;
            for (const auto &ie : track.second.mtrack->events()) {
                  const MidiEvent &e = ie.second;
                  if (e.type() == ME_META && e.metaType() == META_TEMPO) {
                        const uchar* data = (uchar*)e.edata();
                        const unsigned tempo = data[2] + (data[1] << 8) + (data[0] << 16);
                        return 1000000.0 / double(tempo);
                        }
                  }
            }

      return 2;   // default beats per second = 120 beats per minute
      }

void setTempoToScore(Score *score, int tick, double beatsPerSecond)
      {
      if (score->tempomap()->find(tick) != score->tempomap()->end())
            return;
                  // don't repeat tempo, always set only tempo for tick 0
      if (tick > 0 && score->tempo(tick) == beatsPerSecond)
            return;

      score->setTempo(tick, beatsPerSecond);

      auto *data = midiImportOperations.data();
      if (data->trackOpers.showTempoText.value()) {
            const int tempoInBpm = qRound(beatsPerSecond * 60.0);

            TempoText *tempoText = new TempoText(score);
            tempoText->setTempo(beatsPerSecond);
            tempoText->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(tempoInBpm));
            tempoText->setTrack(0);

            Measure *measure = score->tick2measure(tick);
            if (!measure) {
                  qDebug("MidiTempo::setTempoToScore: no measure for tick %d", tick);
                  return;
                  }
            Segment *segment = measure->getSegment(SegmentType::ChordRest, tick);
            if (!segment) {
                  qDebug("MidiTempo::setTempoToScore: no chord/rest segment for tempo at %d", tick);
                  return;
                  }
            segment->add(tempoText);
            data->hasTempoText = true;      // to show tempo text column in the MIDI import panel
            }
      }

double roundToBpm(double beatsPerSecond)
      {
      return qRound(beatsPerSecond * 60.0) / 60.0;
      }

void applyAllTempoEvents(const std::multimap<int, MTrack> &tracks, Score *score)
      {
      for (const auto &track: tracks) {
            if (track.second.isDivisionInTps) {     // ticks per second
                  const double ticksPerBeat = MScore::division;
                  const double beatsPerSecond = roundToBpm(track.second.division / ticksPerBeat);
                  setTempoToScore(score, 0, beatsPerSecond);
                  }
            else {      // beats per second
                  for (const auto &ie : track.second.mtrack->events()) {
                        const MidiEvent &e = ie.second;
                        if (e.type() == ME_META && e.metaType() == META_TEMPO) {
                              const auto tick = toMuseScoreTicks(
                                                ie.first, track.second.division, false);
                              const uchar* data = (uchar*)e.edata();
                              const unsigned tempo = data[2] + (data[1] << 8) + (data[0] << 16);
                              const double beatsPerSecond = roundToBpm(1000000.0 / tempo);
                              setTempoToScore(score, tick.ticks(), beatsPerSecond);
                              }
                        }
                  }
            }
      }

void setTempo(const std::multimap<int, MTrack> &tracks, Score *score)
      {
      score->tempomap()->clear();
      auto *midiData = midiImportOperations.data();
      std::set<ReducedFraction> beats = midiData->humanBeatData.beatSet;    // copy

      if (beats.empty()) {
                        // it's most likely not a human performance;
                        // we find all tempo events and set tempo changes to score
            applyAllTempoEvents(tracks, score);
            }
      else {            // calculate and set tempo from adjusted beat locations
            if (midiData->trackOpers.measureCount2xLess.value())
                  MidiBeat::removeEvery2ndBeat(beats);

            Q_ASSERT_X(beats.size() > 1, "MidiBeat::setTempo", "Human beat count < 2");

            double averageTempoFactor = 0.0;
            int counter = 0;
            auto it = beats.begin();
            auto beatStart = *it;
            const auto newBeatLen = ReducedFraction::fromTicks(MScore::division);

            for (++it; it != beats.end(); ++it) {
                  const auto &beatEnd = *it;

                  Q_ASSERT_X(beatEnd > beatStart, "MidiBeat::detectTempoChanges",
                             "Beat end <= beat start that is incorrect");

                  averageTempoFactor += (newBeatLen / (beatEnd - beatStart)).toDouble();
                  ++counter;
                  beatStart = beatEnd;
                  }
            averageTempoFactor /= counter;

            const double basicTempo = MidiTempo::findBasicTempo(tracks, true);
            const double tempo = roundToBpm(basicTempo * averageTempoFactor);

            score->tempomap()->clear();         // use only one tempo marking for all score
            setTempoToScore(score, 0, tempo);
            }

      if (score->tempomap()->empty())
            score->tempomap()->setTempo(0, 2.0);      // default tempo
      }

} // namespace MidiTempo
} // namespace Ms
