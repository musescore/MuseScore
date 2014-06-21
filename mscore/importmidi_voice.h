#ifndef IMPORTMIDI_VOICE_H
#define IMPORTMIDI_VOICE_H

#include "importmidi_operation.h"


namespace Ms {

class MTrack;
class TimeSigMap;

namespace MidiVoice {

int toIntVoiceCount(MidiOperations::VoiceCount value);
int voiceLimit();
bool separateVoices(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap);

} // namespace LRHand
} // namespace Ms


#endif // IMPORTMIDI_VOICE_H
