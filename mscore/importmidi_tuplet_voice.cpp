#include "importmidi_tuplet_voice.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "libmscore/mscore.h"
#include "preferences.h"

#include <set>


namespace Ms {
namespace MidiTuplet {

int voiceLimit()
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      return (operations.useMultipleVoices) ? VOICES : 1;
      }

int tupletVoiceLimit()
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
                  // for multiple voices: one voice is reserved for non-tuplet chords
      return (operations.useMultipleVoices) ? VOICES - 1 : 1;
      }

std::pair<ReducedFraction, ReducedFraction>
tupletInterval(const TupletInfo &tuplet,
               const ReducedFraction &basicQuant)
      {
      ReducedFraction tupletEnd = tuplet.onTime + tuplet.len;

      for (const auto &chord: tuplet.chords) {
            const auto offTime = Quantize::findMaxQuantizedOffTime(*chord.second, basicQuant);
            if (offTime > tupletEnd)
                  tupletEnd = offTime;
            }
      return std::make_pair(tuplet.onTime, tupletEnd);
      }

std::pair<ReducedFraction, ReducedFraction>
chordInterval(const std::pair<const ReducedFraction, MidiChord> &chord,
              const ReducedFraction &basicQuant)
      {
      const auto onTime = Quantize::findMinQuantizedOnTime(chord, basicQuant);
      const auto offTime = Quantize::findMaxQuantizedOffTime(chord, basicQuant);
      return std::make_pair(onTime, offTime);
      }

int findTupletWithChord(const MidiChord &midiChord,
                        const std::vector<TupletInfo> &tuplets)
      {
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            for (const auto &chord: tuplets[i].chords) {
                  if (&(chord.second->second) == &midiChord)
                        return i;
                  }
            }
      return -1;
      }

std::pair<ReducedFraction, ReducedFraction>
backTiedInterval(const TiedTuplet &tiedTuplet,
                 const std::vector<TupletInfo> &tuplets,
                 const ReducedFraction &basicQuant)
      {
      const TupletInfo &tuplet = tuplets[tiedTuplet.tupletIndex];
      const auto end = tupletInterval(tuplet, basicQuant).second;
      const MidiChord &midiChord = tiedTuplet.chord->second;

      const int tupletIndex = findTupletWithChord(midiChord, tuplets);
      const auto beg = (tupletIndex != -1)
                  ? tupletInterval(tuplets[tupletIndex], basicQuant).first
                  : chordInterval(*tiedTuplet.chord, basicQuant).first;

      return std::make_pair(beg, end);
      }

void setTupletVoice(
            std::map<ReducedFraction,
                     std::multimap<ReducedFraction, MidiChord>::iterator> &tupletChords,
            int voice)
      {
      for (auto &tupletChord: tupletChords) {
            MidiChord &midiChord = tupletChord.second->second;
            midiChord.voice = voice;
            }
      }

void setTupletVoices(
            std::vector<TupletInfo> &tuplets,
            std::set<int> &pendingTuplets,
            std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &tupletIntervals,
            const ReducedFraction &basicQuant)
      {
      const int limit = tupletVoiceLimit();
      int voice = 0;
      while (!pendingTuplets.empty() && voice < limit) {
            for (auto it = pendingTuplets.begin(); it != pendingTuplets.end(); ) {
                  int i = *it;
                  const auto interval = tupletInterval(tuplets[i], basicQuant);
                  if (!haveIntersection(interval, tupletIntervals[voice])) {
                        setTupletVoice(tuplets[i].chords, voice);
                        tupletIntervals[voice].push_back(interval);
                        it = pendingTuplets.erase(it);
                        continue;
                        }
                  ++it;
                  }
            ++voice;
            }
      }

void setNonTupletVoices(
            std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
            const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &tupletIntervals,
            const ReducedFraction &basicQuant)
      {
      const int limit = voiceLimit();
      int voice = 0;
      while (!pendingNonTuplets.empty() && voice < limit) {
            for (auto it = pendingNonTuplets.begin(); it != pendingNonTuplets.end(); ) {
                  auto chord = *it;
                  const auto interval = chordInterval(*chord, basicQuant);
                  const auto fit = tupletIntervals.find(voice);
                  if (fit == tupletIntervals.end() || !haveIntersection(interval, fit->second)) {
                        chord->second.voice = voice;
                        it = pendingNonTuplets.erase(it);
                                    // don't insert chord interval here
                        continue;
                        }
                  ++it;
                  }
            ++voice;
            }
      }

void resetTupletVoices(std::vector<TupletInfo> &tuplets)
      {
      for (auto &tuplet: tuplets) {
            for (auto &chord: tuplet.chords) {
                  MidiChord &midiChord = chord.second->second;
                  midiChord.voice = 0;
                  }
            }
      }

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findNonTupletIntervals(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> nonTupletIntervals;
      for (const auto &nonTuplet: nonTuplets) {
            nonTupletIntervals.push_back(chordInterval(*nonTuplet, basicQuant));
            }
      return nonTupletIntervals;
      }


#ifdef QT_DEBUG

bool areAllElementsUnique(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> chords;
      for (const auto &chord: nonTuplets) {
            if (chords.find(&*chord) == chords.end())
                  chords.insert(&*chord);
            else
                  return false;
            }
      return true;
      }

bool haveTupletsEmptyChords(const std::vector<TupletInfo> &tuplets)
      {
      for (const auto &tuplet: tuplets) {
            if (tuplet.chords.empty())
                  return true;
            }
      return false;
      }

bool doTupletChordsHaveSameVoice(const std::vector<TupletInfo> &tuplets)
      {
      for (const auto &tuplet: tuplets) {
            auto it = tuplet.chords.cbegin();
            const int voice = it->second->second.voice;
            ++it;
            for ( ; it != tuplet.chords.cend(); ++it) {
                  if (it->second->second.voice != voice)
                        return false;
                  }
            }
      return true;
      }

// back tied tuplets are not checked here

bool haveOverlappingVoices(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &basicQuant,
            const std::list<TiedTuplet> &backTiedTuplets)
      {
                  // <voice, intervals>
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> intervals;

      for (const auto &tuplet: tuplets) {
            const int voice = tuplet.chords.begin()->second->second.voice;
            const auto interval = std::make_pair(tuplet.onTime, tuplet.onTime + tuplet.len);
            if (haveIntersection(interval, intervals[voice]))
                  return true;
            else
                  intervals[voice].push_back(interval);
            }

      for (const auto &chord: nonTuplets) {
            const int voice = chord->second.voice;
            const auto interval = chordInterval(*chord, basicQuant);
            if (haveIntersection(interval, intervals[voice])) {
                  bool flag = false;      // if chord is tied then it can intersect tuplet
                  for (const TiedTuplet &tiedTuplet: backTiedTuplets) {
                        if (tiedTuplet.chord == (&*chord) && tiedTuplet.voice == voice) {
                              flag = true;
                              break;
                              }
                        }
                  if (!flag)
                        return true;
                  }
            }

      return false;
      }

size_t chordCount(
            const std::vector<TupletInfo> &tuplets,
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      size_t sum = nonTuplets.size();
      for (const auto &tuplet: tuplets) {
            sum += tuplet.chords.size();
            }
      return sum;
      }

#endif


// for the case !useMultipleVoices

void excludeExtraVoiceTuplets(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant,
            const ReducedFraction &barStart)
      {
                  // remove overlapping tuplets
      size_t sz = tuplets.size();
      if (sz == 0)
            return;
      while (true) {
            bool change = false;
            for (size_t i = 0; i < sz - 1; ++i) {
                  const auto interval1 = tupletInterval(tuplets[i], basicQuant);
                  for (size_t j = i + 1; j < sz; ++j) {
                        const auto interval2 = tupletInterval(tuplets[j], basicQuant);
                        if (haveIntersection(interval1, interval2)) {
                              --sz;
                              if (j < sz)
                                    tuplets[j] = tuplets[sz];
                              --sz;
                              if (i < sz)
                                    tuplets[i] = tuplets[sz];
                              change = true;
                              break;
                              }
                        }
                  if (change)
                        break;
                  }
            if (!change || sz == 0)
                  break;
            }

      if (sz > 0) {     // remove tuplets that are overlapped with non-tuplets
            const auto nonTupletIntervals = findNonTupletIntervals(nonTuplets, basicQuant);

            for (size_t i = 0; i < sz; ) {
                  const auto interval = tupletInterval(tuplets[i], basicQuant);
                  if (haveIntersection(interval, nonTupletIntervals)) {
                        for (const auto &chord: tuplets[i].chords) {
                              if (chord.first >= barStart)
                                    nonTuplets.push_back(chord.second);
                              }
                        --sz;
                        if (i < sz) {
                              tuplets[i] = tuplets[sz];
                              continue;
                              }
                        }
                  ++i;
                  }
            }

      Q_ASSERT_X(areAllElementsUnique(nonTuplets),
                 "MIDI tuplets: excludeExtraVoiceTuplets", "non unique chords in non-tuplets");

      tuplets.resize(sz);
      }

void removeUnusedTuplets(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            std::set<int> &pendingTuplets,
            std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
            const ReducedFraction &barStart)
      {
      if (pendingTuplets.empty())
            return;

      std::vector<TupletInfo> newTuplets;
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            if (pendingTuplets.find(i) == pendingTuplets.end()) {
                  newTuplets.push_back(tuplets[i]);
                  }
            else {
                  for (const auto &chord: tuplets[i].chords) {
                        if (chord.first >= barStart)
                              nonTuplets.push_back(chord.second);
                        pendingNonTuplets.insert(&*chord.second);
                        }
                  }
            }
      pendingTuplets.clear();
      std::swap(tuplets, newTuplets);
      }

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo> &tuplets,
                    const ReducedFraction &basicQuant)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> tupletIntervals;
      for (const auto &tuplet: tuplets)
            tupletIntervals.push_back(tupletInterval(tuplet, basicQuant));

      return tupletIntervals;
      }

std::set<int> findPendingTuplets(const std::vector<TupletInfo> &tuplets)
      {
      std::set<int> pendingTuplets;       // tuplet indexes
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            pendingTuplets.insert(i);
            }
      return pendingTuplets;
      }

std::set<std::pair<const ReducedFraction, MidiChord> *>
findPendingNonTuplets(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> pendingNonTuplets;
      for (const auto &c: nonTuplets) {
            pendingNonTuplets.insert(&*c);
            }
      return pendingNonTuplets;
      }

void setBackTiedVoices(
            std::list<TiedTuplet> &backTiedTuplets,
            std::vector<TupletInfo> &tuplets,
            std::set<int> &pendingTuplets,
            std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
            std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &tupletIntervals,
            const ReducedFraction &basicQuant)
      {
                  // set voices that are already set from previous bar
      bool loopAgain = false;
      do {
            for (auto it = backTiedTuplets.begin(); it != backTiedTuplets.end(); ) {
                  const TiedTuplet &tiedTuplet = *it;
                  TupletInfo &tuplet = tuplets[tiedTuplet.tupletIndex];
                  const auto backInterval = tupletInterval(tuplet, basicQuant);

                  if (tiedTuplet.voice == -1) {
                        ++it;
                        continue;
                        }
                  if (haveIntersection(backInterval, tupletIntervals[tiedTuplet.voice])) {
                        it = backTiedTuplets.erase(it);
                        continue;
                        }

                  for (const auto &chord: tuplet.chords) {
                        for (auto it2 = backTiedTuplets.begin();
                                  it2 != backTiedTuplets.end(); ++it2) {
                              if (it2 == it)
                                    continue;
                              if (&(chord.second->second) == &(it2->chord->second)
                                          && it2->voice == -1) {
                                    it2->voice = tiedTuplet.voice;
                                    loopAgain = true;
                                    break;
                                    }
                              }
                        }
                              // set voices of tied tuplet chords
                  setTupletVoice(tuplet.chords, tiedTuplet.voice);
                  pendingTuplets.erase(tiedTuplet.tupletIndex);
                  tupletIntervals[tiedTuplet.voice].push_back(backInterval);
                  tupletIntervals[tiedTuplet.voice].push_back(
                                    chordInterval(*tiedTuplet.chord, basicQuant));

                  Q_ASSERT_X(pendingNonTuplets.find(tiedTuplet.chord) == pendingNonTuplets.end(),
                             "MidiTuplet::setBackTiedVoices",
                             "Tied non-tuplet chord should not be here");

                  ++it;
                  }
            } while (loopAgain);

                  // set yet unset back tied voices
      const int limit = tupletVoiceLimit();

      for (auto it = backTiedTuplets.begin(); it != backTiedTuplets.end(); ) {
            TiedTuplet &tiedTuplet = *it;
            if (pendingTuplets.find(tiedTuplet.tupletIndex) == pendingTuplets.end()) {
                  ++it;
                  continue;
                  }

            TupletInfo &tuplet = tuplets[tiedTuplet.tupletIndex];
            const auto backInterval = tupletInterval(tuplet, basicQuant);

            if (tiedTuplet.voice == -1) {
                  int voice = 0;
                  for ( ; voice != limit; ++voice) {
                        if (haveIntersection(backInterval, tupletIntervals[voice]))
                              continue;
                        tiedTuplet.voice = voice;
                        break;
                        }
                  if (voice == limit) {     // no available voices
                        it = backTiedTuplets.erase(it);
                        continue;
                        }
                  }
            else {
                  if (haveIntersection(backInterval, tupletIntervals[tiedTuplet.voice])) {
                        it = backTiedTuplets.erase(it);
                        continue;
                        }
                  }

            for (const auto &chord: tuplet.chords) {
                  for (auto it2 = std::next(it); it2 != backTiedTuplets.end(); ++it2) {
                        if (pendingTuplets.find(it2->tupletIndex) == pendingTuplets.end())
                              continue;
                        if (&(chord.second->second) == &(it2->chord->second) && it2->voice == -1) {
                              it2->voice = tiedTuplet.voice;
                              break;
                              }
                        }
                  }
                        // set voices of tied tuplet chords
            setTupletVoice(tuplet.chords, tiedTuplet.voice);
            pendingTuplets.erase(tiedTuplet.tupletIndex);
            tupletIntervals[tiedTuplet.voice].push_back(backInterval);

            const int i = findTupletWithChord(tiedTuplet.chord->second, tuplets);
            if (i != -1) {
                  auto it2 = std::next(it);
                  for ( ; it2 != backTiedTuplets.end(); ++it2) {
                        if (it2->tupletIndex == i)
                              break;
                        }
                  if (it2 != backTiedTuplets.end()) {
                        it2->voice = tiedTuplet.voice;
                        }
                  else {
                                    // set voice of not back-tied tuplet that have tied chord
                        setTupletVoice(tuplets[i].chords, tiedTuplet.voice);
                        pendingTuplets.erase(i);
                        tupletIntervals[tiedTuplet.voice].push_back(
                                          tupletInterval(tuplets[i], basicQuant));
                        }
                  }
            else {
                              // set tied chord (non-tuplet) voice
                  tiedTuplet.chord->second.voice = tiedTuplet.voice;
                  pendingNonTuplets.erase(tiedTuplet.chord);
                  tupletIntervals[tiedTuplet.voice].push_back(
                                    chordInterval(*tiedTuplet.chord, basicQuant));
                  }

            ++it;
            }
      }

std::map<std::pair<const ReducedFraction, MidiChord> *, int>
findMappedTupletChords(const std::vector<TupletInfo> &tuplets)
      {
                  // <chord address, tupletIndex>
      std::map<std::pair<const ReducedFraction, MidiChord> *, int> tupletChords;
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            for (const auto &tupletChord: tuplets[i].chords) {
                  auto tupletIt = tupletChord.second;
                  tupletChords.insert({&*tupletIt, i});
                  }
            }
      return tupletChords;
      }

bool areTupletsIntersect(const TupletInfo &t1, const TupletInfo &t2)
      {
      const auto onTime1 = t1.onTime;
      const auto endTime1 = onTime1 + t1.len;
      const auto onTime2 = t2.onTime;
      const auto endTime2 = onTime1 + t2.len;
      return (endTime1 > onTime2 && onTime1 < endTime2);
      }

// result: tied notes indexes

std::vector<int> findTiedNotes(
            const TupletInfo &tuplet,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      std::vector<int> tiedNotes;
      const auto tupletRatio = tupletLimits(tuplet.tupletNumber).ratio;
      const auto firstTupletChordOnTime = Quantize::findQuantizedChordOnTime(
                                          *tuplet.chords.begin()->second, basicQuant,
                                          tupletRatio, startBarTick);

      const auto maxChordOffTime = Quantize::findMaxQuantizedOffTime(
                        *chordIt, basicQuant, tupletRatio, startBarTick);

      if (maxChordOffTime > firstTupletChordOnTime)
            return tiedNotes;

      const auto onTime = Quantize::findQuantizedChordOnTime(*chordIt, basicQuant);
      if (onTime >= tuplet.onTime)
            return tiedNotes;

      for (int i = 0; i != chordIt->second.notes.size(); ++i) {
            const MidiNote &note = chordIt->second.notes[i];

            const auto offTimeInTuplet = Quantize::findQuantizedNoteOffTime(
                              *chordIt, note.offTime, basicQuant,
                              tupletRatio, startBarTick);

            if (offTimeInTuplet < startBarTick
                        || offTimeInTuplet <= tuplet.onTime)
                  continue;

            const auto regularOffTime = Quantize::findQuantizedNoteOffTime(
                                                *chordIt, note.offTime, basicQuant);
            const auto regularError = (note.offTime - regularOffTime).absValue();
            const auto tupletError = (note.offTime - offTimeInTuplet).absValue();
            if (tupletError > regularError)
                  continue;

            tiedNotes.push_back(i);
            }

      return tiedNotes;
      }

std::list<TiedTuplet>
findBackTiedTuplets(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &prevBarStart,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      std::list<TiedTuplet> tiedTuplets;
      const auto tupletChords = findMappedTupletChords(tuplets);
      const auto tupletIntervals = findTupletIntervals(tuplets, basicQuant);

      for (int i = 0; i != (int)tuplets.size(); ++i) {

            Q_ASSERT_X(!tuplets[i].chords.empty(),
                       "MidiTuplets::findBackTiedTuplets", "Tuplet chords are empty");

            auto chordIt = tuplets[i].chords.begin()->second;
            while (chordIt != chords.begin() && chordIt->first >= prevBarStart) {
                  --chordIt;

                  const auto tupletIt = tupletChords.find(&*chordIt);
                  bool isInTupletOfThisBar = (tupletIt != tupletChords.end());

                              // remember voices of tuplets that have tied chords from previous bar
                              // and that chords don't belong to the tuplets of this bar
                  int voice = (chordIt->second.barIndex != -1 && !isInTupletOfThisBar)
                              ? chordIt->second.voice : -1;
                  if (voice != -1) {
                        bool voiceUsed = false;
                                    // if already found back tied tuplets have same voice
                                    // then they shouldn't intersect with tuplet 'i'
                        for (const auto &t: tiedTuplets) {
                              if (t.voice == voice) {
                                    if (haveIntersection(tupletIntervals[i],
                                                         tupletIntervals[t.tupletIndex])) {
                                          voiceUsed = true;
                                          break;
                                          }
                                    }
                              }
                        if (voiceUsed)
                              continue;
                        }
                              // don't make back tie to the chord in overlapping tuplet
                  if (isInTupletOfThisBar) {
                        if (areTupletsIntersect(tuplets[tupletIt->second], tuplets[i]))
                              continue;
                        }
                  const auto tiedNotes = findTiedNotes(tuplets[i], chordIt, startBarTick, basicQuant);
                  if (!tiedNotes.empty()) {
                        tiedTuplets.push_back({i, voice, &*chordIt, tiedNotes});
                        break;
                        }
                  }
            }
      return tiedTuplets;
      }

// chord notes should not be rearranged here
// because note indexes are stored in tied tuplets

void assignVoices(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            std::list<TiedTuplet> &backTiedTuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
#ifdef QT_DEBUG
      size_t oldChordCount = chordCount(tuplets, nonTuplets);
#endif
      Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
                 "MIDI tuplets: assignVoices", "Empty tuplet chords");

      auto pendingTuplets = findPendingTuplets(tuplets);
      auto pendingNonTuplets = findPendingNonTuplets(nonTuplets);

                  // <voice, intervals>
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> tupletIntervals;

      setBackTiedVoices(backTiedTuplets, tuplets, pendingTuplets, pendingNonTuplets,
                        tupletIntervals, basicQuant);
      setTupletVoices(tuplets, pendingTuplets, tupletIntervals, basicQuant);

      Q_ASSERT_X((voiceLimit() == 1) ? pendingTuplets.empty() : true,
                 "MIDI tuplets: assignVoices", "Unused tuplets for the case !useMultipleVoices");

      removeUnusedTuplets(tuplets, nonTuplets, pendingTuplets, pendingNonTuplets, startBarTick);
      setNonTupletVoices(pendingNonTuplets, tupletIntervals, basicQuant);

      Q_ASSERT_X(pendingNonTuplets.empty(),
                 "MIDI tuplets: assignVoices", "Unused non-tuplets");
      Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
                 "MIDI tuplets: assignVoices", "Empty tuplet chords");
      Q_ASSERT_X(doTupletChordsHaveSameVoice(tuplets),
                 "MIDI tuplets: assignVoices", "Tuplet chords have different voices");
      Q_ASSERT_X(!haveOverlappingVoices(nonTuplets, tuplets, basicQuant, backTiedTuplets),
                 "MIDI tuplets: assignVoices", "Overlapping tuplets of the same voice");
#ifdef QT_DEBUG
      size_t newChordCount = chordCount(tuplets, nonTuplets);
#endif
      Q_ASSERT_X(oldChordCount == newChordCount,
                 "MIDI tuplets: assignVoices", "Chord count is not preserved");
      }

} // namespace MidiTuplet
} // namespace Ms
