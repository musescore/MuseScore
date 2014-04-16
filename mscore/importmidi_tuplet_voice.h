#ifndef IMPORTMIDI_TUPLET_VOICE_H
#define IMPORTMIDI_TUPLET_VOICE_H


namespace Ms {

class ReducedFraction;
class MidiChord;

namespace MidiTuplet {

struct TupletInfo;

struct TiedTuplet
      {
      int tupletIndex;
      int voice;
      std::pair<const ReducedFraction, MidiChord> *chord;  // chord the tuplet is tied with
      std::vector<int> tiedNoteIndexes;   // indexes of tied notes of that chord
      };

int tupletVoiceLimit();
int voiceLimit();
void resetTupletVoices(std::vector<TupletInfo> &tuplets);

void excludeExtraVoiceTuplets(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant,
            const ReducedFraction &barStart);

std::list<TiedTuplet>
findBackTiedTuplets(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &prevBarStart,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant);

void assignVoices(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            std::list<TiedTuplet> &backTiedTuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant);

#ifdef QT_DEBUG

bool haveOverlappingVoices(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &basicQuant,
            const std::list<TiedTuplet> &backTiedTuplets = std::list<TiedTuplet>());

#endif

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_VOICE_H
