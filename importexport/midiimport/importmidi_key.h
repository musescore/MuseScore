#ifndef IMPORTMIDI_KEY_H
#define IMPORTMIDI_KEY_H


namespace Ms {

class Staff;
class MTrack;
class KeyList;

namespace MidiKey {

void assignKeyListToStaff(const KeyList &kl, Staff *staff);
void recognizeMainKeySig(QList<MTrack> &tracks);

} // namespace MidiKey
} // namespace Ms


#endif // IMPORTMIDI_KEY_H
