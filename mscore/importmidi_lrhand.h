#ifndef IMPORTMIDI_LRHAND_H
#define IMPORTMIDI_LRHAND_H

namespace Ms {

class MTrack;

namespace LRHand {

void splitIntoLeftRightHands(std::multimap<int, MTrack> &tracks);

} // namespace LRHand
} // namespace Ms


#endif // IMPORTMIDI_LRHAND_H
