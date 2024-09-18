#ifndef MUSICXMLVOICEDESC_H
#define MUSICXMLVOICEDESC_H

#include "dom/mscore.h"

namespace mu::engraving {
//---------------------------------------------------------
//   VoiceDesc
//---------------------------------------------------------

/**
 The description of a single voice in a MusicXML part.
*/

class VoiceDesc
{
public:
    VoiceDesc();
    void incrChordRests(int s);
    int numberChordRests() const;
    int numberChordRests(int s) const { return (s >= 0 && s < MAX_STAVES) ? m_chordRests[s] : 0; }
    int preferredStaff() const;         // Determine preferred staff for this voice
    void setStaff(int s)
    {
        if (s >= 0) {
            m_staff = s;
        }
    }

    int staff() const { return m_staff; }
    void setVoice(int v)
    {
        if (v >= 0) {
            m_voice = v;
        }
    }

    int voice() const { return m_voice; }
    void setVoice(int s, int v)
    {
        if (s >= 0 && s < MAX_STAVES) {
            m_voices[s] = v;
        }
    }

    int voice(int s) const { return (s >= 0 && s < MAX_STAVES) ? m_voices[s] : -1; }
    void setOverlap(bool b) { m_overlaps = b; }
    bool overlaps() const { return m_overlaps; }
    void setStaffAlloc(int s, int i)
    {
        if (s >= 0 && s < MAX_STAVES) {
            m_staffAlloc[s] = i;
        }
    }

    int staffAlloc(int s) const { return (s >= 0 && s < MAX_STAVES) ? m_staffAlloc[s] : -1; }
    String toString() const;
private:
    int m_chordRests[MAX_STAVES];        // The number of chordrests on each MusicXML staff
    int m_staff;                         // The MuseScore staff allocated
    int m_voice;                         // The MuseScore voice allocated
    bool m_overlaps;                     // This voice contains active notes in multiple staves at the same time
    int m_staffAlloc[MAX_STAVES];        // For overlapping voices: voice is allocated on these staves (note: -2=unalloc -1=undef 1=alloc)
    int m_voices[MAX_STAVES];            // For every voice allocated on the staff, the voice number
};
typedef std::map<int, VoiceDesc> VoiceList;
}
#endif // MUSICXMLVOICEDESC_H
