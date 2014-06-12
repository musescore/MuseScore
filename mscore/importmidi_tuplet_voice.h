#ifndef IMPORTMIDI_TUPLET_VOICE_H
#define IMPORTMIDI_TUPLET_VOICE_H


namespace Ms {

class ReducedFraction;
class MidiChord;

namespace MidiTuplet {

struct TupletInfo;

struct TiedTuplet
      {
      int tupletId;
      int voice;
      std::pair<const ReducedFraction, MidiChord> *chord;  // chord the tuplet is tied with
      std::vector<int> tiedNoteIndexes;   // indexes of tied notes of that chord
      };

int tupletVoiceLimit();
void resetTupletVoices(std::vector<TupletInfo> &tuplets);

bool excludeExtraVoiceTuplets(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            std::list<TiedTuplet> &backTiedTuplets,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant,
            const ReducedFraction &barStart);

std::list<TiedTuplet>
findBackTiedTuplets(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &prevBarStart,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant,
            int barIndex);

void assignVoices(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            std::list<TiedTuplet> &backTiedTuplets,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant,
            int barIndex);

#ifdef QT_DEBUG

bool haveOverlappingVoices(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::vector<TupletInfo> &tuplets,
            const std::list<TiedTuplet> &backTiedTuplets,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant);

#endif

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_VOICE_H
