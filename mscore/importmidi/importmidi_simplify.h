#ifndef IMPORTMIDI_SIMPLIFY_H
#define IMPORTMIDI_SIMPLIFY_H


namespace Ms {

class MTrack;
class TimeSigMap;

namespace Simplify {

void simplifyDurations(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap);

} // Simplify
} // Ms


#endif // IMPORTMIDI_SIMPLIFY_H
