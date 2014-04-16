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
      int limit = tupletVoiceLimit();
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

std::pair<ReducedFraction, ReducedFraction>
chordInterval(const std::pair<const ReducedFraction, MidiChord> &chord,
              const ReducedFraction &basicQuant)
      {
      const auto onTime = Quantize::findMinQuantizedOnTime(chord, basicQuant);
      const auto offTime = Quantize::findMaxQuantizedOffTime(chord, basicQuant);
      return std::make_pair(onTime, offTime);
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
            const std::vector<TiedTuplet> &backTiedTuplets)
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

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo> &tuplets,
                    const ReducedFraction &basicQuant)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> tupletIntervals;
      for (const auto &tuplet: tuplets)
            tupletIntervals.push_back(tupletInterval(tuplet, basicQuant));

      return tupletIntervals;
      }

std::vector<std::pair<int, int> >
findForTiedTuplets(
            const std::vector<TupletInfo> &tuplets,
            const std::vector<TiedTuplet> &backTiedTuplets,
            const std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
            const ReducedFraction &startBarTick)
      {
      std::vector<std::pair<int, int>> forTiedTuplets;  // <tuplet index, voice to assign>

      for (const TiedTuplet &tuplet: backTiedTuplets) {
                        // only for chords in the current bar (because of tol some can be outside)
            if (tuplet.chord->first < startBarTick)
                  continue;
            if (pendingNonTuplets.find(tuplet.chord) == pendingNonTuplets.end()) {
                  const int i = findTupletWithChord(tuplet.chord->second, tuplets);
                  if (i != -1)
                        forTiedTuplets.push_back({i, tuplet.voice});
                  }
            }
      return forTiedTuplets;
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

// chord notes should not be rearranged here
// because note indexes are stored in tied tuplets

void assignVoices(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::vector<TiedTuplet> &backTiedTuplets,
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

      const auto forTiedTuplets = findForTiedTuplets(tuplets, backTiedTuplets,
                                                     pendingNonTuplets, startBarTick);
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> tupletIntervals;

      for (const auto &t: forTiedTuplets) {
            const int i = t.first;
            const int voice = t.second;
            setTupletVoice(tuplets[i].chords, voice);
                        // remove tuplets with already set voices
            pendingTuplets.erase(i);
            tupletIntervals[voice].push_back(tupletInterval(tuplets[i], basicQuant));
            }

      for (const TiedTuplet &tuplet: backTiedTuplets) {
            setTupletVoice(tuplets[tuplet.tupletIndex].chords, tuplet.voice);
            pendingTuplets.erase(tuplet.tupletIndex);
            tupletIntervals[tuplet.voice].push_back(
                              tupletInterval(tuplets[tuplet.tupletIndex], basicQuant));
                        // set for-tied chords
                        // some chords can be the same as in forTiedTuplets

                  // only for chords in the current bar (because of tol some can be outside)
            if (tuplet.chord->first < startBarTick)
                  continue;
            tuplet.chord->second.voice = tuplet.voice;
                        // remove chords with already set voices
            pendingNonTuplets.erase(tuplet.chord);
            }

      {
      setTupletVoices(tuplets, pendingTuplets, tupletIntervals, basicQuant);

      Q_ASSERT_X((voiceLimit() == 1) ? pendingTuplets.empty() : true,
                 "MIDI tuplets: assignVoices", "Unused tuplets for the case !useMultipleVoices");

      removeUnusedTuplets(tuplets, nonTuplets, pendingTuplets, pendingNonTuplets, startBarTick);
      setNonTupletVoices(pendingNonTuplets, tupletIntervals, basicQuant);
      }

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
