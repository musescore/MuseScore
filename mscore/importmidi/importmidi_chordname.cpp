#include "importmidi_chordname.h"
#include "importmidi_inner.h"
#include "importmidi_chord.h"
#include "importmidi_fraction.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/harmony.h"
#include "midi/midifile.h"
#include "mscore/preferences.h"


// From XF Format Specifications V 2.01 (January 13, 1999, YAMAHA CORPORATION)

namespace Ms {
namespace MidiChordName {

int readFirstHalf(uchar byte)
      {
      return (byte >> 4) & 0xf;
      }

int readSecondHalf(uchar byte)
      {
      return byte & 0xf;
      }

QString readChordRoot(uchar byte)
      {
      static const std::vector<QString> inversions = {
            "bbb", "bb", "b", "", "#", "##", "###"
            };
      static const std::vector<QString> notes = {
            "", "C", "D", "E", "F", "G", "A", "B"
            };

      QString chordRoot;

      const size_t noteIndex = readSecondHalf(byte);
      if (noteIndex < notes.size())
            chordRoot += notes[noteIndex];

      const size_t inversionIndex = readFirstHalf(byte);
      if (inversionIndex < inversions.size())
            chordRoot += inversions[inversionIndex];

      return chordRoot;
      }

QString readChordType(uchar chordTypeIndex)
      {
      static const std::vector<QString> chordTypes = {
            "Maj"
          , "Maj6"
          , "Maj7"
          , "Maj7(#11)"
          , "Maj(9)"
          , "Maj7(9)"
          , "Maj6(9)"
          , "aug"
          , "min"
          , "min6"
          , "min7"
          , "min7b5"
          , "min(9)"
          , "min7(9)"
          , "min7(11)"
          , "minMaj7"
          , "minMaj7(9)"
          , "dim"
          , "dim7"
          , "7th"
          , "7sus4"
          , "7b5"
          , "7(9)"
          , "7(#11)"
          , "7(13)"
          , "7(b9)"
          , "7(b13)"
          , "7(#9)"
          , "Maj7aug"
          , "7aug"
          , "1+8"
          , "1+5"
          , "sus4"
          , "1+2+5"
          , "cc"
            };

      if (chordTypeIndex < chordTypes.size())
            return chordTypes[chordTypeIndex];
      return "";
      }

QString readChordName(const MidiEvent &e)
      {
      if (e.type() != ME_META || e.metaType() != META_SPECIFIC)
            return "";
      if (e.len() < 4)
            return "";

      const uchar *data = e.edata();
      if (data[0] != 0x43 || data[1] != 0x7B || data[2] != 0x01)
            return "";

      QString chordName;

      if (e.len() >= 4)
            chordName += readChordRoot(data[3]);
      if (e.len() >= 5)
            chordName += readChordType(data[4]);

      if (e.len() >= 6) {
            const QString chordRoot = readChordRoot(data[5]);
            if (!chordRoot.isEmpty()) {
                  QString chordType;
                  if (e.len() >= 7)
                        chordType = readChordType(data[6]);
                  if (chordRoot + chordType == chordName)
                        chordName += "/" + chordRoot;
                  else
                        chordName += "/" + chordRoot + chordType;
                  }
            }

      return chordName;
      }

QString findChordName(const QList<MidiNote> &notes, const std::multimap<int, MidiEvent>& events)
      {
      for (const MidiNote &note: notes) {
            const auto range = events.equal_range(note.origOnTime.ticks());
            if (range.second == range.first)
                  continue;
            for (auto it = range.first; it != range.second; ++it) {
                  const MidiEvent &e = it->second;
                  const QString chordName = readChordName(e);
                  if (!chordName.isEmpty())
                        return chordName;
                  }
            }
      return "";
      }

// all notes should be already placed to the score

void setChordNames(QList<MTrack> &tracks)
      {
      auto &data = *preferences.midiImportOperations.data();

      for (MTrack &track: tracks) {
            if (data.processingsOfOpenedFile > 0
                        && (!data.trackOpers.showChordNames.value(track.indexOfOperation)
                            || !data.trackOpers.hasChordNames.value(track.indexOfOperation))) {
                  continue;
                  }
            for (const auto &chord: track.chords) {
                  const MidiChord &c = chord.second;
                  const QString chordName = findChordName(c.notes, track.mtrack->events());

                  if (chordName.isEmpty())
                        continue;
                                    // to show chord names column in the MIDI import panel
                  if (data.processingsOfOpenedFile == 0)
                        data.trackOpers.hasChordNames.setValue(track.indexOfOperation, true);

                  if (data.trackOpers.showChordNames.value(track.indexOfOperation)) {
                        Staff *staff = track.staff;
                        Score *score = staff->score();
                        const ReducedFraction &onTime = chord.first;

                        Measure* measure = score->tick2measure(onTime.ticks());
                        Segment* seg = measure->getSegment(Segment::Type::ChordRest,
                                                           onTime.ticks());
                        const int t = staff->idx() * VOICES;

                        Harmony* h = new Harmony(score);
                        h->setHarmony(chordName);
                        h->setTrack(t);
                        seg->add(h);
                        }
                  }
            }
      }

} // namespace MidiChordName
} // namespace Ms
