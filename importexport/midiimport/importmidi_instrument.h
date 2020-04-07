#ifndef IMPORTMIDI_INSTRUMENT_H
#define IMPORTMIDI_INSTRUMENT_H

#include "midi/midifile.h"


class QString;

namespace Ms {

class Score;
class MTrack;

namespace MidiInstr {

QString instrumentName(MidiType type, int program, bool isDrumTrack);
QString msInstrName(int trackIndex);
QString concatenateWithComma(const QString &left, const QString &right);
bool isGrandStaff(const MTrack &t1, const MTrack &t2);
void setGrandStaffProgram(QList<MTrack> &tracks);
void findInstrumentsForAllTracks(const QList<MTrack> &tracks, bool forceReload = false);
void createInstruments(Score *score, QList<MTrack> &tracks);

extern void instrumentTemplatesChanged();

} // namespace MidiInstr
} // namespace Ms


#endif // IMPORTMIDI_INSTRUMENT_H
