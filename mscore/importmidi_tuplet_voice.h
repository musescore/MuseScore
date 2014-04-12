#ifndef IMPORTMIDI_TUPLET_VOICE_H
#define IMPORTMIDI_TUPLET_VOICE_H


namespace Ms {

class ReducedFraction;
class MidiChord;

namespace MidiTuplet {

struct TupletInfo;

int tupletVoiceLimit();
int voiceLimit();
void resetTupletVoices(std::vector<TupletInfo> &tuplets);

void excludeExtraVoiceTuplets(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant);

void assignVoices(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &startBarTick, const ReducedFraction &endBarTick,
            const ReducedFraction &basicQuant,
            const std::multimap<ReducedFraction, MidiChord> &chords);

struct TiedTuplet
      {
      int tupletIndex;
      int voice;
      std::pair<const ReducedFraction, MidiChord> *chord;  // chord the tuplet is tied with
      };

#ifdef QT_DEBUG

bool haveOverlappingVoices(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &basicQuant,
            const std::vector<TiedTuplet> &backTiedTuplets = std::vector<TiedTuplet>());

#endif

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_VOICE_H
